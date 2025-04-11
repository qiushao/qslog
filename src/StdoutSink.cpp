//
// Created by jingle on 2025/3/13.
//

#include "qslog/StdoutSink.h"
#include "qslog/OSUtils.h"
#include <chrono>

namespace qslog {

void StdoutSink::log(LogLevel level, const char *tag, const char *file, uint16_t line, const char *function, const char *msg) {
    const char *format = "{} {} {} {} {} [{}:{} {}] {}";
    auto now = std::chrono::system_clock::now();
    static uint32_t pid = OSUtils::getPid();
    static thread_local uint32_t tid = OSUtils::getTid();
    char levelName = getLevelName(level);
    auto formatArgs = fmt::make_format_args(now, levelName, pid, tid, tag, file, line, function, msg);
    fmt::memory_buffer buf;
    fmt::vformat_to(fmt::appender(buf), format, formatArgs);
    std::string_view str{buf.data(), buf.size()};
    std::cout << str << std::endl;
}
}// namespace qslog