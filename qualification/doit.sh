#!/bin/bash

for i in input/*.in; do
  ./qualification $i $i.out
  lines=$(cat $i.out | wc -l)
  echo $lines > output/$(basename $i).out
  cat $i.out >> output/$(basename $i).out
  rm $i.out
done
