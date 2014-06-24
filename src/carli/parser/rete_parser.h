#ifndef RETE_PARSER_H
#define RETE_PARSER_H

#include "../agent.h"

PARSER_LINKAGE bool rete_get_exit();
PARSER_LINKAGE int rete_parse_file(Carli::Agent &agent, const std::string &filename, const std::string &source_path = "");
PARSER_LINKAGE int rete_parse_string(Carli::Agent &agent, const std::string &str, int &line_number);
PARSER_LINKAGE void rete_set_exit();

inline int rete_parse_string(Carli::Agent &agent, const std::string &str) {
  int line_number = 1;
  return rete_parse_string(agent, str, line_number);
}

#endif
