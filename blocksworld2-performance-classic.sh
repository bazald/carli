#!/bin/bash

TIMECMD="time -f "%e""
MAX=10
FILE=blocksworld2-performance-classic.out
echo "" > $FILE


# TOTAL=0.0
# for i in $(seq 1 $MAX)
# do
#   TIME=$({ $TIMECMD ./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.1 --learning-rate 0.05 --secondary-learning-rate 0.03 --policy off-policy --split-update-count 10 --split-test value --unsplit-test value --unsplit-update-count 20 --resplit-bias boost --concrete-update-count 30 --num-steps 0 --exploration boltzmann --inverse-temperature 25 --inverse-temperature-episodic-increment 5 --output null --rules rules/blocks-world-2-classic-stack.carli --scenario 100 --num-episodes 100 --bw2-goal stack ; } 2>&1 | head -n 1)
#   echo "  $TIME"
#   TOTAL=$(echo "$TOTAL + $TIME" | bc)
# done
# TIME=$(echo "scale=3; $TOTAL / $MAX" | bc -l)
# echo "Stack 3-5 in $TIME Seconds" >> $FILE
# echo "Stack 3-5 in $TIME Seconds"

TOTAL=0.0
for i in $(seq 1 $MAX)
do
  TIME=$({ $TIMECMD ./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.1 --learning-rate 0.05 --secondary-learning-rate 0.03 --policy off-policy --split-update-count 10 --split-test value --unsplit-test value --unsplit-update-count 20 --resplit-bias boost --concrete-update-count 30 --num-steps 0 --exploration boltzmann --inverse-temperature 25 --inverse-temperature-episodic-increment 5 --output null --rules rules/blocks-world-2-classic-stack-full.carli --scenario 100 --num-episodes 100 --bw2-goal stack ; } 2>&1 | head -n 1)
  echo "  $TIME"
  TOTAL=$(echo "$TOTAL + $TIME" | bc)
done
TIME=$(echo "scale=3; $TOTAL / $MAX" | bc -l)
echo "Stack (Full) 3-5 in $TIME Seconds" >> $FILE
echo "Stack (Full) 3-5 in $TIME Seconds"

# TOTAL=0.0
# for i in $(seq 1 $MAX)
# do
#   TIME=$({ $TIMECMD ./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.1 --learning-rate 0.05 --secondary-learning-rate 0.03 --policy off-policy --split-update-count 10 --split-test value --unsplit-test value --unsplit-update-count 20 --resplit-bias boost --concrete-update-count 30 --scenario 100 --num-steps 0 --exploration boltzmann --inverse-temperature 25 --inverse-temperature-episodic-increment 5 --output null --rules rules/blocks-world-2-classic-stack.carli --num-episodes 1500 --scenario 1500 --bw2-goal stack ; } 2>&1 | head -n 1)
#   echo "  $TIME"
#   TOTAL=$(echo "$TOTAL + $TIME" | bc)
# done
# TIME=$(echo "scale=3; $TOTAL / $MAX" | bc -l)
# echo "Stack 3-7 in $TIME Seconds" >> $FILE
# echo "Stack 3-7 in $TIME Seconds"

TOTAL=0.0
for i in $(seq 1 $MAX)
do
  TIME=$({ $TIMECMD ./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.1 --learning-rate 0.05 --secondary-learning-rate 0.03 --policy off-policy --split-update-count 10 --split-test value --unsplit-test value --unsplit-update-count 20 --resplit-bias boost --concrete-update-count 30 --scenario 100 --num-steps 0 --exploration boltzmann --inverse-temperature 25 --inverse-temperature-episodic-increment 5 --output null --rules rules/blocks-world-2-classic-stack-full.carli --num-episodes 1500 --scenario 1500 --bw2-goal stack ; } 2>&1 | head -n 1)
  echo "  $TIME"
  TOTAL=$(echo "$TOTAL + $TIME" | bc)
done
TIME=$(echo "scale=3; $TOTAL / $MAX" | bc -l)
echo "Stack (Full) 3-7 in $TIME Seconds" >> $FILE
echo "Stack (Full) 3-7 in $TIME Seconds"


# TOTAL=0.0
# for i in $(seq 1 $MAX)
# do
#   TIME=$({ $TIMECMD ./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.1 --learning-rate 0.05 --secondary-learning-rate 0.03 --policy off-policy --split-update-count 10 --split-test value --unsplit-test value --unsplit-update-count 20 --resplit-bias boost --concrete-update-count 30 --scenario 100 --num-steps 0 --exploration boltzmann --inverse-temperature 25 --inverse-temperature-episodic-increment 5 --output null --rules rules/blocks-world-2-classic-unstack.carli --num-episodes 100 --scenario 100 --bw2-goal unstack ; } 2>&1 | head -n 1)
#   echo "  $TIME"
#   TOTAL=$(echo "$TOTAL + $TIME" | bc)
# done
# TIME=$(echo "scale=3; $TOTAL / $MAX" | bc -l)
# echo "Unstack 3-5 in $TIME Seconds" >> $FILE
# echo "Unstack 3-5 in $TIME Seconds"

TOTAL=0.0
for i in $(seq 1 $MAX)
do
  TIME=$({ $TIMECMD ./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.1 --learning-rate 0.05 --secondary-learning-rate 0.03 --policy off-policy --split-update-count 10 --split-test value --unsplit-test value --unsplit-update-count 20 --resplit-bias boost --concrete-update-count 30 --scenario 100 --num-steps 0 --exploration boltzmann --inverse-temperature 25 --inverse-temperature-episodic-increment 5 --output null --rules rules/blocks-world-2-classic-unstack-full.carli --num-episodes 100 --scenario 100 --bw2-goal unstack ; } 2>&1 | head -n 1)
  echo "  $TIME"
  TOTAL=$(echo "$TOTAL + $TIME" | bc)
done
TIME=$(echo "scale=3; $TOTAL / $MAX" | bc -l)
echo "Unstack (Full) 3-5 in $TIME Seconds" >> $FILE
echo "Unstack (Full) 3-5 in $TIME Seconds"

# TOTAL=0.0
# for i in $(seq 1 $MAX)
# do
#   TIME=$({ $TIMECMD ./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.1 --learning-rate 0.05 --secondary-learning-rate 0.03 --policy off-policy --split-update-count 10 --split-test value --unsplit-test value --unsplit-update-count 20 --resplit-bias boost --concrete-update-count 30 --scenario 100 --num-steps 0 --exploration boltzmann --inverse-temperature 25 --inverse-temperature-episodic-increment 5 --output null --rules rules/blocks-world-2-classic-unstack.carli --num-episodes 1500 --scenario 1500 --bw2-goal unstack ; } 2>&1 | head -n 1)
#   echo "  $TIME"
#   TOTAL=$(echo "$TOTAL + $TIME" | bc)
# done
# TIME=$(echo "scale=3; $TOTAL / $MAX" | bc -l)
# echo "Unstack 3-7 in $TIME Seconds" >> $FILE
# echo "Unstack 3-7 in $TIME Seconds"

TOTAL=0.0
for i in $(seq 1 $MAX)
do
  TIME=$({ $TIMECMD ./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.1 --learning-rate 0.05 --secondary-learning-rate 0.03 --policy off-policy --split-update-count 10 --split-test value --unsplit-test value --unsplit-update-count 20 --resplit-bias boost --concrete-update-count 30 --scenario 100 --num-steps 0 --exploration boltzmann --inverse-temperature 25 --inverse-temperature-episodic-increment 5 --output null --rules rules/blocks-world-2-classic-unstack-full.carli --num-episodes 1500 --scenario 1500 --bw2-goal unstack ; } 2>&1 | head -n 1)
  echo "  $TIME"
  TOTAL=$(echo "$TOTAL + $TIME" | bc)
done
TIME=$(echo "scale=3; $TOTAL / $MAX" | bc -l)
echo "Unstack (Full) 3-7 in $TIME Seconds" >> $FILE
echo "Unstack (Full) 3-7 in $TIME Seconds"


# TOTAL=0.0
# for i in $(seq 1 $MAX)
# do
#   TIME=$({ $TIMECMD ./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.1 --learning-rate 0.05 --secondary-learning-rate 0.03 --policy off-policy --split-update-count 10 --split-test value --unsplit-test value --unsplit-update-count 20 --resplit-bias boost --concrete-update-count 30 --scenario 100 --num-steps 0 --exploration boltzmann --inverse-temperature 25 --inverse-temperature-episodic-increment 5 --output null --rules rules/blocks-world-2-classic-on-a-b.carli --num-episodes 100 --scenario 100 --bw2-goal on-a-b ; } 2>&1 | head -n 1)
#   echo "  $TIME"
#   TOTAL=$(echo "$TOTAL + $TIME" | bc)
# done
# TIME=$(echo "scale=3; $TOTAL / $MAX" | bc -l)
# echo "On(a,b) 3-5 in $TIME Seconds" >> $FILE
# echo "On(a,b) 3-5 in $TIME Seconds"

TOTAL=0.0
for i in $(seq 1 $MAX)
do
  TIME=$({ $TIMECMD ./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.1 --learning-rate 0.05 --secondary-learning-rate 0.03 --policy off-policy --split-update-count 10 --split-test value --unsplit-test value --unsplit-update-count 20 --resplit-bias boost --concrete-update-count 30 --scenario 100 --num-steps 0 --exploration boltzmann --inverse-temperature 25 --inverse-temperature-episodic-increment 5 --output null --rules rules/blocks-world-2-classic-on-a-b-full.carli --num-episodes 100 --scenario 100 --bw2-goal on-a-b ; } 2>&1 | head -n 1)
  echo "  $TIME"
  TOTAL=$(echo "$TOTAL + $TIME" | bc)
done
TIME=$(echo "scale=3; $TOTAL / $MAX" | bc -l)
echo "On(a,b) (Full) 3-5 in $TIME Seconds" >> $FILE
echo "On(a,b) (Full) 3-5 in $TIME Seconds"

# TOTAL=0.0
# for i in $(seq 1 $MAX)
# do
#   TIME=$({ $TIMECMD ./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.1 --learning-rate 0.05 --secondary-learning-rate 0.03 --policy off-policy --split-update-count 10 --split-test value --unsplit-test value --unsplit-update-count 20 --resplit-bias boost --concrete-update-count 30 --scenario 100 --num-steps 0 --exploration boltzmann --inverse-temperature 25 --inverse-temperature-episodic-increment 5 --output null --rules rules/blocks-world-2-classic-on-a-b.carli --num-episodes 1500 --scenario 1500 --bw2-goal on-a-b ; } 2>&1 | head -n 1)
#   echo "  $TIME"
#   TOTAL=$(echo "$TOTAL + $TIME" | bc)
# done
# TIME=$(echo "scale=3; $TOTAL / $MAX" | bc -l)
# echo "On(a,b) 3-7 in $TIME Seconds" >> $FILE
# echo "On(a,b) 3-7 in $TIME Seconds"

TOTAL=0.0
for i in $(seq 1 $MAX)
do
  TIME=$({ $TIMECMD ./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.1 --learning-rate 0.05 --secondary-learning-rate 0.03 --policy off-policy --split-update-count 10 --split-test value --unsplit-test value --unsplit-update-count 20 --resplit-bias boost --concrete-update-count 30 --scenario 100 --num-steps 0 --exploration boltzmann --inverse-temperature 25 --inverse-temperature-episodic-increment 5 --output null --rules rules/blocks-world-2-classic-on-a-b-full.carli --num-episodes 1500 --scenario 1500 --bw2-goal on-a-b ; } 2>&1 | head -n 1)
  echo "  $TIME"
  TOTAL=$(echo "$TOTAL + $TIME" | bc)
done
TIME=$(echo "scale=3; $TOTAL / $MAX" | bc -l)
echo "On(a,b) (Full) 3-7 in $TIME Seconds" >> $FILE
echo "On(a,b) (Full) 3-7 in $TIME Seconds"
