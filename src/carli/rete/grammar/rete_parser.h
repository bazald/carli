#ifndef RETE_PARSER_H
#define RETE_PARSER_H

#include "../rete_agent.h"

RETE_LINKAGE bool rete_get_exit();
RETE_LINKAGE int rete_parse_file(Rete::Rete_Agent &agent, const std::string &filename, const std::string &source_path = "");
RETE_LINKAGE int rete_parse_string(Rete::Rete_Agent &agent, const std::string &str, int &line_number);
RETE_LINKAGE void rete_set_exit();

inline int rete_parse_string(Rete::Rete_Agent &agent, const std::string &str) {
  int line_number = 1;
  return rete_parse_string(agent, str, line_number);
}

#endif
