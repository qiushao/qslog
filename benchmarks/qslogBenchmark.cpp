#include "qslog/FileSink.h"
#include "qslog/Logger.h"
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
    auto fileSink = std::make_shared<qslog::FileSink>("console", "fileSinkBench.log", true);
    qslog::Logger::addSink(fileSink);
    uint64_t count = 0;
    for (auto _: state) {
        qslog::Logger::log(qslog::LogLevel::DEBUG, "tag", "hello world it count {}", ++count);
    }
}
BENCHMARK(bmFileSink);

BENCHMARK_MAIN();