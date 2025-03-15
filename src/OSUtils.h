#ifndef QSLOG_OSUTILS_H
#define QSLOG_OSUTILS_H

#include <cstdio>
namespace qslog {

class OSUtils {
public:
    static size_t getPid();
    
    static size_t getTid();
};

}// namespace qslog

#endif//QSLOG_OSUTILS_H