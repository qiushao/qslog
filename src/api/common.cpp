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

}// namespace qslog