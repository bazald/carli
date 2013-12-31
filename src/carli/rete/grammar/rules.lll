%option never-interactive
%option prefix="rete_"
%option noyywrap

%{
#include <cstdio>
#include <cstring>
#include "rete_parser.h"

struct Condition_Base;
struct Rule;

#include "rules.tab.hpp"

#define YY_DECL int rete_lex()

using namespace std;

size_t g_line_number = 1u;
%}

%x READ_STRING
%%
[ \t\r]                                   ;
#.*                                       ;
<INITIAL,READ_STRING>\n                   { ++g_line_number; }
-?[0-9]+\.[0-9]+                          { rete_lval.fval = atof(rete_text); return FLOAT; }
-?[0-9]+                                  { rete_lval.ival = atoi(rete_text); return INT; }
\<[a-zA-Z0-9\-\*]+\>                      { rete_lval.sval = new string(rete_text + 1);
                                            rete_lval.sval->resize(rete_lval.sval->size() - 1);
                                            return VARIABLE;
                                          }
[a-zA-Z0-9]+                              { rete_lval.sval = new string(rete_text); return STRING; }
[a-zA-Z0-9]+[a-zA-Z0-9\-*]*[a-zA-Z0-9]+   { rete_lval.sval = new string(rete_text); return STRING; }
[\^\(\)\{\}+-]                            { return rete_text[0]; }
==                                        { rete_lval.predicate = Rete::Rete_Predicate::EQ; return PREDICATE; }
!=                                        { rete_lval.predicate = Rete::Rete_Predicate::NEQ; return PREDICATE; }
>                                         { rete_lval.predicate = Rete::Rete_Predicate::GT; return PREDICATE; }
>=                                        { rete_lval.predicate = Rete::Rete_Predicate::GTE; return PREDICATE; }
\<                                        { rete_lval.predicate = Rete::Rete_Predicate::LT; return PREDICATE; }
\<=                                       { rete_lval.predicate = Rete::Rete_Predicate::LTE; return PREDICATE; }
\|                                        { BEGIN(READ_STRING); return '|'; }
<READ_STRING>\\\|                         { rete_lval.sval = new string("\\|"); return STRING_PART_S; }
<READ_STRING>\\t                          { rete_lval.sval = new string("\\t"); return STRING_PART_S; }
<READ_STRING>\\r                          { rete_lval.sval = new string("\\r"); return STRING_PART_S; }
<READ_STRING>\\n                          { rete_lval.sval = new string("\\n"); return STRING_PART_S; }
<READ_STRING>\|                           { BEGIN(INITIAL); return '|'; }
<READ_STRING>\r                           ;
<READ_STRING>[^\|\r\n]                    { rete_lval.cval = rete_text[0]; return STRING_PART_C; }
%%
