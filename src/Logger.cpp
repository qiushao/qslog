#include "qslog/Logger.h"
#include "fmt/format.h"

namespace qslog {

LogLevel Logger::logLevel_ = LogLevel::DEBUG;
std::vector<std::shared_ptr<BaseSink>> Logger::sinks_;

void Logger::setLogLevel(LogLevel level) {
    logLevel_ = level;
}

LogLevel Logger::getLogLevel() {
    return logLevel_;
}

void Logger::addSink(std::shared_ptr<BaseSink> sink) {
    sinks_.push_back(std::move(sink));
}

void Logger::sync() {
    for (auto &sink: sinks_) {
        sink->sync();
    }
}

}// namespace qslog