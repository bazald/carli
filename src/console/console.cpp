#include "carli/parser/rete_parser.h"

#include "carli/experiment.h"
#include "blocks_world_env/blocks_world.h"

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
  if(!Rete::rete_get_exit())
    signal(sig, signal_handler);
  if(sig == SIGTERM)
    Rete::rete_set_exit();
  else
    std::cerr << std::endl << "Ctrl+" << (sig == SIGINT ? "C" : "Break") <<" captured. Call 'exit' to quit." << std::endl;
}

int main(int argc, char **argv) {
#ifdef SIGBREAK
  signal(SIGBREAK, signal_handler);
#endif
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  Carli::Experiment experiment;
  experiment.take_args(argc, argv);

  std::shared_ptr<Carli::Environment> env = std::make_shared<Blocks_World::Environment>();
  std::shared_ptr<Carli::Agent> agent = std::make_shared<Blocks_World::Agent>(env);

  std::string line;
  int line_number = 1;

  for(int64_t i = Options::get_global().optind; !Rete::rete_get_exit() && i < argc; ++i) {
    Rete::rete_parse_file(*agent, argv[i]);
    std::cout << std::endl;
  }

  while(!Rete::rete_get_exit()) {
    std::cout << "carli % ";
    getline(std::cin, line);
    if(std::cin) {
      Rete::rete_parse_string(*agent, line, line_number);
      std::cout << std::endl;
    }
    else if(redirected_cin())
      Rete::rete_set_exit();
    else
      std::cin.clear();
  }

  return 0;
}
