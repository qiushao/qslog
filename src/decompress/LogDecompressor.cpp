#include "LogDecompressor.h"
#include "qslog/common.h"
#include <iostream>

namespace qslog {

LogDecompressor::LogDecompressor(const std::string &inputFile, const std::string &outputFile) {
    inFile_.open(inputFile, std::ios::binary);
    outFile_.open(outputFile);
}

LogDecompressor::~LogDecompressor() {
    if (inFile_.is_open()) {
        inFile_.close();
    }
    if (outFile_.is_open()) {
        outFile_.close();
    }
}

bool LogDecompressor::decompress() {
    if (!inFile_.is_open() || !outFile_.is_open()) {
        std::cerr << "Failed to open input or output file" << std::endl;
        return false;
    }

    while (readEntry()) {
        // 继续读取下一个条目
    }

    return true;
}

bool LogDecompressor::readEntry() {
    uint8_t header;
    if (!read(header)) {
        return false;// 文件结束或读取错误
    }

    // 提取条目类型（高2位）
    uint8_t entryType = (header >> 6) & 0x03;

    switch (entryType) {
        case EntryType::LOG_ENTRY:
            return readLogEntry(header);
        case EntryType::FORMAT_ENTRY:
            return readFormatEntry(header);
        case EntryType::INFO_ENTRY:// info entry
        {
            // 提取info类型（接下来2位）
            uint8_t infoType = (header >> 4) & 0x03;
            if (infoType == 0) {// pid type
                return readPidInfoEntry(header);
            } else if (infoType == 1) {// ts type
                return readTsInfoEntry(header);
            } else {
                std::cerr << "Unknown info type: " << static_cast<int>(infoType) << std::endl;
                return false;
            }
        }
        default:
            std::cerr << "Unknown entry type: " << static_cast<int>(entryType) << std::endl;
            return false;
    }
}

bool LogDecompressor::readPidInfoEntry(uint8_t header) {
    // 读取进程ID
    if (!read(pid_)) {
        return false;
    }

    return true;
}

bool LogDecompressor::readTsInfoEntry(uint8_t header) {
    // 读取时间戳
    if (!read(lastTs_)) {
        return false;
    }

    return true;
}

bool LogDecompressor::readFormatEntry(uint8_t header) {
    // 提取日志级别（低6位）
    uint8_t logLevel = header & 0x3F;
    auto formatEntry = std::make_shared<FormatEntry>();
    formatEntry->logLevel_ = logLevel;

    if (!read(formatEntry->argc_)) {
        return false;
    }

    if (!read(formatEntry->formatId_)) {
        return false;
    }

    formatEntry->argTypes_.resize(formatEntry->argc_);
    if (!readBuffer(formatEntry->argTypes_.data(), formatEntry->argc_)) {
        return false;
    }

    if (!readString(formatEntry->formatStr_)) {
        return false;
    }

    if (formatEntry->formatId_ >= formatEntries_.size()) {
        formatEntries_.resize(formatEntry->formatId_ + 1);
    }
    formatEntries_[formatEntry->formatId_] = formatEntry;
    return true;
}

bool LogDecompressor::readLogEntry(uint8_t header) {
    // 读取时间戳差值
    uint16_t tsDiff = decodeLEB128(inFile_);
    // 计算实际时间戳
    uint64_t timestamp = lastTs_ + tsDiff * 1000000;

    // 读取格式ID
    uint16_t formatId = decodeLEB128(inFile_);
    auto formatEntry = formatEntries_[formatId];

    // 读取线程ID
    uint32_t tid = decodeLEB128(inFile_);

    // 读取参数
    fmt::dynamic_format_arg_store<fmt::format_context> argStore;
    for (uint8_t i = 0; i < formatEntry->argc_; ++i) {
        uint8_t argType = formatEntry->argTypes_[i];
        // 根据参数类型读取参数值
        switch (argType) {
            case ArgTypeId::BOOL: {
                bool value;
                if (!read(value)) return false;
                argStore.push_back(value);
                break;
            }
            case ArgTypeId::CHAR: {
                char value;
                if (!read(value)) return false;
                argStore.push_back(value);
                break;
            }
            case ArgTypeId::UINT8: {
                uint8_t value;
                if (!read(value)) return false;
                argStore.push_back(value);
                break;
            }
            case ArgTypeId::INT8: {
                int8_t value;
                if (!read(value)) return false;
                argStore.push_back(value);
                break;
            }
            case ArgTypeId::UINT64: {
                uint64_t value = decodeLEB128(inFile_);
                argStore.push_back(value);
                break;
            }
            case ArgTypeId::INT64: {
                int64_t value = decodeLEB128(inFile_);
                argStore.push_back(value);
                break;
            }
            case ArgTypeId::FLOAT: {
                float value;
                if (!read(value)) return false;
                argStore.push_back(value);
                break;
            }
            case ArgTypeId::DOUBLE: {// double
                double value;
                if (!read(value)) return false;
                argStore.push_back(value);
                break;
            }
            case ArgTypeId::STR: {// string
                std::string str;
                if (!readString(str)) return false;
                argStore.push_back(str);
                break;
            }
            default:
                std::cerr << "Unknown argument type: " << static_cast<int>(argType) << std::endl;
                return false;
        }
    }

    // 格式化日志条目并写入输出文件
    auto timeStr = formatTimespec(timestamp);
    fmt::memory_buffer buf;
    fmt::vformat_to(fmt::appender(buf), formatEntry->formatStr_, argStore);
    std::string_view msg(buf.data(), buf.size());
    std::string formattedLog = fmt::format("{} {} {} {}", timeStr, pid_, tid, msg);
    outFile_ << formattedLog << std::endl;

    return true;
}

}// namespace qslog
