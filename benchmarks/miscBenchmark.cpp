#include <ctime>
#include <benchmark/benchmark.h>
#include "qslog/tscns.h"

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
BENCHMARK(clockGetTimeBench);

static void rdtscBench(benchmark::State &state) {
    for (auto _: state) {
        benchmark::DoNotOptimize(TSCNS::rdtsc());
    }
}
BENCHMARK(rdtscBench);

BENCHMARK_MAIN();