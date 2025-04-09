#include "qslog/FileSink.h"
#include <sstream>

namespace qslog {

FileSink::FileSink(std::string_view name, std::string_view fileName, bool truncate)
    : BaseSink(name), fileName_(fileName) {
    openFile(truncate);
}

FileSink::~FileSink() {
    if (outFile_.is_open()) {
        doSync();
        outFile_.close();
    }
}

void FileSink::log(LogEntry &entry) {
    if (!outFile_.is_open()) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        entry.formatLogEntry(buf_);
        buf_.push_back('\n');
        if (buf_.size() > kBufferSize) {
            outFile_.write(buf_.data(), buf_.size());
            buf_.resize(0);
        }
    }
}

void FileSink::openFile(bool truncate) {
    std::ios_base::openmode mode = std::ios::out;
    if (truncate) {
        mode |= std::ios::trunc;
    } else {
        mode |= std::ios::app;
    }
    outFile_.open(fileName_, mode);
}

void FileSink::sync() {
    doSync();
}

void FileSink::doSync() {
    if (buf_.size() > 0) {
        outFile_.write(buf_.data(), buf_.size());
        buf_.resize(0);
    }
    outFile_.flush();
}

}// namespace qslog