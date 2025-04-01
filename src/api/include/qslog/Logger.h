#ifndef QSLOG_LOGGER_H
#define QSLOG_LOGGER_H

#include "fmt/format.h"
#include "qslog/BaseSink.h"
#include "qslog/LogEntry.h"
#include "qslog/common.h"
#include <memory>

namespace qslog {

#define QSLOG_LINE(x) #x
#define QSLOG_LINE_S(x) QSLOG_LINE(x)
#define QSLOG_SOURCE_LOCATION __FILE__ ":" QSLOG_LINE_S(__LINE__)

// Compile-time string literal parsing to extract filename
// This is a constexpr function that can be evaluated at compile time
constexpr std::string_view extractFilename(std::string_view path) {
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string_view::npos) {
        return path;// No path separator, return the entire string
    }
    return path.substr(pos + 1);
}

#define QSLOG_BASE_SOURCE_LOCATION (::qslog::extractFilename(QSLOG_SOURCE_LOCATION))

typedef std::function<void(const LogEntry &entry)> LogHandler;

// 用于static_assert的辅助模板，始终返回false
template<typename T>
struct always_false : std::false_type {
    // 这个静态方法会在编译错误消息中显示类型名
    static constexpr bool dependent_false() {
        return false;
    }
};

template<typename T>
static void serializeArg(std::vector<uint8_t> &buffer, T &&arg) {
    using CleanType = std::remove_reference_t<T>;

    // 确定类型ID
    uint8_t typeId;

    // 布尔类型
    if constexpr (std::is_same_v<CleanType, bool>) {
        typeId = 1;
        buffer.push_back(typeId);
        buffer.insert(buffer.end(), reinterpret_cast<const uint8_t *>(&arg),
                      reinterpret_cast<const uint8_t *>(&arg) + sizeof(bool));
    }
    // 有符号整型
    else if constexpr (std::is_signed_v<CleanType> && std::is_integral_v<CleanType>) {
        if constexpr (sizeof(CleanType) == 1) {
            typeId = 2;// int8_t
        } else if constexpr (sizeof(CleanType) == 2) {
            typeId = 3;// int16_t
        } else if constexpr (sizeof(CleanType) == 4) {
            typeId = 4;// int32_t
        } else if constexpr (sizeof(CleanType) == 8) {
            typeId = 5;// int64_t
        }
        buffer.push_back(typeId);
        buffer.insert(buffer.end(), reinterpret_cast<const uint8_t *>(&arg),
                      reinterpret_cast<const uint8_t *>(&arg) + sizeof(CleanType));
    }
    // 无符号整型
    else if constexpr (std::is_unsigned_v<CleanType> && std::is_integral_v<CleanType>) {
        if constexpr (sizeof(CleanType) == 1) {
            typeId = 6;// uint8_t
        } else if constexpr (sizeof(CleanType) == 2) {
            typeId = 7;// uint16_t
        } else if constexpr (sizeof(CleanType) == 4) {
            typeId = 8;// uint32_t
        } else if constexpr (sizeof(CleanType) == 8) {
            typeId = 9;// uint64_t
        }
        buffer.push_back(typeId);
        buffer.insert(buffer.end(), reinterpret_cast<const uint8_t *>(&arg),
                      reinterpret_cast<const uint8_t *>(&arg) + sizeof(CleanType));
    }
    // 浮点类型
    else if constexpr (std::is_floating_point_v<CleanType>) {
        if constexpr (sizeof(CleanType) == 4) {
            typeId = 10;// float
        } else if constexpr (sizeof(CleanType) == 8) {
            typeId = 11;// double
        }
        buffer.push_back(typeId);
        buffer.insert(buffer.end(), reinterpret_cast<const uint8_t *>(&arg),
                      reinterpret_cast<const uint8_t *>(&arg) + sizeof(CleanType));
    }
    // 字符串类型
    else if constexpr (std::is_same_v<CleanType, std::string> ||
                       std::is_same_v<CleanType, std::string_view>) {
        typeId = 12;
        buffer.push_back(typeId);
        uint32_t length = arg.length();
        buffer.insert(buffer.end(), reinterpret_cast<const uint8_t *>(&length),
                      reinterpret_cast<const uint8_t *>(&length) + sizeof(length));
        buffer.insert(buffer.end(), reinterpret_cast<const uint8_t *>(arg.data()),
                      reinterpret_cast<const uint8_t *>(arg.data()) + length);
    }
    // C风格字符串 (const char* 和 char*)
    else if constexpr (std::is_pointer_v<CleanType> &&
                       (std::is_same_v<std::remove_const_t<std::remove_pointer_t<CleanType>>, char>) ) {
        typeId = 12;
        buffer.push_back(typeId);
        if (arg == nullptr) {
            uint32_t length = 0;
            buffer.insert(buffer.end(), reinterpret_cast<const uint8_t *>(&length),
                          reinterpret_cast<const uint8_t *>(&length) + sizeof(length));
        } else {
            uint32_t length = std::strlen(arg);
            buffer.insert(buffer.end(), reinterpret_cast<const uint8_t *>(&length),
                          reinterpret_cast<const uint8_t *>(&length) + sizeof(length));
            buffer.insert(buffer.end(), reinterpret_cast<const uint8_t *>(arg),
                          reinterpret_cast<const uint8_t *>(arg) + length);
        }
    }
    // 字符类型
    else if constexpr (std::is_same_v<CleanType, char>) {
        typeId = 13;
        buffer.push_back(typeId);
        buffer.insert(buffer.end(), reinterpret_cast<const uint8_t *>(&arg),
                      reinterpret_cast<const uint8_t *>(&arg) + sizeof(char));
    }
    // 普通指针类型 (除了已处理的 char*)
    else if constexpr (std::is_pointer_v<CleanType> &&
                       !std::is_same_v<std::remove_const_t<std::remove_pointer_t<CleanType>>, char>) {
        // 使用与 long 相同的 typeId (int64_t)
        typeId = 5;// 与 int64_t 相同
        buffer.push_back(typeId);

        // 将指针转换为 uintptr_t 并序列化
        auto ptrValue = reinterpret_cast<uintptr_t>(arg);
        buffer.insert(buffer.end(), reinterpret_cast<const uint8_t *>(&ptrValue),
                      reinterpret_cast<const uint8_t *>(&ptrValue) + sizeof(uintptr_t));
    }
    // un support type
    else {
        static_assert(always_false<CleanType>::dependent_false(),
                      "Unsupported type for serialization");
    }
}

class Logger {
public:
    static void setLogLevel(LogLevel level);

    static LogLevel getLogLevel();

    static void setLogHandler(LogHandler handler);

    static void addSink(std::shared_ptr<BaseSink> sink);

    static void sync();

    template<typename... Args>
    static void log(std::string_view sourceLocation, LogLevel level, std::string_view tag, fmt::format_string<Args...> format, Args &&...args) {
        if (level < logLevel_) {
            return;
        }

        std::vector<uint8_t> argStore;
        // 使用折叠表达式序列化参数
        (serializeArg(argStore, std::forward<Args>(args)), ...);
        LogEntry entry(sourceLocation, level, tag, format.str.data(), argStore);

        if (logHandler_) {
            logHandler_(entry);
            return;
        }

        for (auto &sink: sinks_) {
            sink->log(entry);
        }
    }

    static LogLevel logLevel_;
    static LogHandler logHandler_;
    static std::vector<std::shared_ptr<BaseSink>> sinks_;
};

#define QSLOG(level, tag, format, ...) \
    qslog::Logger::log(QSLOG_BASE_SOURCE_LOCATION, level, tag, FMT_STRING(format), ##__VA_ARGS__)

#define QSLOGV(format, ...) (QSLOG(qslog::LogLevel::VERBOSE, QSLOG_TAG, format, ##__VA_ARGS__))
#define QSLOGD(format, ...) (QSLOG(qslog::LogLevel::DEBUG, QSLOG_TAG, format, ##__VA_ARGS__))
#define QSLOGI(format, ...) (QSLOG(qslog::LogLevel::INFO, QSLOG_TAG, format, ##__VA_ARGS__))
#define QSLOGW(format, ...) (QSLOG(qslog::LogLevel::WARN, QSLOG_TAG, format, ##__VA_ARGS__))
#define QSLOGE(format, ...) (QSLOG(qslog::LogLevel::ERROR, QSLOG_TAG, format, ##__VA_ARGS__))
#define QSLOGF(format, ...) (QSLOG(qslog::LogLevel::FATAL, QSLOG_TAG, format, ##__VA_ARGS__))
}// namespace qslog

#endif//QSLOG_LOGGER_H
