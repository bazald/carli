#!/bin/bash

premake4 --clang=false --scu=true gmake
CFLAGS=-Og make config=profiling clean
CFLAGS=-Og make config=profiling
premake4 --clang=false --scu=false gmake

RV=$?
if [ $RV -ne 0 ]; then
  exit $RV
fi

SEED=$RANDOM
#HEAPCHECK=normal         $CMD -n 1000   "$@"
#HEAPPROFILE=carli_p.heap $CMD -n 1000   "$@"
# CPUPROFILE=carli_p.prof CPUPROFILE_FREQUENCY=100000 \
valgrind --tool=callgrind -v --dump-every-bb=100000000 \
  ./carli_p -o null --seed $SEED --num-steps 10 --environment tetris --split-min 9 --split-max 9 "$@"
#  ./carli_p -o null --seed $SEED --num-steps 10000 --environment puddle-world --random-start true --learning-rate 0.1 --discount-rate 0.999 --epsilon-greedy 0.1 --policy off-policy --pseudoepisode-threshold 20 --credit-assignment even --split-min 5 --split-max 11 "$@"
#pprof --gv ./carli_p ./carli_p.prof
#pprof --callgrind ./carli_p ./carli_p.prof > carli_p.callgrind
