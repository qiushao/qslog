#define QSLOG_TAG "becnmark"

#include "qslog/CompressFileSink.h"
#include "qslog/Logger.h"
#include <benchmark/benchmark.h>

std::shared_ptr<qslog::CompressFileSink> fileSink = nullptr;
static int initFileSink() {
    fileSink = std::make_shared<qslog::CompressFileSink>("file", "compressFileSinkBench.log", true);
    qslog::Logger::addSink(fileSink);
    return 0;
}
static void bmFileSink(benchmark::State &state) {
    static uint64_t count = initFileSink();
    static bool flag = false;
    for (auto _: state) {
        QSLOGD("hello world it count = {} {}", ++count, flag);
    }
}
BENCHMARK(bmFileSink);


BENCHMARK_MAIN();