#ifndef QSLOG_BASESINK_H
#define QSLOG_BASESINK_H

#include "qslog/LogEntry.h"
#include "qslog/common.h"

namespace qslog {

class BaseSink {
public:
    explicit BaseSink(std::string_view sinkName);

    std::string_view getName();

    // 二进制压缩数据
    virtual void log(LogEntry &msg) {}

    // 直接输出，stdout sink, android logcat sink
    virtual void log(LogLevel level, const char *tag, const char *file, uint16_t line, const char *function, const char *msg) {}

    virtual bool isBinarySink() {return true;}

    virtual void sync() {}

private:
    std::string_view sinkName_;
};

}// namespace qslog

#endif//QSLOG_BASESINK_H
