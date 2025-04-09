#define QSLOG_TAG "becnmark"

#include "qslog/CompressFileSink.h"
#include "qslog/Logger.h"
#include <benchmark/benchmark.h>

std::shared_ptr<qslog::CompressFileSink> qslogCompressFileSink = nullptr;
static int initQslogCompressFileSink() {
    qslog::Logger::init();
    qslogCompressFileSink = std::make_shared<qslog::CompressFileSink>("file", "compressFileSinkBench.log", true);
    qslog::Logger::addSink(qslogCompressFileSink);
    return 0;
}
static void bmQslogCompressFileSink(benchmark::State &state) {
    static uint64_t count = initQslogCompressFileSink();
    for (auto _: state) {
        QSLOGD("compress sink message #{}", ++count);
    }
}
BENCHMARK(bmQslogCompressFileSink);


BENCHMARK_MAIN();