#!/bin/bash

premake4 --clang=false --scu=true gmake
CFLAGS=-Og make config=profiling clean
CFLAGS=-Og make config=profiling
premake4 --clang=true --scu=false gmake

RV=$?
if [ $RV -ne 0 ]; then
  exit $RV
fi

SEED=$RANDOM
ENVIRONMENT=blocks_world_2_p
#HEAPCHECK=normal         $CMD -n 1000   "$@"
#HEAPPROFILE=$ENVIRONMENT.heap $CMD -n 1000   "$@"
# CPUPROFILE=$ENVIRONMENT.prof CPUPROFILE_FREQUENCY=100000 \
valgrind --tool=callgrind -v --dump-every-bb=100000000 \
  ./blocks_world_2_p -o null --seed $SEED --num-steps 1000 --discount-rate 0.9 --eligibility-trace-decay-rate 0.3 --learning-rate 0.1 --secondary-learning-rate 0.1 --policy on-policy --num-blocks 4
#  ./$ENVIRONMENT -o null --seed $SEED --num-steps 10000 --split-min 2 "$@"
#  ./$ENVIRONMENT -o null --seed $SEED --num-steps 10000 --random-start true --learning-rate 0.1 --discount-rate 0.999 --epsilon-greedy 0.1 --policy off-policy --pseudoepisode-threshold 20 --credit-assignment even --split-min 5 --split-max 11 "$@"
#pprof --gv ./$ENVIRONMENT ./$ENVIRONMENT.prof
#pprof --callgrind ./$ENVIRONMENT ./$ENVIRONMENT.prof > $ENVIRONMENT.callgrind
