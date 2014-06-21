#ifndef RETE_PARSER_H
#define RETE_PARSER_H

#include "../rete_agent.h"

RETE_LINKAGE int rete_parse_file(Rete::Rete_Agent &agent, const std::string &filename);
RETE_LINKAGE int rete_parse_string(Rete::Rete_Agent &agent, const std::string &str, const int &line_number = 1);

#endif
