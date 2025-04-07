#ifndef QSLOG_LOGGER_H
#define QSLOG_LOGGER_H

#include "../../../OSUtils.h"
#include "../../FormatIdManager.h"
#include "fmt/format.h"
#include "qslog/BaseSink.h"
#include "qslog/LogEntry.h"
#include "qslog/common.h"
#include <memory>

namespace qslog {

constexpr std::string_view getBaseFilename(std::string_view path) {
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string_view::npos) {
        return path;
    }
    return path.substr(pos + 1);
}

#define QS_BASE_FILENAME (qslog::getBaseFilename(__FILE__))

// 计算单个参数序列化后的大小
template<typename T>
static constexpr size_t getSerializedSize() {
    using CleanType = std::remove_reference_t<T>;

    size_t size = 0;

    // 字符串类型 - 这里只能计算固定部分，字符串内容长度是运行时才知道的
    if constexpr (std::is_same_v<CleanType, std::string> ||
                  std::is_same_v<CleanType, std::string_view>) {
        // 注意：字符串内容的长度在运行时才能确定
    }
    // C风格字符串
    else if constexpr (std::is_pointer_v<CleanType> &&
                       (std::is_same_v<std::remove_const_t<std::remove_pointer_t<CleanType>>, char>) ) {
        // 注意：字符串内容的长度在运行时才能确定
    } else {
        size += sizeof(CleanType);
    }

    return size;
}

// 计算所有参数序列化后的最小大小（不包括字符串内容）
template<typename... Args>
static constexpr size_t getMinSerializedSize() {
    if constexpr (sizeof...(Args) == 0) {
        return 0;// 没有参数时返回0
    } else {
        return (getSerializedSize<Args>() + ...);
    }
}

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

    if constexpr (std::is_same_v<CleanType, bool> || std::is_same_v<CleanType, char> || std::is_same_v<CleanType, uint8_t> || std::is_same_v<CleanType, int8_t> || std::is_floating_point_v<CleanType>) {
        buffer.insert(buffer.end(), reinterpret_cast<const uint8_t *>(&arg),
                      reinterpret_cast<const uint8_t *>(&arg) + sizeof(CleanType));
    } else if constexpr (std::is_unsigned_v<CleanType> && std::is_integral_v<CleanType>) {
        // 无符号整型 uint16, 32, 64, 统一按 uint64 位处理
        auto value = static_cast<uint64_t>(arg);
        encodeLEB128(value, buffer);
    } else if constexpr (std::is_signed_v<CleanType> && std::is_integral_v<CleanType>) {
        // 有符号整型int16, 32, 64, 统一按 int64 位处理
        auto value = static_cast<int64_t>(arg);
        encodeLEB128(value, buffer);
    }
    // 字符串类型
    else if constexpr (std::is_same_v<CleanType, std::string> ||
                       std::is_same_v<CleanType, std::string_view>) {
        uint32_t length = arg.length();
        buffer.insert(buffer.end(), reinterpret_cast<const uint8_t *>(arg.data()),
                      reinterpret_cast<const uint8_t *>(arg.data()) + length);
    }
    // C风格字符串 (const char* 和 char*)
    else if constexpr (std::is_pointer_v<CleanType> &&
                       (std::is_same_v<std::remove_const_t<std::remove_pointer_t<CleanType>>, char>) ) {
        if (arg == nullptr) {
            // 空字符串，长度为0
            buffer.push_back(0);// LEB128 for 0
        } else {
            uint32_t length = std::strlen(arg);
            buffer.insert(buffer.end(), reinterpret_cast<const uint8_t *>(arg),
                          reinterpret_cast<const uint8_t *>(arg) + length);
        }
    }
    // 字符数组（字符串字面量 "hello"）
    else if constexpr (std::is_array_v<CleanType> &&
                       std::is_same_v<std::remove_const_t<std::remove_extent_t<CleanType>>, char>) {
        uint32_t length = std::strlen(arg);
        buffer.insert(buffer.end(), reinterpret_cast<const uint8_t *>(arg),
                      reinterpret_cast<const uint8_t *>(arg) + length);
    }
    // 普通指针类型 (除了已处理的 char*)
    else if constexpr (std::is_pointer_v<CleanType> &&
                       !std::is_same_v<std::remove_const_t<std::remove_pointer_t<CleanType>>, char>) {
        auto value = reinterpret_cast<uint64_t>(arg);
        encodeLEB128(value, buffer);
    }
    // un support type
    else {
        static_assert(always_false<CleanType>::dependent_false(),
                      "Unsupported type for serialization");
    }
}

template<typename T>
void parseArgType(std::vector<uint8_t> &buffer, T &&arg) {
    using CleanType = std::remove_reference_t<T>;
    uint8_t typeId;

    if constexpr (std::is_same_v<CleanType, bool>) {
        typeId = ArgTypeId::BOOL;
        buffer.push_back(typeId);
    } else if constexpr (std::is_same_v<CleanType, char>) {
        typeId = ArgTypeId::CHAR;
        buffer.push_back(typeId);
    } else if constexpr (std::is_same_v<CleanType, uint8_t>) {
        typeId = ArgTypeId::UINT8;
        buffer.push_back(typeId);
    } else if constexpr (std::is_same_v<CleanType, int8_t>) {
        typeId = ArgTypeId::INT8;
        buffer.push_back(typeId);
    } else if constexpr (std::is_unsigned_v<CleanType> && std::is_integral_v<CleanType>) {
        // 无符号整型 uint16, 32, 64, 统一按 uint64 位处理
        typeId = ArgTypeId::UINT64;
        buffer.push_back(typeId);
    } else if constexpr (std::is_signed_v<CleanType> && std::is_integral_v<CleanType>) {
        // 有符号整型int16, 32, 64, 统一按 int64 位处理
        typeId = ArgTypeId::INT64;
        buffer.push_back(typeId);
    } else if constexpr (std::is_floating_point_v<CleanType>) {
        // 浮点类型
        if constexpr (sizeof(CleanType) == 4) {
            typeId = ArgTypeId::FLOAT;
        } else if constexpr (sizeof(CleanType) == 8) {
            typeId = ArgTypeId::DOUBLE;
        }
        buffer.push_back(typeId);
    }
    // 字符串类型
    else if constexpr (std::is_same_v<CleanType, std::string> ||
                       std::is_same_v<CleanType, std::string_view>) {
        typeId = ArgTypeId::STR;
        buffer.push_back(typeId);
    }
    // C风格字符串 (const char* 和 char*)
    else if constexpr (std::is_pointer_v<CleanType> &&
                       (std::is_same_v<std::remove_const_t<std::remove_pointer_t<CleanType>>, char>) ) {
        typeId = ArgTypeId::STR;
        buffer.push_back(typeId);
    }
    // 字符数组（字符串字面量 "hello"）
    else if constexpr (std::is_array_v<CleanType> &&
                       std::is_same_v<std::remove_const_t<std::remove_extent_t<CleanType>>, char>) {
        typeId = ArgTypeId::STR;
        buffer.push_back(typeId);
    }
    // 普通指针类型 (除了已处理的 char*)
    else if constexpr (std::is_pointer_v<CleanType> &&
                       !std::is_same_v<std::remove_const_t<std::remove_pointer_t<CleanType>>, char>) {
        typeId = ArgTypeId::UINT64;// 与 uint64_t 相同
        buffer.push_back(typeId);
    }
}

class Logger {
public:
    static void setLogLevel(LogLevel level);

    static LogLevel getLogLevel();

    static void addSink(std::shared_ptr<BaseSink> sink);

    static void sync();

    template<typename... Args>
    static void log(uint16_t &formatId, LogLevel level, std::string_view tag,
                    std::string_view file, uint16_t line, std::string_view function,
                    fmt::format_string<Args...> format, Args &&...args) {
        if (level < logLevel_) {
            return;
        }

        constexpr uint8_t argc = sizeof...(args);
        static uint32_t pid = OSUtils::getPid();
        static thread_local uint32_t tid = OSUtils::getTid();

        if (formatId == UINT16_MAX) {
            auto formatEntry = std::make_shared<FormatEntry>();
            formatEntry->entryType_ = EntryType::FORMAT_ENTRY;
            formatEntry->logLevel_ = level;
            formatEntry->argc_ = argc;
            (parseArgType(formatEntry->argTypes_, std::forward<Args>(args)), ...);
            formatEntry->formatStr_ = fmt::format("{} [{}:{} {}] {}", tag, file, line, function, format.str.data());
            FormatIdManager::registerFormatId(formatId, formatEntry);
        }

        LogEntry logEntry;
        logEntry.formatId_ = formatId;
        logEntry.time_ = OSUtils::realTimeNanosecond();
        logEntry.pid_ = pid;
        logEntry.tid_ = tid;
        if constexpr (argc > 0) {
            constexpr size_t minSize = getMinSerializedSize<Args...>();
            logEntry.argStore_.reserve(minSize);
            (serializeArg(logEntry.argStore_, std::forward<Args>(args)), ...);
        }

        for (auto &sink: sinks_) {
            sink->log(logEntry);
        }
    }

private:
    static LogLevel logLevel_;
    static std::vector<std::shared_ptr<BaseSink>> sinks_;
};

#define QSLOG(level, tag, format, ...)                                                                   \
    do {                                                                                                 \
        static uint16_t qslog_formatId = UINT16_MAX;                                                     \
        qslog::Logger::log(qslog_formatId, level, tag,                                                   \
                           QS_BASE_FILENAME, __LINE__, __FUNCTION__, FMT_STRING(format), ##__VA_ARGS__); \
    } while (0)

#define QSLOGV(format, ...) QSLOG(qslog::LogLevel::VERBOSE, QSLOG_TAG, format, ##__VA_ARGS__)
#define QSLOGD(format, ...) QSLOG(qslog::LogLevel::DEBUG, QSLOG_TAG, format, ##__VA_ARGS__)
#define QSLOGI(format, ...) QSLOG(qslog::LogLevel::INFO, QSLOG_TAG, format, ##__VA_ARGS__)
#define QSLOGW(format, ...) QSLOG(qslog::LogLevel::WARN, QSLOG_TAG, format, ##__VA_ARGS__)
#define QSLOGE(format, ...) QSLOG(qslog::LogLevel::ERROR, QSLOG_TAG, format, ##__VA_ARGS__)
#define QSLOGF(format, ...) QSLOG(qslog::LogLevel::FATAL, QSLOG_TAG, format, ##__VA_ARGS__)
}// namespace qslog

#endif//QSLOG_LOGGER_H
