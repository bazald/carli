/* A Bison parser, made by GNU Bison 2.7.12-4996.  */

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
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int retedebug;
#endif
/* "%code requires" blocks.  */
/* Line 2053 of yacc.c  */
#line 15 "rules.yyy"

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void *yyscan_t;
#endif
typedef std::unordered_map<Rete::Rete_Node_Ptr, std::pair<Carli::Node_Unsplit_Ptr, Carli::Fringe_Values>> Grandparent_to_UFV;


/* Line 2053 of yacc.c  */
#line 55 "rules.tab.hh"

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     ARROW_RIGHT = 258,
     ASSIGNMENT = 259,
     COMMAND_EXCISE = 260,
     COMMAND_EXCISE_ALL = 261,
     COMMAND_EXIT = 262,
     COMMAND_INSERT_WME = 263,
     COMMAND_REMOVE_WME = 264,
     COMMAND_SOURCE = 265,
     COMMAND_SP = 266,
     FLAG_FEATURE = 267,
     FLOAT = 268,
     INT = 269,
     NODE_TYPE = 270,
     STRING = 271,
     STRING_PART_C = 272,
     STRING_PART_S = 273,
     VARIABLE = 274,
     PREDICATE = 275
   };
#endif


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{
/* Line 2053 of yacc.c  */
#line 121 "rules.yyy"

  char cval;
  const char *csval;
  std::tuple<int64_t, std::string, Carli::Feature *> *flag_ptr;
  double fval;
  int64_t ival;
  std::list<std::string> *slist;
  std::string *sval;
  std::pair<Rete::Rete_Node_Ptr, std::vector<std::array<std::string, 3>>> *rete_node_ptr;
  std::tuple<std::pair<Rete::Rete_Node_Ptr, std::vector<std::array<std::string, 3>>>, std::string, std::tuple<int64_t, std::string, Carli::Feature *>, std::list<std::string>, double> *rule_ptr;
  Rete::Symbol_Ptr_C *symbol_ptr;
  Rete::Rete_Predicate::Predicate predicate;


/* Line 2053 of yacc.c  */
#line 105 "rules.tab.hh"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int reteparse (void *YYPARSE_PARAM);
#else
int reteparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int reteparse (yyscan_t yyscanner, Carli::Agent &agent, const std::string &filename, const std::string &source_path, Grandparent_to_UFV &gufv);
#else
int reteparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_RETE_RULES_TAB_HH_INCLUDED  */
