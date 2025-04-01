#define QSLOG_TAG "becnmark"

#include "qslog/FileSink.h"
#include "qslog/Logger.h"
#include <benchmark/benchmark.h>

std::shared_ptr<qslog::FileSink> fileSink = nullptr;
static int initFileSink() {
    fileSink = std::make_shared<qslog::FileSink>("file", "fileSinkBench.log", true);
    qslog::Logger::addSink(fileSink);
    return 0;
}
static void bmFileSink(benchmark::State &state) {
    static uint64_t count = initFileSink();
    for (auto _: state) {
        QSLOGD("hello world it count {}", ++count);
    }
}
BENCHMARK(bmFileSink);


BENCHMARK_MAIN();