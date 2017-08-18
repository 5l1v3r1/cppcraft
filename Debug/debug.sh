#!/bin/bash
pushd ../build
make -j
popd
./cppcraft
