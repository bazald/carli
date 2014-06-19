#include "carli/rete/rete_agent.h"
#include "carli/rete/grammar/rete_parser.h"

#include <array>
#include <sstream>

static void execute(Rete::Rete_Agent &ragent, const std::string &line);

static void exit(const std::vector<std::string> &args);
static void source(Rete::Rete_Agent &ragent, const std::vector<std::string> &args);
static void wme_toggle(Rete::Rete_Agent &ragent, const std::vector<std::string> &args, const bool &insert);

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
  
  if(command == "exit")
    exit(args);
  else if(command == "source")
    source(ragent, args);
  else if(command == "wme-add")
    wme_toggle(ragent, args, true);
  else if(command == "wme-remove")
    wme_toggle(ragent, args, false);
  else
    std::cerr << "Unknown command: " << command << std::endl;
}

void exit(const std::vector<std::string> &args) {
  if(args.empty())
    exit(0);
  else {
    std::istringstream iss(args[0]);
    int i;
    iss >> i;
    if(iss)
      exit(i);
    else
      exit(-1);
  }
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

void wme_toggle(Rete::Rete_Agent &ragent, const std::vector<std::string> &args, const bool &insert) {
  if(args.empty())
    std::cerr << "wme-add usage: wme-" << (insert ? "add" : "remove") << " (sym1,sym2,sym3) [...]" << std::endl;
  else {
    for(const auto &arg : args) {
      const size_t paren_open = arg.find('(');
      const size_t comma1 = arg.find(',', paren_open + 1);
      const size_t comma2 = arg.find(',', comma1 + 1);
      const size_t paren_close = arg.find(')', comma2 + 1);
      if(paren_open > comma1 || comma1 > comma2 || comma2 > paren_close || paren_close == std::string::npos) {
        std::cerr << "Invalid WME: " << arg << std::endl;
        return;
      }
      
      std::vector<std::string> syms = {{arg.substr(paren_open + 1, comma1      - paren_open  - 1),
                                        arg.substr(comma1     + 1, comma2      - comma1      - 1),
                                        arg.substr(comma2     + 1, paren_close - comma2      - 1)}};
      std::array<Rete::Symbol_Ptr_C, 3> sym_ptrs;
      
      for(int i = 0; i != 3; ++i) {
        if(syms[i].find_first_not_of("-0.123456789") == std::string::npos)
          sym_ptrs[i] = std::make_shared<Rete::Symbol_Constant_Float>(atof(syms[i].c_str()));
        else
          sym_ptrs[i] = std::make_shared<Rete::Symbol_Constant_String>(syms[i]);
      }
      
      const auto wme = std::make_shared<Rete::WME>(sym_ptrs[0], sym_ptrs[1], sym_ptrs[2]);

      if(insert)
        ragent.insert_wme(wme);
      else
        ragent.remove_wme(wme);
    }
  }
}
