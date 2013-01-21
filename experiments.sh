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

./puddleworld-batch.py -r 3
./puddleworld.py experiment-pw/whiteson-policy_4/*.out
./puddleworld.py experiment-pw/whiteson-value_4/*.out
./puddleworld.py experiment-pw/*_0/*.out
./memory.py experiment-pw/*_0/*.out
./puddleworld.py experiment-pw/*_1/*.out
./memory.py experiment-pw/*_1/*.out
./puddleworld.py experiment-pw/inv-log-*_2/*.out
./memory.py experiment-pw/inv-log-*_2/*.out
./puddleworld.py experiment-pw/inv-root-*_2/*.out
./memory.py experiment-pw/inv-root-*_2/*.out
./puddleworld.py experiment-pw/*_3/*.out
./memory.py experiment-pw/*_3/*.out
./puddleworld.py experiment-pw/*_4/*.out
./puddleworld.py experiment-pw/*_100/*.out
./memory.py experiment-pw/*_100/*.out
./puddleworld.py experiment-pw/*_101/*.out
./memory.py experiment-pw/*_101/*.out
./puddleworld.py experiment-pw/*_110/*.out
./memory.py experiment-pw/*_110/*.out
./puddleworld.py experiment-pw/*_111/*.out
./memory.py experiment-pw/*_111/*.out
./puddleworld.py experiment-pw/*_120/*.out
./memory.py experiment-pw/*_120/*.out
./puddleworld.py experiment-pw/*_121/*.out
./memory.py experiment-pw/*_121/*.out
./puddleworld.py experiment-pw/*_130/*.out
./memory.py experiment-pw/*_130/*.out
./puddleworld.py experiment-pw/*_131/*.out
./memory.py experiment-pw/*_131/*.out
./puddleworld.py experiment-pw/*_200/*.out
./memory.py experiment-pw/*_200/*.out
./puddleworld.py experiment-pw/*_201/*.out
./memory.py experiment-pw/*_201/*.out
./puddleworld.py experiment-pw/*_210/*.out
./memory.py experiment-pw/*_210/*.out
./puddleworld.py experiment-pw/*_211/*.out
./memory.py experiment-pw/*_211/*.out
./puddleworld.py experiment-pw/*_220/*.out
./memory.py experiment-pw/*_220/*.out
./puddleworld.py experiment-pw/*_221/*.out
./memory.py experiment-pw/*_221/*.out
./puddleworld.py experiment-pw/*_300/*.out
./memory.py experiment-pw/*_300/*.out
./puddleworld.py experiment-pw/*_301/*.out
./memory.py experiment-pw/*_301/*.out
./puddleworld.py experiment-pw/*_310/*.out
./memory.py experiment-pw/*_310/*.out
./puddleworld.py experiment-pw/*_311/*.out
./memory.py experiment-pw/*_311/*.out
./puddleworld.py experiment-pw/*_320/*.out
./memory.py experiment-pw/*_320/*.out
./puddleworld.py experiment-pw/*_321/*.out
./memory.py experiment-pw/*_321/*.out

./mountaincar-batch.py -r 3
./mountaincar.py experiment-mc/whiteson-policy_4/*.out
./mountaincar.py experiment-mc/whiteson-value_4/*.out
./mountaincar.py experiment-mc/*_0/*.out
./memory.py experiment-mc/*_0/*.out
./mountaincar.py experiment-mc/*_1/*.out
./memory.py experiment-mc/*_1/*.out
./mountaincar.py experiment-mc/inv-log-*_2/*.out
./memory.py experiment-mc/inv-log-*_2/*.out
./mountaincar.py experiment-mc/inv-root-*_2/*.out
./memory.py experiment-mc/inv-root-*_2/*.out
./mountaincar.py experiment-mc/*_3/*.out
./memory.py experiment-mc/*_3/*.out
./mountaincar.py experiment-mc/*_4/*.out
./mountaincar.py experiment-mc/*_400/*.out
./memory.py experiment-mc/*_400/*.out
./mountaincar.py experiment-mc/*_401/*.out
./memory.py experiment-mc/*_401/*.out
./mountaincar.py experiment-mc/*_410/*.out
./memory.py experiment-mc/*_410/*.out
./mountaincar.py experiment-mc/*_411/*.out
./memory.py experiment-mc/*_411/*.out
./mountaincar.py experiment-mc/*_420/*.out
./memory.py experiment-mc/*_420/*.out
./mountaincar.py experiment-mc/*_421/*.out
./memory.py experiment-mc/*_421/*.out

echo ""
averages experiment-pw/*_0
averages experiment-pw/*_1
averages experiment-pw/inv-log-*_2
averages experiment-pw/inv-root-*_2
averages experiment-pw/*_3
averages experiment-pw/*_4
averages experiment-pw/*_100
averages experiment-pw/*_101
averages experiment-pw/*_110
averages experiment-pw/*_111
averages experiment-pw/*_120
averages experiment-pw/*_121
averages experiment-pw/*_130
averages experiment-pw/*_131
averages experiment-pw/*_200
averages experiment-pw/*_201
averages experiment-pw/*_210
averages experiment-pw/*_211
averages experiment-pw/*_220
averages experiment-pw/*_221
averages experiment-pw/*_300
averages experiment-pw/*_301
averages experiment-pw/*_310
averages experiment-pw/*_311
averages experiment-pw/*_320
averages experiment-pw/*_321
echo ""
averages experiment-mc/*_0
averages experiment-mc/*_1
averages experiment-mc/inv-log-*_2
averages experiment-mc/inv-root-*_2
averages experiment-mc/*_3
averages experiment-mc/*_4
averages experiment-mc/*_400
averages experiment-mc/*_401
averages experiment-mc/*_410
averages experiment-mc/*_411
averages experiment-mc/*_420
averages experiment-mc/*_421
echo ""
