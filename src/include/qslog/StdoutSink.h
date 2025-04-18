#ifndef QSLOG_STDOUTSINK_H
#define QSLOG_STDOUTSINK_H

#include "qslog/BaseSink.h"
#include <iostream>

namespace qslog {

class StdoutSink : public BaseSink {
public:
    explicit StdoutSink(std::string_view name) : BaseSink(name) {}

    void log(LogEntry &entry) override {
        std::cout << entry.formatLogEntry() << std::endl;
    }
};

}// namespace qslog

#endif//QSLOG_STDOUTSINK_H
