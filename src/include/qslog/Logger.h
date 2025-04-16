#ifndef QSLOG_LOGGER_H
#define QSLOG_LOGGER_H

#include "fmt/format.h"
#include "qslog/BaseSink.h"
#include "qslog/FormatIdManager.h"
#include "qslog/LogEntry.h"
#include "qslog/OSUtils.h"
#include "qslog/common.h"
#include "qslog/tscns.h"
#include <memory>

namespace qslog {

template<typename Arg>
static inline constexpr bool isCstring() {
    return fmt::detail::mapped_type_constant<Arg, char>::value == fmt::detail::type::cstring_type;
}

template<typename Arg>
static inline constexpr bool isString() {
    return fmt::detail::mapped_type_constant<Arg, char>::value == fmt::detail::type::string_type;
}

template<size_t CstringIdx>
static inline constexpr size_t getArgSizes(size_t* cstringSize) {
    return 0;
}

template<size_t CstringIdx, typename Arg, typename... Args>
static inline constexpr size_t getArgSizes(size_t* cstringSize, const Arg& arg,
                                           const Args&... args) {
    if constexpr (isCstring<Arg>()) {
        size_t len = strlen(arg) + 1;
        cstringSize[CstringIdx] = len;
        return len + getArgSizes<CstringIdx + 1>(cstringSize, args...);
    }
    else if constexpr (isString<Arg>()) {
        size_t len = arg.size() + 1;
        return len + getArgSizes<CstringIdx>(cstringSize, args...);
    }
    else {
        return sizeof(Arg) + getArgSizes<CstringIdx>(cstringSize, args...);
    }
}

template<size_t CstringIdx>
static inline constexpr char* encodeArgs(size_t* cstringSize, char* out) {
    return out;
}

template<size_t CstringIdx, typename Arg, typename... Args>
static inline constexpr char* encodeArgs(size_t* cstringSize, char* out, Arg&& arg,
                                         Args&&... args) {
    if constexpr (isCstring<Arg>()) {
        memcpy(out, arg, cstringSize[CstringIdx]);
        return encodeArgs<CstringIdx + 1>(cstringSize, out + cstringSize[CstringIdx],
                                          std::forward<Args>(args)...);
    }
    else if constexpr (isString<Arg>()) {
        size_t len = arg.size();
        memcpy(out, arg.data(), len);
        out[len] = 0;
        return encodeArgs<CstringIdx>(cstringSize, out + len + 1, std::forward<Args>(args)...);
    }
    else {
        memcpy(out, &arg, sizeof(Arg));
        return encodeArgs<CstringIdx>(cstringSize, out + sizeof(Arg), std::forward<Args>(args)...);
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
static void serializeArg(LogEntry &entry, T &&arg) {
    using CleanType = std::remove_reference_t<T>;

    if constexpr (std::is_same_v<CleanType, bool> || std::is_same_v<CleanType, char> || std::is_same_v<CleanType, uint8_t> || std::is_same_v<CleanType, int8_t> || std::is_floating_point_v<CleanType>) {
        memcpy(entry.argStore_ + entry.argsSize_, &arg, sizeof(arg));
    } else if constexpr (std::is_unsigned_v<CleanType> && std::is_integral_v<CleanType>) {
        // 无符号整型 uint16, 32, 64, 统一按 uint64 位处理
        auto value = static_cast<uint64_t>(arg);
        auto ret = encodeLEB128(value, entry.argStore_);
        entry.argsSize_ += ret;
    } else if constexpr (std::is_signed_v<CleanType> && std::is_integral_v<CleanType>) {
        // 有符号整型int16, 32, 64, 统一按 int64 位处理
        auto value = static_cast<int64_t>(arg);
        auto ret = encodeLEB128(value, entry.argStore_);
        entry.argsSize_ += ret;
    }
    // 字符串类型
    else if constexpr (std::is_same_v<CleanType, std::string> ||
                       std::is_same_v<CleanType, std::string_view>) {
        uint32_t length = arg.length() + 1;//str.length 不包含结尾的 '\0'
        memcpy(entry.argStore_ + entry.argsSize_, arg.data(), length);
        entry.argsSize_ += length;
    }
    // C风格字符串 (const char* 和 char*)
    else if constexpr (std::is_pointer_v<CleanType> &&
                       (std::is_same_v<std::remove_const_t<std::remove_pointer_t<CleanType>>, char>) ) {
        if (arg == nullptr) {
            // 空字符串，长度为0
            memcpy(entry.argStore_ + entry.argsSize_, "\0", 1);
            ++entry.argsSize_;
        } else {
            uint32_t length = std::strlen(arg) + 1;//strlen 不包含结尾的 '\0'
            memcpy(entry.argStore_ + entry.argsSize_, arg, length);
            entry.argsSize_ += length;
        }
    }
    // 字符数组（字符串字面量 "hello"）
    else if constexpr (std::is_array_v<CleanType> &&
                       std::is_same_v<std::remove_const_t<std::remove_extent_t<CleanType>>, char>) {
        uint32_t length = std::strlen(arg) + 1;//strlen 不包含结尾的 '\0'
        memcpy(entry.argStore_ + entry.argsSize_, arg, length);
        entry.argsSize_ += length;
    }
    // 普通指针类型 (除了已处理的 char*)
    else if constexpr (std::is_pointer_v<CleanType> &&
                       !std::is_same_v<std::remove_const_t<std::remove_pointer_t<CleanType>>, char>) {
        auto value = reinterpret_cast<uint64_t>(arg);
        auto ret = encodeLEB128(value, entry.argStore_);
        entry.argsSize_ += ret;
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
    static void init();

    static void setLogLevel(LogLevel level);

    static LogLevel getLogLevel();

    static void addSink(std::shared_ptr<BaseSink> sink);

    static void sync();

    template<typename... Args>
    static void log(uint16_t &formatId, LogLevel level, const char *tag,
                    const char *file, uint16_t line, const char *function,
                    fmt::format_string<Args...> format, Args &&...args) {
        if (level < logLevel_) {
            return;
        }

        static thread_local uint32_t tid = OSUtils::getTid();
        constexpr uint8_t argc = sizeof...(args);
        constexpr size_t numCstring = fmt::detail::count<isCstring<Args>()...>();
        size_t cStringSizes[std::max(numCstring, (size_t)1)];
        // 防止在 leb128 时，编码后的数据长度比原本的还长。所以加上 (argc - numCstring)
        auto allocSize = getArgSizes<0>(cStringSizes, args...) + (argc - numCstring);

        //printf("allocSize = %u\n", allocSize);

        if (formatId == UINT16_MAX) {
            auto formatEntry = std::make_shared<FormatEntry>();
            formatEntry->logLevel_ = level;
            formatEntry->argc_ = argc;
            (parseArgType(formatEntry->argTypes_, std::forward<Args>(args)), ...);
            formatEntry->formatStr_ = fmt::format("{} [{}:{} {}] {}", tag, getBaseFilename(file), line, function, format.str.data());
            FormatIdManager::registerFormatId(formatId, formatEntry);
        }
        static thread_local uint8_t argsBuffer[2048];
        LogEntry logEntry{
                .formatId_ = formatId,
                .time_ = tscns_.rdns(),
                .tid_ = tid,
                .argsSize_ = 0,
                .argStore_ = argsBuffer};

        if constexpr (argc > 0) {
            (serializeArg(logEntry, std::forward<Args>(args)), ...);
        }

        for (auto &sink: sinks_) {
            sink->log(logEntry);
        }
    }

private:
    static LogLevel logLevel_;
    static std::vector<std::shared_ptr<BaseSink>> sinks_;
    static TSCNS tscns_;
};

#define QSLOG(level, tag, format, ...)                                                           \
    do {                                                                                         \
        static uint16_t qslogFormatId = UINT16_MAX;                                              \
        qslog::Logger::log(qslogFormatId, level, tag,                                            \
                           __FILE__, __LINE__, __FUNCTION__, FMT_STRING(format), ##__VA_ARGS__); \
    } while (0)

#define QSLOGV(format, ...) QSLOG(qslog::LogLevel::VERBOSE, QSLOG_TAG, format, ##__VA_ARGS__)
#define QSLOGD(format, ...) QSLOG(qslog::LogLevel::DEBUG, QSLOG_TAG, format, ##__VA_ARGS__)
#define QSLOGI(format, ...) QSLOG(qslog::LogLevel::INFO, QSLOG_TAG, format, ##__VA_ARGS__)
#define QSLOGW(format, ...) QSLOG(qslog::LogLevel::WARN, QSLOG_TAG, format, ##__VA_ARGS__)
#define QSLOGE(format, ...) QSLOG(qslog::LogLevel::ERROR, QSLOG_TAG, format, ##__VA_ARGS__)
#define QSLOGF(format, ...) QSLOG(qslog::LogLevel::FATAL, QSLOG_TAG, format, ##__VA_ARGS__)
}// namespace qslog

#endif//QSLOG_LOGGER_H
