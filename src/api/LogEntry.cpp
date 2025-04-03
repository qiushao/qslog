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
        switch (typeId >> 4) {// 高 4 位为参数类型
            case TypeId::BOOL: {
                bool value = typeId & 0b00001111;//  低 4 位保存的 bool 值
                argStore.push_back(value);
                pos += sizeof(bool);
                break;
            }
            case TypeId::CHAR: {
                char value = *reinterpret_cast<const char *>(&buffer[pos]);
                argStore.push_back(value);
                pos += sizeof(char);
                break;
            }
            case TypeId::UINT8: {
                uint8_t value = *reinterpret_cast<const uint8_t *>(&buffer[pos]);
                uint8_t flag = typeId & 0b00001000;// 第 5 位保存正负号
                if (flag == 0) {
                    argStore.push_back(value);
                } else {
                    auto sValue = (int8_t) (-value);
                    argStore.push_back(sValue);
                }
                pos += sizeof(uint8_t);
                break;
            }
            case TypeId::UINT64: {
                size_t nRead;
                uint64_t value = decodeLEB128(&buffer[pos], buffer.size() - pos, &nRead);
                uint8_t flag = typeId & 0b00001000;// 第 5 位保存正负号
                if (flag == 0) {
                    argStore.push_back(value);
                } else {
                    auto sValue = (int64_t) (-value);
                    argStore.push_back(sValue);
                }
                pos += nRead;
                break;
            }
            case TypeId::FLOAT: {// float
                float value = *reinterpret_cast<const float *>(&buffer[pos]);
                argStore.push_back(value);
                pos += sizeof(float);
                break;
            }
            case TypeId::DOUBLE: {// double
                double value = *reinterpret_cast<const double *>(&buffer[pos]);
                argStore.push_back(value);
                pos += sizeof(double);
                break;
            }
            case TypeId::STR: {// string
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
            default:
                // 未知类型，跳过
                std::cout << "Error: Unknown type in argument store" << std::endl;
                return false;
        }
    }
    return true;
}

}// namespace qslog