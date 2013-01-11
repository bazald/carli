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

./puddleworld-batch.py -r 1 -s 200000
averages experiment-pw/*_*/
./puddleworld.py experiment-pw/*/*.out

./mountaincar-batch.py -r 1 -s 500000
averages experiment-mc/*_*/
./mountaincar.py experiment-mc/*/*.out
