#!/bin/bash

set -e

cfgopts="--with-debug --with-debug2"

./configure-full
make clean
make
./y

for opt in $cfgopts; do
    make clean
    ./configure-full $opt
    make
    ./y
done

echo ">> Congraturations !"
