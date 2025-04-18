#ifndef QSLOG_COMPRESSFILESINK_H
#define QSLOG_COMPRESSFILESINK_H

#include "fmt/format.h"
#include "qslog/BaseSink.h"
#include <fstream>
#include <mutex>
#include <string_view>

namespace qslog {

class CompressFileSink : public BaseSink {
public:
    CompressFileSink(std::string_view name, std::string_view fileName, bool truncate = false);
    virtual ~CompressFileSink();
    void log(LogEntry &entry) override;
    void sync() override;

private:
    void openFile(bool truncate);
    void doSync();
    void writeBuffer(const void *data, uint32_t size);
    void writePidInfoEntry();
    void writeTsInfoEntry();
    void writeFormatEntry(std::shared_ptr<FormatEntry> formatEntry);
    void writeLogEntry(const LogEntry &entry, uint16_t tsDiff);

    static constexpr int kBufferSize = 512 * 1024;
    std::ofstream outFile_;
    std::string fileName_;
    uint8_t buf_[kBufferSize]{0};
    uint32_t pos_ = 0;
    std::mutex mutex_;
    uint64_t lastTs_ = 0;
};

}// namespace qslog

#endif//QSLOG_COMPRESSFILESINK_H