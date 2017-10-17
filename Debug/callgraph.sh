#!/bin/bash
gprof ../build/cppcraft | gprof2dot | dot -Tpng -o callgraph.png
