#include "parser.h"
#include "ast.h"
#include "portability.h"
#include <sstream>

namespace cglang
{

parser::parser(logger *logger, bool &ok)
: _logger(logger), _ok(ok)
{}

parser::~parser()
{}

void parser::set_step_table(step* step, table &&table, const location &table_loc)
{
    if (step->_mtext.empty() && step->_table == nullptr) {
        step->_table_loc = table_loc;
        step->_table = std::move(table);
    }
    else {
        _logger->write_error(table_loc, L"A step must have either a table or multiline text, or none, but cannot have both.");
        _ok = false;
    }
}

void parser::set_step_mtext(step* step, const wchar_t *text, const location &text_loc)
{
    if (step->_mtext.empty() && step->_table == nullptr) {
        step->_mtext = text;
        step->_mtext_loc = text_loc;
    }
    else {
        _logger->write_error(text_loc, L"A step must have either a table or multiline text, or none, but cannot have both.");
        _ok = false;
    }
}

void parser::process_error(const location &loc, const char *parser_error, const char *lexer_error)
{
    std::wstringstream stm;
    if (parser_error) {
        if (cglang::stricmp(parser_error, "syntax error") != 0) {
            std::string pe(parser_error);
            stm << std::wstring(pe.begin(), pe.end()) << L'.';
            if (lexer_error) stm << L' ';
        }
        if (lexer_error) {
            std::string le(lexer_error);
            stm << std::wstring(le.begin(), le.end());
        }
    }
    else if (lexer_error) {
        std::string le(lexer_error);
        stm << std::wstring(le.begin(), le.end());
    }
    else {
        stm << L"Syntax error";
    }

    _logger->write_error(loc, stm.str().c_str());
    _ok = false;
}

}