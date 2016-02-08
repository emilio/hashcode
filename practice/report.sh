#!/bin/sh

TO_TRY="practice.py practice"

for i in *.in; do
  for program in $TO_TRY; do
    ./$program $i > "$program.$i.out" 2> /dev/null
    echo "$program $i --> $(head -1 "$program.$i.out")"
  done
done
