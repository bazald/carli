SEED=$RANDOM
./cart_pole --num-steps 1000000 --num-episodes 0 --discount-rate 0.9 --learning-rate 0.001 --secondary-learning-rate 1.0 --eligibility-trace-decay-rate 0.3 --ignore-x true --policy on-policy --split-max 24 --split-test policy --epsilon-greedy 0.01 --credit-assignment even --rules-out rules-out.txt --seed $SEED
echo $SEED
