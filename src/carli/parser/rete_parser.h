#ifndef RETE_PARSER_H
#define RETE_PARSER_H

#include "../agent.h"

namespace Rete {

  PARSER_LINKAGE bool rete_get_exit();
  PARSER_LINKAGE int rete_parse_file(Carli::Agent &agent, const std::string &filename, const std::string &source_path = "");
  PARSER_LINKAGE int rete_parse_string(Carli::Agent &agent, const std::string &str, int &line_number);
  PARSER_LINKAGE void rete_set_exit();

  struct PARSER_LINKAGE Parser_Variable {
    Parser_Variable(const std::string &s0, const std::string &s1, const std::string &s2)
     : name_index({{std::make_pair(s0, Rete::WME_Token_Index(0u, 0u, 0)),
                    std::make_pair(s1, Rete::WME_Token_Index(0u, 0u, 1)),
                    std::make_pair(s2, Rete::WME_Token_Index(0u, 0u, 2))}})
    {
    }

    std::array<std::pair<std::string, Rete::WME_Token_Index>, 3> name_index;
  };

  typedef std::tuple<std::shared_ptr<int64_t>, std::shared_ptr<std::tuple<int64_t, std::string, std::string, Carli::Feature *>>> Parser_Flag;
  typedef std::pair<Rete::Rete_Node_Ptr, std::vector<Rete::Parser_Variable>> Parser_Rete_Node;
  typedef std::tuple<Rete::Parser_Rete_Node, std::string, Rete::Parser_Flag, Carli::Q_Value::Token> Parser_Rule;

  inline int rete_parse_string(Carli::Agent &agent, const std::string &str) {
    int line_number = 1;
    return rete_parse_string(agent, str, line_number);
  }

}

#endif
