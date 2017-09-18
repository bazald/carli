#!/bin/bash

BASE_CMD="./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.3 --learning-rate 0.01 --secondary-learning-rate 0.3 --num-steps 10000 --policy on-policy --bw2-goal color"

$BASE_CMD --num-blocks-min 3 --num-blocks-max 7 --rules-out rules.carli

$BASE_CMD                                      --num-steps 100 --num-blocks-min 20 --num-blocks-max 20 --rules rules.carli
$BASE_CMD --epsilon-greedy 0 --learning-rate 0 --num-steps 100 --num-blocks-min 20 --num-blocks-max 20 --rules rules.carli
