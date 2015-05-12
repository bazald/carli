./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.3 --learning-rate 0.3 --num-steps 10000 --num-blocks 3 --policy on-policy --rules-out bw2.txt
./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.3 --learning-rate 0.3 --num-steps 10000 --num-blocks 4 --policy on-policy --rules-out bw3.txt --rules bw2.txt
./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.3 --learning-rate 0.3 --num-steps 10000 --num-blocks 5 --policy on-policy --rules-out bw4.txt --rules bw3.txt

./blocks_world_2 --epsilon-greedy 0 --learning-rate 0 --num-steps 1000 --num-blocks 20 --rules bw4.txt
