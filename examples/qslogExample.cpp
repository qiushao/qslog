#define QSLOG_TAG "example"

#include "qslog/CompressFileSink.h"
#include "qslog/Logger.h"
#include "qslog/StdoutSink.h"
#include <csignal>

int main() {
    auto consoleSink = std::make_shared<qslog::StdoutSink>("console");
    auto fileSink = std::make_shared<qslog::CompressFileSink>("FileSink", "example.log", true);
    qslog::Logger::addSink(consoleSink);
    //qslog::Logger::addSink(fileSink);
    uint32_t pos = 168;
    int ng = -10;
    char c = 'D';
    float f = 1.34;
    double d = 1.3415926;
    QSLOGD("hello qslog {} {} {} {} {}", pos, ng, c, f, d);
    qslog::Logger::sync();
    return 0;
}