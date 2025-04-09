#ifndef QSLOG_LOGENTRY_H
#define QSLOG_LOGENTRY_H

#include "fmt/args.h"
#include "qslog/common.h"
#include <chrono>

namespace qslog {

struct FormatEntry {
    uint8_t entryType_;
    uint8_t logLevel_;
    uint8_t argc_;
    uint16_t formatId_;
    std::vector<uint8_t> argTypes_;
    std::string formatStr_;//format str = tag [file:line function] format
};

struct LogEntry {
    uint16_t formatId_;
    uint64_t time_;
    uint32_t pid_;
    uint32_t tid_;
    std::vector<uint8_t> argStore_;

    std::string formatLogEntry();

    void formatLogEntry(fmt::memory_buffer &buf);

    std::string parserMsg(const std::vector<uint8_t> &buffer, const std::string &format);

    bool extractArgs(const std::vector<uint8_t> &buffer, fmt::dynamic_format_arg_store<fmt::format_context> &argStore);
};

}// namespace qslog

#endif//QSLOG_LOGENTRY_H
