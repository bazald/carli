#include "getopt.h"

#include <cstring>

int getopt(const int &argc, const char * const * const &argv, const char * const &options) {
  if(argc == optind || !options)
    return -1;

  const int arglen = strlen(argv[optind]);
  if(arglen < 2 || argv[optind][0] != '-')
    return -1;

  for(const char * opt = options; *opt; ++opt) {
    if(*opt == argv[optind][1]) {
      if(*(opt + 1) == ':') {
        if(arglen == 2) {
          optarg = argv[optind + 1];
          optind += 2;
        }
        else
          optarg = argv[optind++] + 2;
      }
      else
        optarg = nullptr;

      return *opt;
    }
  }

  return '?';
}

const char * optarg = nullptr;
int optind = 1;
