//
// Created by jingle on 2025/4/6.
//

#include "FormatIdManager.h"

#include <utility>

namespace qslog {

std::vector<std::shared_ptr<FormatEntry>> FormatIdManager::formatEntries_;
std::mutex FormatIdManager::mutex_;
OnNewFormatEntryCallback FormatIdManager::onNewFormatEntryCallback_ = nullptr;

void FormatIdManager::registerFormatId(uint16_t &formatId, std::shared_ptr<FormatEntry> formatEntry) {
    std::lock_guard<std::mutex> lock(mutex_);
    formatId = formatEntries_.size();
    formatEntries_.emplace_back(formatEntry);
    formatEntry->formatId_ = formatId;
    if (onNewFormatEntryCallback_) {
        onNewFormatEntryCallback_(formatEntry);
    }
}

std::shared_ptr<FormatEntry> FormatIdManager::getFormatEntry(uint16_t formatId) {
    return formatEntries_[formatId];
}
void FormatIdManager::setOnNewFormatEntryCallback(OnNewFormatEntryCallback callback) {
    onNewFormatEntryCallback_ = std::move(callback);
}
}// namespace qslog