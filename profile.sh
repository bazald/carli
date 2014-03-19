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
ENVIRONMENT=tetris_p
#HEAPCHECK=normal         $CMD -n 1000   "$@"
#HEAPPROFILE=$ENVIRONMENT.heap $CMD -n 1000   "$@"
# CPUPROFILE=$ENVIRONMENT.prof CPUPROFILE_FREQUENCY=100000 \
valgrind --tool=callgrind -v --dump-every-bb=100000000 \
  ./$ENVIRONMENT -o null --seed $SEED --num-steps 10 --split-min 9 --split-max 9 "$@"
#  ./$ENVIRONMENT -o null --seed $SEED --num-steps 10000 --random-start true --learning-rate 0.1 --discount-rate 0.999 --epsilon-greedy 0.1 --policy off-policy --pseudoepisode-threshold 20 --credit-assignment even --split-min 5 --split-max 11 "$@"
#pprof --gv ./$ENVIRONMENT ./$ENVIRONMENT.prof
#pprof --callgrind ./$ENVIRONMENT ./$ENVIRONMENT.prof > $ENVIRONMENT.callgrind
