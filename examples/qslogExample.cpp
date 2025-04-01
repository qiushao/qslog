#define QSLOG_TAG "example"

#include "qslog/FileSink.h"
#include "qslog/Logger.h"
#include "qslog/StdoutSink.h"

int main() {
    auto consoleSink = std::make_shared<qslog::StdoutSink>("console");
    auto fileSink = std::make_shared<qslog::FileSink>("FileSink", "example.log", true);
    qslog::Logger::addSink(consoleSink);
    //    qslog::Logger::addSink(fileSink);
    for (int i = 0; i < 10; ++i) {
        QSLOGD("hello qslog {} {}", i, "test");
    }
    return 0;
}