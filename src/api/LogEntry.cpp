#include "qslog/LogEntry.h"
#include "OSUtils.h"

namespace qslog {
LogEntry::LogEntry(LogLevel level, std::string_view tag, std::string_view msg)
    : tid(OSUtils::getTid()),
      level(level),
      tag(tag),
      time(std::chrono::system_clock::now()),
      msg(msg) {
}

}// namespace qslog