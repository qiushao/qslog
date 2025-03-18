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
2025-03-19T00:00:18+08:00
Running /home/jingle/projects/clion/qslog/cmake-build-release/benchmarks/qslogBenchmark
Run on (24 X 4276.52 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x12)
  L1 Instruction 32 KiB (x12)
  L2 Unified 512 KiB (x12)
  L3 Unified 16384 KiB (x4)
Load Average: 0.15, 0.60, 0.68
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
bmFileSink       2179 ns         2178 ns       372675

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

看 perf.svg，程序执行的 72% 时间在 __GI___libc_write 函数中，也就是标准库的文件写入函数中。
所以优化的焦点是文件的写入，尝试 mmap。