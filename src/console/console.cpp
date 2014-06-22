#include "carli/rete/grammar/rete_parser.h"

#include <array>
#include <csignal>
#include <sstream>

#ifdef _WINDOWS
#include <cstdio>
#include <io.h>
inline bool redirected_cin() {return !_isatty(_fileno(stdin));}
#else
#include <unistd.h>
inline bool redirected_cin() {return !isatty(STDIN_FILENO);}
#endif

void signal_handler(int sig) {
  if(!rete_get_exit())
    signal(sig, signal_handler);
  if(sig == SIGTERM)
    rete_set_exit();
  else
    std::cerr << std::endl << "Ctrl+" << (sig == SIGINT ? "C" : "Break") <<" captured. Call 'exit' to quit." << std::endl;
}

int main(int argc, char **argv) {
#ifdef SIGBREAK
  signal(SIGBREAK, signal_handler);
#endif
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  Rete::Rete_Agent ragent;
  std::string line;
  int line_number = 1;

  for(int i = 1; !rete_get_exit() && i < argc; ++i)
    rete_parse_file(ragent, argv[i]);

  while(!rete_get_exit()) {
    std::cout << "carli % ";
    getline(std::cin, line);
    if(std::cin) {
      rete_parse_string(ragent, line, line_number);
      std::cout << std::endl;
    }
    else if(redirected_cin())
      rete_set_exit();
    else
      std::cin.clear();
  }

  return 0;
}
