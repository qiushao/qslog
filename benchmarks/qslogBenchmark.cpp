#include "qslog/FileSink.h"
#include "qslog/Logger.h"
#include "qslog/MmapSink.h"
#include "qslog/StdoutSink.h"
#include <benchmark/benchmark.h>

static void bmStdoutSink(benchmark::State &state) {
    auto consoleSink = std::make_shared<qslog::StdoutSink>("console");
    qslog::Logger::addSink(consoleSink);
    uint64_t count = 0;
    for (auto _: state) {
        qslog::Logger::log(qslog::LogLevel::DEBUG, "tag", "hello world it count {}", ++count);
    }
}
//BENCHMARK(bmStdoutSink);

static void bmFileSink(benchmark::State &state) {
    auto fileSink = std::make_shared<qslog::FileSink>("file", "fileSinkBench.log", true);
    qslog::Logger::addSink(fileSink);
    uint64_t count = 0;
    for (auto _: state) {
        qslog::Logger::log(qslog::LogLevel::DEBUG, "tag", "hello world it count {}", ++count);
    }
}
BENCHMARK(bmFileSink);

std::shared_ptr<qslog::MmapSink> sink = nullptr;
static int init() {
    sink = std::make_shared<qslog::MmapSink>("mmap", "mmapSinkBench.log", true);
    qslog::Logger::addSink(sink);
    return 0;
}
static void bmMmapSink(benchmark::State &state) {
    printf("bmMmapSink\n");
    static uint64_t count = init();
    for (auto _: state) {
        qslog::Logger::log(qslog::LogLevel::DEBUG, "tag", "hello world it count {}\n", ++count);
    }
}
BENCHMARK(bmMmapSink);

BENCHMARK_MAIN();