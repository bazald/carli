./blocks_world_2 --learning-rate 0.01 --num-steps 10000 --num-blocks 3 --policy off-policy --rules-out bw2.txt
./blocks_world_2 --learning-rate 0.01 --num-steps 10000 --num-blocks 4 --policy off-policy --rules-out bw2.txt --rules bw2.txt
./blocks_world_2 --learning-rate 0.01 --num-steps 10000 --num-blocks 5 --policy off-policy --rules-out bw2.txt --rules bw2.txt

./blocks_world_2 --epsilon-greedy 0 --learning-rate 0 --num-steps 1000 --num-blocks 20 --rules bw2.txt
