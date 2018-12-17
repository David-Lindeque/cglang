#ifndef __CODEGEN
#define __CODEGEN

#include "parser.h"
#include <vector>
#include <regex>
#include <unordered_map>

namespace cglang
{
    class code_generator_parser;

    class code_generator {
    private:
        std::regex_constants::syntax_option_type _grammar = std::regex_constants::syntax_option_type::ECMAScript;
        bool _case_sensitive = true;

        std::wstring _file_skeleton;
        std::string _file_skeleton_fn;
        location _file_skeleton_loc;
        std::wstring _test_skeleton;
        std::string _test_skeleton_fn;
        location _test_skeleton_loc;
        std::unordered_map<std::wstring, std::wstring> _defines;
        struct step {
            std::wregex _re;
            location _loc;
            std::wstring _cpp;
        };
        std::vector<step> _step_skeletons;

        std::wstring _feature_name;

        std::wstring _tests;
        void replace_std_tokens(std::wstring &text, const std::string &fn, const location &loc);
    public:
        typedef code_generator_parser Parser;

        bool _gen_wide;

        void set_grammar(const std::regex_constants::syntax_option_type &grammar);
        void set_case_sensitive(bool sensitive);
        
        bool try_set_file_skeleton(const std::string &fn, logger *logger, const wchar_t *cpp, const location &loc);
        bool try_set_test_skeleton(const std::string &fn, logger *logger, const wchar_t *cpp, const location &loc);
        bool try_add_step_skeleton(const std::string &fn, logger *logger, const wchar_t *pattern, const location &pattern_loc, const wchar_t *cpp, const location &cpp_loc);
        bool try_set_define_skeleton(const std::string &fn, logger *logger, const wchar_t *name, const location &name_loc, const wchar_t *cpp, const location &cpp_loc);
        
        bool try_set_feature(logger* logger, const wchar_t *feature_name, const location &loc);
        
        bool try_get_step_cpp(logger *logger, std::wstring &cpp, const pstep &step, const std::function<std::wstring(const std::wstring&)> &prepare);
        bool try_add_test(logger* logger, const wchar_t *scenario_name, const location &loc, const std::vector<std::wstring> &step_cpps);

        bool try_write_file(std::ostream &err, FILE* f);
    };

    class code_generator_parser : public parser {
    private:
        code_generator *_cg;
        psteps _background;
        std::wstring _outline_name;
        psteps _outline_steps;
        location _outline_loc;
        std::string _fn;
    public:
        code_generator_parser() = delete;
        code_generator_parser(const code_generator_parser&) = delete;
        code_generator_parser(code_generator_parser&&) = delete;
        explicit code_generator_parser(const char *fn, logger *logger, bool &ok, code_generator *cg);
        virtual ~code_generator_parser();

        code_generator_parser& operator=(const code_generator_parser&) = delete;
        code_generator_parser& operator=(code_generator_parser&&) = delete;

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