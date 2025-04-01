#!/bin/bash

set -e

SCRIPT_DIR=$(cd $(dirname ${BASH_SOURCE[0]});pwd)
cd ${SCRIPT_DIR}/..
pwd

echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

#rm -rf build
#mkdir build
#cd build
#cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
#make -j8

cd cmake-build-release/benchmarks
sudo perf record -g ./qslogBenchmark
sudo chmod 777 perf.data
perf script -i perf.data &> perf.unfold
stackcollapse-perf.pl perf.unfold &> perf.folded
flamegraph.pl perf.folded > perf.svg

cp perf.svg ${SCRIPT_DIR}/../docs/benchmark/perf.svg