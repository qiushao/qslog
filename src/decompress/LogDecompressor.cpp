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
        case 0:// log entry
            return readLogEntry(header);
        case 1:// format entry
            return readFormatEntry(header);
        case 2:// info entry
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
    // 提取数据长度（低4位）
    uint8_t dataLength = header & 0x0F;

    if (dataLength != 4) {
        std::cerr << "Invalid pid info entry data length: " << static_cast<int>(dataLength) << std::endl;
        return false;
    }

    // 读取进程ID
    if (!read(pid_)) {
        return false;
    }

    return true;
}

bool LogDecompressor::readTsInfoEntry(uint8_t header) {
    // 提取数据长度（低4位）
    uint8_t dataLength = header & 0x0F;

    if (dataLength != 8) {
        std::cerr << "Invalid ts info entry data length: " << static_cast<int>(dataLength) << std::endl;
        return false;
    }

    // 读取时间戳
    if (!read(lastTs_)) {
        return false;
    }

    return true;
}

bool LogDecompressor::readFormatEntry(uint8_t header) {
    // 提取日志级别（低6位）
    uint8_t logLevel = header & 0x3F;
    auto levelName = getLevelName(static_cast<LogLevel>(logLevel));

    // 读取格式ID
    uint16_t formatId;
    if (!read(formatId)) {
        return false;
    }

    // 读取格式字符串长度
    uint16_t formatStrLen;
    if (!read(formatStrLen)) {
        return false;
    }

    // 读取格式字符串
    std::string formatStr(formatStrLen + 2, '\0');
    formatStr[0] = levelName;
    formatStr[1] = ' ';
    if (!readBuffer(&formatStr[2], formatStrLen)) {
        return false;
    }

    // 存储格式字符串
    formatMap_[formatId] = formatStr;

    return true;
}

bool LogDecompressor::readLogEntry(uint8_t header) {
    // 提取参数数量（低6位）
    uint8_t argc = header & 0x3F;

    // 读取时间戳差值
    uint16_t tsDiff;
    if (!read(tsDiff)) {
        return false;
    }

    // 计算实际时间戳
    uint64_t timestamp = lastTs_ + tsDiff * 1000000;

    // 读取格式ID
    uint16_t formatId;
    if (!read(formatId)) {
        return false;
    }

    // 读取线程ID
    int32_t tid;
    if (!read(tid)) {
        return false;
    }

    // 检查格式ID是否存在
    auto it = formatMap_.find(formatId);
    if (it == formatMap_.end()) {
        std::cerr << "Format ID not found: " << formatId << std::endl;
        return false;
    }

    // 获取格式字符串
    std::string formatStr = it->second;

    // 读取参数
    std::vector<uint8_t> argStore;
    for (uint8_t i = 0; i < argc; ++i) {
        // 读取参数类型
        uint8_t argType;
        if (!read(argType)) {
            return false;
        }

        argStore.push_back(argType);

        // 根据参数类型读取参数值
        switch (argType) {
            case 1: {// bool
                bool value;
                if (!read(value)) return false;
                argStore.insert(argStore.end(), reinterpret_cast<uint8_t *>(&value),
                                reinterpret_cast<uint8_t *>(&value) + sizeof(bool));
                break;
            }
            case 2: {// int8_t
                int8_t value;
                if (!read(value)) return false;
                argStore.insert(argStore.end(), reinterpret_cast<uint8_t *>(&value),
                                reinterpret_cast<uint8_t *>(&value) + sizeof(int8_t));
                break;
            }
            case 3: {// int16_t
                int16_t value;
                if (!read(value)) return false;
                argStore.insert(argStore.end(), reinterpret_cast<uint8_t *>(&value),
                                reinterpret_cast<uint8_t *>(&value) + sizeof(int16_t));
                break;
            }
            case 4: {// int32_t
                int32_t value;
                if (!read(value)) return false;
                argStore.insert(argStore.end(), reinterpret_cast<uint8_t *>(&value),
                                reinterpret_cast<uint8_t *>(&value) + sizeof(int32_t));
                break;
            }
            case 5: {// int64_t
                int64_t value;
                if (!read(value)) return false;
                argStore.insert(argStore.end(), reinterpret_cast<uint8_t *>(&value),
                                reinterpret_cast<uint8_t *>(&value) + sizeof(int64_t));
                break;
            }
            case 6: {// uint8_t
                uint8_t value;
                if (!read(value)) return false;
                argStore.push_back(value);
                break;
            }
            case 7: {// uint16_t
                uint16_t value;
                if (!read(value)) return false;
                argStore.insert(argStore.end(), reinterpret_cast<uint8_t *>(&value),
                                reinterpret_cast<uint8_t *>(&value) + sizeof(uint16_t));
                break;
            }
            case 8: {// uint32_t
                uint32_t value;
                if (!read(value)) return false;
                argStore.insert(argStore.end(), reinterpret_cast<uint8_t *>(&value),
                                reinterpret_cast<uint8_t *>(&value) + sizeof(uint32_t));
                break;
            }
            case 9: {// uint64_t
                uint64_t value;
                if (!read(value)) return false;
                argStore.insert(argStore.end(), reinterpret_cast<uint8_t *>(&value),
                                reinterpret_cast<uint8_t *>(&value) + sizeof(uint64_t));
                break;
            }
            case 10: {// float
                float value;
                if (!read(value)) return false;
                argStore.insert(argStore.end(), reinterpret_cast<uint8_t *>(&value),
                                reinterpret_cast<uint8_t *>(&value) + sizeof(float));
                break;
            }
            case 11: {// double
                double value;
                if (!read(value)) return false;
                argStore.insert(argStore.end(), reinterpret_cast<uint8_t *>(&value),
                                reinterpret_cast<uint8_t *>(&value) + sizeof(double));
                break;
            }
            case 12: {// string
                uint32_t length;
                if (!read(length)) return false;

                argStore.insert(argStore.end(), reinterpret_cast<uint8_t *>(&length),
                                reinterpret_cast<uint8_t *>(&length) + sizeof(uint32_t));

                if (length > 0) {
                    std::string value(length, '\0');
                    if (!readBuffer(&value[0], length)) return false;
                    argStore.insert(argStore.end(), value.begin(), value.end());
                }
                break;
            }
            case 13: {// char
                char value;
                if (!read(value)) return false;
                argStore.insert(argStore.end(), reinterpret_cast<uint8_t *>(&value),
                                reinterpret_cast<uint8_t *>(&value) + sizeof(char));
                break;
            }
            default:
                std::cerr << "Unknown argument type: " << static_cast<int>(argType) << std::endl;
                return false;
        }
    }

    // 格式化日志条目并写入输出文件
    auto timeStr = formatTimespec(timestamp);
    auto msg = LogEntry::parserMsg(argStore, formatStr);
    std::string formattedLog = fmt::format("{} {} {} {}", timeStr, pid_, tid, msg);
    outFile_ << formattedLog << std::endl;

    return true;
}

}// namespace qslog
