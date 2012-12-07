SEED=$RANDOM; ./carli -s $SEED -e cart-pole --ignore-x true -c inv-update-count -d 0.9 -g 0.01 -l 0.1 -p off-policy --split-min 3 --num-episodes 500; echo $SEED
