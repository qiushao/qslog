#ifndef QSLOG_LOGENTRY_H
#define QSLOG_LOGENTRY_H

#include "fmt/args.h"
#include "qslog/common.h"
#include <chrono>

namespace qslog {

struct FormatEntry {
    uint8_t logLevel_;
    uint8_t argc_;
    uint16_t formatId_;
    std::vector<uint8_t> argTypes_;
    std::string formatStr_;//format str = tag [file:line function] format
};

struct LogEntry {
    uint16_t formatId_ = 0;
    uint64_t time_ = 0;
    uint32_t tid_ = 0;
    size_t argsSize_ = 0;
    uint8_t *argStore_ = nullptr;

    std::string formatLogEntry();

    void formatLogEntry(fmt::memory_buffer &buf);

    std::string parserMsg(const std::string &format);

    bool extractArgs(fmt::dynamic_format_arg_store<fmt::format_context> &argStore);
};

}// namespace qslog

#endif//QSLOG_LOGENTRY_H
