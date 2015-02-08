./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.3 --learning-rate 0.3 --num-steps 10000 --num-blocks 4 --policy on-policy --rules-out bw2.txt --rules rules/blocks-world-4-blocks.carli

./blocks_world_2 --epsilon-greedy 0 --learning-rate 0 --num-steps 1000 --num-blocks 4 --rules bw2.txt
