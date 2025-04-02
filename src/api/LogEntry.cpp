#include "qslog/LogEntry.h"
#include "OSUtils.h"
#include "fmt/args.h"
#include <fmt/format.h>
#include <utility>

namespace qslog {
LogEntry::LogEntry(LogLevel level, std::string_view format, uint8_t argc, std::vector<uint8_t> args)
    : tid_(OSUtils::getTid()),
      time_(OSUtils::realTimeMillisecond()),
      level_(level),
      format_(format),
      argc_(argc),
      argStore_(std::move(args)) {
}

std::string LogEntry::formatLogEntry() const {
    std::string msg = parserMsg();
    fmt::memory_buffer buf;
    auto levelName = getLevelName(level_);
    static int32_t pid = OSUtils::getPid();
    auto formatArgs = fmt::make_format_args(time_, pid, tid_, levelName, msg);
    fmt::vformat_to(fmt::appender(buf), "{} {} {} {} {}", formatArgs);
    return {buf.data(), buf.size()};
}

std::string LogEntry::parserMsg() const {
    fmt::dynamic_format_arg_store<fmt::format_context> argStore;

    // 从 argStore_ 中解析 args 放到 argStore
    size_t pos = 0;
    while (pos < argStore_.size()) {
        uint8_t typeId = argStore_[pos++];

        switch (typeId) {
            case 1: {// bool
                bool value = *reinterpret_cast<const bool *>(&argStore_[pos]);
                argStore.push_back(value);
                pos += sizeof(bool);
                break;
            }
            case 2: {// int8_t
                int8_t value = *reinterpret_cast<const int8_t *>(&argStore_[pos]);
                argStore.push_back(value);
                pos += sizeof(int8_t);
                break;
            }
            case 3: {// int16_t
                int16_t value = *reinterpret_cast<const int16_t *>(&argStore_[pos]);
                argStore.push_back(value);
                pos += sizeof(int16_t);
                break;
            }
            case 4: {// int32_t
                int32_t value = *reinterpret_cast<const int32_t *>(&argStore_[pos]);
                argStore.push_back(value);
                pos += sizeof(int32_t);
                break;
            }
            case 5: {// int64_t 或指针
                int64_t value = *reinterpret_cast<const int64_t *>(&argStore_[pos]);
                argStore.push_back(value);
                pos += sizeof(int64_t);
                break;
            }
            case 6: {// uint8_t
                uint8_t value = *reinterpret_cast<const uint8_t *>(&argStore_[pos]);
                argStore.push_back(value);
                pos += sizeof(uint8_t);
                break;
            }
            case 7: {// uint16_t
                uint16_t value = *reinterpret_cast<const uint16_t *>(&argStore_[pos]);
                argStore.push_back(value);
                pos += sizeof(uint16_t);
                break;
            }
            case 8: {// uint32_t
                uint32_t value = *reinterpret_cast<const uint32_t *>(&argStore_[pos]);
                argStore.push_back(value);
                pos += sizeof(uint32_t);
                break;
            }
            case 9: {// uint64_t
                uint64_t value = *reinterpret_cast<const uint64_t *>(&argStore_[pos]);
                argStore.push_back(value);
                pos += sizeof(uint64_t);
                break;
            }
            case 10: {// float
                float value = *reinterpret_cast<const float *>(&argStore_[pos]);
                argStore.push_back(value);
                pos += sizeof(float);
                break;
            }
            case 11: {// double
                double value = *reinterpret_cast<const double *>(&argStore_[pos]);
                argStore.push_back(value);
                pos += sizeof(double);
                break;
            }
            case 12: {// string
                uint32_t length = *reinterpret_cast<const uint32_t *>(&argStore_[pos]);
                pos += sizeof(uint32_t);

                if (length > 0) {
                    std::string value(reinterpret_cast<const char *>(&argStore_[pos]), length);
                    argStore.push_back(value);
                    pos += length;
                } else {
                    argStore.push_back(std::string());
                }
                break;
            }
            case 13: {// char
                char value = *reinterpret_cast<const char *>(&argStore_[pos]);
                argStore.push_back(value);
                pos += sizeof(char);
                break;
            }
            default:
                // 未知类型，跳过
                return "Error: Unknown type in argument store";
        }
    }

    fmt::memory_buffer buf;
    fmt::vformat_to(fmt::appender(buf), format_, argStore);
    return {buf.data(), buf.size()};
}


}// namespace qslog