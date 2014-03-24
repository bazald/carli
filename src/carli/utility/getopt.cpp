#include "getopt.h"

Options & Options::get_global() {
  static Options g_options;
  return g_options;
}
