#include "qslog/common.h"
#include <iomanip>

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

}// namespace qslog