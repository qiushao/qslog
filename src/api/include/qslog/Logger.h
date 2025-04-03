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

// 计算单个参数序列化后的大小
template<typename T>
static constexpr size_t getSerializedSize() {
    using CleanType = std::remove_reference_t<T>;

    // 类型ID占用1字节
    size_t size = 1;

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

    // 确定类型ID (高4位保存数据类型)
    uint8_t typeId;

    if constexpr (std::is_same_v<CleanType, bool>) {
        // 布尔类型 - 直接将值存储在typeId的低4位
        typeId = (TypeId::BOOL << 4) | (arg ? 1 : 0);// 高4位为0表示bool类型，低4位存储值
        buffer.push_back(typeId);
    } else if constexpr (std::is_same_v<CleanType, char>) {
        // char 直接存储，不使用编码
        typeId = TypeId::CHAR << 4;
        buffer.push_back(typeId);
        buffer.push_back(static_cast<uint8_t>(arg));
    } else if constexpr (std::is_same_v<CleanType, uint8_t>) {
        // uint8 直接存储，不使用编码
        typeId = TypeId::UINT8 << 4;
        buffer.push_back(typeId);
        buffer.push_back(static_cast<uint8_t>(arg));
    } else if constexpr (std::is_same_v<CleanType, int8_t>) {
        // int8 正数直接转换成 uint8，负数先取反，把正负号位保存在 typeId 第 5 位， 为1 表示负数
        typeId = TypeId::UINT8 << 4;
        uint8_t tmp = arg;
        if (arg < 0) {
            tmp = -arg;
            typeId |= 1 << 3;
        }
        buffer.push_back(typeId);
        buffer.push_back(static_cast<uint8_t>(tmp));
    } else if constexpr (std::is_unsigned_v<CleanType> && std::is_integral_v<CleanType>) {
        // 无符号整型 uint16, 32, 64, 统一按 uint64 位处理
        typeId = TypeId::UINT64 << 4;// uint64_t
        buffer.push_back(typeId);
        // 使用LEB128编码
        auto value = static_cast<uint64_t>(arg);
        encodeLEB128(value, buffer);
    } else if constexpr (std::is_signed_v<CleanType> && std::is_integral_v<CleanType>) {
        // 有符号整型int16, 32, 64, 统一按 uint64 位处理
        typeId = TypeId::UINT64 << 4;// uint64_t
        uint64_t tmp = arg;
        if (arg < 0) {
            tmp = -arg;
            typeId |= 1 << 3;
        }
        buffer.push_back(typeId);
        encodeLEB128(tmp, buffer);
    } else if constexpr (std::is_floating_point_v<CleanType>) {
        // 浮点类型
        if constexpr (sizeof(CleanType) == 4) {
            typeId = TypeId::FLOAT << 4;// float
        } else if constexpr (sizeof(CleanType) == 8) {
            typeId = TypeId::DOUBLE << 4;// double
        }
        buffer.push_back(typeId);
        buffer.insert(buffer.end(), reinterpret_cast<const uint8_t *>(&arg),
                      reinterpret_cast<const uint8_t *>(&arg) + sizeof(CleanType));
    }
    // 字符串类型
    else if constexpr (std::is_same_v<CleanType, std::string> ||
                       std::is_same_v<CleanType, std::string_view>) {
        typeId = TypeId::STR << 4;
        buffer.push_back(typeId);
        // 字符串长度
        uint32_t length = arg.length();
        encodeLEB128(length, buffer);
        // 字符串内容
        buffer.insert(buffer.end(), reinterpret_cast<const uint8_t *>(arg.data()),
                      reinterpret_cast<const uint8_t *>(arg.data()) + length);
    }
    // C风格字符串 (const char* 和 char*)
    else if constexpr (std::is_pointer_v<CleanType> &&
                       (std::is_same_v<std::remove_const_t<std::remove_pointer_t<CleanType>>, char>) ) {
        typeId = TypeId::STR << 4;
        buffer.push_back(typeId);

        if (arg == nullptr) {
            // 空字符串，长度为0
            buffer.push_back(0);// LEB128 for 0
        } else {
            // 字符串长度
            uint32_t length = std::strlen(arg);
            encodeLEB128(length, buffer);
            // 字符串内容
            buffer.insert(buffer.end(), reinterpret_cast<const uint8_t *>(arg),
                          reinterpret_cast<const uint8_t *>(arg) + length);
        }
    }
    // 字符数组（字符串字面量 "hello"）
    else if constexpr (std::is_array_v<CleanType> &&
                       std::is_same_v<std::remove_const_t<std::remove_extent_t<CleanType>>, char>) {
        typeId = TypeId::STR << 4;
        buffer.push_back(typeId);
        // 字符串长度
        uint32_t length = std::strlen(arg);
        encodeLEB128(length, buffer);
        // 字符串内容
        buffer.insert(buffer.end(), reinterpret_cast<const uint8_t *>(arg),
                      reinterpret_cast<const uint8_t *>(arg) + length);
    }
    // 普通指针类型 (除了已处理的 char*)
    else if constexpr (std::is_pointer_v<CleanType> &&
                       !std::is_same_v<std::remove_const_t<std::remove_pointer_t<CleanType>>, char>) {
        typeId = TypeId::UINT64 << 4;// 与 uint64_t 相同
        buffer.push_back(typeId);
        auto value = reinterpret_cast<uint64_t>(arg);
        encodeLEB128(value, buffer);
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
    static void log(std::string_view sourceLocation, std::string_view function,
                    LogLevel level, std::string_view tag, fmt::format_string<Args...> format, Args &&...args) {
        if (level < logLevel_) {
            return;
        }

        std::string formatStr = fmt::format("{} [{} {}] {}", tag, sourceLocation, function, format.str);
        std::vector<uint8_t> argStore;
        constexpr uint8_t argc = sizeof...(args);
        if constexpr (argc > 0) {
            // 计算参数序列化后的最小大小
            constexpr size_t minSize = getMinSerializedSize<Args...>();
            argStore.reserve(minSize);
            // 使用折叠表达式序列化参数
            (serializeArg(argStore, std::forward<Args>(args)), ...);
        }

        LogEntry entry(level, formatStr, argc, argStore);

        if (logHandler_) {
            logHandler_(entry);
            return;
        }

        for (auto &sink: sinks_) {
            sink->log(entry);
        }
    }

private:
    static LogLevel logLevel_;
    static LogHandler logHandler_;
    static std::vector<std::shared_ptr<BaseSink>> sinks_;
};

#define QSLOG(level, tag, format, ...) \
    qslog::Logger::log(QSLOG_BASE_SOURCE_LOCATION, __FUNCTION__, level, tag, FMT_STRING(format), ##__VA_ARGS__)

#define QSLOGV(format, ...) (QSLOG(qslog::LogLevel::VERBOSE, QSLOG_TAG, format, ##__VA_ARGS__))
#define QSLOGD(format, ...) (QSLOG(qslog::LogLevel::DEBUG, QSLOG_TAG, format, ##__VA_ARGS__))
#define QSLOGI(format, ...) (QSLOG(qslog::LogLevel::INFO, QSLOG_TAG, format, ##__VA_ARGS__))
#define QSLOGW(format, ...) (QSLOG(qslog::LogLevel::WARN, QSLOG_TAG, format, ##__VA_ARGS__))
#define QSLOGE(format, ...) (QSLOG(qslog::LogLevel::ERROR, QSLOG_TAG, format, ##__VA_ARGS__))
#define QSLOGF(format, ...) (QSLOG(qslog::LogLevel::FATAL, QSLOG_TAG, format, ##__VA_ARGS__))
}// namespace qslog

#endif//QSLOG_LOGGER_H
