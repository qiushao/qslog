#include "qslog/common.h"
#include <fstream>

namespace qslog {

char getLevelName(qslog::LogLevel level) {
    static char levelChar[] = {'A', 'V', 'D', 'I', 'W', 'E', 'F', 'S'};
    return levelChar[static_cast<size_t>(level)];
}

std::string formatTimespec(uint64_t ts) {
    // 将纳秒转换为秒和毫秒部分
    auto seconds = (time_t) (ts / 1000000000);
    int milliseconds = (int) ((ts % 1000000000) / 1000000);

    // 预分配足够的空间以避免重新分配
    char buffer[24];

    // 获取本地时间结构
    struct tm tm_info {};
    localtime_r(&seconds, &tm_info);

    // 格式化日期时间部分 (YYYY-MM-DD HH:MM:SS)
    size_t pos = strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm_info);

    // 添加毫秒部分 (.mmm)
    pos += snprintf(buffer + pos, sizeof(buffer) - pos, ".%03d", milliseconds);

    return {buffer, pos};
}

std::string_view getBaseFilename(std::string_view path) {
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string_view::npos) {
        return path;
    }
    return path.substr(pos + 1);
}

/**
 * 从LEB128格式解码无符号整数
 *
 * @param input 输入缓冲区
 * @param size 输入缓冲区大小
 * @param bytesRead 输出参数，存储读取的字节数
 * @return 解码后的无符号整数
 */
uint64_t decodeLEB128(const uint8_t *input, size_t size, size_t *bytesRead) {
    uint64_t result = 0;
    uint8_t shift = 0;
    size_t pos = 0;
    uint8_t byte;

    do {
        if (pos >= size) {
            *bytesRead = pos;
            return result;
        }

        byte = input[pos++];
        result |= static_cast<uint64_t>(byte & 0x7F) << shift;
        shift += 7;
    } while (byte & 0x80);

    *bytesRead = pos;
    return result;
}

uint64_t decodeLEB128(std::ifstream &inFile) {
    uint64_t result = 0;
    uint8_t shift = 0;
    uint8_t byte;

    do {
        inFile.read(reinterpret_cast<char *>(&byte), 1);
        if (!inFile) {
            throw std::runtime_error("Unexpected end of file while decoding LEB128");
        }
        result |= static_cast<uint64_t>(byte & 0x7F) << shift;
        shift += 7;
    } while (byte & 0x80);

    return result;
}

}// namespace qslog