#!/bin/bash

cd "$(dirname $0)"

cargo build --release
for f in *.in; do
  ./target/release/hashcode $f > $f.out
done
