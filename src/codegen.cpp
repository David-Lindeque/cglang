#include "codegen.h"
#include <sstream>
#include <locale>

namespace cglang
{

bool strrep(std::wstring& str, const std::wstring &find, const std::wstring &replace)
{
    bool found = false;
	size_t n = str.find(find);
	while( n != std::wstring::npos) {
        found = true;
		str.replace(n, find.size(), replace);
		n = str.find(find, n + find.size());
	}
    return found;
}

std::wstring cpptext(const std::wstring &s)
{
    std::wstring res;
    for(auto &ch : s) {
        switch(ch) {
            case L'\n': res.append(L"\\n"); break;
            case L'\r': res.append(L"\\r"); break;
            case L'\t': res.append(L"\\t"); break;
            case L'\v': res.append(L"\\v"); break;
            case L'\f': res.append(L"\\f"); break;
            case L'"' : res.append(L"\\\""); break;
            default: res += ch;
        }
    }
    return res;
}

code_generator_parser::code_generator_parser(const char *fn, logger *logger, bool &ok, code_generator *cg)
: parser(logger, ok), _cg(cg), _fn(fn)
{}

code_generator_parser::~code_generator_parser()
{
    if (_outline_steps != nullptr) {
        _logger->write_error(_outline_loc, L"Missing Examples: clause");
        _ok = false;
    }
}

void code_generator_parser::set_grammar(const std::regex_constants::syntax_option_type &grammar, const location &loc)
{
    if (_outline_steps != nullptr) {
        _logger->write_error(_outline_loc, L"Missing Examples: clause");
        _ok = false;
    }
    _cg->set_grammar(grammar);
}

void code_generator_parser::process_import(const wchar_t *, const location &)
{}

void code_generator_parser::set_case_sensitive(bool sensitive, const location &loc)
{
    if (_outline_steps != nullptr) {
        _logger->write_error(_outline_loc, L"Missing Examples: clause");
        _ok = false;
    }
    _cg->set_case_sensitive(sensitive);
}

void code_generator_parser::process_feature_declaration(const wchar_t *name, const location &loc)
{
    if (_outline_steps != nullptr) {
        _logger->write_error(loc, L"Missing Examples: clause");
        _ok = false;
    }
    _ok &= _cg->try_set_feature(_logger, name, loc);
}

void code_generator_parser::process_file_skeleton_declaration(const wchar_t *cpp, const location &loc)
{
    if (_outline_steps != nullptr) {
        _logger->write_error(_outline_loc, L"Missing Examples: clause");
        _ok = false;
    }
    _ok &= _cg->try_set_file_skeleton(_fn, _logger, cpp, loc);
}

void code_generator_parser::process_test_skeleton_declaration(const wchar_t *cpp, const location &loc)
{
    if (_outline_steps != nullptr) {
        _logger->write_error(_outline_loc, L"Missing Examples: clause");
        _ok = false;
    }
    _ok &= _cg->try_set_test_skeleton(_fn, _logger, cpp, loc);
}

void code_generator_parser::process_define_skeleton_declaration(const wchar_t *name, const location &name_loc, const wchar_t *cpp, const location &cpp_loc)
{
    if (_outline_steps != nullptr) {
        _logger->write_error(_outline_loc, L"Missing Examples: clause");
        _ok = false;
    }
    _ok &= _cg->try_set_define_skeleton(_fn, _logger, name, name_loc, cpp, cpp_loc);
}

void code_generator_parser::process_step_skeleton_declaration(const wchar_t *pattern, const location &pattern_loc, const wchar_t *cpp, const location &cpp_loc)
{
    if (_outline_steps != nullptr) {
        _logger->write_error(_outline_loc, L"Missing Examples: clause");
        _ok = false;
    }
    _ok &= _cg->try_add_step_skeleton(_fn, _logger, pattern, pattern_loc, cpp, cpp_loc);
}

void code_generator_parser::process_background_declaration(psteps &&steps, const location &loc)
{
    if (_outline_steps != nullptr) {
        _logger->write_error(_outline_loc, L"Missing Examples: clause");
        _ok = false;
    }
    if (_background != nullptr) {
        _logger->write_error(loc, L"Cannot specify more than one background");
        _ok = false;
    }
    else {
        _background = std::move(steps);
    }
}

void code_generator_parser::process_scenario_outline_declaration(const wchar_t *name, const location &name_loc, psteps &&steps, const location &steps_loc)
{
    if (_outline_steps != nullptr) {
        _logger->write_error(_outline_loc, L"Missing Examples: clause");
        _ok = false;
    }
    _outline_name = std::wstring(name) + L"_";
    _outline_steps = std::move(steps);
    _outline_loc = name_loc;
}

void code_generator_parser::process_examples_declaration(table &&table, const location &loc)
{
    if (_outline_steps == nullptr) {
        _logger->write_error(loc, L"Missing Scenario Outline: clause");
        _ok = false;
        return;
    }
    if (table == nullptr || table->empty() || (*table)[0] == nullptr || (*table)[0]->empty()) {
        _logger->write_error(loc, L"Invalid Examples table");
        _ok = false;
        return;
    }

    std::vector<std::wstring> headers;
    for(auto &col : *(*table)[0]) {
        headers.push_back(L"<" + col->value() + L">");
    }

    for(size_t i = 1; i < table->size(); i++) {
        if ((*table)[i] == nullptr || (*table)[i]->size() != headers.size()) {
            _logger->write_error(loc, L"Table rows must all match in length");
            _ok = false;
            return;
        }

        // Get the scenario name
        std::wstring name(_outline_name);
        name.append((*(*table)[i])[0]->value());

        // Replace each step's details with the values from the table and generate the cpp
        std::vector<std::wstring> cpps;
        if (_background != nullptr) {
            for(auto &step : *_background) {
                cpps.emplace_back();
                if (!_cg->try_get_step_cpp(name.c_str(), _logger, cpps.back(), step, [](const std::wstring& s) -> std::wstring { 
                    return s; // We do not process tokens in background
                })) {
                    _ok = false;
                    return; // Error would be logged
                }
            }
        }
        for(auto &step : *_outline_steps) {
            cpps.emplace_back();
            if (!_cg->try_get_step_cpp(name.c_str(), _logger, cpps.back(), step, [&headers, &table, &i](const std::wstring& s) -> std::wstring { 
                std::wstring res(s);
                for(size_t h = 0; h < headers.size(); h++) {
                    strrep(res, headers[h], (*(*table)[i])[h]->value());
                }
                return res;
            })) {
                _ok = false;
                return; // Error would be logged
            }
        }

        // Write the test
        if (!_cg->try_add_test(_logger, name.c_str(), _outline_loc, cpps)) {
            _ok = false;
            return; // Error would be logged
        }
    }

    _outline_steps.reset();
}

void code_generator_parser::process_scenario_declaration(const wchar_t *name, const location &name_loc, psteps &&steps, const location &steps_loc)
{
    if (_outline_steps != nullptr) {
        _logger->write_error(_outline_loc, L"Missing Examples: clause");
        _ok = false;
    }

    if (steps == nullptr) return;

    std::vector<std::wstring> cpps;
    if (_background != nullptr) {
        for(auto &step : *_background) {
            cpps.emplace_back();
            if (!_cg->try_get_step_cpp(name, _logger, cpps.back(), step, [](const std::wstring& s) -> std::wstring { 
                return s;
            })) {
                _ok = false;
                return; // Error would be logged
            }
        }
    }
    for(auto &step : *steps) {
        cpps.emplace_back();
        if (!_cg->try_get_step_cpp(name, _logger, cpps.back(), step, [](const std::wstring& s) -> std::wstring { 
            return s;
        })) {
            _ok = false;
            return; // Error would be logged
        }
    }

    // Write the test
    _ok &= _cg->try_add_test(_logger, name, name_loc, cpps); // Error would be logged
}

void code_generator::write_skeleton(std::wostream &stm, const span &text, const std::unordered_map<std::wstring, std::wstring> &singe_line_tokens, const std::unordered_map<std::wstring, std::wstring> &multi_line_tokens)
{
    // We read the text line by line, replacing the following tokens as we find them
    // 1. $feature
    // 2. $FEATURE
    // 3. Any of the defines
    // 4. Any additional tokens passed as 'tokens'
    // We also keep tally of the current line number and reset the
    // line whenever we actually made a replacement (of more than a single line)
    const wchar_t *s = text._text.c_str();
    while(*s == L' ' || *s == L'\t' || *s == L'\n' || *s == L'\r' || *s == L'\v' || *s == L'\f') {
        s++;
    }
    if (*s == L'\0') {
        // No text, return empty string (no redirection)
        return;
    }

    // Write the redirection
    stm << std::endl << L"#line " << text._line << L" \"" << std::wstring(text._fn.begin(), text._fn.end()) << "\"" << std::endl;

    std::locale locl;
    std::wstring FEATURE_NAME;
    for(auto &ch : _feature_name) {
        FEATURE_NAME += std::toupper(ch, locl);
    }

    std::wstringstream source(text._text);
    std::wstring line;
    int lineno = text._line;
    while(std::getline(source, line)) {
        //stm << std::wstring(text._fn.begin(), text._fn.end()) << ':' << lineno << ' ';
        strrep(line, L"$feature", _feature_name);
        strrep(line, L"$FEATURE", FEATURE_NAME);
        for(auto &t : singe_line_tokens) {
            strrep(line, t.first.c_str(), t.second);
        }
        bool updated = false;
        for(auto &d : _defines) {
            updated |= strrep(line, d.first.c_str(), d.second);
        }
        for(auto &t : multi_line_tokens) {
            updated |= strrep(line, t.first.c_str(), t.second);
        }
        stm << line << std::endl;
        if (updated) {
            stm << L"#line " << (lineno+1) << L" \"" << std::wstring(text._fn.begin(), text._fn.end()) << L"\"" << std::endl;
        }

        lineno++;
    }
}

void code_generator::set_grammar(const std::regex_constants::syntax_option_type &grammar)
{
    _grammar = grammar;
}

void code_generator::set_case_sensitive(bool sensitive)
{
    _case_sensitive = sensitive;
}

bool code_generator::try_set_file_skeleton(const std::string &fn, logger *logger, const wchar_t *cpp, const location &loc)
{
    if (_file_skeleton._text.empty()) {
        _file_skeleton._text = cpp;
        _file_skeleton._fn = fn;
        _file_skeleton._line = loc.first_line;
        return true;
    }
    else {
        logger->write_error(loc, L"Cannot specify more than one %file option.");
        return false;
    }
}

bool code_generator::try_set_test_skeleton(const std::string &fn, logger *logger, const wchar_t *cpp, const location &loc)
{
    if (_test_skeleton._text.empty()) {
        _test_skeleton._text = cpp;
        _test_skeleton._fn = fn;
        _test_skeleton._line = loc.first_line;
        return true;
    }
    else {
        logger->write_error(loc, L"Cannot specify more than one %test option.");
        return false;
    }
}

bool code_generator::try_add_step_skeleton(const std::string &fn, logger *logger, const wchar_t *pattern, const location &pattern_loc, const wchar_t *cpp, const location &cpp_loc)
{
    std::wregex re;
    std::regex::flag_type flags(_grammar);
    if (!_case_sensitive) {
        flags |= std::regex_constants::syntax_option_type::icase;
    }
    try {
        re.assign(pattern, flags);
    } catch(std::regex_error &ex) {
        std::string exwhat(ex.what());
        logger->write_error(pattern_loc, std::wstring(exwhat.begin(), exwhat.end()).c_str());
        return false;
    }

    _step_skeletons.push_back(code_generator::step { std::move(re), pattern_loc, code_generator::span { cpp, fn, cpp_loc.first_line } });
    return true;
}

bool code_generator::try_set_define_skeleton(const std::string &fn, logger *logger, const wchar_t *name, const location &name_loc, const wchar_t *cpp, const location &cpp_loc)
{
    std::wstring n(L"$");
    n.append(name);

    std::wstringstream stm;
    write_skeleton(stm, code_generator::span{ cpp, fn, cpp_loc.first_line });
    auto f = _defines.emplace(n, stm.str());
    if (!f.second) {
        f.first->second = stm.str();
    }
    return true;
}

bool code_generator::try_set_feature(logger *logger, const wchar_t *feature_name, const location &loc)
{
    if (_feature_name.empty()) {
        _feature_name = feature_name;
        return true;
    }
    else {
        logger->write_error(loc, L"Cannot define a feature more than once.");
        return false;
    }
}

bool code_generator::try_get_step_cpp(const wchar_t *scenario_name, logger *logger, std::wstring &cpp, const pstep &step, const std::function<std::wstring(const std::wstring&)> &prepare)
{
    bool found = false;
    std::wstring text(prepare(step->text()));
    auto f = text.find_last_not_of(L" \t\f\v");
    if (f != std::wstring::npos) {
        text = text.substr(0, f + 1);
    }
    for(auto &s : _step_skeletons) {
        if (found) {
            if (std::regex_match(text, s._re)) {
                std::wstringstream stm;
                stm << L"Multiple patterns matched the text of this step (" << text << ")";
                logger->write_error(step->span(), stm.str().c_str());
                return false;
            }
        }
        else {
            std::wsmatch m;
            if (std::regex_match(text, m, s._re)) {
                found = true;
                std::wstringstream stm;
                stm << L"{ ";
                // Generate the table
                if (step->_table != nullptr) {
                    if (_gen_wide) {
                        stm << std::endl << L"         static const wchar_t data[] = L\"";
                    }
                    else {
                        stm << std::endl << L"         static const char data[] = \"";
                    }
                    for(size_t r = 0; r < step->_table->size(); r++) {
                        if ((*step->_table)[r]->size() != (*step->_table)[0]->size()) {
                            logger->write_error(step->span(), L"Table rows must be equal length.");
                            return false;
                        }
                        for(size_t c = 0; c < (*step->_table)[r]->size(); c++) {
                            stm << cpptext(prepare((*(*step->_table)[r])[c]->value())) << L"\\000";
                        }
                        stm << L"\\000";
                    }
                    stm << L"\\000\";" << std::endl;
                    if (_gen_wide) {
                        stm << L"         static const wchar_t *indexes[] = {";
                    }
                    else {
                        stm << L"         static const char *indexes[] = {";
                    }
                    size_t ofs = 0;
                    for(size_t r = 0; r < step->_table->size(); r++) {
                        for(size_t c = 0; c < (*step->_table)[r]->size(); c++) {
                            if (ofs == 0) {
                                stm << L" data";
                            }
                            else {
                                stm << L", data + " << ofs;
                            }
                            ofs += 1 + (*(*step->_table)[r])[c]->value().size();
                        }
                        ofs++;
                    }
                    stm << L" };" << std::endl;
                    stm << L"         struct headers" << std::endl;
                    stm << L"         {" << std::endl;
                    stm << L"             enum" << std::endl;
                    stm << L"             {";
                    for(size_t c = 0; c < (*step->_table)[0]->size(); c++) {
                        if (c == 0) {
                            stm << std::endl;
                        }
                        else {
                            stm << L',' << std::endl;
                        }
                        stm << L"                 " << prepare((*(*step->_table)[0])[c]->value()) << " = " << c;
                    }
                    stm << std::endl << L"             };" << std::endl;
                    stm << L"         };" << std::endl;
                    if (_gen_wide) {
                        stm << L"         cglang::utils::table<" << (*step->_table)[0]->size() << L", wchar_t> tbl(indexes, " << step->_table->size() << L");" << std::endl;
                    }
                    else {
                        stm << L"         cglang::utils::table<" << (*step->_table)[0]->size() << L", char> tbl(indexes, " << step->_table->size() << L");" << std::endl;
                    }
                }
                // Generate the mtext
                if (!step->_mtext.empty()) {
                    if (_gen_wide) {
                        stm << std::endl << L"         static const wchar_t mtext[] = L\"";
                    }
                    else {
                        stm << std::endl << L"         static const char mtext[] = \"";
                    }
                    stm << cpptext(prepare(step->_mtext));
                    stm << L"\";" << std::endl;
                }
                // Generate the step
                std::wstringstream stm2;
                write_skeleton(stm2, s._text);
                std::wstring stepcpp(stm2.str());
                strrep(stepcpp, L"$scenario", scenario_name);
                if (step->_table) {
                    strrep(stepcpp, L"$0", L"tbl");
                }
                if (!step->_mtext.empty()) {
                    strrep(stepcpp, L"$0", L"mtext");
                }
                for(size_t i = 1; i < m.size(); i++) {
                    std::wstringstream var;
                    var << L'$' << i;
                    strrep(stepcpp, var.str(), cpptext(m[i].str()));
                }
                stm << stepcpp;
                stm << "    }";
                cpp = stm.str();
            }
        }
    }

    if (!found) {
        std::wstringstream stm;
        stm << L"No pattern could be matched to this step (" << text << ")";
        logger->write_error(step->span(), stm.str().c_str());
        return false;
    }

    return true;
}

bool code_generator::try_add_test(logger *logger, const wchar_t *scenario_name, const location &loc, const std::vector<std::wstring> &step_cpps)
{
    if (_feature_name.empty()) {
        logger->write_error(loc, L"A feature must be specified before a scenario or scenario outline.");
        return false;
    }

    std::wstring steps;
    for(auto &step : step_cpps) {
        steps.append(L"    ");
        steps.append(step);
        steps.append(L"\n");
    }

    std::unordered_map<std::wstring, std::wstring> single_line_tokens, multi_line_tokens;
    multi_line_tokens.emplace(L"$steps", steps);
    single_line_tokens.emplace(L"$scenario", scenario_name);

    std::wstringstream stm;
    write_skeleton(stm, _test_skeleton, single_line_tokens, multi_line_tokens);
    stm << std::endl;
    
    _tests.append(stm.str());

    return true;
}

bool code_generator::try_write_file(std::ostream &err, FILE* f)
{
    if (_file_skeleton._text.empty()) {
        err << "No %file option found." << std::endl;
        return false;
    }
    if (_test_skeleton._text.empty()) {
        err << "No %test option found." << std::endl;
        return false;
    }

    std::unordered_map<std::wstring, std::wstring> tokens;
    tokens.emplace(L"$tests", _tests);

    std::wstringstream stm;
    write_skeleton(stm, _file_skeleton, std::unordered_map<std::wstring, std::wstring>(), tokens);
    stm << std::endl;

    fwprintf(f, L"%ls", stm.str().c_str());
    return true;
}


}