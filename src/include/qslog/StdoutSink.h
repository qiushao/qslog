#ifndef QSLOG_STDOUTSINK_H
#define QSLOG_STDOUTSINK_H

#include "qslog/BaseSink.h"
#include <iostream>

namespace qslog {

class StdoutSink : public BaseSink {
public:
    explicit StdoutSink(std::string_view name) : BaseSink(name) {}

    void log(LogLevel level, const char *tag, const char *file, uint16_t line, const char *function, const char *msg) override;

    void log(LogEntry &entry) override {
        std::cout << entry.formatLogEntry() << std::endl;
    }
};

}// namespace qslog

#endif//QSLOG_STDOUTSINK_H
