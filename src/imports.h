#ifndef __IMPORT_PARSERH
#define __IMPORT_PARSERH

#include "parser.h"
#include "file.h"
#include <set>

namespace cglang
{
    class import_parser;

    class import_graph {
    private:
        struct edge {
            std::string from;
            std::string to;

            edge() = delete;
            edge(const edge&) = default;
            edge(edge&&) = default;
            explicit edge(const std::string &from, const std::string &to)
            : from(from), to(to)
            {}

            edge& operator=(const edge&) = default;
            edge& operator=(edge&&) = default;
        };

        std::string _current;
        std::string _current_dir;
        std::set<std::string> _all_files;
        std::vector<edge> _edges;
        std::vector<std::string> &_unprocessed;
        const std::vector<std::string> &_i_dirs;
    public:
        typedef import_parser Parser;

        import_graph() = delete;
        import_graph(const import_graph&) = delete;
        import_graph(import_graph&&) = delete;

        explicit import_graph(const std::vector<std::string> &i_dirs, std::vector<std::string> &unprocessed);

        import_graph& operator=(const import_graph&) = delete;
        import_graph& operator=(import_graph&&) = delete;

        void set_current(const std::string &current);
        bool add_import(logger *logger, const location &loc, const wchar_t *name);

        bool try_get_files(std::ostream &err, std::vector<std::string> &files) const;
    };

    class import_parser : public parser {
    private:
        import_graph *_ig;
    public:
        import_parser() = delete;
        import_parser(const import_parser&) = delete;
        import_parser(import_parser&&) = delete;
        explicit import_parser(const char *fn, logger *logger, bool &ok, import_graph *ig);
        virtual ~import_parser();

        import_parser& operator=(const import_parser&) = delete;
        import_parser& operator=(import_parser&&) = delete;

        virtual void set_grammar(const std::regex_constants::syntax_option_type &grammar, const location &loc) override;
        virtual void set_case_sensitive(bool sensitive, const location &loc) override;
        virtual void process_import(const wchar_t *name, const location &loc) override;
        virtual void process_feature_declaration(const wchar_t *name, const location &loc) override;
        virtual void process_file_skeleton_declaration(const wchar_t *cpp, const location &loc) override;
        virtual void process_test_skeleton_declaration(const wchar_t *cpp, const location &loc) override;
        virtual void process_step_skeleton_declaration(const wchar_t *pattern, const location &pattern_loc, const wchar_t *cpp, const location &cpp_loc) override;
        virtual void process_define_skeleton_declaration(const wchar_t *name, const location &name_loc, const wchar_t *cpp, const location &cpp_loc) override;
        virtual void process_background_declaration(psteps &&steps, const location &loc) override;
        virtual void process_scenario_outline_declaration(const wchar_t *name, const location &name_loc, psteps &&steps, const location &steps_loc) override;
        virtual void process_examples_declaration(table &&table, const location &loc) override;
        virtual void process_scenario_declaration(const wchar_t *name, const location &name_loc, psteps &&steps, const location &steps_loc) override;
    };
}

#endif