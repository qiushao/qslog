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
Run on (16 X 819.684 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 1280 KiB (x8)
  L3 Unified 18432 KiB (x1)
Load Average: 2.49, 1.51, 1.16
----------------------------------------------------------------------------
Benchmark                                  Time             CPU   Iterations
----------------------------------------------------------------------------
bmQslogCompressFileSink/threads:1       26.9 ns         26.9 ns     26032344
bmQslogCompressFileSink/threads:2        281 ns          276 ns      2553324
bmQslogCompressFileSink/threads:4        737 ns          616 ns      1149620
bmQslogCompressFileSink/threads:8       2242 ns         1886 ns       380032
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

使用同步模式日志，在多线程的时候性能有明显下降，看 perf 结果，基本时间都花在锁上了
但要是使用 fmtlog 那样的异步日志的话又会搞得很复杂，
