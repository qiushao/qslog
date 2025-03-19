#ifndef QSLOG_FILESINK_H
#define QSLOG_FILESINK_H

#include "fmt/format.h"
#include "qslog/BaseSink.h"
#include <fstream>
#include <mutex>
#include <string_view>

namespace qslog {

class FileSink : public BaseSink {
public:
    FileSink(std::string_view name, std::string_view fileName, bool truncate = false);
    virtual ~FileSink();
    void log(const LogEntry &entry) override;

private:
    static constexpr int kBufferSize = 512 * 1024;
    void openFile(bool truncate);
    std::ofstream outFile_;
    std::string fileName_;
    fmt::memory_buffer buf_;
    std::mutex mutex_;
};

}// namespace qslog

#endif//QSLOG_FILESINK_H