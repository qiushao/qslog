#ifndef QSLOG_LOGENTRY_H
#define QSLOG_LOGENTRY_H

#include "qslog/common.h"
#include <chrono>
namespace qslog {

struct LogEntry {
    std::chrono::system_clock::time_point time;
    int32_t tid;
    LogLevel level;
    std::string_view tag;
    std::string_view msg;

    LogEntry(LogLevel level, std::string_view tag, std::string_view msg);
};

}// namespace qslog

#endif//QSLOG_LOGENTRY_H
