#!/bin/bash

CMD="time -f "%e" ./blocks_world_2 --output null --learning-rate 0 --secondary-learning-rate 0 --epsilon-greedy 0 --num-episodes 100 --num-steps 0 --rules rules/blocks-world-2-classic-exact-mt5.carli --bw2-goal exact"
MAX=100

FILE=blocks-world-performance.out
#echo "" > $FILE

for i in $(seq 3 $MAX)
do
  TIME=$({ time $CMD --num-blocks-min $i --num-blocks-max $i --num-episodes 1 --bw2-goal stack --rules rules/blocks-world-2-classic-stack-mt.carli ; } 2>&1 | head -n 1)
  echo "Stack $i Blocks in $TIME Seconds" >> $FILE
  echo "Stack $i Blocks in $TIME Seconds"
done

for i in $(seq 3 $MAX)
do
  TIME=$({ time $CMD --num-blocks-min $i --num-blocks-max $i --num-episodes 1 --bw2-goal unstack --rules rules/blocks-world-2-classic-unstack-mt.carli ; } 2>&1 | head -n 1)
  echo "Unstack $i Blocks in $TIME Seconds" >> $FILE
  echo "Unstack $i Blocks in $TIME Seconds"
done

for i in $(seq 3 $MAX)
do
  TIME=$({ time $CMD --num-blocks-min $i --num-blocks-max $i --num-episodes 1 --bw2-goal on-a-b --rules rules/blocks-world-2-classic-on-a-b.carli ; } 2>&1 | head -n 1)
  echo "On(a,b) $i Blocks in $TIME Seconds" >> $FILE
  echo "On(a,b) $i Blocks in $TIME Seconds"
done

for i in $(seq 3 $MAX)
do
  TIME=$({ time $CMD --num-blocks-min $i --num-blocks-max $i --num-episodes 1 --bw2-goal exact --rules rules/blocks-world-2-trained.carli ; } 2>&1 | head -n 1)
  echo "Exact $i Blocks in $TIME Seconds" >> $FILE
  echo "Exact $i Blocks in $TIME Seconds"
done

for i in $(seq 3 $MAX)
do
  TIME=$({ time $CMD --num-blocks-min $i --num-blocks-max $i --num-episodes 1 --bw2-goal exact --rules rules/blocks-world-2-trained.carli --rete-disable-node-sharing true ; } 2>&1 | head -n 1)
  echo "Exact Disabled Node Sharing $i Blocks in $TIME Seconds" >> $FILE
  echo "Exact Disabled Node Sharing $i Blocks in $TIME Seconds"
done

for i in $(seq 3 $MAX)
do
  TIME=$({ time $CMD --num-blocks-min $i --num-blocks-max $i --num-episodes 1 --bw2-goal exact --rules rules/blocks-world-2-trained.carli --rete-flush-wmes true ; } 2>&1 | head -n 1)
  echo "Exact Flushing WMEs $i Blocks in $TIME Seconds" >> $FILE
  echo "Exact Flushing WMEs $i Blocks in $TIME Seconds"
done

for i in $(seq 3 $MAX)
do
  TIME=$({ time $CMD --num-blocks-min $i --num-blocks-max $i --num-episodes 1 --bw2-goal exact --rules rules/blocks-world-2-trained.carli --rete-disable-node-sharing true --rete-flush-wmes true ; } 2>&1 | head -n 1)
  echo "Exact Disabled Node Sharing & Flushing WMEs $i Blocks in $TIME Seconds" >> $FILE
  echo "Exact Disabled Node Sharing & Flushing WMEs $i Blocks in $TIME Seconds"
done
