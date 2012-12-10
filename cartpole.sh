SEED=$RANDOM; ./carli -s $SEED -e cart-pole --ignore-x true -c inv-update-count --eligibility-trace-decay-rate 0.3 -d 0.9 -g 0.01 -l 1 -p off-policy --split-min 3 --num-episodes 100; echo $SEED
