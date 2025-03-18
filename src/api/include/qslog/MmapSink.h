#ifndef QSLOG_MMAPSINK_H
#define QSLOG_MMAPSINK_H

#include "qslog/BaseSink.h"
#include <string_view>

namespace qslog {

class MmapSink : public BaseSink {
public:
    MmapSink(std::string_view name, std::string_view fileName, bool truncate = false);

    virtual ~MmapSink();

    void log(const LogEntry &entry) override;

private:
    void extendFile();

private:
    static constexpr int kHeaderSize_ = 128;
    static constexpr int kChunkSize_ = 256 * 1024;
    int pageSize_ = 0;
    std::string_view fileName_;
    bool truncate_;
    size_t fileSize_ = 0;
    size_t actualUsedSize_ = 0;
    uint8_t *headerPtr_ = nullptr;
    uint8_t *dataPtr_ = nullptr;
    size_t curPos_ = 0;
};

}// namespace qslog

#endif//QSLOG_MMAPSINK_H
