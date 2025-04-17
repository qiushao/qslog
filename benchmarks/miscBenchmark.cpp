#include <ctime>
#include <benchmark/benchmark.h>
#include "qslog/tscns.h"
#include <fmt/core.h>
#include <fmt/chrono.h>

uint64_t realTimeNanosecond() {
    struct timespec ts{};
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t(ts.tv_sec) * 1000000000LL + ts.tv_nsec);
}

static void clockGetTimeBench(benchmark::State &state) {
    for (auto _: state) {
        benchmark::DoNotOptimize(realTimeNanosecond());
    }
}
BENCHMARK(clockGetTimeBench)->ThreadRange(1, 8);

static void rdtscBench(benchmark::State &state) {
    for (auto _: state) {
        benchmark::DoNotOptimize(TSCNS::rdtsc());
    }
}
BENCHMARK(rdtscBench)->ThreadRange(1, 8);

static void formatInt() {
    static thread_local uint64_t count = 0;
    fmt::memory_buffer buf;
    fmt::vformat_to(fmt::appender(buf), "hello fmt i = {}", fmt::make_format_args(++count));
}
static void fmtFormatBench(benchmark::State &state) {
    for (auto _: state) {
        formatInt();
    }
}
BENCHMARK(fmtFormatBench)->ThreadRange(1, 8);

static void formatDate() {
    static thread_local auto tp = std::chrono::system_clock::now();
    fmt::memory_buffer buf;
    fmt::vformat_to(fmt::appender(buf), "{} hello fmt i", fmt::make_format_args(tp));
    // 多线程情况下 format time point 性能明显下降
    // 因为这个过程需要调用到两个系统调用 localtime_r 和 strftime，内部都是有锁的
    // fmtlog 针对这个问题有优化方案
    // 缓存 年月日， 手动计算时分秒毫秒，拼接起来就行，避免前面两个系统调用。只在日期有变化时更新下年月日即可
}
static void fmtFormatDateBench(benchmark::State &state) {
    for (auto _: state) {
        formatDate();
    }
}
BENCHMARK(fmtFormatDateBench)->ThreadRange(1, 8);

BENCHMARK_MAIN();