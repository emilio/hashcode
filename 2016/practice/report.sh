#!/bin/sh

TO_TRY="practice.py practice"
INPUTS="logo right_angle learn_and_teach"

for program in $TO_TRY; do
  echo "# $program"
  for i in $INPUTS; do
    ./$program inputs/$i.in > "outputs/$program.$i.out" 2> /dev/null
    echo "$(head -1 "outputs/$program.$i.out")    $i"
  done
  echo ""
done
