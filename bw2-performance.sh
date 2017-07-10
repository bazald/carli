#!/bin/bash

for name in opt flu dup dup-flu; do
  mkdir -p bw2-performance-again/$name
done

COUNT=70

# for i in $(seq 2 $COUNT); do echo; echo "$i Blocks:"; time ./blocks_world_2 --output null --learning-rate 0 --num-steps 100 --rules rules/blocks-world-2-trained.carli --seed 1339 --num-blocks $i --num-goal-blocks $i --rules-out bw2-performance-again/opt/rules-out-$i.txt; done
for i in $(seq 2 $COUNT); do echo; echo "$i Blocks:"; time ./blocks_world_2 --output null --learning-rate 0 --num-steps 100 --rules rules/blocks-world-2-trained.carli --seed 1339 --num-blocks $i --num-goal-blocks $i --rete-flush-wmes true --rules-out bw2-performance-again/flu/rules-out-$i.txt; done
for i in $(seq 2 $COUNT); do echo; echo "$i Blocks:"; time ./blocks_world_2 --output null --learning-rate 0 --num-steps 100 --rules rules/blocks-world-2-trained.carli --seed 1339 --num-blocks $i --num-goal-blocks $i --rete-disable-node-sharing true --rules-out bw2-performance-again/dup/rules-out-$i.txt; done
for i in $(seq 2 $COUNT); do echo; echo "$i Blocks:"; time ./blocks_world_2 --output null --learning-rate 0 --num-steps 100 --rules rules/blocks-world-2-trained.carli --seed 1339 --num-blocks $i --num-goal-blocks $i --rete-disable-node-sharing true --rete-flush-wmes true --rules-out bw2-performance-again/dup-flu/rules-out-$i.txt; done

for name in opt flu dup dup-flu; do
  dd if=/dev/null of=bw2-performance-again/$name.txt && for i in $(seq 2 $COUNT); do head -n 1 bw2-performance-again/$name/rules-out-$i.txt | sed 's/.*= //' | sed 's/ .*//' >> bw2-performance-again/$name.txt; done
done

./bw2-performance.py bw2-performance-again/*/*.txt
