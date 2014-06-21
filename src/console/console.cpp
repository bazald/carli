#include "carli/rete/grammar/rete_parser.h"

#include <array>
#include <csignal>
#include <sstream>

#ifdef _WINDOWS
#include <cstdio>
#include <io.h>
#else
#include <unistd.h>
#endif

volatile sig_atomic_t g_quit = 0;
void signal_handler(int sig) {
  if(!g_quit)
    signal(sig, signal_handler);
  if(sig == SIGTERM)
    g_quit = true;
  else
    std::cerr << std::endl << "Ctrl+" << (sig == SIGINT ? "C" : "Break") <<" captured. Call 'exit' to quit." << std::endl;
}

int main(int argc, char **argv) {
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);
#ifdef SIGBREAK
  signal(SIGBREAK, signal_handler);
#endif

  Rete::Rete_Agent ragent;
  std::string line;
  int line_number = 1;

  for(int i = 1; !g_quit && i < argc; ++i)
    rete_parse_file(ragent, argv[i]);

  while(!g_quit) {
    std::cout << "carli % ";
    getline(std::cin, line);
    if(std::cin) {
      rete_parse_string(ragent, line, line_number);
      std::cout << std::endl;
    }
#ifdef _WINDOWS
    else if(!_isatty(_fileno(stdin)))
#else
    else if(!isatty(STDIN_FILENO))
#endif
      g_quit = true;
    else
      std::cin.clear();
  }

  std::cout << "Now exiting." << std::endl;

  return 0;
}
