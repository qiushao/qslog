#include "qslog/CompressFileSink.h"
#include "../../FormatIdManager.h"
#include "OSUtils.h"
#include <sstream>
#include <utility>

namespace qslog {

CompressFileSink::CompressFileSink(std::string_view name, std::string_view fileName, bool truncate)
    : BaseSink(name), fileName_(fileName) {
    FormatIdManager::setOnNewFormatEntryCallback([this](std::shared_ptr<FormatEntry> formatEntry) {
        writeFormatEntry(std::move(formatEntry));
    });
    openFile(truncate);
}

CompressFileSink::~CompressFileSink() {
    if (outFile_.is_open()) {
        doSync();
        outFile_.close();
    }
}

void CompressFileSink::log(LogEntry &entry) {
    if (!outFile_.is_open()) {
        return;
    }

    uint64_t tsDiffMs = (entry.time_ - lastTs_) / 1000000;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (tsDiffMs > 16384) {
            // 2 ^ 14 = 16384, 16 秒
            lastTs_ = entry.time_;
            tsDiffMs = 0;
            writeTsInfoEntry();
        }

        writeLogEntry(entry, tsDiffMs);
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
    writePidInfoEntry();
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

void CompressFileSink::writeBuffer(const void *data, uint32_t size) {
    if (pos_ + size > kBufferSize) {
        outFile_.write(reinterpret_cast<const char *>(buf_), pos_);
        pos_ = 0;
    }
    memcpy(buf_ + pos_, data, size);
    pos_ += size;
}

void CompressFileSink::writePidInfoEntry() {
    //printf("writePidInfoEntry\n");
    // 构造 info entry 头部
    // 高2位为2(info entry类型)，接下来2位为0(pid类型)，低4位为4(uint32_t的字节数)
    uint8_t header = EntryType::INFO_ENTRY << 6;
    writeBuffer(&header, sizeof(header));

    // 写入当前进程ID
    int32_t pid = OSUtils::getPid();
    writeBuffer(&pid, sizeof(pid));
}

void CompressFileSink::writeTsInfoEntry() {
    //printf("writeTsInfoEntry\n");
    // 构造 info entry 头部
    // 高2位为2(info entry类型)，接下来2位为1(ts类型)，低4位为8(uint64_t的字节数)
    uint8_t header = (EntryType::INFO_ENTRY << 6) | (1 << 4) | 8;
    writeBuffer(&header, sizeof(header));

    // 写入当前时间戳
    writeBuffer(&lastTs_, sizeof(lastTs_));
}

void CompressFileSink::writeLogEntry(const LogEntry &entry, uint16_t tsDiff) {
    // 构造 log entry 头部
    // 高2位为0(log entry类型)
    uint8_t header = (EntryType::LOG_ENTRY << 6);
    writeBuffer(&header, sizeof(header));

    uint8_t encodeBuffer[16]{0};
    // 写入时间戳差值
    size_t encodeSize = encodeLEB128(tsDiff, encodeBuffer);
    writeBuffer(encodeBuffer, encodeSize);

    // 写入格式ID
    encodeSize = encodeLEB128(entry.formatId_, encodeBuffer);
    writeBuffer(encodeBuffer, encodeSize);

    // 写入线程ID
    encodeSize = encodeLEB128(entry.tid_, encodeBuffer);
    writeBuffer(encodeBuffer, encodeSize);

    // 写入参数数据
    if (!entry.argStore_.empty()) {
        writeBuffer(entry.argStore_.data(), entry.argStore_.size());
    }
}

void CompressFileSink::writeFormatEntry(std::shared_ptr<FormatEntry> formatEntry) {
    //printf("writeFormatEntry\n");
    // 构造 format entry 头部
    // 高2位为1(format entry类型)，低6位为日志级别
    uint8_t header = (EntryType::FORMAT_ENTRY << 6) | (static_cast<uint8_t>(formatEntry->logLevel_) & 0x3F);
    writeBuffer(&header, sizeof(header));
    writeBuffer(&formatEntry->argc_, sizeof(formatEntry->argc_));
    writeBuffer(&formatEntry->formatId_, sizeof(formatEntry->formatId_));
    writeBuffer(formatEntry->argTypes_.data(), formatEntry->argTypes_.size());
    writeBuffer(formatEntry->formatStr_.data(), formatEntry->formatStr_.length());
}

}// namespace qslog