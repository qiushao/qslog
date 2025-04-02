#include "qslog/LogEntry.h"
#include "OSUtils.h"
#include "fmt/args.h"
#include <fmt/format.h>
#include <iostream>
#include <utility>

namespace qslog {
LogEntry::LogEntry(LogLevel level, std::string_view format, uint8_t argc, std::vector<uint8_t> args)
    : tid_(OSUtils::getTid()),
      time_(OSUtils::realTimeNanosecond()),
      level_(level),
      format_(format),
      argc_(argc),
      argStore_(std::move(args)) {
}

std::string LogEntry::formatLogEntry() const {
    static int32_t pid = OSUtils::getPid();
    auto levelName = getLevelName(level_);
    std::string msg = parserMsg(argStore_, format_);
    auto timeStr = formatTimespec(time_);
    fmt::memory_buffer buf;
    auto formatArgs = fmt::make_format_args(timeStr, pid, tid_, levelName, msg);
    fmt::vformat_to(fmt::appender(buf), "{} {} {} {} {}", formatArgs);
    return {buf.data(), buf.size()};
}

std::string LogEntry::parserMsg(const std::vector<uint8_t> &buffer, const std::string &format) {
    fmt::dynamic_format_arg_store<fmt::format_context> argStore;
    extractArgs(buffer, argStore);
    fmt::memory_buffer buf;
    fmt::vformat_to(fmt::appender(buf), format, argStore);
    return {buf.data(), buf.size()};
}

bool LogEntry::extractArgs(const std::vector<uint8_t> &buffer, fmt::dynamic_format_arg_store<fmt::format_context> &argStore) {
    // 从 buffer 中解析 args 放到 argStore
    size_t pos = 0;
    while (pos < buffer.size()) {
        uint8_t typeId = buffer[pos++];

        switch (typeId) {
            case 1: {// bool
                bool value = *reinterpret_cast<const bool *>(&buffer[pos]);
                argStore.push_back(value);
                pos += sizeof(bool);
                break;
            }
            case 2: {// int8_t
                int8_t value = *reinterpret_cast<const int8_t *>(&buffer[pos]);
                argStore.push_back(value);
                pos += sizeof(int8_t);
                break;
            }
            case 3: {// int16_t
                int16_t value = *reinterpret_cast<const int16_t *>(&buffer[pos]);
                argStore.push_back(value);
                pos += sizeof(int16_t);
                break;
            }
            case 4: {// int32_t
                int32_t value = *reinterpret_cast<const int32_t *>(&buffer[pos]);
                argStore.push_back(value);
                pos += sizeof(int32_t);
                break;
            }
            case 5: {// int64_t 或指针
                int64_t value = *reinterpret_cast<const int64_t *>(&buffer[pos]);
                argStore.push_back(value);
                pos += sizeof(int64_t);
                break;
            }
            case 6: {// uint8_t
                uint8_t value = *reinterpret_cast<const uint8_t *>(&buffer[pos]);
                argStore.push_back(value);
                pos += sizeof(uint8_t);
                break;
            }
            case 7: {// uint16_t
                uint16_t value = *reinterpret_cast<const uint16_t *>(&buffer[pos]);
                argStore.push_back(value);
                pos += sizeof(uint16_t);
                break;
            }
            case 8: {// uint32_t
                uint32_t value = *reinterpret_cast<const uint32_t *>(&buffer[pos]);
                argStore.push_back(value);
                pos += sizeof(uint32_t);
                break;
            }
            case 9: {// uint64_t
                uint64_t value = *reinterpret_cast<const uint64_t *>(&buffer[pos]);
                argStore.push_back(value);
                pos += sizeof(uint64_t);
                break;
            }
            case 10: {// float
                float value = *reinterpret_cast<const float *>(&buffer[pos]);
                argStore.push_back(value);
                pos += sizeof(float);
                break;
            }
            case 11: {// double
                double value = *reinterpret_cast<const double *>(&buffer[pos]);
                argStore.push_back(value);
                pos += sizeof(double);
                break;
            }
            case 12: {// string
                uint32_t length = *reinterpret_cast<const uint32_t *>(&buffer[pos]);
                pos += sizeof(uint32_t);

                if (length > 0) {
                    std::string value(reinterpret_cast<const char *>(&buffer[pos]), length);
                    argStore.push_back(value);
                    pos += length;
                } else {
                    argStore.push_back(std::string());
                }
                break;
            }
            case 13: {// char
                char value = *reinterpret_cast<const char *>(&buffer[pos]);
                argStore.push_back(value);
                pos += sizeof(char);
                break;
            }
            default:
                // 未知类型，跳过
                std::cout << "Error: Unknown type in argument store" << std::endl;
                return false;
        }
    }
    return true;
}

}// namespace qslog