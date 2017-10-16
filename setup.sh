#!/bin/bash
sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev
set -e
git submodule update --init --recursive
mkdir -p build
pushd build
cmake ..
make -j
popd
ln -s Debug/libbass.so .
