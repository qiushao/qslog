//
// Created by jingle on 2025/4/6.
//

#ifndef QSLOG_FORMATIDMANAGER_H
#define QSLOG_FORMATIDMANAGER_H

#include "qslog/LogEntry.h"
#include <memory>
#include <mutex>
#include <string_view>
#include <vector>

namespace qslog {

typedef std::function<void(std::shared_ptr<FormatEntry>)> OnNewFormatEntryCallback;

class FormatIdManager {
public:
    static void registerFormatId(uint16_t &formatId, std::shared_ptr<FormatEntry> formatEntry);

    static std::shared_ptr<FormatEntry> getFormatEntry(uint16_t formatId);

    static void setOnNewFormatEntryCallback(OnNewFormatEntryCallback callback);

private:
    static std::vector<std::shared_ptr<FormatEntry>> formatEntries_;
    static std::mutex mutex_;
    static OnNewFormatEntryCallback onNewFormatEntryCallback_;
};

}// namespace qslog

#endif//QSLOG_FORMATIDMANAGER_H
