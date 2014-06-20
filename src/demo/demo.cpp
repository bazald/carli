#include "carli/rete/rete_agent.h"
#include "carli/rete/grammar/rete_parser.h"

#include <array>
#include <sstream>

static void execute(Rete::Rete_Agent &ragent, const std::string &line);
static void source(Rete::Rete_Agent &ragent, const std::vector<std::string> &args);

int main(int /*argc*/, char ** /*argv*/) {
  Rete::Rete_Agent ragent;
  std::string line;

  while(getline(std::cin, line))
    execute(ragent, line);

  std::cout << "Now exiting." << std::endl;

  return 0;
}

void execute(Rete::Rete_Agent &ragent, const std::string &line) {
  std::string command;
  std::vector<std::string> args;
  
  size_t nonspace = line.find_first_not_of(" \t\r\n");

  if(nonspace == std::string::npos) {
    std::cerr << "Invalid command line: " << line << std::endl;
    return;
  }
  else {
    size_t space = line.find_first_of(" \t\r\n", nonspace);
    command = line.substr(nonspace, space - nonspace);

    nonspace = line.find_first_not_of(" \t\r\n", space);
    while(nonspace != std::string::npos) {
      space = line.find_first_of(" \t\r\n", nonspace);
      args.push_back(line.substr(nonspace, space - nonspace));
      nonspace = line.find_first_not_of(" \t\r\n", space);
    };
  }
  
  if(command == "source")
    source(ragent, args);
  else
    rete_parse_string(ragent, line);
}

void source(Rete::Rete_Agent &ragent, const std::vector<std::string> &args) {
  if(args.empty())
    std::cerr << "source usage: source filename1 [filename2 [...]]" << std::endl;
  else {
    for(const auto &arg : args) {
      if(rete_parse_file(ragent, arg)) {
        std::cerr << "Failed to source: " << arg << std::endl;
        return;
      }
    }
  }
}
