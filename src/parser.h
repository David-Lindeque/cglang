#ifndef __PARSERH
#define __PARSERH

#include <stdio.h>
#include <regex>
#include "ast.h"
#include "logger.h"

namespace cglang
{
    class parser {
    protected:
        logger *_logger;
        bool &_ok;
    public:
        parser() = delete;
        parser(const parser&) = delete;
        parser(parser&&) = delete;
        explicit parser(logger *logger, bool &ok);
        virtual ~parser();

        parser& operator=(const parser&) = delete;
        parser& operator=(parser&&) = delete;

        virtual void set_grammar(const std::regex_constants::syntax_option_type &grammar, const location &loc) = 0;
        virtual void set_case_sensitive(bool sensitive, const location &loc) = 0;
        virtual void process_import(const wchar_t *name, const location &loc) = 0;
        virtual void process_feature_declaration(const wchar_t *name, const location &loc) = 0;
        virtual void process_file_skeleton_declaration(const wchar_t *cpp, const location &loc) = 0;
        virtual void process_test_skeleton_declaration(const wchar_t *cpp, const location &loc) = 0;
        virtual void process_step_skeleton_declaration(const wchar_t *pattern, const location &pattern_loc, const wchar_t *cpp, const location &cpp_loc) = 0;
        virtual void process_define_skeleton_declaration(const wchar_t *name, const location &name_loc, const wchar_t *cpp, const location &cpp_loc) = 0;
        virtual void process_background_declaration(psteps &&steps, const location &loc) = 0;
        virtual void process_scenario_outline_declaration(const wchar_t *name, const location &name_loc, psteps &&steps, const location &steps_loc) = 0;
        virtual void process_examples_declaration(table &&table, const location &loc) = 0;
        virtual void process_scenario_declaration(const wchar_t *name, const location &name_loc, psteps &&steps, const location &steps_loc) = 0;

        void set_step_table(step* step, table &&table, const location &table_loc);
        void set_step_mtext(step* step, const wchar_t *text, const location &text_loc);

        void process_error(const location &loc, const char *parser_error, const char *lexer_error);

        static void parse(parser *sink, FILE *source);
    };
}

#endif