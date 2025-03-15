#ifndef QSLOG_LOGGER_H
#define QSLOG_LOGGER_H

#include "fmt/format.h"
#include "qslog/BaseSink.h"
#include "qslog/LogEntry.h"
#include "qslog/common.h"
#include <memory>

namespace qslog {


typedef std::function<void(const LogEntry &entry)> LogHandler;

class Logger {
public:
    static void setLogLevel(LogLevel level);

    static LogLevel getLogLevel();

    static void setLogHandler(LogHandler handler);

    static void addSink(std::shared_ptr<BaseSink> sink);

    template<typename... Args>
    static void log(LogLevel level, std::string_view tag, std::string_view format, Args &&...args) {
        if (level < logLevel_) {
            return;
        }

        fmt::memory_buffer buf;
        fmt::vformat_to(fmt::appender(buf), format, fmt::make_format_args(args...));
        LogEntry entry(level, tag, std::string_view(buf.data(), buf.size()));

        if (logHandler_) {
            logHandler_(entry);
            return;
        }

        for (auto &sink: sinks_) {
            sink->log(entry);
        }
    }

private:
    static LogLevel logLevel_;
    static LogHandler logHandler_;
    static std::vector<std::shared_ptr<BaseSink>> sinks_;
};

}// namespace qslog

#endif//QSLOG_LOGGER_H
