#define QSLOG_TAG "example"

#include "qslog/FileSink.h"
#include "qslog/Logger.h"
#include "qslog/MmapSink.h"
#include "qslog/StdoutSink.h"

int main() {
    auto consoleSink = std::make_shared<qslog::StdoutSink>("console");
    auto fileSink = std::make_shared<qslog::FileSink>("FileSink", "example.log", true);
    auto mmapSink = std::make_shared<qslog::MmapSink>("MmapSink", "example-mmap.log", true);
    qslog::Logger::addSink(consoleSink);
    //    qslog::Logger::addSink(fileSink);
    //    qslog::Logger::addSink(mmapSink);
    for (int i = 0; i < 10; ++i) {
        QSLOGD("hello qslog {} {}", i, "test");
    }
    return 0;
}