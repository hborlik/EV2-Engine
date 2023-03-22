#!/bin/bash

cd "$(dirname "$0")"

cd ..

sudo perf record -F max -g -- ./build/3d_tree_ex/3d_tree_ex

#--call-graph dwarf
# perf script | ./stackcollapse-perf.pl --all |./flamegraph.pl > perf_3d_plant_ex.svg