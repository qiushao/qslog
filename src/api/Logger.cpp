#include "qslog/Logger.h"
#include "fmt/format.h"

namespace qslog {

LogLevel Logger::logLevel_ = LogLevel::DEBUG;
LogHandler Logger::logHandler_ = nullptr;
std::vector<std::shared_ptr<BaseSink>> Logger::sinks_;

void Logger::setLogLevel(LogLevel level) {
    logLevel_ = level;
}

LogLevel Logger::getLogLevel() {
    return logLevel_;
}

void Logger::setLogHandler(LogHandler handler) {
    logHandler_ = std::move(handler);
}

void Logger::addSink(std::shared_ptr<BaseSink> sink) {
    sinks_.push_back(std::move(sink));
}

}// namespace qslog