#!/bin/sh
if [ ! -d "./build/" ]; then
    mkdir build
fi

cd build
rm * -rf
cmake ../ -DCMAKE_BUILD_TYPE=Debug
make -j