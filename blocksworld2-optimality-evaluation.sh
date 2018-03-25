#!/bin/bash

OUT=blocksworld2-optimality-evaluation.out

echo "SEED 42" > $OUT

for NUM_BLOCKS in $(seq 3 100)
do
  LINE=$(./blocks_world_2 --print-every 1 --output experiment --learning-rate 0 --secondary-learning-rate 0 --epsilon-greedy 0 --num-episodes 1 --num-steps 0 --rules rules/blocks-world-2-trained.carli --bw2-goal exact --evaluate-optimality true --num-blocks-min $NUM_BLOCKS --num-blocks-max $NUM_BLOCKS --stderr /dev/null | tail -n 1)
  echo "$LINE" >> $OUT
  echo "$NUM_BLOCKS => $LINE"
done
