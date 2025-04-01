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
    void log(const LogEntry &entry) override;
    void sync() override;

private:
    void write(const void *data, uint32_t size);
    uint32_t getFormatId(const std::string& format);
    void doSync();

    static constexpr int kBufferSize = 512 * 1024;
    void openFile(bool truncate);
    std::ofstream outFile_;
    std::string fileName_;
    uint8_t buf_[kBufferSize]{0};
    uint32_t pos_ = 0;
    std::mutex mutex_;
    std::unordered_map<std::string, uint32_t> formatIdMap_;
};

}// namespace qslog

#endif//QSLOG_COMPRESSFILESINK_H