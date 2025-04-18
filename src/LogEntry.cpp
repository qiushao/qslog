#include "qslog/LogEntry.h"
#include "fmt/args.h"
#include "qslog/FormatIdManager.h"
#include "qslog/OSUtils.h"
#include <fmt/format.h>
#include <iostream>
#include <utility>

namespace qslog {
void LogEntry::formatLogEntry(fmt::memory_buffer &buf) {
    static uint32_t pid = OSUtils::getPid();
    auto formatEntry = FormatIdManager::getFormatEntry(formatId_);
    auto levelName = getLevelName(static_cast<LogLevel>(formatEntry->logLevel_));
    std::string msg = parserMsg(formatEntry->formatStr_);
    auto timeStr = formatTimespec(time_);
    auto formatArgs = fmt::make_format_args(timeStr, pid, tid_, levelName, msg);
    fmt::vformat_to(fmt::appender(buf), "{} {} {} {} {}", formatArgs);
}

std::string LogEntry::formatLogEntry() {
    fmt::memory_buffer buf;
    formatLogEntry(buf);
    return {buf.data(), buf.size()};
}

std::string LogEntry::parserMsg(const std::string &format) {
    fmt::dynamic_format_arg_store<fmt::format_context> argStore;
    extractArgs(argStore);
    fmt::memory_buffer buf;
    fmt::vformat_to(fmt::appender(buf), format, argStore);
    return {buf.data(), buf.size()};
}

static std::string readString(const uint8_t *buffer, size_t &pos) {
    size_t start = pos;
    while (buffer[pos] != '\0') {
        ++pos;
    }
    return {reinterpret_cast<const char *>(buffer), pos - start};
}

bool LogEntry::extractArgs(fmt::dynamic_format_arg_store<fmt::format_context> &argStore) {
    // 从 buffer 中解析 args 放到 argStore
    size_t pos = 0;
    auto formatEntry = FormatIdManager::getFormatEntry(formatId_);
    for (int i = 0; i < formatEntry->argc_; ++i) {
        uint8_t argType = formatEntry->argTypes_[i];
        // 根据参数类型读取参数值
        switch (argType) {
            case ArgTypeId::BOOL: {
                bool value = argStore_[pos];
                argStore.push_back(value);
                pos += sizeof(value);
                break;
            }
            case ArgTypeId::CHAR: {
                char value = argStore_[pos];
                argStore.push_back(value);
                pos += sizeof(value);
            }
            case ArgTypeId::UINT8: {
                uint8_t value = argStore_[pos];
                argStore.push_back(value);
                pos += sizeof(value);
            }
            case ArgTypeId::INT8: {
                int value = argStore_[pos];
                argStore.push_back(value);
                pos += sizeof(value);
            }
            case ArgTypeId::UINT64: {
                size_t nRead;
                uint64_t value = decodeLEB128(&argStore_[pos], argsSize_ - pos, &nRead);
                argStore.push_back(value);
                pos += nRead;
                break;
            }
            case ArgTypeId::INT64: {
                size_t nRead;
                int64_t value = decodeLEB128(&argStore_[pos], argsSize_ - pos, &nRead);
                argStore.push_back(value);
                pos += nRead;
                break;
            }
            case ArgTypeId::FLOAT: {
                float value = *reinterpret_cast<const float *>(&argStore_[pos]);
                argStore.push_back(value);
                pos += sizeof(value);
                break;
            }
            case ArgTypeId::DOUBLE: {// double
                double value = *reinterpret_cast<const double *>(&argStore_[pos]);
                argStore.push_back(value);
                pos += sizeof(value);
                break;
            }
            case ArgTypeId::STR: {// string
                std::string str = readString(argStore_, pos);
                argStore.push_back(str);
                break;
            }
            default:
                std::cerr << "Unknown argument type: " << static_cast<int>(argType) << std::endl;
                return false;
        }
    }

    return true;
}

}// namespace qslog