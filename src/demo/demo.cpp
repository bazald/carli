#include "carli/rete/rete_agent.h"
#include "carli/rete/grammar/rete_parser.h"

#include <array>
#include <sstream>

int main(int /*argc*/, char ** /*argv*/) {
  Rete::Rete_Agent ragent;
  std::string line;
  int line_number = 0;

  while(getline(std::cin, line))
    rete_parse_string(ragent, line, ++line_number);

  std::cout << "Now exiting." << std::endl;

  return 0;
}
