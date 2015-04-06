SEED=$RANDOM
#./cart_pole -s $SEED --ignore-x true --set-goal false -c inv-update-count --eligibility-trace-decay-rate 0.99 -d 0.99 -g 0.01 -l 0.1 --policy off-policy --split-min 3 --pseudoepisode-threshold 10 --num-episodes 100 --num-steps 0
./cart_pole --num-steps 0 --num-episodes 100 --discount-rate 0.99 --learning-rate 0.01 --secondary-learning-rate 0.3 --eligibility-trace-decay-rate 0.99 --ignore-x true --policy on-policy
echo $SEED
