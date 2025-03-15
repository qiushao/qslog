#ifndef QSLOG_FILESINK_H
#define QSLOG_FILESINK_H

#include "qslog/BaseSink.h"
#include <fstream>
#include <string_view>

namespace qslog {

class FileSink : public BaseSink {
public:
    FileSink(std::string_view name, std::string_view fileName, bool truncate = false);
    virtual ~FileSink();
    void log(const LogEntry &entry) override;

private:
    void openFile(bool truncate);
    std::ofstream outFile_;
    std::string fileName_;
};

}// namespace qslog

#endif//QSLOG_FILESINK_H