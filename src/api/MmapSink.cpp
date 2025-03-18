#include "qslog/MmapSink.h"
#include "FileUtils.h"
#include "OSUtils.h"
#include <csignal>
#include <cstring>
#include <sys/mman.h>

namespace qslog {

MmapSink::MmapSink(std::string_view name, std::string_view fileName, bool truncate)
    : BaseSink(name), fileName_(fileName), truncate_(truncate) {
    pageSize_ = sysconf(_SC_PAGESIZE);
    fileSize_ = FileUtils::getFileSize(fileName_);
    printf("fileSize = %zu\n", fileSize_);
    int logFd = -1;
    if (fileSize_ == 0 || truncate_) {
        logFd = FileUtils::openFileToWrite(fileName_, truncate_);
        fileSize_ = kChunkSize_;
        actualUsedSize_ = kHeaderSize_;
        ftruncate(logFd, fileSize_);
        dataPtr_ = FileUtils::mapFileToWrite(logFd, 0, kChunkSize_);
        headerPtr_ = FileUtils::mapFileToWrite(logFd, 0, pageSize_);
        ;
        curPos_ = kHeaderSize_;
    } else {
        FILE *fp = fopen(fileName_.data(), "r");
        fscanf(fp, "%zu\n", &actualUsedSize_);
        fclose(fp);
        printf("actualUsedSize_ = %zu\n", actualUsedSize_);

        logFd = FileUtils::openFileToWrite(fileName_, truncate_);
        size_t curChunkStart = ((actualUsedSize_ / kChunkSize_) * kChunkSize_);
        dataPtr_ = FileUtils::mapFileToWrite(logFd, curChunkStart, kChunkSize_);
        headerPtr_ = FileUtils::mapFileToWrite(logFd, 0, pageSize_);
        curPos_ = actualUsedSize_ % kChunkSize_;
    }
    headerPtr_[kHeaderSize_ - 1] = '\n';
    close(logFd);
}

MmapSink::~MmapSink() {
    printf("~MmapSink\n");
    if (dataPtr_) {
        munmap(dataPtr_, kChunkSize_);
    }
    if (headerPtr_) {
        munmap(headerPtr_, pageSize_);
    }
}

void MmapSink::log(const LogEntry &entry) {
    if (dataPtr_ == nullptr || headerPtr_ == nullptr) {
        return;
    }
    auto msgSize = entry.msg.size();
    size_t offset = 0;
    while (msgSize > 0) {
        size_t available = kChunkSize_ - curPos_;
        if (available == 0) {
            extendFile();
            available = kChunkSize_;
        }
        size_t toWrite = std::min(msgSize, available);
        memcpy(dataPtr_ + curPos_, entry.msg.data() + offset, toWrite);
        curPos_ += toWrite;
        msgSize -= toWrite;
        offset += toWrite;
    }
    actualUsedSize_ += entry.msg.size();
    sprintf(reinterpret_cast<char *>(headerPtr_), "%zu\n", actualUsedSize_);
}

void MmapSink::extendFile() {
    printf("extendFile\n");
    munmap(dataPtr_, kChunkSize_);
    fileSize_ += kChunkSize_;
    int logFd = FileUtils::openFileToWrite(fileName_, false);
    ftruncate(logFd, fileSize_);

    dataPtr_ = FileUtils::mapFileToWrite(logFd, fileSize_ - kChunkSize_, kChunkSize_);
    curPos_ = 0;
}
}// namespace qslog