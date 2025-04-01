#ifndef QSLOG_LOGENTRY_H
#define QSLOG_LOGENTRY_H

#include "qslog/common.h"
#include <chrono>
namespace qslog {

struct LogEntry {
    LogLevel level_;
    uint32_t time_;
    int32_t tid_;
    uint16_t line_;
    std::string_view file_;
    std::string_view tag_;
    std::string_view msg_;

    LogEntry(std::string_view file, uint16_t line, LogLevel level, std::string_view tag, std::string_view msg);
};

}// namespace qslog

#endif//QSLOG_LOGENTRY_H
