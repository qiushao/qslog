#include <cstring>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <variant>

// 用于static_assert的辅助模板，始终返回false
template<typename T>
struct always_false : std::false_type {};

// 序列化单个参数的辅助函数
template<typename T>
static void serializeArg(std::ofstream &file, T &&arg) {
    using CleanType = std::remove_reference_t<T>;

    // 确定类型ID
    uint8_t typeId;

    // 布尔类型
    if constexpr (std::is_same_v<CleanType, bool>) {
        typeId = 1;
        file.write(reinterpret_cast<const char *>(&typeId), sizeof(typeId));
        file.write(reinterpret_cast<const char *>(&arg), sizeof(bool));
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
        file.write(reinterpret_cast<const char *>(&typeId), sizeof(typeId));
        file.write(reinterpret_cast<const char *>(&arg), sizeof(CleanType));
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
        file.write(reinterpret_cast<const char *>(&typeId), sizeof(typeId));
        file.write(reinterpret_cast<const char *>(&arg), sizeof(CleanType));
    }
    // 浮点类型
    else if constexpr (std::is_floating_point_v<CleanType>) {
        if constexpr (sizeof(CleanType) == 4) {
            typeId = 10;// float
        } else if constexpr (sizeof(CleanType) == 8) {
            typeId = 11;// double
        }
        file.write(reinterpret_cast<const char *>(&typeId), sizeof(typeId));
        file.write(reinterpret_cast<const char *>(&arg), sizeof(CleanType));
    }
    // 字符串类型
    else if constexpr (std::is_same_v<CleanType, std::string> ||
                       std::is_same_v<CleanType, std::string_view>) {
        typeId = 12;
        file.write(reinterpret_cast<const char *>(&typeId), sizeof(typeId));
        uint32_t length = arg.length();
        file.write(reinterpret_cast<const char *>(&length), sizeof(length));
        file.write(arg.data(), length);
    }
    // C风格字符串 (const char* 和 char*)
    else if constexpr (std::is_pointer_v<CleanType> &&
                       (std::is_same_v<std::remove_const_t<std::remove_pointer_t<CleanType>>, char>) ) {
        typeId = 12;
        file.write(reinterpret_cast<const char *>(&typeId), sizeof(typeId));
        if (arg == nullptr) {
            uint32_t length = 0;
            file.write(reinterpret_cast<const char *>(&length), sizeof(length));
        } else {
            uint32_t length = std::strlen(arg);
            file.write(reinterpret_cast<const char *>(&length), sizeof(length));
            file.write(arg, length);
        }
    }
    // 字符类型
    else if constexpr (std::is_same_v<CleanType, char>) {
        typeId = 13;
        file.write(reinterpret_cast<const char *>(&typeId), sizeof(typeId));
        file.write(reinterpret_cast<const char *>(&arg), sizeof(char));
    }
    // Unsupported type
    else {
        static_assert(always_false<CleanType>::value, "Unsupported type for serialization");
    }
}

template<typename... Args>
static void log(std::string_view file, uint32_t line, int level, std::string_view tag, std::string_view format, Args &&...args) {
    static std::mutex logMutex;
    static std::unordered_map<std::string, uint32_t> formatCache;
    static uint32_t nextFormatId = 0;
    static std::ofstream logFile("compressed_log.bin", std::ios::binary | std::ios::app);

    // 获取当前时间
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                             now.time_since_epoch())
                             .count();

    std::lock_guard<std::mutex> lock(logMutex);

    // 为格式字符串分配ID
    std::string formatStr(format);
    uint32_t formatId;

    auto it = formatCache.find(formatStr);
    if (it == formatCache.end()) {
        formatId = nextFormatId++;
        formatCache[formatStr] = formatId;

        // 写入新的格式字符串定义
        uint8_t recordType = 0;// 0表示格式字符串定义
        logFile.write(reinterpret_cast<const char *>(&recordType), sizeof(recordType));
        logFile.write(reinterpret_cast<const char *>(&formatId), sizeof(formatId));

        uint32_t formatLength = formatStr.length();
        logFile.write(reinterpret_cast<const char *>(&formatLength), sizeof(formatLength));
        logFile.write(formatStr.data(), formatLength);
    } else {
        formatId = it->second;
    }

    // 写入日志记录头
    uint8_t recordType = 1;// 1表示日志记录
    logFile.write(reinterpret_cast<const char *>(&recordType), sizeof(recordType));
    logFile.write(reinterpret_cast<const char *>(&timestamp), sizeof(timestamp));
    logFile.write(reinterpret_cast<const char *>(&formatId), sizeof(formatId));

    // 写入文件名和行号
    //    uint32_t fileNameLength = file.length();
    //    logFile.write(reinterpret_cast<const char*>(&fileNameLength), sizeof(fileNameLength));
    //    logFile.write(file.data(), fileNameLength);
    //    logFile.write(reinterpret_cast<const char*>(&line), sizeof(line));

    // 写入日志级别
    uint8_t levelValue = static_cast<uint8_t>(level);
    logFile.write(reinterpret_cast<const char *>(&levelValue), sizeof(levelValue));

    // 写入标签
    uint32_t tagLength = tag.length();
    logFile.write(reinterpret_cast<const char *>(&tagLength), sizeof(tagLength));
    logFile.write(tag.data(), tagLength);

    // 使用折叠表达式序列化参数
    (serializeArg(logFile, std::forward<Args>(args)), ...);

    // 确保数据写入磁盘
    logFile.flush();
}


int main() {
    std::string str = "just for today";
    std::mutex mutex;
    int *ptr;
    for (int i = 0; i < 10000; ++i) {
        log(__FILE__, __LINE__, 3, "qiushao", "bin log test {} {} {} ", i, 1.4 * i, str);
    }

    for (int i = 0; i < 10000; ++i) {
        log(__FILE__, __LINE__, 4, "qiushao", "bin log test {} {}", i, 1.4 * i);
    }
    return 0;
}
