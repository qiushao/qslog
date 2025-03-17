#include "qslog/FileSink.h"
#include "qslog/Logger.h"
#include "qslog/MmapSink.h"
#include "qslog/StdoutSink.h"

int main() {
    auto consoleSink = std::make_shared<qslog::StdoutSink>("console");
    auto fileSink = std::make_shared<qslog::FileSink>("FileSink", "example.log", false);
    auto mmapSink = std::make_shared<qslog::MmapSink>("MmapSink", "example-mmap.log", false);
    qslog::Logger::addSink(consoleSink);
    qslog::Logger::addSink(fileSink);
    qslog::Logger::addSink(mmapSink);
    for (int i = 0; i < 10; ++i) {
        qslog::Logger::log(qslog::INFO, "qiushao", "Hello qslog, count {}", i);
    }
    return 0;
}