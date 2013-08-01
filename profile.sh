#!/bin/bash

make config=profiling_clang clean
make config=profiling_clang

RV=$?
if [ $RV -ne 0 ]; then
  exit $RV
fi

SEED=$RANDOM
CMD="./carli_p -o null -s $SEED -e puddle-world --split-min 5"
#HEAPCHECK=normal         $CMD -n 1000   "$@"
#HEAPPROFILE=carli_p.heap $CMD -n 1000   "$@"
CPUPROFILE=carli_p.prof  $CMD -n 100000 "$@"
pprof --gv ./carli_p ./carli_p.prof