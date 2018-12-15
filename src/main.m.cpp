#include <stdio.h>
#include <iostream>
#include <set>
#include "parser.h"
#include "codegen.h"
#include "logger.h"
#include "imports.h"
#include "file.h"
#include <regex>

#include "../bin/feature.y.h"
extern YYSTYPE yylval;
extern YYLTYPE yylloc;
extern FILE* yyin;;
int yylex_all();
extern const char *lexer_error;

struct smart_file {
    FILE *f;

    smart_file() : f(NULL) {}
    ~smart_file() {
        if (f != NULL) fclose(f);
    }
};

template<typename TProcessor>
bool try_process_file(const char *fn, FILE *f, TProcessor *processor)
{
    bool ok = true;
    cglang::ostream_logger log(fn, std::cout, std::cout, std::cerr);
    {
        typename TProcessor::Parser parser(&log, ok, processor);
        cglang::parser::parse(&parser, f);
    }
    return ok;
}

int main(int argc, char *argv[])
{
    char *o = nullptr;
    std::vector<char*> inputs;
    std::vector<std::string> i_dirs;
    bool gen_wide = false;
    ++argv, --argc;
    while(argc != 0) {
        if (strcmp(argv[0], "--help") == 0) {
            std::cout << "cglang transpiler" << std::endl;
            std::cout << "Transpile from Gherkin" << std::endl;
            std::cout << "cglang OPTIONS <sources>" << std::endl;
            std::cout << "<sources> should be augmented gherkin feature files, which will be evaluated in the given order" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << " --help    : Show this help" << std::endl;
            std::cout << " -o <file> : Specify the output file" << std::endl;
            std::cout << " -I <dir>  : Specify a search directory for imported modules" << std::endl;
            std::cout << " -w        : Specify that the generated files should use wide characters" << std::endl;
            return 0;
        }
        else if (strcmp(argv[0], "-o") == 0) {
            --argc;
            if (argc == 0) {
                std::cerr << "Expected something after -o" << std::endl;
                return 1;
            }
            ++argv;
            if (o != nullptr) {
                std::cerr << "Cannot specify -o more than once" << std::endl;
                return 1;
            }
            o = argv[0];
        }
        else if (strcmp(argv[0], "-I") == 0) {
            --argc;
            if (argc == 0) {
                std::cerr << "Expected something after -I" << std::endl;
                return 1;
            }
            ++argv;
            i_dirs.emplace_back(argv[0]);
        }
        else if (strcmp(argv[0], "-w") == 0) {
            gen_wide = true;
        }
        else {
            inputs.push_back(argv[0]);
        }
        --argc;
        ++argv;
    }

    smart_file outf;
    FILE *out;
    if (o != nullptr) {
        out = outf.f = fopen(o, "w");
        if (!out) {
            std::cerr << "Failure opening '" << o << "' for output." << std::endl;
            return 1;
        }
    }
    else {
        out = stdout;
    }

    std::vector<std::string> unprocessed;
    cglang::import_graph ig(i_dirs, unprocessed);

    if (inputs.empty()) {
        std::cerr << "No input" << std::endl;
    }
    else {
        for(auto &fn : inputs) {
            ig.set_current(fn);
            FILE *f = fopen(fn, "r");
            if (f) {
                if (!try_process_file(fn, f, &ig)) {
                    fclose(f);
                    return 1;
                }
                else {
                    fclose(f);
                }
            }
            else {
                std::cerr << "Failure opening '" << fn << "' for input." << std::endl;
                return 1;
            }
        }
    }

    std::set<std::string> seen;
    while(!unprocessed.empty()) {
        auto fn = unprocessed.back();
        unprocessed.pop_back();

        if (!seen.emplace(fn).second) continue;

        ig.set_current(fn);
        FILE *f = fopen(fn.c_str(), "r");
        if (f) {
            if (!try_process_file(fn.c_str(), f, &ig)) {
                fclose(f);
                return 1;
            }
            else {
                fclose(f);
            }
        }
        else {
            std::cerr << "Failure opening '" << fn << "' for input." << std::endl;
            return 1;
        }
    }

    cglang::code_generator cg;
    cg._gen_wide = gen_wide;

    std::vector<std::string> files;
    if (!ig.try_get_files(std::cerr, files)) {
        return 1;
    }

    for(auto &fn : files) {
        FILE *f = fopen(fn.c_str(), "r");
        if (f) {
            if (!try_process_file(fn.c_str(), f, &cg)) {
                fclose(f);
                return 1;
            }
            else {
                fclose(f);
            }
        }
        else {
            std::cerr << "Failure opening '" << fn << "' for input." << std::endl;
            return 1;
        }
    }

    if (!cg.try_write_file(std::cerr, out)) {
        return 1;
    }

    return 0;
}