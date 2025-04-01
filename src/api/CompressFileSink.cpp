#include "qslog/CompressFileSink.h"
#include <sstream>

namespace qslog {

CompressFileSink::CompressFileSink(std::string_view name, std::string_view fileName, bool truncate)
    : BaseSink(name), fileName_(fileName) {
    openFile(truncate);
}

CompressFileSink::~CompressFileSink() {
    if (outFile_.is_open()) {
        doSync();
        outFile_.close();
    }
}

void CompressFileSink::log(const LogEntry &entry) {
    if (!outFile_.is_open()) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        uint32_t formatId = getFormatId(entry.format_);

        write(&entry.time_, sizeof(entry.time_));
        write(&entry.pid_, sizeof(entry.pid_));
        write(&entry.tid_, sizeof(entry.tid_));
        write(entry.sourceLocation_.data(), entry.sourceLocation_.length());
        write(entry.tag_.data(), entry.tag_.length());
        write(&formatId, sizeof(formatId));
        write(entry.argStore_.data(), entry.argStore_.size());
    }
}

void CompressFileSink::openFile(bool truncate) {
    std::ios_base::openmode mode = std::ios::out;
    if (truncate) {
        mode |= std::ios::trunc;
    } else {
        mode |= std::ios::app;
    }
    outFile_.open(fileName_, mode);
}

void CompressFileSink::write(const void *data, uint32_t size) {
    if (pos_ + size > kBufferSize) {
        outFile_.write(reinterpret_cast<const char *>(buf_), pos_);
        pos_ = 0;
    }
    memcpy(buf_ + pos_, data, size);
    pos_ += size;
}

uint32_t CompressFileSink::getFormatId(const std::string& format) {
    uint32_t formatId = 0;
    auto it = formatIdMap_.find(format);
    if (it != formatIdMap_.end()) {
        formatId = it->second;
    } else {
        formatId = formatIdMap_.size();
        formatIdMap_[format] = formatId;
    }
    return formatId;
}

void CompressFileSink::sync() {
    doSync();
}

void CompressFileSink::doSync() {
    if (pos_ > 0) {
        outFile_.write(reinterpret_cast<const char *>(buf_), pos_);
        pos_ = 0;
    }
    outFile_.flush();
}

}// namespace qslog