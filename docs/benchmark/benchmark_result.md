# env setup
before run benchmark, please set cpu governor to performance
```shell
cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
```

after run benchmark, reset cpu governor to powersave
```shell
echo schedutil | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
```


# result
```shell
2025-03-19T23:32:05+08:00
Running /home/jingle/projects/clion/qslog/cmake-build-release/benchmarks/qslogBenchmark
Run on (24 X 4236.04 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x12)
  L1 Instruction 32 KiB (x12)
  L2 Unified 512 KiB (x12)
  L3 Unified 16384 KiB (x4)
Load Average: 1.56, 0.74, 0.28
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
bmFileSink        368 ns          368 ns      1887624

Process finished with exit code 0
```


# 性能热点分析
```shell
sudo perf record -g ./qslogBenchmark  #数据采集
sudo chmod 777 perf.data
perf report # 生成数据报告
```

## 生成火焰图
```shell
git clone https://github.com/brendangregg/FlameGraph.git
export PATH=$PATH:$PWD/FlameGraph
perf script -i perf.data &> perf.unfold
stackcollapse-perf.pl perf.unfold &> perf.folded
flamegraph.pl perf.folded > perf.svg
```

FileSink 使用 fmt 来格式化，写入文件加缓存之后，性能翻了十几倍。
从火焰图上看，文件写入占 16%， fmt 格式化相关操作占了大头，加起来有 50% 左右的时间。 光是时间的格式化都占了 10%.
