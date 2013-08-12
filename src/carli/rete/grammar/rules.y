%error-verbose
%locations
%name-prefix "rete_"

%{
#include <cstdio>
#include "rete_parser.h"

#define YYDEBUG 1

using namespace std;

int rete_lex();
int rete_parse();
extern FILE *rete_in;

static array<string, 3> Variable(const string &s0, const string &s1, const string &s2) {
  array<string, 3> variable;
  variable[0] = s0;
  variable[1] = s1;
  variable[2] = s2;
  return variable;
}
static vector<array<string, 3>> Variables() {
  vector<array<string, 3>> rv;
  return rv;
}
static vector<array<string, 3>> Variables(const array<string, 3> &variable) {
  vector<array<string, 3>> rv;
  rv.push_back(variable);
  return rv;
}
static pair<Rete::Rete_Node_Ptr, vector<array<string, 3>>> *
Rete_Node_Ptr_and_Variables(const Rete::Rete_Node_Ptr &rete_node, const vector<array<string, 3>> &variables) {
  return new pair<Rete::Rete_Node_Ptr, vector<array<string, 3>>>(rete_node, variables);
}

static Rete::WME_Token_Index find_index(const std::vector<std::array<std::string, 3>> &variables, const std::string &variable) {
  for(size_t i = 0; i != variables.size(); ++i) {
    for(uint8_t ii = 0; ii != 3; ++ii) {
      if(variables[i][ii] == variable)
        return Rete::WME_Token_Index(i, ii);
    }
  }
  return Rete::WME_Token_Index(size_t(-1), uint8_t(-1));
}

static void rete_error(Rete::Agent &agent, const char *s);

%}

%union {
  char cval;
  double fval;
  int64_t ival;
  std::string *sval;
  std::pair<Rete::Rete_Node_Ptr, std::vector<std::array<std::string, 3>>> *rete_node_ptr;
  Rete::Symbol_Ptr_C *symbol_ptr;
  Rete::Rete_Predicate::Predicate predicate;
}

%token <fval> FLOAT
%token <ival> INT
%token <sval> STRING
%token <cval> STRING_PART_C
%token <sval> STRING_PART_S
%token <sval> VARIABLE
%token <predicate> PREDICATE

%type <rete_node_ptr> final_conditions conditions condition_group condition_type condition
%type <symbol_ptr> symbol_constant
%type <sval> string_or_literal literal literal_parts

%parse-param {Rete::Agent &agent}

%%

rules:
  rules rule
  | rule
  ;
rule:
  STRING '{' final_conditions '}' { string name = *$1;
                                    auto node = agent.make_action_retraction([name](const Rete::Rete_Action &, const Rete::WME_Token &wme_vector) {
                                                                               cout << wme_vector << "->" << name << endl;
                                                                             }, [name](const Rete::Rete_Action &, const Rete::WME_Token &wme_vector) {
                                                                                  cout << wme_vector << "<-" << name << endl;
                                                                                }, $3->first);
                                    agent.source_rule(name, node);
                                    delete $1;
                                    delete $3; }
  ;
final_conditions:
  conditions { $$ = $1; }
  | '+' conditions { $$ = Rete_Node_Ptr_and_Variables(Rete::Rete_Node_Ptr(agent.make_existential($2->first)), Variables()); delete $2; }
  | '-' conditions { $$ = Rete_Node_Ptr_and_Variables(Rete::Rete_Node_Ptr(agent.make_negation($2->first)), Variables()); delete $2; }
  | final_conditions '+' condition_type { Rete::WME_Bindings bindings;
                                          unordered_set<string> joined;
                                          for(size_t i = 0; i != $1->second.size(); ++i) {
                                            for(uint8_t ii = 0; ii != 3; ++ii) {
                                              if(!$1->second[i][ii].empty() && joined.find($1->second[i][ii]) == joined.end()) {
                                                for(size_t j = 0; j != $3->second.size(); ++j) {
                                                  for(uint8_t jj = 0; jj != 3; ++jj) {
                                                    if($1->second[i][ii] == $3->second[j][jj]) {
                                                      bindings.insert(Rete::WME_Binding(Rete::WME_Token_Index(i, ii), Rete::WME_Token_Index(j, jj)));
                                                      joined.insert($1->second[i][ii]);
                                                      goto DONE_EXISTENTIAL_JOINING;
                                                    }
                                                  }
                                                }
                                              }
                                              DONE_EXISTENTIAL_JOINING:
                                                ;
                                            }
                                          }
                                          $$ = Rete_Node_Ptr_and_Variables(Rete::Rete_Node_Ptr(agent.make_existential_join(bindings, $1->first, $3->first)), $1->second);
                                          delete $1;
                                          delete $3; }
  | final_conditions '-' condition_type { Rete::WME_Bindings bindings;
                                          unordered_set<string> joined;
                                          for(size_t i = 0; i != $1->second.size(); ++i) {
                                            for(uint8_t ii = 0; ii != 3; ++ii) {
                                              if(!$1->second[i][ii].empty() && joined.find($1->second[i][ii]) == joined.end()) {
                                                for(size_t j = 0; j != $3->second.size(); ++j) {
                                                  for(uint8_t jj = 0; jj != 3; ++jj) {
                                                    if($1->second[i][ii] == $3->second[j][jj]) {
                                                      bindings.insert(Rete::WME_Binding(Rete::WME_Token_Index(i, ii), Rete::WME_Token_Index(j, jj)));
                                                      joined.insert($1->second[i][ii]);
                                                      goto DONE_NEGATION_JOINING;
                                                    }
                                                  }
                                                }
                                              }
                                              DONE_NEGATION_JOINING:
                                                ;
                                            }
                                          }
                                          $$ = Rete_Node_Ptr_and_Variables(Rete::Rete_Node_Ptr(agent.make_negation_join(bindings, $1->first, $3->first)), $1->second);
                                          delete $1;
                                          delete $3; }
conditions:
  conditions condition_type { Rete::WME_Bindings bindings;
                              unordered_set<string> joined;
                              for(size_t i = 0; i != $1->second.size(); ++i) {
                                for(uint8_t ii = 0; ii != 3; ++ii) {
                                  if(!$1->second[i][ii].empty() && joined.find($1->second[i][ii]) == joined.end()) {
                                    for(size_t j = 0; j != $2->second.size(); ++j) {
                                      for(uint8_t jj = 0; jj != 3; ++jj) {
                                        if($1->second[i][ii] == $2->second[j][jj]) {
                                          bindings.insert(Rete::WME_Binding(Rete::WME_Token_Index(i, ii), Rete::WME_Token_Index(j, jj)));
                                          joined.insert($1->second[i][ii]);
                                          goto DONE_JOINING;
                                        }
                                      }
                                    }
                                  }
                                  DONE_JOINING:
                                    ;
                                }
                              }
                              vector<array<string, 3>> variables($1->second);
                              variables.insert(variables.end(), $2->second.begin(), $2->second.end());
                              $$ = Rete_Node_Ptr_and_Variables(Rete::Rete_Node_Ptr(agent.make_join(bindings, $1->first, $2->first)), variables);
                              delete $1;
                              delete $2; }
  | conditions '(' VARIABLE PREDICATE symbol_constant ')' { auto lhs_index = find_index($1->second, *$3);
                                                            if(lhs_index.second > 2)
                                                              rete_error(agent, "Unbound variable tested by predicate.");
                                                            $$ = Rete_Node_Ptr_and_Variables(Rete::Rete_Node_Ptr(agent.make_predicate_vc($4, lhs_index, *$5, $1->first)), $1->second);
                                                            delete $1;
                                                            delete $3;
                                                            delete $5; }
  | conditions '(' VARIABLE PREDICATE VARIABLE ')' { auto lhs_index = find_index($1->second, *$3);
                                                     auto rhs_index = find_index($1->second, *$5);
                                                     if(lhs_index.second > 2 || rhs_index.second > 2)
                                                       rete_error(agent, "Unbound variable tested by predicate.");
                                                     $$ = Rete_Node_Ptr_and_Variables(Rete::Rete_Node_Ptr(agent.make_predicate_vv($4, lhs_index, rhs_index, $1->first)), $1->second);
                                                     delete $1;
                                                     delete $3;
                                                     delete $5; }
  | condition_type { $$ = $1; }
  ;
condition_group:
  '{' final_conditions '}' { $$ = $2; }
  ;
condition_type:
  condition { $$ = $1; }
  | condition_group { $$ = $1; }
condition:
  '(' VARIABLE '^' symbol_constant symbol_constant ')' { $$ = Rete_Node_Ptr_and_Variables(Rete::Rete_Node_Ptr(agent.make_filter(Rete::WME(make_shared<Rete::Symbol_Variable>(Rete::Symbol_Variable::First), *$4, *$5))), Variables(Variable(*$2, "", ""))); delete $2; delete $4; delete $5; }
  | '(' VARIABLE '^' symbol_constant VARIABLE ')'  { $$ = Rete_Node_Ptr_and_Variables(Rete::Rete_Node_Ptr(agent.make_filter(Rete::WME(make_shared<Rete::Symbol_Variable>(Rete::Symbol_Variable::First), *$4, make_shared<Rete::Symbol_Variable>(*$2 == *$5 ? Rete::Symbol_Variable::First : Rete::Symbol_Variable::Third)))), Variables(Variable(*$2, "", *$5))); delete $2; delete $4; delete $5; }
  | '(' VARIABLE '^' VARIABLE symbol_constant ')' { $$ = Rete_Node_Ptr_and_Variables(Rete::Rete_Node_Ptr(agent.make_filter(Rete::WME(make_shared<Rete::Symbol_Variable>(Rete::Symbol_Variable::First), make_shared<Rete::Symbol_Variable>(*$2 == *$4 ? Rete::Symbol_Variable::First : Rete::Symbol_Variable::Second), *$5))), Variables(Variable(*$2, *$4, ""))); delete $2; delete $4; delete $5; }
  | '(' VARIABLE '^' VARIABLE VARIABLE ')'  { $$ = Rete_Node_Ptr_and_Variables(Rete::Rete_Node_Ptr(agent.make_filter(Rete::WME(make_shared<Rete::Symbol_Variable>(Rete::Symbol_Variable::First), make_shared<Rete::Symbol_Variable>(*$2 == *$4 ? Rete::Symbol_Variable::First : Rete::Symbol_Variable::Second), make_shared<Rete::Symbol_Variable>(*$2 == *$5 ? Rete::Symbol_Variable::First : *$4 == *$5 ? Rete::Symbol_Variable::Second : Rete::Symbol_Variable::Third)))), Variables(Variable(*$2, *$4, *$5))); delete $2; delete $4; delete $5; }
  ;
symbol_constant:
  FLOAT { $$ = new Rete::Symbol_Ptr_C(make_shared<Rete::Symbol_Constant_Float>($1)); }
  | INT { $$ = new Rete::Symbol_Ptr_C(make_shared<Rete::Symbol_Constant_Int>($1)); }
  | string_or_literal { $$ = new Rete::Symbol_Ptr_C(make_shared<Rete::Symbol_Constant_String>(*$1)); delete $1; }
string_or_literal:
  STRING { $$ = $1; }
  | literal { $$ = $1; }
  ;
literal:
  '|' literal_parts '|' { $$ = $2; }
  ;
literal_parts:
  literal_parts STRING_PART_C { $$ = $1; *$$ += $2; }
  | literal_parts STRING_PART_S { $$ = $1; *$$ += *$2; delete $2; }
  | STRING_PART_C { $$ = new string; *$$ += $1; }
  | STRING_PART_S { $$ = $1; }
  ;

%%

extern size_t g_line_number;

static void rete_error(Rete::Agent &, const char *s) {
  cout << "Parse error, line " << g_line_number << ": " << s << endl;
  exit(-1);
}

void rete_parse_file(Rete::Agent &agent, const string &filename) {
  FILE * file = fopen(filename.c_str(), "r");
  if(!file)
    abort();

  rete_in = file;

  // rete_lex();
  do {
    if(rete_parse(agent))
      abort();
  } while (!feof(rete_in));

  rete_in = stdin;
  fclose(file);
}
