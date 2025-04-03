#ifndef QSLOG_LOGENTRY_H
#define QSLOG_LOGENTRY_H

#include "fmt/args.h"
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

    void formatLogEntry(fmt::memory_buffer &buf) const;

    static std::string parserMsg(const std::vector<uint8_t> &buffer, const std::string &format);

    static bool extractArgs(const std::vector<uint8_t> &buffer, fmt::dynamic_format_arg_store<fmt::format_context> &argStore);
};

}// namespace qslog

#endif//QSLOG_LOGENTRY_H
