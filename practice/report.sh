#!/bin/sh

for i in *.in; do
  ./practice $i > $i.out 2> /dev/null
  echo "$i --> $(head -1 $i.out)"
done
