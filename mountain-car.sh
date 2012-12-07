SEED=$RANDOM; ./carli -s $SEED -e mountain-car -c epsilon-even-depth --reward-negative true -d 1.0 -g 0.05 -l 0.15 -p off-policy --split-min 3 -t 10 --num-episodes 1000; echo $SEED
