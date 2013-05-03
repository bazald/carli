#ifndef GETOPT_H
#define GETOPT_H

#include <algorithm>
#include <cstring>
#include <iostream>
#include <map>
#include <vector>

class Options {
public:
  typedef std::vector<const char *> Arguments;
  typedef std::function<void (const Arguments &)> Handler;

  struct Option {
    Option() : num_args(0) {}
    Option(const Handler &handler_, const int &num_args_ = 0)
     : handler(handler_),
     num_args(num_args_)
    {
    }

    Handler handler;
    int num_args;
  };

  Options(const std::string &name_ = "a.out")
   : name(name_),
   optind(1)
  {
  }

  void add(const char &short_arg, const std::string &long_arg, const Option &option, const char * const help = nullptr) {
    add(short_arg, option);
    add(long_arg, option);

    if(help)
      m_help.push_back(std::string("  -") + short_arg + " --" + long_arg + ' ' + help);
  }

  void add(const char &short_arg, const Option &option, const char * const help = nullptr) {
    if(m_short_options.find(std::string(short_arg, 1)) != m_short_options.end()) {
      std::cerr << "Short option already exists: -" << short_arg << std::endl;
      throw std::runtime_error("Short option already exists.");
    }

    m_short_options[std::string(short_arg, 1)] = option;

    if(help)
      m_help.push_back(std::string("  -") + short_arg + ' ' + help);
  }

  void add(const std::string &long_arg, const Option &option, const char * const help = nullptr) {
    if(m_long_options.find(long_arg) != m_long_options.end()) {
      std::cerr << "Long option already exists: --" << long_arg << std::endl;
      throw std::runtime_error("Long option already exists.");
    }

    m_long_options[long_arg] = option;

    if(help)
      m_help.push_back(std::string("     --") + long_arg + ' ' + help);
  }

  void get(const int &argc, const char * const * const &argv) {
    while(argc != optind) {
      const int arglen = int(strlen(argv[optind]));
      switch(arglen) {
        case 0:
        case 1:
          return;

        case 2:
          if(argv[optind][0] != '-' || argv[optind][1] == '-')
            return;
          else {
            handle_short(argc, argv);
            break;
          }

        default:
          if(argv[optind][0] != '-' || argv[optind][1] != '-')
            return;
          else {
            handle_long(argc, argv);
            break;
          }
      }
    }
  }

  void handle_short(const int &argc, const char * const * const &argv) {
    auto opt = m_short_options.find(std::string(argv[optind][1], 1));
    if(opt == m_short_options.end()) {
      std::cerr << "Unknown short option: " << argv[optind] << std::endl;
      throw std::runtime_error("Unknown short option.");
    }

    handle_arguments(argc, argv, opt->second);
  }

  void handle_long(const int &argc, const char * const * const &argv) {
    auto opt = m_long_options.find(argv[optind] + 2);
    if(opt == m_long_options.end()) {
      std::cerr << "Unknown long option: " << argv[optind] << std::endl;
      throw std::runtime_error("Unknown long option.");
    }

    handle_arguments(argc, argv, opt->second);
  }

  void handle_arguments(const int &argc, const char * const * const &argv, const Option &opt) {
    if(opt.num_args + 1 > argc - optind) {
      std::cerr << "Insufficient arguments for option: " << argv[optind] << std::endl;
      throw std::runtime_error("Insufficient arguments for option.");
    }

    Arguments arguments;
    for(auto arg = optind + 1, argend = optind + opt.num_args + 1; arg != argend; ++arg)
      arguments.push_back(argv[arg]);

    opt.handler(arguments);

    optind += opt.num_args + 1;
  }

  void print_help(std::ostream &os) {
    os << "Usage: " << name << " [...]" << std::endl;
    for(const auto &opt : m_help) {
      os << opt << std::endl;
    }
  }

  std::string name;
  int optind;

private:
  typedef std::map<std::string, Option> Option_Map;

  Option_Map m_short_options;
  Option_Map m_long_options;

  std::list<std::string> m_help;
};

#endif
