#ifndef RETE_PARSER_H
#define RETE_PARSER_H

#include "../agent.h"

PARSER_LINKAGE bool rete_get_exit();
PARSER_LINKAGE int rete_parse_file(Carli::Agent &agent, const std::string &filename, const std::string &source_path = "");
PARSER_LINKAGE int rete_parse_string(Carli::Agent &agent, const std::string &str, int &line_number);
PARSER_LINKAGE void rete_set_exit();

struct PARSER_LINKAGE Variable {
  Variable(const std::string &s0, const std::string &s1, const std::string &s2)
   : name_index({{std::make_pair(s0, Rete::WME_Token_Index(0u, 0u, 0)),
                  std::make_pair(s1, Rete::WME_Token_Index(0u, 0u, 1)),
                  std::make_pair(s2, Rete::WME_Token_Index(0u, 0u, 2))}})
  {
  }

  std::array<std::pair<std::string, Rete::WME_Token_Index>, 3> name_index;
};

inline int rete_parse_string(Carli::Agent &agent, const std::string &str) {
  int line_number = 1;
  return rete_parse_string(agent, str, line_number);
}

#endif
