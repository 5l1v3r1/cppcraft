#!/bin/bash
set -e
pushd ../build
if [ "$(expr substr $(uname -s) 1 10)" == "MINGW64_NT" ]; then
    mingw32-make -j
else
    make -j
fi
popd
../build/cppcraft
