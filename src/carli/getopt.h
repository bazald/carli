#ifndef GETOPT_H
#define GETOPT_H

#include <algorithm>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <vector>

class Option : public std::enable_shared_from_this<Option> {
public:
  typedef std::vector<const char *> Arguments;

  Option(const std::string &name_, const int &num_args_ = 0)
   : name(name_),
   num_args(num_args_)
  {
  }

  std::string get_name() const {return name;}
  int get_num_args() const {return num_args;}

  virtual std::string get_help() const {return "";}
  virtual std::string print() const {return "";}
  virtual void operator()(const Arguments &) = 0;

private:
  std::string name;
  int num_args;
};

class Options {
public:
  static Options & get_global() {
    static Options g_options;
    return g_options;
  }

  Options(const std::string &name_ = "a.out")
   : name(name_),
   optind(1)
  {
  }

  void add(const char &short_arg, const std::shared_ptr<Option> &option, const char * const help = nullptr) {
    if(m_short_options.find(std::string(short_arg, 1)) != m_short_options.end()) {
      std::cerr << "Short option already exists: --" << option->get_name() << std::endl;
      throw std::runtime_error("Short option already exists.");
    }

    add(option);

    m_short_options[std::string(short_arg, 1)] = option;

    if(help)
      m_help.push_back(std::string("  -") + short_arg + " --" + option->get_name() + ' ' + option->get_help() + help);
  }

  void add(const std::shared_ptr<Option> &option, const char * const help = nullptr) {
    if(m_long_options.find(option->get_name()) != m_long_options.end()) {
      std::cerr << "Long option already exists: --" << option->get_name() << std::endl;
      throw std::runtime_error("Long option already exists.");
    }

    m_long_options[option->get_name()] = option;

    if(help)
      m_help.push_back(std::string("     --") + option->get_name() + ' ' + option->get_help() + help);
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

  void handle_arguments(const int &argc, const char * const * const &argv, std::shared_ptr<Option> &opt) {
    if(opt->get_num_args() + 1 > argc - optind) {
      std::cerr << "Insufficient arguments for option: " << argv[optind] << std::endl;
      throw std::runtime_error("Insufficient arguments for option.");
    }

    Option::Arguments arguments;
    for(auto arg = optind + 1, argend = optind + opt->get_num_args() + 1; arg != argend; ++arg)
      arguments.push_back(argv[arg]);

    (*opt)(arguments);

    optind += opt->get_num_args() + 1;
  }

  void print_help(std::ostream &os) const {
    os << "Usage: " << name << " [...]" << std::endl;
    for(const auto &opt : m_help)
      os << opt << std::endl;
  }

  void print(std::ostream &os) const {
    os << "Options:" << std::endl;
    for(const auto &opt : m_long_options) {
      const std::string str = opt.second->print();
      if(!str.empty())
        os << "  " << str;
    }
  }

  const Option & operator[](const char &short_arg) {
    auto opt = m_short_options.find(std::string(short_arg, 1));
    if(opt == m_short_options.end()) {
      std::cerr << "Unknown short option: " << short_arg << std::endl;
      throw std::runtime_error("Unknown short option.");
    }

    return *opt->second;
  }

  const Option & operator[](const std::string &long_arg) {
    auto opt = m_long_options.find(long_arg);
    if(opt == m_long_options.end()) {
      std::cerr << "Unknown long option: " << long_arg << std::endl;
      throw std::runtime_error("Unknown long option.");
    }

    return *opt->second;
  }

  std::string name;
  int optind;

private:
  typedef std::map<std::string, std::shared_ptr<Option>> Option_Map;

  Option_Map m_short_options;
  Option_Map m_long_options;

  std::list<std::string> m_help;
};

inline std::ostream & operator<<(std::ostream &os, const Options &options) {
  options.print(os);
  return os;
}

class Option_Function : public Option {
public:
  typedef std::function<void (const Arguments &)> Function;

  Option_Function(const std::string &name_,
                  const int &num_args_,
                  const Function &function_)
   : Option(name_, num_args_),
   m_function(function_)
  {
  }

  void operator()(const Arguments &args) {
    m_function(args);
  }

private:
  Function m_function;
};

class Option_Itemized : public Option {
public:
  Option_Itemized(const std::string &name_,
                  const std::set<std::string> &items_,
                  const std::string &default_value)
   : Option(name_, 1),
   value(default_value),
   items(items_)
  {
    check(value);
  }

  const std::string & get_value() const {return value;}

  std::string get_help() const {
    return get_items();
  }

  std::string print() const {
    std::ostringstream oss;
    oss << get_name() << " = " << value << std::endl;
    return oss.str();
  }

  void operator()(const Arguments &args) {
    std::string new_value;

    std::istringstream iss(args.at(0));
    iss >> new_value;

    check(new_value);

    value = new_value;
  }

private:
  void check(const std::string &item) {
    if(items.find(item) == items.end()) {
      std::ostringstream oss;
      oss << "Argument '" << item << "' not in " << get_items();

      std::cerr << oss.str() << std::endl;
      throw std::range_error(oss.str());
    }
  }

  std::string get_items() const {
    std::ostringstream oss;

    oss << '{';
    bool first = true;
    for(const std::string &s : items) {
      if(first)
        first = false;
      else
        oss << ", ";

      oss << s;
    }
    oss << '}';

    return oss.str();
  }

  std::string value;
  std::set<std::string> items;
};

template <typename TYPE>
class Option_Ranged : public Option {
public:
  Option_Ranged(const std::string &name_,
                const TYPE &lower_bound_,
                const bool &inclusive_lower_bound_,
                const TYPE &upper_bound_,
                const bool &inclusive_upper_bound_,
                const TYPE &default_value = TYPE())
   : Option(name_, 1),
   value(default_value),
   lower_bound(lower_bound_),
   upper_bound(upper_bound_),
   inclusive_lower_bound(inclusive_lower_bound_),
   inclusive_upper_bound(inclusive_upper_bound_)
  {
    check(value);
  }

  const TYPE & get_value() const {return value;}

  std::string get_help() const {
    return get_range();
  }

  std::string print() const {
    std::ostringstream oss;
    oss << get_name() << " = " << value << std::endl;
    return oss.str();
  }

  void operator()(const Arguments &args) {
    TYPE new_value;

    std::istringstream iss(args.at(0));
    iss >> new_value;

    check(new_value);

    value = new_value;
  }

private:
  void check(const TYPE &value_) {
    if((inclusive_lower_bound ? value_ < lower_bound : value_ <= lower_bound) ||
       (inclusive_upper_bound ? value_ > upper_bound : value_ >= upper_bound))
    {
      std::ostringstream oss;
      oss << "Argument '" << value_ << "' outside the range " << get_range();

      std::cerr << oss.str() << std::endl;
      throw std::range_error(oss.str());
    }
  }

  std::string get_range() const {
    std::ostringstream oss;

    oss << (inclusive_lower_bound ? '[' : '(') << lower_bound << ','
        << upper_bound << (inclusive_upper_bound ? ']' : ')');

    return oss.str();
  }

  TYPE value;
  TYPE lower_bound;
  TYPE upper_bound;
  bool inclusive_lower_bound;
  bool inclusive_upper_bound;
};

#endif
