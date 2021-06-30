#!/bin/bash

mpic++ --prefix /usr/local/share/OpenMPI -o ots ots.cpp

dd if=/dev/random bs=1 count=$1 of=numbers > /dev/null 2>&1

mpirun --prefix /usr/local/share/OpenMPI -np $1 ots

rm -f ots numbers