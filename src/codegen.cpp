#include "codegen.h"
#include <sstream>
#include <locale>

namespace cglang
{

code_generator_parser::code_generator_parser(logger *logger, bool &ok, code_generator *cg)
: parser(logger, ok), _cg(cg)
{}

code_generator_parser::~code_generator_parser()
{
    if (_outline_steps != nullptr) {
        _logger->write_error(_outline_loc, "Missing Examples: clause");
        _ok = false;
    }
}

void code_generator_parser::set_grammar(const std::regex_constants::syntax_option_type &grammar, const location &loc)
{
    if (_outline_steps != nullptr) {
        _logger->write_error(_outline_loc, "Missing Examples: clause");
        _ok = false;
    }
    _cg->set_grammar(grammar);
}

void code_generator_parser::process_import(const wchar_t *, const location &)
{}

void code_generator_parser::set_case_sensitive(bool sensitive, const location &loc)
{
    if (_outline_steps != nullptr) {
        _logger->write_error(_outline_loc, "Missing Examples: clause");
        _ok = false;
    }
    _cg->set_case_sensitive(sensitive);
}

void code_generator_parser::process_feature_declaration(const wchar_t *name, const location &loc)
{
    if (_outline_steps != nullptr) {
        _logger->write_error(loc, "Missing Examples: clause");
        _ok = false;
    }
    _ok &= _cg->try_set_feature(_logger, name, loc);
}

void code_generator_parser::process_file_skeleton_declaration(const wchar_t *cpp, const location &loc)
{
    if (_outline_steps != nullptr) {
        _logger->write_error(_outline_loc, "Missing Examples: clause");
        _ok = false;
    }
    _ok &= _cg->try_set_file_skeleton(_logger, cpp, loc);
}

void code_generator_parser::process_test_skeleton_declaration(const wchar_t *cpp, const location &loc)
{
    if (_outline_steps != nullptr) {
        _logger->write_error(_outline_loc, "Missing Examples: clause");
        _ok = false;
    }
    _ok &= _cg->try_set_test_skeleton(_logger, cpp, loc);
}

void code_generator_parser::process_define_skeleton_declaration(const wchar_t *name, const location &name_loc, const wchar_t *cpp, const location &cpp_loc)
{
    if (_outline_steps != nullptr) {
        _logger->write_error(_outline_loc, "Missing Examples: clause");
        _ok = false;
    }
    _ok &= _cg->try_set_define_skeleton(_logger, name, name_loc, cpp, cpp_loc);
}

void code_generator_parser::process_step_skeleton_declaration(const wchar_t *pattern, const location &pattern_loc, const wchar_t *cpp, const location &cpp_loc)
{
    if (_outline_steps != nullptr) {
        _logger->write_error(_outline_loc, "Missing Examples: clause");
        _ok = false;
    }
    _ok &= _cg->try_add_step_skeleton(_logger, pattern, pattern_loc, cpp, cpp_loc);
}

void code_generator_parser::process_background_declaration(psteps &&steps, const location &loc)
{
    if (_outline_steps != nullptr) {
        _logger->write_error(_outline_loc, "Missing Examples: clause");
        _ok = false;
    }
    if (_background != nullptr) {
        _logger->write_error(loc, "Cannot specify more than one background");
        _ok = false;
    }
    else {
        _background = std::move(steps);
    }
}

void code_generator_parser::process_scenario_outline_declaration(const wchar_t *name, const location &name_loc, psteps &&steps, const location &steps_loc)
{
    if (_outline_steps != nullptr) {
        _logger->write_error(_outline_loc, "Missing Examples: clause");
        _ok = false;
    }
    _outline_name = std::wstring(name) + L"_";
    _outline_steps = std::move(steps);
    _outline_loc = name_loc;
}

void strrep(std::wstring& str, std::wstring find, std::wstring replace)
{
	size_t n = str.find(find);
	while( n != std::wstring::npos) {
		str.replace(n, find.size(), replace);
		n = str.find(find, n + find.size());
	}
}

void code_generator_parser::process_examples_declaration(table &&table, const location &loc)
{
    if (_outline_steps == nullptr) {
        _logger->write_error(loc, "Missing Scenario Outline: clause");
        _ok = false;
        return;
    }
    if (table == nullptr || table->empty() || (*table)[0] == nullptr || (*table)[0]->empty()) {
        _logger->write_error(loc, "Invalid Examples table");
        _ok = false;
        return;
    }

    std::vector<std::wstring> headers;
    for(auto &col : *(*table)[0]) {
        headers.push_back(L"<" + col->value() + L">");
    }

    for(size_t i = 1; i < table->size(); i++) {
        if ((*table)[i] == nullptr || (*table)[i]->size() != headers.size()) {
            _logger->write_error(loc, "Table rows must all match in length");
            _ok = false;
            return;
        }

        // Replace each step's details with the values from the table and generate the cpp
        std::vector<std::wstring> cpps;
        if (_background != nullptr) {
            for(auto &step : *_background) {
                cpps.emplace_back();
                if (!_cg->try_get_step_cpp(_logger, cpps.back(), step, [](const std::wstring& s) -> std::wstring { 
                    return s; // We do not process tokens in background
                })) {
                    _ok = false;
                    return; // Error would be logged
                }
            }
        }
        for(auto &step : *_outline_steps) {
            cpps.emplace_back();
            if (!_cg->try_get_step_cpp(_logger, cpps.back(), step, [&headers, &table, &i](const std::wstring& s) -> std::wstring { 
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
        std::wstring name(_outline_name);
        name.append((*(*table)[i])[0]->value());
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
        _logger->write_error(_outline_loc, "Missing Examples: clause");
        _ok = false;
    }

    if (steps == nullptr) return;

    std::vector<std::wstring> cpps;
    if (_background != nullptr) {
        for(auto &step : *_background) {
            cpps.emplace_back();
            if (!_cg->try_get_step_cpp(_logger, cpps.back(), step, [](const std::wstring& s) -> std::wstring { 
                return s;
            })) {
                _ok = false;
                return; // Error would be logged
            }
        }
    }
    for(auto &step : *steps) {
        cpps.emplace_back();
        if (!_cg->try_get_step_cpp(_logger, cpps.back(), step, [](const std::wstring& s) -> std::wstring { 
            return s;
        })) {
            _ok = false;
            return; // Error would be logged
        }
    }

    // Write the test
    _ok &= _cg->try_add_test(_logger, name, name_loc, cpps); // Error would be logged
}

void code_generator::replace_std_tokens(std::wstring &result)
{
    for(auto &d : _defines) {
        strrep(result, (L"$" + d.first).c_str(), d.second);
    }
    strrep(result, L"$feature", _feature_name);
    std::locale locl;
    std::wstring tmp;
    for(auto &ch : _feature_name) {
        tmp += std::toupper(ch, locl);
    }
    strrep(result, L"$FEATURE", tmp);
}

void code_generator::set_grammar(const std::regex_constants::syntax_option_type &grammar)
{
    _grammar = grammar;
}

void code_generator::set_case_sensitive(bool sensitive)
{
    _case_sensitive = sensitive;
}

bool code_generator::try_set_file_skeleton(logger *logger, const wchar_t *cpp, const location &loc)
{
    if (_file_skeleton.empty()) {
        std::wstringstream stm;
        stm << "#line " << loc.first_line << " \"file\"" << std::endl << cpp;
        _file_skeleton = stm.str();
        return true;
    }
    else {
        logger->write_error(loc, "Cannot specify more than one %file option.");
        return false;
    }
}

bool code_generator::try_set_test_skeleton(logger *logger, const wchar_t *cpp, const location &loc)
{
    if (_test_skeleton.empty()) {
        _test_skeleton = cpp;
        return true;
    }
    else {
        logger->write_error(loc, "Cannot specify more than one %test option.");
        return false;
    }
}

bool code_generator::try_add_step_skeleton(logger *logger, const wchar_t *pattern, const location &pattern_loc, const wchar_t *cpp, const location &cpp_loc)
{
    std::wregex re;
    std::regex::flag_type flags(_grammar);
    if (!_case_sensitive) {
        flags |= std::regex_constants::syntax_option_type::icase;
    }
    try {
        re.assign(pattern, flags);
    } catch(std::regex_error &ex) {
        logger->write_error(pattern_loc, ex.what());
        return false;
    }
    _step_skeletons.push_back(code_generator::step { std::move(re), pattern_loc, cpp });
    return true;
}

bool code_generator::try_set_define_skeleton(logger *logger, const wchar_t *name, const location &name_loc, const wchar_t *cpp, const location &cpp_loc)
{
    std::wstring text(cpp);
    replace_std_tokens(text);
    auto f = _defines.emplace(name, text);
    if (!f.second) f.first->second = text;
    return true;
}

bool code_generator::try_set_feature(logger *logger, const wchar_t *feature_name, const location &loc)
{
    if (_feature_name.empty()) {
        _feature_name = feature_name;
        return true;
    }
    else {
        logger->write_error(loc, "Cannot define a feature more than once.");
        return false;
    }
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

bool code_generator::try_get_step_cpp(logger *logger, std::wstring &cpp, const pstep &step, const std::function<std::wstring(const std::wstring&)> &prepare)
{
    bool found = false;
    for(auto &s : _step_skeletons) {
        std::wstring text(prepare(step->text()));
        if (found) {
            if (std::regex_match(text, s._re)) {
                logger->write_error(step->span(), "Multiple patterns matched the text of this step");
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
                            logger->write_error(step->span(), "Table rows must be equal length.");
                            return false;
                        }
                        for(size_t c = 0; c < (*step->_table)[r]->size(); c++) {
                            stm << cpptext((*(*step->_table)[r])[c]->value()) << L"\\0";
                        }
                        stm << L"\\0";
                    }
                    stm << L"\\0\";" << std::endl;
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
                        stm << L"                 " << (*(*step->_table)[0])[c]->value() << " = " << c;
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
                std::wstring stepcpp(s._cpp);
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
        logger->write_error(step->span(), "No pattern could be matched to this step.");
        return false;
    }

    return true;
}

bool code_generator::try_add_test(logger *logger, const wchar_t *scenario_name, const location &loc, const std::vector<std::wstring> &step_cpps)
{
    if (_feature_name.empty()) {
        logger->write_error(loc, "A feature must be specified before a scenario or scenario outline.");
        return false;
    }
    
    std::wstring test(_test_skeleton);
    replace_std_tokens(test);
    strrep(test, L"$scenario", scenario_name);
    std::wstring steps;
    for(auto &step : step_cpps) {
        steps.append(L"    ");
        steps.append(step);
        steps.append(L"\n");
    }
    test.append(L"\n");
    strrep(test, L"$steps", steps);
    _tests.append(test);

    return true;
}

bool code_generator::try_write_file(std::ostream &err, FILE* f)
{
    if (_file_skeleton.empty()) {
        err << "No %file option found." << std::endl;
        return false;
    }
    if (_test_skeleton.empty()) {
        err << "No %test option found." << std::endl;
        return false;
    }
    std::wstring content(_file_skeleton);
    replace_std_tokens(content);
    strrep(content, L"$tests", _tests);
    fwprintf(f, L"%ls", content.c_str());
    return true;
}


}