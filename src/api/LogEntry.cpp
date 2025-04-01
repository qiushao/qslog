#include "qslog/LogEntry.h"
#include "OSUtils.h"

namespace qslog {
LogEntry::LogEntry(std::string_view file, uint16_t line, LogLevel level, std::string_view tag, std::string_view msg)
    : tid_(OSUtils::getTid()),
      time_(0),
      file_(file),
      line_(line),
      level_(level),
      tag_(tag),
      msg_(msg) {
}

}// namespace qslog