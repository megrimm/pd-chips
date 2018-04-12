#!/bin/bash

for p in atari2600~ atari5200~ autotuned~ colecovision~ spaces~ vocoder~
do
echo -n "building $p..."
cd $p
make clean
make
make install
cd ..
echo "done."
done
