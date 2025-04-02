#ifndef QSLOG_LOGENTRY_H
#define QSLOG_LOGENTRY_H

#include "qslog/common.h"
#include <chrono>

namespace qslog {

struct LogEntry {
    LogLevel level_;
    uint64_t time_;
    int32_t tid_;
    std::string format_;
    uint8_t argc_;
    std::vector<uint8_t> argStore_;

    LogEntry(LogLevel level, std::string_view format, uint8_t argc, std::vector<uint8_t> args);

    std::string formatLogEntry() const;

    std::string parserMsg() const;
};

}// namespace qslog

#endif//QSLOG_LOGENTRY_H
