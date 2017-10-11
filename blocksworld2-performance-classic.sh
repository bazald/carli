#!/bin/bash

TIMECMD="time -f "%e""
MAX=10
FILE=blocksworld2-performance-classic.out
echo "" > $FILE


TOTAL=0.0
for i in $(seq 1 $MAX)
do
  TIME=$({ $TIMECMD ./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.3 --learning-rate 0.01 --secondary-learning-rate 0.2 --policy on-policy --split-update-count 20 --split-test value --unsplit-test value --unsplit-update-count 30 --resplit-bias boost --concrete-update-count 50 --rules rules/blocks-world-2-classic-stack.carli --num-episodes 100 --num-steps 0 --bw2-goal stack --num-blocks-min 3 --num-blocks-max 5 --exploration boltzmann --inverse-temperature 25 --inverse-temperature-episodic-increment 5 --output null ; } 2>&1 | head -n 1)
  echo "  $TIME"
  TOTAL=$(echo "$TOTAL + $TIME" | bc)
done
TIME=$(echo "scale=3; $TOTAL / $MAX" | bc -l)
echo "Stack 3-5 in $TIME Seconds" >> $FILE
echo "Stack 3-5 in $TIME Seconds"

TOTAL=0.0
for i in $(seq 1 $MAX)
do
  TIME=$({ $TIMECMD ./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.3 --learning-rate 0.01 --secondary-learning-rate 0.2 --policy on-policy --split-update-count 20 --split-test value --unsplit-test value --unsplit-update-count 30 --resplit-bias boost --concrete-update-count 50 --rules rules/blocks-world-2-classic-stack-full.carli --num-episodes 100 --num-steps 0 --bw2-goal stack --num-blocks-min 3 --num-blocks-max 5 --exploration boltzmann --inverse-temperature 25 --inverse-temperature-episodic-increment 5 --output null ; } 2>&1 | head -n 1)
  echo "  $TIME"
  TOTAL=$(echo "$TOTAL + $TIME" | bc)
done
TIME=$(echo "scale=3; $TOTAL / $MAX" | bc -l)
echo "Stack (Full) 3-5 in $TIME Seconds" >> $FILE
echo "Stack (Full) 3-5 in $TIME Seconds"

TOTAL=0.0
for i in $(seq 1 $MAX)
do
  TIME=$({ $TIMECMD ./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.3 --learning-rate 0.01 --secondary-learning-rate 0.2 --policy on-policy --split-update-count 30 --split-test value --unsplit-test value --unsplit-update-count 50 --resplit-bias boost --concrete-update-count 100 --rules rules/blocks-world-2-classic-stack.carli --num-episodes 1500 --num-steps 0 --bw2-goal stack --num-blocks-min 3 --num-blocks-max 7 --exploration boltzmann --inverse-temperature 50 --inverse-temperature-episodic-increment 5 --output null ; } 2>&1 | head -n 1)
  echo "  $TIME"
  TOTAL=$(echo "$TOTAL + $TIME" | bc)
done
TIME=$(echo "scale=3; $TOTAL / $MAX" | bc -l)
echo "Stack 3-7 in $TIME Seconds" >> $FILE
echo "Stack 3-7 in $TIME Seconds"

TOTAL=0.0
for i in $(seq 1 $MAX)
do
  TIME=$({ $TIMECMD ./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.3 --learning-rate 0.01 --secondary-learning-rate 0.2 --policy on-policy --split-update-count 30 --split-test value --unsplit-test value --unsplit-update-count 50 --resplit-bias boost --concrete-update-count 100 --rules rules/blocks-world-2-classic-stack-full.carli --num-episodes 1500 --num-steps 0 --bw2-goal stack --num-blocks-min 3 --num-blocks-max 7 --exploration boltzmann --inverse-temperature 50 --inverse-temperature-episodic-increment 5 --output null ; } 2>&1 | head -n 1)
  echo "  $TIME"
  TOTAL=$(echo "$TOTAL + $TIME" | bc)
done
TIME=$(echo "scale=3; $TOTAL / $MAX" | bc -l)
echo "Stack (Full) 3-7 in $TIME Seconds" >> $FILE
echo "Stack (Full) 3-7 in $TIME Seconds"


TOTAL=0.0
for i in $(seq 1 $MAX)
do
  TIME=$({ $TIMECMD ./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.3 --learning-rate 0.01 --secondary-learning-rate 0.2 --policy on-policy --split-update-count 20 --split-test value --unsplit-test value --unsplit-update-count 30 --resplit-bias boost --concrete-update-count 50 --rules rules/blocks-world-2-classic-unstack.carli --num-episodes 100 --num-steps 0 --bw2-goal unstack --num-blocks-min 3 --num-blocks-max 5 --exploration boltzmann --inverse-temperature 25 --inverse-temperature-episodic-increment 5 --output null ; } 2>&1 | head -n 1)
  echo "  $TIME"
  TOTAL=$(echo "$TOTAL + $TIME" | bc)
done
TIME=$(echo "scale=3; $TOTAL / $MAX" | bc -l)
echo "Unstack 3-5 in $TIME Seconds" >> $FILE
echo "Unstack 3-5 in $TIME Seconds"

TOTAL=0.0
for i in $(seq 1 $MAX)
do
  TIME=$({ $TIMECMD ./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.3 --learning-rate 0.01 --secondary-learning-rate 0.2 --policy on-policy --split-update-count 20 --split-test value --unsplit-test value --unsplit-update-count 30 --resplit-bias boost --concrete-update-count 50 --rules rules/blocks-world-2-classic-unstack-full.carli --num-episodes 100 --num-steps 0 --bw2-goal unstack --num-blocks-min 3 --num-blocks-max 5 --exploration boltzmann --inverse-temperature 25 --inverse-temperature-episodic-increment 5 --output null ; } 2>&1 | head -n 1)
  echo "  $TIME"
  TOTAL=$(echo "$TOTAL + $TIME" | bc)
done
TIME=$(echo "scale=3; $TOTAL / $MAX" | bc -l)
echo "Unstack (Full) 3-5 in $TIME Seconds" >> $FILE
echo "Unstack (Full) 3-5 in $TIME Seconds"

TOTAL=0.0
for i in $(seq 1 $MAX)
do
  TIME=$({ $TIMECMD ./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.3 --learning-rate 0.01 --secondary-learning-rate 0.2 --policy on-policy --split-update-count 30 --split-test value --unsplit-test value --unsplit-update-count 50 --resplit-bias boost --concrete-update-count 100 --rules rules/blocks-world-2-classic-unstack.carli --num-episodes 1500 --num-steps 0 --bw2-goal unstack --num-blocks-min 3 --num-blocks-max 7 --exploration boltzmann --inverse-temperature 50 --inverse-temperature-episodic-increment 5 --output null ; } 2>&1 | head -n 1)
  echo "  $TIME"
  TOTAL=$(echo "$TOTAL + $TIME" | bc)
done
TIME=$(echo "scale=3; $TOTAL / $MAX" | bc -l)
echo "Unstack 3-7 in $TIME Seconds" >> $FILE
echo "Unstack 3-7 in $TIME Seconds"

TOTAL=0.0
for i in $(seq 1 $MAX)
do
  TIME=$({ $TIMECMD ./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.3 --learning-rate 0.01 --secondary-learning-rate 0.2 --policy on-policy --split-update-count 30 --split-test value --unsplit-test value --unsplit-update-count 50 --resplit-bias boost --concrete-update-count 100 --rules rules/blocks-world-2-classic-unstack-full.carli --num-episodes 1500 --num-steps 0 --bw2-goal unstack --num-blocks-min 3 --num-blocks-max 7 --exploration boltzmann --inverse-temperature 50 --inverse-temperature-episodic-increment 5 --output null ; } 2>&1 | head -n 1)
  echo "  $TIME"
  TOTAL=$(echo "$TOTAL + $TIME" | bc)
done
TIME=$(echo "scale=3; $TOTAL / $MAX" | bc -l)
echo "Unstack (Full) 3-7 in $TIME Seconds" >> $FILE
echo "Unstack (Full) 3-7 in $TIME Seconds"


TOTAL=0.0
for i in $(seq 1 $MAX)
do
  TIME=$({ $TIMECMD ./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.3 --learning-rate 0.01 --secondary-learning-rate 0.2 --policy on-policy --split-update-count 30 --split-test value --unsplit-test value --unsplit-update-count 50 --resplit-bias boost --concrete-update-count 100 --rules rules/blocks-world-2-classic-on-a-b.carli --num-episodes 100 --num-steps 0 --bw2-goal on-a-b --num-blocks-min 3 --num-blocks-max 5 --exploration boltzmann --inverse-temperature 25 --inverse-temperature-episodic-increment 1 --output null ; } 2>&1 | head -n 1)
  echo "  $TIME"
  TOTAL=$(echo "$TOTAL + $TIME" | bc)
done
TIME=$(echo "scale=3; $TOTAL / $MAX" | bc -l)
echo "On(a,b) 3-5 in $TIME Seconds" >> $FILE
echo "On(a,b) 3-5 in $TIME Seconds"

TOTAL=0.0
for i in $(seq 1 $MAX)
do
  TIME=$({ $TIMECMD ./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.3 --learning-rate 0.01 --secondary-learning-rate 0.2 --policy on-policy --split-update-count 30 --split-test value --unsplit-test value --unsplit-update-count 50 --resplit-bias boost --concrete-update-count 100 --rules rules/blocks-world-2-classic-on-a-b-full.carli --num-episodes 100 --num-steps 0 --bw2-goal on-a-b --num-blocks-min 3 --num-blocks-max 5 --exploration boltzmann --inverse-temperature 25 --inverse-temperature-episodic-increment 1 --output null ; } 2>&1 | head -n 1)
  echo "  $TIME"
  TOTAL=$(echo "$TOTAL + $TIME" | bc)
done
TIME=$(echo "scale=3; $TOTAL / $MAX" | bc -l)
echo "On(a,b) (Full) 3-5 in $TIME Seconds" >> $FILE
echo "On(a,b) (Full) 3-5 in $TIME Seconds"

TOTAL=0.0
for i in $(seq 1 $MAX)
do
  TIME=$({ $TIMECMD ./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.3 --learning-rate 0.01 --secondary-learning-rate 0.2 --policy on-policy --split-update-count 30 --split-test value --unsplit-test value --unsplit-update-count 50 --resplit-bias boost --concrete-update-count 100 --rules rules/blocks-world-2-classic-on-a-b.carli --num-episodes 1500 --num-steps 0 --bw2-goal on-a-b --num-blocks-min 3 --num-blocks-max 7 --exploration boltzmann --inverse-temperature 25 --inverse-temperature-episodic-increment 1 --output null ; } 2>&1 | head -n 1)
  echo "  $TIME"
  TOTAL=$(echo "$TOTAL + $TIME" | bc)
done
TIME=$(echo "scale=3; $TOTAL / $MAX" | bc -l)
echo "On(a,b) 3-7 in $TIME Seconds" >> $FILE
echo "On(a,b) 3-7 in $TIME Seconds"

TOTAL=0.0
for i in $(seq 1 $MAX)
do
  TIME=$({ $TIMECMD ./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.3 --learning-rate 0.01 --secondary-learning-rate 0.2 --policy on-policy --split-update-count 30 --split-test value --unsplit-test value --unsplit-update-count 50 --resplit-bias boost --concrete-update-count 100 --rules rules/blocks-world-2-classic-on-a-b-full.carli --num-episodes 1500 --num-steps 0 --bw2-goal on-a-b --num-blocks-min 3 --num-blocks-max 7 --exploration boltzmann --inverse-temperature 25 --inverse-temperature-episodic-increment 1 --output null ; } 2>&1 | head -n 1)
  echo "  $TIME"
  TOTAL=$(echo "$TOTAL + $TIME" | bc)
done
TIME=$(echo "scale=3; $TOTAL / $MAX" | bc -l)
echo "On(a,b) (Full) 3-7 in $TIME Seconds" >> $FILE
echo "On(a,b) (Full) 3-7 in $TIME Seconds"
