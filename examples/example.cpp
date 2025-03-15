#include "qslog/FileSink.h"
#include "qslog/Logger.h"
#include "qslog/StdoutSink.h"

int main() {
    auto consoleSink = std::make_shared<qslog::StdoutSink>("console");
    auto fileSink = std::make_shared<qslog::FileSink>("console", "test.log", true);
    qslog::Logger::addSink(consoleSink);
    qslog::Logger::addSink(fileSink);
    for (int i = 0; i < 10; ++i) {
        qslog::Logger::log(qslog::INFO, "qiushao", "Hello qslog, count {}",  i);
    }
    return 0;
}