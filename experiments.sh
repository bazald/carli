#!/bin/bash

function average {
  COUNT=0
  VALUE=0

  for file in $@
  do
    NEW=$(tail -n 1 "$file" | sed 's/^.* .* \(.*\) .*$/\1/')
    VALUE=$(echo "scale=4; $VALUE * ($COUNT / ($COUNT + 1)) + $NEW / ($COUNT + 1)" | bc)
    COUNT=$(($COUNT + 1))
  done

  echo $VALUE
}

function averages {
  BEST_VALUE=-99999
  BEST_EXP=""

  for exp in $@; do
    VALUE=$(average $exp/*.out)
    if [ $(echo "scale=4; $VALUE > $BEST_VALUE" | bc) -eq 1 ]; then
      BEST_VALUE=$VALUE
      BEST_EXP="$exp"
    fi
  done

  echo "$BEST_VALUE for $BEST_EXP"
}

./puddleworld-batch.py -r 10 -s 200000
./puddleworld.py experiment-pw/*_5_5_*/*.out
./puddleworld.py experiment-pw/*_7_7_*/*.out
./puddleworld.py experiment-pw/*_9_9_*/*.out
./puddleworld.py experiment-pw/*_11_11_*/*.out
./puddleworld.py experiment-pw/*_13_13_*/*.out
./puddleworld.py experiment-pw/*_3_13_*/*.out
averages experiment-pw/specific_*/
averages experiment-pw/*_5_5_*/
averages experiment-pw/*_7_7_*/
averages experiment-pw/*_9_9_*/
averages experiment-pw/*_11_11_*/
averages experiment-pw/*_13_13_*/
averages experiment-pw/*_3_13_*/
./puddleworld.py \
	experiment-pw/specific_*_7_7_*/*.out \
	experiment-pw/inv-root-update-count_*_5_5_*/*.out \
	experiment-pw/inv-root-update-count_*_7_7_*/*.out \
	experiment-pw/inv-root-update-count_*_9_9_*/*.out \
	experiment-pw/inv-log-update-count_*_11_11_*/*.out \
  experiment-pw/inv-log-update-count_*_13_13_*/*.out \
	experiment-pw/inv-root-update-count_*_3_13_*/*.out

./mountaincar-batch.py -r 10 -s 500000
./mountaincar.py experiment-mc/*_5_5_*/*.out
./mountaincar.py experiment-mc/*_7_7_*/*.out
./mountaincar.py experiment-mc/*_9_9_*/*.out
./mountaincar.py experiment-mc/*_11_11_*/*.out
./mountaincar.py experiment-mc/*_13_13_*/*.out
./mountaincar.py experiment-mc/*_3_13_*/*.out
averages experiment-mc/specific_*/
averages experiment-mc/*_5_5_*/
averages experiment-mc/*_7_7_*/
averages experiment-mc/*_9_9_*/
averages experiment-mc/*_11_11_*/
averages experiment-mc/*_13_13_*/
averages experiment-mc/*_3_13_*/
./mountaincar.py \
	experiment-mc/specific_*_9_9_*/*.out \
	experiment-mc/inv-root-update-count_*_5_5_*/*.out \
	experiment-mc/inv-root-update-count_*_7_7_*/*.out \
	experiment-mc/inv-root-update-count_*_9_9_*/*.out \
	experiment-mc/inv-log-update-count_*_11_11_*/*.out \
  experiment-mc/inv-log-update-count_*_13_13_*/*.out \
	experiment-mc/inv-root-update-count_*_3_13_*/*.out
