#!/bin/bash

premake4 --clang=true --scu=false gmake
CC=clang CXX=clang++ make config=profiling clean
CC=clang CXX=clang++ make config=profiling

RV=$?
if [ $RV -ne 0 ]; then
  exit $RV
fi

SEED=$RANDOM
CMD="./carli_p -o null -s $SEED -e puddle-world --split-min 5"
#HEAPCHECK=normal         $CMD -n 1000   "$@"
#HEAPPROFILE=carli_p.heap $CMD -n 1000   "$@"
CPUPROFILE=carli_p.prof CPUPROFILE_FREQUENCY=100000 $CMD -n 100000 "$@"
pprof --gv ./carli_p ./carli_p.prof

#pprof --callgrind ./carli_p ./carli_p.prof > carli_p.callgrind
#valgrind --tool=callgrind -v --dump-every-bb=100000000 ./carli_p
