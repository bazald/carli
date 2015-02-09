#!/bin/bash

sh -c 'tempfile' &> /dev/null
if [ $? -eq 0 ]
then
  TEMPFILE=$(tempfile -s .svg)
else
  TEMPFILE=".$ROOT.$RANDOM.svg"
fi

for DOTFILE in *.dot; do
  PREFIX=$(echo $DOTFILE | sed s/\.dot//)
  dot      -Tsvg $DOTFILE  -o $TEMPFILE
  inkscape -f    $TEMPFILE -A $PREFIX.pdf
done
