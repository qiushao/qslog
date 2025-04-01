#include "qslog/common.h"

namespace qslog {

std::string_view getLevelName(qslog::LogLevel level) {
    static constexpr std::array<std::string_view, 8> levelNames = {
            "",       // 0, unused
            "VERBOSE",// 1
            "DEBUG",  // 2
            "INFO",   // 3
            "WARN",   // 4
            "ERROR",  // 5
            "FATAL",  // 6
            "SILENT"  // 7
    };
    return levelNames[static_cast<size_t>(level)];
}

// 辅助函数：从路径中提取文件名
std::string_view extractFilename(std::string_view path) {
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string_view::npos) {
        return path; // 没有路径分隔符，返回整个字符串
    }
    return path.substr(pos + 1);
}

}// namespace qslog