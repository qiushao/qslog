#ifndef QSLOG_OSUTILS_H
#define QSLOG_OSUTILS_H

#include <cstdint>
#include <cstdio>
namespace qslog {

class OSUtils {
public:
    static int32_t getPid();

    static int32_t getTid();

    static uint64_t realTimeNanosecond();
};

}// namespace qslog

#endif//QSLOG_OSUTILS_H