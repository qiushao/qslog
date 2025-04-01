#ifndef QSLOG_LOGENTRY_H
#define QSLOG_LOGENTRY_H

#include "qslog/common.h"
#include <chrono>

namespace qslog {

struct LogEntry {
    LogLevel level_;
    uint32_t time_;
    int32_t pid_;
    int32_t tid_;
    uint16_t line_;
    std::string file_;
    std::string tag_;
    std::string format_;
    std::vector<uint8_t> argStore_;

    LogEntry(std::string_view file, uint16_t line, LogLevel level, std::string_view tag, std::string_view format, std::vector<uint8_t> args);

    std::string formatLogEntry() const;

    std::string parserMsg() const;
};

}// namespace qslog

#endif//QSLOG_LOGENTRY_H
