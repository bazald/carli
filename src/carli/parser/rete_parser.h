#ifndef RETE_PARSER_H
#define RETE_PARSER_H

#include "../agent.h"

namespace Rete {

  PARSER_LINKAGE bool rete_get_exit();
  PARSER_LINKAGE int rete_parse_file(Carli::Agent &agent, const std::string &filename, const std::string &source_path = "");
  PARSER_LINKAGE int rete_parse_string(Carli::Agent &agent, const std::string &str, int &line_number);
  PARSER_LINKAGE void rete_set_exit();

  typedef std::tuple<std::shared_ptr<int64_t>, std::shared_ptr<std::tuple<int64_t, std::string, std::string, Carli::Feature *>>> Parser_Flag;
  typedef std::pair<Rete_Node_Ptr, Variable_Indices> Parser_Rete_Node;
  typedef std::tuple<Parser_Rete_Node, std::string, Parser_Flag, Carli::Q_Value::Token> Parser_Rule;

  inline int rete_parse_string(Carli::Agent &agent, const std::string &str) {
    int line_number = 1;
    return rete_parse_string(agent, str, line_number);
  }

}

#endif
