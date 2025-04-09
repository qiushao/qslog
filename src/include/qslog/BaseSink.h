#ifndef QSLOG_BASESINK_H
#define QSLOG_BASESINK_H

#include "qslog/LogEntry.h"
#include "qslog/common.h"

namespace qslog {

class BaseSink {
public:
    explicit BaseSink(std::string_view sinkName);

    std::string_view getName();

    virtual void log(LogEntry &msg) = 0;

    virtual void sync() {}

private:
    std::string_view sinkName_;
};

}// namespace qslog

#endif//QSLOG_BASESINK_H
