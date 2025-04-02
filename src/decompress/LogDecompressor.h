#ifndef QSLOG_LOGDECOMPRESSOR_H
#define QSLOG_LOGDECOMPRESSOR_H

#include "qslog/LogEntry.h"
#include "qslog/common.h"
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace qslog {

class LogDecompressor {
public:
    LogDecompressor(const std::string &inputFile, const std::string &outputFile);
    ~LogDecompressor();

    bool decompress();

private:
    bool readEntry();
    bool readPidInfoEntry(uint8_t header);
    bool readTsInfoEntry(uint8_t header);
    bool readFormatEntry(uint8_t header);
    bool readLogEntry(uint8_t header);

    template<typename T>
    bool read(T &value) {
        if (inFile_.read(reinterpret_cast<char *>(&value), sizeof(T))) {
            return true;
        }
        return false;
    }

    bool readBuffer(void *buffer, size_t size) {
        if (inFile_.read(reinterpret_cast<char *>(buffer), size)) {
            return true;
        }
        return false;
    }

private:
    std::ifstream inFile_;
    std::ofstream outFile_;

    int32_t pid_ = 0;
    uint64_t lastTs_ = 0;
    std::unordered_map<uint16_t, std::string> formatMap_;
};

}// namespace qslog

#endif//QSLOG_LOGDECOMPRESSOR_H
