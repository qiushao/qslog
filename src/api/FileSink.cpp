#include "qslog/FileSink.h"
#include "OSUtils.h"
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <iomanip>
#include <sstream>

namespace qslog {

FileSink::FileSink(std::string_view name, std::string_view fileName, bool truncate)
    : BaseSink(name), fileName_(fileName) {
    openFile(truncate);
}

FileSink::~FileSink() {
    if (outFile_.is_open()) {
        outFile_.close();
    }
}

void FileSink::log(const LogEntry &entry) {
    if (!outFile_.is_open()) {
        return;
    }

    static int32_t pid = OSUtils::getPid();
    auto levelName = getLevelName(entry.level);
    fmt::memory_buffer buf;
    fmt::vformat_to(fmt::appender(buf), "{:%Y-%m-%d %H:%M:%S} {} {} {} {}: {}\n",
                    fmt::make_format_args(entry.time, pid, entry.tid, levelName, entry.tag, entry.msg));

    outFile_.write(buf.data(), buf.size());
}

void FileSink::openFile(bool truncate) {
    std::ios_base::openmode mode = std::ios::out;
    if (truncate) {
        mode |= std::ios::trunc;
    } else {
        mode |= std::ios::app;
    }
    outFile_.open(fileName_, mode);
}

}// namespace qslog