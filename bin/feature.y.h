/* A Bison parser, made by GNU Bison 3.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_BIN_FEATURE_Y_H_INCLUDED
# define YY_YY_BIN_FEATURE_Y_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    TOK_END = 0,
    TOK_FEATURE = 258,
    TOK_FILE_SKELETON = 259,
    TOK_TEST_SKELETON = 260,
    TOK_DEFINE = 261,
    TOK_BACKGROUND = 262,
    TOK_SCENARIO = 263,
    TOK_OUTLINE = 264,
    TOK_EXAMPLES = 265,
    TOK_GRAMMER = 266,
    TOK_CASE = 267,
    TOK_IMPORT = 268,
    TOK_ECMASCRIPT = 269,
    TOK_BASIC = 270,
    TOK_EXTENDED = 271,
    TOK_AWK = 272,
    TOK_GREP = 273,
    TOK_EGREP = 274,
    TOK_SENSITIVE = 275,
    TOK_INSENSITIVE = 276,
    TOK_IDENTIFIER = 277,
    TOK_CPP = 278,
    TOK_PATTERN = 279,
    TOK_GIVEN = 280,
    TOK_WHEN = 281,
    TOK_THEN = 282,
    TOK_MTEXT = 283,
    TOK_CELL = 284,
    TOK_COMMENT = 285,
    TOK_IMPORT_NAME = 286,
    TOK_COLON = 287,
    TOK_EQ = 288,
    TOK_ROW_END = 289,
    TOK_ERROR = 290,
    TOK_OUT_OF_MEMORY = 291
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 28 "src/feature.y" /* yacc.c:1912  */

    wchar_t        *str;
    cglang::step   *step;
    cglang::steps  *steps;
    cglang::cells  *cells;
    cglang::rows   *rows;

#line 103 "bin/feature.y.h" /* yacc.c:1912  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE yylval;
extern YYLTYPE yylloc;
int yyparse (cglang::parser *parser);

#endif /* !YY_YY_BIN_FEATURE_Y_H_INCLUDED  */
