#!/bin/bash
set -e
git submodule update --init --recursive
mkdir -p build
pushd build
cmake ..
make -j
popd
ln -s Debug/libbass.so .
