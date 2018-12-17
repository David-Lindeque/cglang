#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include "ast.h"
#include "parser.h"
#include "../bin/feature.y.h"

FILE* yyin;
const char *lexer_error = NULL;
int prev_last_column;
int prev_last_line;
int last_step = TOK_GIVEN;

const char unexpected_end_of_line_while_reading_pattern[] = "Unexpected end-of-line while reading a pattern.";
const char expected_3_quotes_for_multiline_text[] = "Expected 3 quotes as the multiline text opening.";
const char no_text_allowed_after_opening_quotes_for_multiline_text[] = "No text allowed behind the opening quotes of a multiline text.";
const char unexpected_end_of_file_while_reading_multiline_text[] = "Unexpected end-of-file while reading multiline text.";
const char no_text_allowed_in_the_margin_of_multiline_text[] = "No text allowed in the margin of multiline text.";
const char unexpected_end_of_line_while_reading_cell[] = "Unexpected end-of-line while reading a table cell.";
const char unexpected_character_for_identifier[] = "Unexpected character read. Expected _, a-z or A-Z.";
const char unexpected_option_name[] = "Unexpected option name.";
const char invalid_option_syntax[] = "Invalid option syntax.";
const char out_of_memory[] = "Out of memory.";
const char unexpected_end_of_line_reading_import[] = "Unexpected end-of-line while reading an import statement";

namespace {

class text_buf {
private:
    size_t _size;
    size_t _cur;
    wchar_t *_buf;

    void ensure_capacity() {
        if (_cur != _size) return;
        _size <<= 1;
        wchar_t *n = (wchar_t*)malloc(_size*sizeof(wchar_t));
        if (n != NULL) {
            memcpy(n, _buf, _cur*sizeof(wchar_t));
        }
        free(_buf);
        _buf = n;
    }
public:
    text_buf()
    : _size(32), _cur(0)
    {
        _buf = (wchar_t*)malloc(_size*sizeof(wchar_t));
    }
    text_buf(const text_buf&) = delete;
    text_buf(text_buf &&) = delete;
    ~text_buf()
    {
        if (_buf) free(_buf);
    }

    void append(const wchar_t &ch) {
        ensure_capacity();
        if (_buf) {
            _buf[_cur] = ch;
        }
        _cur++;
    }

    wchar_t *take() {
        ensure_capacity();
        if(_buf) {
            _buf[_cur] = L'\0';
        }
        wchar_t* ret = _buf;
        _buf = nullptr;
        return ret;
    }
};

wint_t read()
{
    prev_last_column = yylloc.last_column;
    prev_last_line = yylloc.last_line;
    wint_t v = fgetwc(yyin);
    switch(v) {
        case (wint_t)L'\n':
            yylloc.last_line++;
            yylloc.last_column=1;
            return v;
        case (wint_t)L'\r':
            yylloc.last_column=1;
            return v;
        case (wint_t)WEOF:
            return v;
        default:
            yylloc.last_column++;
            return v;
    }
}

void unread(wint_t ch)
{
    yylloc.last_column = prev_last_column;
    yylloc.last_line = prev_last_line;
    ungetwc(ch, yyin);    
}

int read_cpp()
{
    // Read until } (taking {}, strings, chars and comments into account)
    text_buf buf;
    int state = 0;
    int scope_depth = 1;
    while (true) {
        wint_t v = read();
        if (v == WEOF) {
            yylval.str = buf.take();
            return TOK_CPP;
        }
        switch(state) {
            case 0:
                switch(v) {
                    case (wint_t)L'{':
                        scope_depth++;
                        buf.append(L'{');
                        break;
                    case (wint_t)L'}':
                        scope_depth--;
                        if (scope_depth == 0) {
                            yylval.str = buf.take();
                            return TOK_CPP;
                        }
                        else {
                            buf.append(L'}');
                        }
                        break;
                    case (wint_t)L'"':
                        state = 1;
                        buf.append(L'"');
                        break;
                    case (wint_t)L'\'':
                        state = 2;
                        buf.append(L'\'');
                        break;
                    case (wint_t)L'/':
                        state = 3;
                        buf.append('/');
                        break;
                    default:
                        buf.append((wchar_t)v);
                        break;
                }
                break;
            case 1: // We've read a ". We notice \" escape, and look for closing "
                switch(v) {
                    case (wint_t)L'\\':
                        state = 11;
                        buf.append((wchar_t)v);
                        break;
                    case (wint_t)L'"': // Final quote
                        state = 0;
                        buf.append(L'"');
                        break;
                    default:
                        buf.append((wchar_t)v);
                        break;
                }
                break;
            case 2: // We've read a '
                switch(v) {
                    case (wint_t)L'\\':
                        state = 21;
                        buf.append((wchar_t)v);
                        break;
                    case (wint_t)L'\'': // Final apos
                        state = 0;
                        buf.append(L'\'');
                        break;
                    default:
                        buf.append((wchar_t)v);
                        break;
                }
                break;
            case 3: // We've read a /
                switch(v) {
                    case (wint_t)L'/': // line comment
                        state = 31;
                        buf.append(L'/');
                        break;
                    case (wint_t)L'*': // block comment
                        state = 32;
                        buf.append(L'*');
                        break;
                    default:
                        buf.append((wchar_t)v);
                        state = 0;
                        break;
                }
                break;
            case 11: // We're reading a string, but read a slash
                state = 1;
                buf.append((wchar_t)v);
                break;
            case 21: // We're reading a char, but read a slash
                state = 2;
                buf.append((wchar_t)v);
                break;
            case 31: // Reading a line comment
                if (v == (wint_t)L'\n') {
                    // end of line comment
                    state = 0;
                    buf.append(L'\n');
                }
                else {
                    buf.append((wchar_t)v);
                }
                break;
            case 32: // Reading a block comment
                if (v == (wint_t)L'*') {
                    state = 33;
                    buf.append((wchar_t)v);
                }
                else {
                    buf.append((wchar_t)v);
                }
                break;
            case 33: // Reading a block comment and read '*'
                switch(v) {
                    case (wint_t)L'*': // stay here
                        buf.append(L'*');
                        break;
                    case (wint_t)L'/': // end of comment
                        state = 0;
                        buf.append(L'/');
                        break;
                    default: // not end of comment
                        state = 32;
                        buf.append((wchar_t)v);
                        break;
                }
        }
    }
}

int read_pattern()
{
    // Read until ' (taking escaping into account)
    // escaping is only \'
    text_buf buf;
    while(true) {
        wint_t v = read();
        if (v == WEOF || v == (wint_t)L'\n') {
            lexer_error = unexpected_end_of_line_while_reading_pattern;
            return TOK_ERROR;
        }
        else if (v == (wint_t)L'\'') {
            // End of pattern
            yylval.str = buf.take();
            return TOK_PATTERN;
        }
        else if (v == (wint_t)L'\\') {
            v = read();
            if(v == WEOF || v == (wint_t)L'\n') {
                lexer_error = unexpected_end_of_line_while_reading_pattern;
                return TOK_ERROR;
            }
            else if (v == (wint_t)L'\'') {
                buf.append(L'\'');
            }
            else {
                buf.append(L'\\');
                buf.append((wchar_t)v);
            }
        }
        else {
            buf.append((wchar_t)v);
        }
    }
}

int read_import_name()
{
    text_buf buf;
    while(true) {
        wint_t v = read();
        if (v == WEOF || v == (wint_t)L'\n') {
            lexer_error = unexpected_end_of_line_reading_import;
            return TOK_ERROR;
        }
        if (v == (wint_t)L'>') {
            yylval.str = buf.take();
            return TOK_IMPORT_NAME;
        }
        buf.append((wchar_t)v);
    }
}

int read_mtext()
{
    bool margin_violation = false;
    // Read "" (additional two "'s - else error). Read until """ (skip over " or "")
    int margin = yylloc.first_column - 1;
    wint_t v = read();
    if (v != (wint_t)L'"') {
        lexer_error = expected_3_quotes_for_multiline_text;
        return TOK_ERROR;
    }
    v = read();
    if (v != (wint_t)L'"') {
        lexer_error = expected_3_quotes_for_multiline_text;
        return TOK_ERROR;
    }
    v = read();
    while(v == (wint_t)L' ' || v == (wint_t)L'\t' || v == (wint_t)L'\f' || v == (wint_t)L'\v' || v == (wint_t)L'\r') v = read();
    if (v != (wint_t)L'\n') {
        lexer_error = no_text_allowed_after_opening_quotes_for_multiline_text;
        return TOK_ERROR; // We do not allow any text behind """
    }

    // We're now at the first column of the text - we must skip the margin
    text_buf buf;
    while(true) {
        v = read();
        if (v == WEOF) {
            lexer_error = unexpected_end_of_file_while_reading_multiline_text;
            return TOK_ERROR;
        }

        // We should skip the margin. v contains the first space
        for(int i = 0; i < margin; i++) {
            if (v == (wint_t)L' ' || v == (wint_t)L'\r') {
                v = read();
            }
            else if (v == WEOF) {
                lexer_error = unexpected_end_of_file_while_reading_multiline_text;
                return TOK_ERROR;
            }
            else if (v == (wint_t)L'\n') {
                break; // We allow nl in margin, as if the margin was fully read
            }
            else {
                margin_violation = true;
                break;
            }
        }
        // now v contains the first character of the text - read to the end of the line
        while(true) {
            if (v == WEOF) {
                lexer_error = unexpected_end_of_file_while_reading_multiline_text;
                return TOK_ERROR;
            }
            else if (v == (wint_t)L'\n') {
                buf.append(L'\n');
                break;
            }
            else if (v == (wint_t)L'\\') {
                v = read();
                if (v == (wint_t)L'"') {
                    buf.append(L'"');
                }
                else if (v == (wint_t)L't') {
                    buf.append(L'\t');
                }
                else {
                    buf.append(L'\\');
                    buf.append(L'\\');
                    buf.append((wchar_t)v);
                }
                v = read();
            }
            else if (v == (wint_t)L'"') {
                v = read();
                if (v == (wint_t)L'"') {
                    v = read();
                    if (v == (wint_t)L'"') {
                        v = read();
                        while(v == (wint_t)L'"') {
                            v = read();
                            buf.append(L'"');
                        }
                        unread(v);
                        if (margin_violation) {
                            lexer_error = no_text_allowed_in_the_margin_of_multiline_text;
                            return TOK_ERROR;
                        }
                        else {
                            yylval.str = buf.take();
                            return TOK_MTEXT;
                        }
                    }
                    else {
                        buf.append(L'"');
                        buf.append(L'"');
                    }
                }
                else {
                    buf.append(L'"');
                }
            }
            else {
                buf.append((wchar_t)v);
                v = read();
            }
        }
    }
}

int read_cell()
{
    // Read until we see '|', '\n' or '#'
    text_buf buf;
    int state = 0;
    while(true) {
        wint_t v = read();
        switch(state) {
            case 0:
                switch(v) {
                    case WEOF:
                        return TOK_ROW_END;
                    case (wint_t)L' ':
                    case (wint_t)L'\t':
                    case (wint_t)L'\r':
                        // skip initial whitespace
                        break;
                    case (wint_t)L'\n':
                        return TOK_ROW_END;
                    case (wint_t)L'#':
                        unread((wint_t)L'#');
                        return TOK_ROW_END;
                    case (wint_t)L'|': // Empty cell
                        unread(L'|');
                        yylval.str = buf.take();
                        return TOK_CELL;
                    default:
                        buf.append((wchar_t)v);
                        state = 1;
                        break;
                }
                break;
            case 1:
                switch(v) {
                    case (wint_t)L'|':
                        unread(L'|');
                        yylval.str = buf.take();
                        // Remove the trailing whitespace
                        if(true) {
                            wchar_t *p = yylval.str;
                            while(*p) p++;
                            while(p != yylval.str) {
                                p--;
                                if (*p != L' ' && *p != L'\t' && *p != L'\r' && *p != L'\n' && *p != L'\v' && *p != L'\f') {
                                    p[1] = L'\0';
                                    break;
                                }
                            }
                        }
                        return TOK_CELL;
                    case (wint_t)L'\n':
                        lexer_error = unexpected_end_of_line_while_reading_cell;
                        return TOK_ERROR;
                    case (wint_t)L'#':
                        unread((wint_t)L'#');
                        lexer_error = unexpected_end_of_line_while_reading_cell;
                        return TOK_ERROR;
                    default:
                        buf.append((wchar_t)v);
                        break;
                }
        }
    }
}

int read_identifier()
{
    text_buf buf;
    wint_t v = read();
    if (v != (wint_t)L'_' && (v < (wint_t)L'a' || v > (wint_t)L'z') && (v < (wint_t)L'A' || v > (wint_t)L'Z')) {
        lexer_error = unexpected_character_for_identifier;
        return TOK_ERROR;
    }
    buf.append((wchar_t)v);
    while(true) {
        v = read();
        if (v == (wint_t)L'_' || (v >= (wint_t)L'a' && v <= (wint_t)L'z') || (v >= (wint_t)L'A' && v <= (wint_t)L'Z') || (v >= (wint_t)L'0' && v <= (wint_t)L'9')) {
            buf.append((wchar_t)v);
        }
        else {
            // Found the end of the identifier - put back the last read character
            unread(v);
            yylval.str = buf.take();
            if (yylval.str) {
                return TOK_IDENTIFIER;
            }
            else {
                lexer_error = out_of_memory;
                return TOK_OUT_OF_MEMORY;
            }
        }
    }
}

int read_option()
{
    int v = read_identifier();
    if (v == TOK_IDENTIFIER) {
        if (wcscasecmp(yylval.str, L"file")==0) {
            free(yylval.str);
            return TOK_FILE_SKELETON;
        }
        else if (wcscasecmp(yylval.str, L"test")==0) {
            free(yylval.str);
            return TOK_TEST_SKELETON;
        }
        else if (wcscasecmp(yylval.str, L"grammar")==0) {
            free(yylval.str);
            return TOK_GRAMMER;
        }
        else if (wcscasecmp(yylval.str, L"case")==0) {
            free(yylval.str);
            return TOK_CASE;
        }
        else if (wcscasecmp(yylval.str, L"import")==0) {
            free(yylval.str);
            return TOK_IMPORT;
        }
        else if (wcscasecmp(yylval.str, L"define")==0) {
            free(yylval.str);
            return TOK_DEFINE;
        }
        else {
            lexer_error = unexpected_option_name;
            free(yylval.str);
            return TOK_ERROR;
        }
    }
    else {
        lexer_error = invalid_option_syntax;
        return TOK_ERROR;
    }
}

int read_step(int token)
{
    // Read to the end of the line, taking comments into account
    text_buf buf;
    // Skip the initial whitespace
    wint_t v = read();
    while(v == (wint_t)L' ' 
        || v == (wint_t)L'\t'
        || v == (wint_t)L'\r'
        || v == (wint_t)L'\f'
        || v == (wint_t)L'\v') 
    {
        v = read();    
    }
    // Read all text
    while(true) {
        switch(v) {
            case WEOF:
                yylval.str = buf.take();
                return token;
            case (wint_t)L'\n':
                unread(L'\n');
                yylval.str = buf.take();
                return token;
            case (wint_t)L'#':
                unread((wint_t)L'#');
                yylval.str = buf.take();
                return token;
            default:
                buf.append((wchar_t)v);
                break;
        }
        v = read();
    }
}

int read_comment()
{
    // Read to the end of the line
    text_buf buf;
    buf.append(L'#');
    while(true) {
        wint_t v = read();
        switch(v) {
            case WEOF:
                yylval.str = buf.take();
                return TOK_COMMENT;
            case (wint_t)L'\n':
                unread('\n');
                yylval.str = buf.take();
                return TOK_COMMENT;
            default:
                buf.append((wchar_t)v);
                break;
        }
    }
}

}

int yylex_all()
{
    lexer_error = NULL;

    // Skip all whitespace and comments
    wint_t v = read();
    while (v == (wint_t)L' ' 
        || v == (wint_t)L'\t'
        || v == (wint_t)L'\r'
        || v == (wint_t)L'\f'
        || v == (wint_t)L'\v'
        || v == (wint_t)L'\n') {
        v = read();            
    }
    if (v == WEOF) return TOK_END;

    yylloc.first_column = prev_last_column;
    yylloc.first_line = prev_last_line;

    switch(v) {
        case (wint_t)L'#':
            return read_comment();
        case (wint_t)L'%':
            return read_option();
        case (wint_t)L'{':
            return read_cpp();
        case (wint_t)L'\'':
            return read_pattern();
        case (wint_t)L'"':
            return read_mtext();
        case (wint_t)L'|':
            return read_cell();
        case (wint_t)L':':
            return TOK_COLON;
        case (wint_t)L'=':
            return TOK_EQ;
        case (wint_t)L'<':
            return read_import_name();
        default:
            if (true) {
                unread(v);
                int t = read_identifier();
                if (t == TOK_IDENTIFIER) {
                    if (wcscasecmp(yylval.str, L"Feature")==0) {
                        free(yylval.str);
                        return TOK_FEATURE;
                    }
                    else if (wcscasecmp(yylval.str, L"Background")==0) {
                        free(yylval.str);
                        return TOK_BACKGROUND;
                    }
                    else if (wcscasecmp(yylval.str, L"Scenario")==0) {
                        free(yylval.str);
                        return TOK_SCENARIO;
                    }
                    else if (wcscasecmp(yylval.str, L"Outline")==0) {
                        free(yylval.str);
                        return TOK_OUTLINE;
                    }
                    else if (wcscasecmp(yylval.str, L"Examples")==0) {
                        free(yylval.str);
                        return TOK_EXAMPLES;
                    }
                    else if (wcscasecmp(yylval.str, L"ECMAScript")==0) {
                        free(yylval.str);
                        return TOK_ECMASCRIPT;
                    }
                    else if (wcscasecmp(yylval.str, L"basic")==0) {
                        free(yylval.str);
                        return TOK_BASIC;
                    }
                    else if (wcscasecmp(yylval.str, L"extended")==0) {
                        free(yylval.str);
                        return TOK_EXTENDED;
                    }
                    else if (wcscasecmp(yylval.str, L"awk")==0) {
                        free(yylval.str);
                        return TOK_AWK;
                    }
                    else if (wcscasecmp(yylval.str, L"grep")==0) {
                        free(yylval.str);
                        return TOK_GREP;
                    }
                    else if (wcscasecmp(yylval.str, L"egrep")==0) {
                        free(yylval.str);
                        return TOK_EGREP;
                    }
                    else if (wcscasecmp(yylval.str, L"sensitive")==0) {
                        free(yylval.str);
                        return TOK_SENSITIVE;
                    }
                    else if (wcscasecmp(yylval.str, L"insensitive")==0) {
                        free(yylval.str);
                        return TOK_INSENSITIVE;
                    }
                    else if (wcscasecmp(yylval.str, L"given")==0) {
                        last_step = TOK_GIVEN;
                        // We're reading a step
                        free(yylval.str);
                        return read_step(TOK_GIVEN);
                    }
                    else if (wcscasecmp(yylval.str, L"when")==0) {
                        last_step = TOK_WHEN;
                        // We're reading a step
                        free(yylval.str);
                        return read_step(TOK_WHEN);
                    }
                    else if (wcscasecmp(yylval.str, L"then")==0) {
                        last_step = TOK_THEN;
                        // We're reading a step
                        free(yylval.str);
                        return read_step(TOK_THEN);
                    }
                    else if (wcscasecmp(yylval.str, L"and")==0) {
                        free(yylval.str);
                        return read_step(last_step);
                    }
                    else {
                        return TOK_IDENTIFIER;
                    }
                }
                else {
                    return t;
                }
            }
            break;
    }
}

int yylex()
{
    int token = yylex_all();
    while(token == TOK_COMMENT) {
        token = yylex_all();
    }
    return token;
}