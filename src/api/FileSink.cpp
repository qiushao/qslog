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

    std::string time = fmt::format("{:%Y-%m-%d %H:%M:%S}", entry.time);

    // 写入日志条目
    outFile_ << OSUtils::getPid() << " "
             << OSUtils::getTid() << " "
             << time << " "
             << getLevelName(entry.level) << " "
             << entry.tag << " "
             << entry.msg << std::endl;
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