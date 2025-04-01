#define QSLOG_TAG "example"

#include "qslog/CompressFileSink.h"
#include "qslog/Logger.h"
#include "qslog/StdoutSink.h"
#include <csignal>

int main() {
    auto consoleSink = std::make_shared<qslog::StdoutSink>("console");
    auto fileSink = std::make_shared<qslog::CompressFileSink>("FileSink", "example.log", true);
    qslog::Logger::addSink(consoleSink);
    qslog::Logger::addSink(fileSink);
    for (int i = 0; i < 10; ++i) {
        QSLOGD("hello qslog {}", i);
    }
    qslog::Logger::sync();
    return 0;
}