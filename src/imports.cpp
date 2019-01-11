#include "imports.h"
#include <unordered_map>
#include <map>
#include <sstream>

namespace cglang
{

import_parser::import_parser(const char *fn, logger *logger, bool &ok, import_graph *ig)
: parser(logger, ok), _ig(ig)
{}

import_parser::~import_parser()
{}

void import_parser::set_grammar(const std::regex_constants::syntax_option_type &grammar, const location &loc)
{}

void import_parser::set_case_sensitive(bool sensitive, const location &loc)
{}

void import_parser::process_import(const wchar_t *name, const location &loc)
{
    _ok &= _ig->add_import(_logger, loc, name);
}

void import_parser::process_feature_declaration(const wchar_t *name, const location &loc)
{}

void import_parser::process_file_skeleton_declaration(const wchar_t *cpp, const location &loc)
{}

void import_parser::process_test_skeleton_declaration(const wchar_t *cpp, const location &loc)
{}

void import_parser::process_step_skeleton_declaration(const wchar_t *pattern, const location &pattern_loc, const wchar_t *cpp, const location &cpp_loc)
{}

void import_parser::process_define_skeleton_declaration(const wchar_t *name, const location &name_loc, const wchar_t *cpp, const location &cpp_loc)
{}

void import_parser::process_background_declaration(psteps &&steps, const location &loc)
{}

void import_parser::process_scenario_outline_declaration(const wchar_t *name, const location &name_loc, psteps &&steps, const location &steps_loc)
{}

void import_parser::process_examples_declaration(table &&table, const location &loc)
{}

void import_parser::process_scenario_declaration(const wchar_t *name, const location &name_loc, psteps &&steps, const location &steps_loc)
{}

import_graph::import_graph(const std::vector<std::string> &i_dirs, std::vector<std::string> &unprocessed)
: _i_dirs(i_dirs), _unprocessed(unprocessed)
{}

void import_graph::set_current(const std::string &current)
{
    _all_files.emplace(current);
    _current = current;
    _current_dir = _current;
    // slice off the file name
    auto p = _current_dir.find_last_of('/');
    if (p != std::string::npos) {
        _current_dir = _current_dir.substr(0, p);
    }
    else {
        // It's only a filename, i.e.: test.s.cgfeature
        _current_dir = "./";
    }
    // add the trailing slash
    if (_current_dir[_current_dir.size() - 1] != '/') {
        _current_dir += '/';
    }
}

bool import_graph::add_import(logger *logger, const location &loc, const wchar_t* name)
{
    std::wstring w(name);
    std::string n(w.begin(), w.end());
    std::set<std::string> found, tries;
    search_fn(_i_dirs, _current_dir, n, found, tries);
    std::string fn;
    switch(found.size()) {
        case 0:
            if (true) {
                std::wstringstream stm;
                stm << L"The import '" << w << L"; could not be found. ";
                if (tries.empty()) {
                    stm << L"No locations was tried.";
                }
                else {
                    stm << L"Looked at the following locations: ";
                    for(auto &t : tries) {
                        stm << std::endl << L"  " << std::wstring(t.begin(), t.end());
                    }
                }
                logger->write_error(loc, stm.str().c_str());
            }
            return false;
        case 1:
            fn = *found.begin();
            break;
        default:
            logger->write_error(loc, (L"The import '" + w + L"' was found more than once.").c_str());
            return false;
    }
    _all_files.emplace(fn);
    _edges.emplace_back(_current, fn);
    _unprocessed.push_back(std::move(fn));
    return true;
}

bool import_graph::try_get_files(std::ostream &err, std::vector<std::string> &files) const
{
    std::unordered_map<std::string, size_t> level;
    std::set<std::string> leafs(_all_files);
    std::unordered_map<std::string, std::vector<std::string>> graph; // upwards
    for(auto &edge : _edges) {
        leafs.erase(edge.to);
        auto f = graph.emplace(edge.from, std::vector<std::string>());
        f.first->second.push_back(edge.to);
    }
    for(auto &leaf : leafs) {
        level.emplace(leaf, 0);
    }

    std::vector<std::string> unprocessed(leafs.begin(), leafs.end());

    while(!unprocessed.empty()) {
        auto n = unprocessed.back();
        unprocessed.pop_back();
        auto nl = level[n] + 1;
        for(auto &x : graph[n]) {
            auto f = level.emplace(x, 0);
            if (f.first->second < nl) {
                unprocessed.push_back(x);
                f.first->second = nl;
            }
        }
    }

    // Now we build from lowest level up
    std::multimap<size_t, std::string> ordered;
    for(auto &k : level) {
        ordered.emplace(1000000-k.second, k.first);
    }

    for(auto &f : ordered) {
        files.push_back(f.second);
    }

    return true;
}

}