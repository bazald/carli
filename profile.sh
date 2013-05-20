#!/bin/bash

SEED=$RANDOM
CMD="./carli_d -o null -s $SEED -e puddle-world"
#HEAPCHECK=normal         $CMD -n 1000   "$@"
#HEAPPROFILE=carli_d.heap $CMD -n 1000   "$@"
CPUPROFILE=carli_d.prof  $CMD -n 100000 "$@"
pprof --gv ./carli_d ./carli_d.prof
