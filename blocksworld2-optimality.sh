#!/bin/bash

# VARIANTS=("" "-full")
VARIANTS=("-full")
GOALS=(stack unstack on-a-b)
TRAINING_SEEDS=(19536 15282 16350 16720 4130 22322 6962 4364 14267 24831)
TRAINING_RULES=rules.carli
TRAINING_OUTPUT=training.out
TEST_SEED=21791
TEST_OUTPUT=test.out
OUTPUT_DIR=experiment-bw2-comparison-100

TRAINING_CMD="./blocks_world_2 --discount-rate 0.9 --eligibility-trace-decay-rate 0.1 --learning-rate 0.05 --secondary-learning-rate 0.03 --policy off-policy --split-update-count 10 --split-test value --unsplit-test value --unsplit-update-count 20 --resplit-bias boost --concrete-update-count 30 --scenario 100 --num-steps 0 --exploration boltzmann --inverse-temperature 25 --inverse-temperature-episodic-increment 5 --output null --rules-out $TRAINING_RULES"
TEST_CMD="./blocks_world_2 --epsilon-greedy 0 --learning-rate 0 --num-episodes 156 --num-steps 0 --num-blocks-min 3 --num-blocks-max 10 --seed $TEST_SEED --rules $TRAINING_RULES"

for VARIANT in "${VARIANTS[@]}"
do
  for GOAL in "${GOALS[@]}"
  do
    mkdir -p $OUTPUT_DIR/$GOAL$VARIANT

    $TEST_CMD --bw2-goal $GOAL --stdout $TEST_OUTPUT --rules rules/blocks-world-2-classic-$GOAL-mt.carli --stderr test.err
    head -n 157 $TEST_OUTPUT > temp
    cat temp > $TEST_OUTPUT

    for SEED in "${TRAINING_SEEDS[@]}"
    do
      echo $GOAL$VARIANT for $SEED:

      for NUM_EPISODES in $(seq 0 100)
      do
        if [ $NUM_EPISODES -eq 0 ]
        then
          cp rules/blocks-world-2-classic-$GOAL.carli $TRAINING_RULES
        else
          $TRAINING_CMD --seed $SEED --num-episodes $NUM_EPISODES --bw2-goal $GOAL --rules rules/blocks-world-2-classic-$GOAL$VARIANT.carli --stderr null
        fi

        $TEST_CMD --bw2-goal $GOAL --step-cutoff 25 --stdout $TRAINING_OUTPUT --stderr training.err
        head -n 157 $TRAINING_OUTPUT > temp
        cat temp > $TRAINING_OUTPUT

#         NUM_RIGHT=0
#         NUM_WRONG=-1
#         while read -r TRAIN_LINE <&3 && read -r TEST_LINE <&4; do
#           TRAIN=$(echo $TRAIN_LINE | cut -d' ' -f3)
#           TEST=$(echo $TEST_LINE | cut -d' ' -f3)
#           if [ $TRAIN -gt $TEST ]
#           then
#             NUM_WRONG=$(($NUM_WRONG + 1))
#           elif [ $TRAIN -lt $TEST ]
#           then
#             echo "$TRAIN < $TEST"
#             exit
#           else
#             NUM_RIGHT=$(($NUM_RIGHT + 1))
#           fi
#         done 3<$TRAINING_OUTPUT 4<$TEST_OUTPUT
        
        NUM_WRONG=$(lbldiff $TRAINING_OUTPUT $TEST_OUTPUT)
#         NUM_RIGHT=$((156-$NUM_WRONG))

        PERCENT_CORRECT=$(echo "scale=3; (156 - $NUM_WRONG) / 156" | bc -l)
        echo $NUM_EPISODES $PERCENT_CORRECT

        if [ $NUM_EPISODES -eq 0 ]
        then
          echo $NUM_EPISODES $PERCENT_CORRECT > $OUTPUT_DIR/$GOAL$VARIANT/$SEED.out
        else
          echo $NUM_EPISODES $PERCENT_CORRECT >> $OUTPUT_DIR/$GOAL$VARIANT/$SEED.out
        fi
      done
    done
  done
done
