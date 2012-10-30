#ifndef GETOPT_H
#define GETOPT_H

int getopt(const int &argc, const char * const * const &argv, const char * const &options);

extern const char * optarg;
extern int optind;

#endif
