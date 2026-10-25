#!/bin/bash
#source ../source_this
pushd ./pdp11_src
make clean ; make
popd
make -C build
./sram.sh $PWD/build/src/initial_test.elf
