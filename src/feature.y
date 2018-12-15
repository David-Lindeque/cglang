%{

#include <stdint.h>
#include <vector>
#include "../src/parser.h"
#include "../src/ast.h"

extern int yylex();
extern int yyparse(cglang::parser *);
extern FILE* yyin;
void yyerror(cglang::parser *, const char*);

%}

%parse-param { cglang::parser *parser };

%locations

%initial-action
{
    @$.first_line = 1;
    @$.first_column = 1;
    @$.last_line = 1;
    @$.last_column = 1;
}

%union
{
    wchar_t        *str;
    cglang::step   *step;
    cglang::steps  *steps;
    cglang::cells  *cells;
    cglang::rows   *rows;
}

%destructor { if ($$) free($$); }   <str>
%destructor { if ($$) delete $$; }  <steps>
%destructor { if ($$) delete $$; }  <step>
%destructor { if ($$) delete $$; }  <rows>
%destructor { if ($$) delete $$; }  <cells>

%token              TOK_END          0      "end-of-file"
%token              TOK_FEATURE             "Feature"
%token              TOK_FILE_SKELETON       "%file"
%token              TOK_TEST_SKELETON       "%test"
%token              TOK_DEFINE              "%define"
%token              TOK_BACKGROUND          "Background"
%token              TOK_SCENARIO            "Scenario"
%token              TOK_OUTLINE             "Outline"
%token              TOK_EXAMPLES            "Examples"
%token              TOK_GRAMMER             "%grammar"
%token              TOK_CASE                "%case"
%token              TOK_IMPORT              "%import"
%token              TOK_ECMASCRIPT          "ECMAScript"
%token              TOK_BASIC               "basic"
%token              TOK_EXTENDED            "extended"
%token              TOK_AWK                 "awk"
%token              TOK_GREP                "grep"
%token              TOK_EGREP               "egrep"
%token              TOK_SENSITIVE           "sensitive"
%token              TOK_INSENSITIVE         "insensitive"
%token <str>        TOK_IDENTIFIER
%token <str>        TOK_CPP
%token <str>        TOK_PATTERN
%token <str>        TOK_GIVEN
%token <str>        TOK_WHEN
%token <str>        TOK_THEN
%token <str>        TOK_MTEXT
%token <str>        TOK_CELL
%token <str>        TOK_COMMENT
%token <str>        TOK_IMPORT_NAME
%token              TOK_COLON               ":"
%token              TOK_EQ                  "="
%token              TOK_ROW_END
%token              TOK_ERROR
%token              TOK_OUT_OF_MEMORY

%type <steps>           steps
%type <step>            step
%type <step>            basic_step
%type <rows>            table
%type <cells>           row
%type <cells>           cells

%start document

%%

document:
    declarations;

declarations: /* empty */
    | declarations declaration;

declaration:
      feature_declaration
    | grammar_declaration
    | case_declaration
    | import_declaration
    | file_skeleton_declaration
    | test_skeleton_declaration
    | step_skeleton_declaration
    | define_skeleton_declaration
    | background_declaration
    | scenario_outline_declaration
    | examples_declaration
    | scenario_declaration;

feature_declaration: "Feature" ":" TOK_IDENTIFIER               { parser->process_feature_declaration($3, reinterpret_cast<const cglang::location&>(@3)); free($3); };

grammar_declaration: 
      "%grammar" "=" "ECMAScript"                               { parser->set_grammar(std::regex::ECMAScript, reinterpret_cast<const cglang::location&>(@3)); }
    | "%grammar" "=" "basic"                                    { parser->set_grammar(std::regex::basic, reinterpret_cast<const cglang::location&>(@3)); }
    | "%grammar" "=" "extended"                                 { parser->set_grammar(std::regex::extended, reinterpret_cast<const cglang::location&>(@3)); }
    | "%grammar" "=" "awk"                                      { parser->set_grammar(std::regex::awk, reinterpret_cast<const cglang::location&>(@3)); }
    | "%grammar" "=" "grep"                                     { parser->set_grammar(std::regex::grep, reinterpret_cast<const cglang::location&>(@3)); }
    | "%grammar" "=" "egrep"                                    { parser->set_grammar(std::regex::egrep, reinterpret_cast<const cglang::location&>(@3)); };
case_declaration:
      "%case" "=" "sensitive"                                   { parser->set_case_sensitive(true, reinterpret_cast<const cglang::location&>(@3)); }
    | "%case" "=" "insensitive"                                 { parser->set_case_sensitive(false, reinterpret_cast<const cglang::location&>(@3)); };
import_declaration:
      "%import" TOK_IMPORT_NAME                                 { parser->process_import($2, reinterpret_cast<const cglang::location&>(@2)); free($2); };
file_skeleton_declaration: "%file" TOK_CPP                      { parser->process_file_skeleton_declaration($2, reinterpret_cast<const cglang::location&>(@2)); free($2); };
test_skeleton_declaration: "%test" TOK_CPP                      { parser->process_test_skeleton_declaration($2, reinterpret_cast<const cglang::location&>(@2)); free($2); };
step_skeleton_declaration: TOK_PATTERN TOK_CPP                  { parser->process_step_skeleton_declaration($1, reinterpret_cast<const cglang::location&>(@1), $2, reinterpret_cast<const cglang::location&>(@2)); free($1); free($2); };
define_skeleton_declaration: "%define" TOK_IDENTIFIER TOK_CPP   { parser->process_define_skeleton_declaration($2, reinterpret_cast<const cglang::location&>(@2), $3, reinterpret_cast<const cglang::location&>(@3)); free($2); free($3); };
background_declaration: "Background" ":" steps                  { parser->process_background_declaration(cglang::psteps($3), reinterpret_cast<const cglang::location&>(@3)); };
scenario_outline_declaration: "Scenario" "Outline" ":" TOK_IDENTIFIER steps     { parser->process_scenario_outline_declaration($4, reinterpret_cast<const cglang::location&>(@4), cglang::psteps($5), reinterpret_cast<const cglang::location&>(@5)); free($4); };
examples_declaration: "Examples" ":" table                      { parser->process_examples_declaration(cglang::table($3), reinterpret_cast<const cglang::location&>(@3)); };
scenario_declaration: "Scenario" ":" TOK_IDENTIFIER steps       { parser->process_scenario_declaration($3, reinterpret_cast<const cglang::location&>(@3), cglang::psteps($4), reinterpret_cast<const cglang::location&>(@4)); free($3); };

steps:
      step                                                      { $$ = new cglang::steps(); $$->reserve(4); $$->emplace_back($1); }
    | steps step                                                { $$ = $1; $$->emplace_back($2); };

step:
      basic_step                                                { $$ = $1; }
    | basic_step table                                          { $$ = $1; parser->set_step_table($$, cglang::table($2), reinterpret_cast<const cglang::location&>(@2)); }
    | basic_step TOK_MTEXT                                      { $$ = $1; parser->set_step_mtext($$, $2, reinterpret_cast<const cglang::location&>(@2)); free($2); };

basic_step:
      TOK_GIVEN                                                 { $$ = new cglang::step(cglang::step_kind::given, $1, reinterpret_cast<const cglang::location&>(@1)); free($1); }
    | TOK_WHEN                                                  { $$ = new cglang::step(cglang::step_kind::when, $1, reinterpret_cast<const cglang::location&>(@1)); free($1); }
    | TOK_THEN                                                  { $$ = new cglang::step(cglang::step_kind::then, $1, reinterpret_cast<const cglang::location&>(@1)); free($1); };

table:
      row                                                       { $$ = new cglang::rows(); $$->reserve(4); $$->emplace_back($1); }
    | table row                                                 { $$ = $1; $$->emplace_back($2); };

row: cells TOK_ROW_END                                          { $$ = $1; };

cells:
      TOK_CELL                                                  { $$ = new cglang::cells(); $$->reserve(4); $$->emplace_back(new cglang::cell($1, reinterpret_cast<const cglang::location&>(@1))); free($1); }
    | cells TOK_CELL                                            { $$ = $1; $$->emplace_back(new cglang::cell($2, reinterpret_cast<const cglang::location&>(@2))); free($2); };

%%
extern const char *lexer_error;
void yyerror(cglang::parser *parser, const char *msg)
{
    cglang::location loc;
    loc.first_line = yylloc.first_line;
    loc.first_column = yylloc.first_column;
    loc.last_line = yylloc.last_line;
    loc.last_column = yylloc.last_column;
    parser->process_error(loc, msg, lexer_error);
}

void cglang::parser::parse(cglang::parser *sink, FILE *source)
{
    yyin = source;
    yyparse(sink);
}
