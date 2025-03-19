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
    fmt::vformat_to(fmt::appender(buf_), "{:%Y-%m-%d %H:%M:%S} {} {} {} {}: {}\n",
                    fmt::make_format_args(entry.time, pid, entry.tid, levelName, entry.tag, entry.msg));

    if (buf_.size() > kBufferSize) {
        outFile_.write(buf_.data(), buf_.size());
        buf_.resize(0);
    }
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