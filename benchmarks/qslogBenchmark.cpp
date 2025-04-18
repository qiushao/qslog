#define QSLOG_TAG "becnmark"

#include "qslog/CompressFileSink.h"
#include "qslog/Logger.h"
#include <benchmark/benchmark.h>

static void bmQslogCompressFileSink(benchmark::State &state) {
    static std::once_flag flag;
    std::call_once(flag, [] {
        qslog::Logger::init();
        auto sink = std::make_shared<qslog::CompressFileSink>("file", "qslog-bench.log", true);
        qslog::Logger::addSink(sink);
    });
    static thread_local uint64_t count = 0;
    for (auto _: state) {
        QSLOGD("compress sink message #{}", ++count);
    }
}
BENCHMARK(bmQslogCompressFileSink)->ThreadRange(1, 8);


BENCHMARK_MAIN();