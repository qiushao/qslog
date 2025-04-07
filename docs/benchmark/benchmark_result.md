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
2025-04-07T18:09:36+08:00
Running ./qslogBenchmark
Run on (24 X 3597.28 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x12)
  L1 Instruction 32 KiB (x12)
  L2 Unified 512 KiB (x12)
  L3 Unified 16384 KiB (x4)
Load Average: 0.08, 0.36, 0.50
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
bmFileSink       80.4 ns         80.4 ns      9001090
[ perf record: Woken up 2 times to write data ]
[ perf record: Captured and wrote 0.312 MB perf.data (3392 samples) ]
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

在 release 模式下运行 benchmark，有时候会出现一些系统调用很占时间，但又没有调用堆栈，这时候可以使用 debug 模式来跑 benchmark， 可以看到完整的调用堆栈。
