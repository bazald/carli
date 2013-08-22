#!/bin/bash

premake4 --clang=false --scu=true gmake
CC=gcc-4.8 CXX=g++-4.8 CFLAGS=-Og make config=profiling clean
CC=gcc-4.8 CXX=g++-4.8 CFLAGS=-Og make config=profiling
premake4 --clang=false --scu=false gmake

RV=$?
if [ $RV -ne 0 ]; then
  exit $RV
fi

SEED=$RANDOM
#HEAPCHECK=normal         $CMD -n 1000   "$@"
#HEAPPROFILE=carli_p.heap $CMD -n 1000   "$@"
# CPUPROFILE=carli_p.prof CPUPROFILE_FREQUENCY=100000 \
LD_LIBRARY_PATH=/home/bazald/Software/gcc-4.8/lib64 \
  valgrind --tool=callgrind -v --dump-every-bb=100000000 \
  ./carli_p -o null -s $SEED -e blocks-world --num-steps 10000 "$@"

#pprof --gv ./carli_p ./carli_p.prof
#pprof --callgrind ./carli_p ./carli_p.prof > carli_p.callgrind
