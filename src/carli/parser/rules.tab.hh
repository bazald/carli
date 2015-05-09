/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

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

#ifndef YY_RETE_RULES_TAB_HH_INCLUDED
# define YY_RETE_RULES_TAB_HH_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int retedebug;
#endif
/* "%code requires" blocks.  */
#line 14 "rules.yyy" /* yacc.c:1909  */

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void *yyscan_t;
#endif

#line 51 "rules.tab.hh" /* yacc.c:1909  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    ARROW_RIGHT = 258,
    ASSIGNMENT = 259,
    COMMAND_EXCISE = 260,
    COMMAND_EXCISE_ALL = 261,
    COMMAND_EXIT = 262,
    COMMAND_INSERT_WME = 263,
    COMMAND_REMOVE_WME = 264,
    COMMAND_SET_TOTAL_STEP_COUNT = 265,
    COMMAND_SOURCE = 266,
    COMMAND_SP = 267,
    EXISTENTIAL_MATCH = 268,
    FLAG_CREATION_TIME = 269,
    FLAG_FEATURE = 270,
    FLOAT = 271,
    INT = 272,
    NODE_TYPE = 273,
    STRING = 274,
    STRING_PART_C = 275,
    STRING_PART_S = 276,
    VARIABLE = 277,
    PREDICATE = 278
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 118 "rules.yyy" /* yacc.c:1909  */

  char cval;
  const char *csval;
  std::tuple<std::shared_ptr<int64_t>, std::shared_ptr<std::tuple<int64_t, std::string, std::string, Carli::Feature *>>> *flag_ptr;
  double fval;
  int64_t ival;
  std::list<std::string> *slist;
  std::string *sval;
  std::pair<Rete::Rete_Node_Ptr, std::vector<Variable>> *rete_node_ptr;
  std::tuple<std::pair<Rete::Rete_Node_Ptr, std::vector<Variable>>, std::string, std::tuple<std::shared_ptr<int64_t>, std::shared_ptr<std::tuple<int64_t, std::string, std::string, Carli::Feature *>>>, double> *rule_ptr;
  Rete::Symbol_Ptr_C *symbol_ptr;
  Rete::Rete_Predicate::Predicate predicate;

#line 101 "rules.tab.hh" /* yacc.c:1909  */
};
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



int reteparse (yyscan_t yyscanner, Carli::Agent &agent, const std::string &filename, const std::string &source_path);

#endif /* !YY_RETE_RULES_TAB_HH_INCLUDED  */
