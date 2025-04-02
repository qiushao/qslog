#ifndef QSLOG_COMMON_H
#define QSLOG_COMMON_H

#include <array>
#include <chrono>
#include <functional>
#include <string>

namespace qslog {

enum LogLevel {
    ALL = 0,
    VERBOSE = 1,
    DEBUG = 2,
    INFO = 3,
    WARN = 4,
    ERROR = 5,
    FATAL = 6,
    SILENT = 7
};

char getLevelName(LogLevel level);

std::string formatTimespec(uint64_t ts);

}// namespace qslog
#endif//QSLOG_COMMON_H