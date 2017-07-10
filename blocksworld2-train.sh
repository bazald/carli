#!/bin/bash

BASE_CMD="./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.3 --learning-rate 0.01 --secondary-learning-rate 0.3 --num-steps 10000 --policy on-policy"
TRAIN_CAP=6

$BASE_CMD --num-blocks 3 --num-goal-blocks 3 --rules-out bw2.txt

for i in $(seq 4 $TRAIN_CAP); do
  $BASE_CMD --num-blocks $i --num-goal-blocks $i --rules-out bw$(($i-1)).txt --rules bw$(($i-2)).txt
done

$BASE_CMD                                      --num-steps 100 --num-blocks 20 --num-goal-blocks 20 --rules bw$(($TRAIN_CAP-1)).txt
$BASE_CMD --epsilon-greedy 0 --learning-rate 0 --num-steps 100 --num-blocks 20 --num-goal-blocks 20 --rules bw$(($TRAIN_CAP-1)).txt
