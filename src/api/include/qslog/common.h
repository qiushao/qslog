#ifndef QSLOG_COMMON_H
#define QSLOG_COMMON_H

#include <array>
#include <chrono>
#include <functional>
#include <string>

namespace qslog {

enum LogLevel {
    ALL = 0,
    VERBOSE = 1,
    DEBUG = 2,
    INFO = 3,
    WARN = 4,
    ERROR = 5,
    FATAL = 6,
    SILENT = 7
};

enum EntryType {
    INFO_ENTRY = 0,
    FORMAT_ENTRY = 1,
    LOG_ENTRY = 2
};

enum ArgTypeId {
    BOOL = 0,
    CHAR = 1,
    UINT8 = 2,
    UINT64 = 3,
    INT8 = 4,
    INT64 = 5,
    FLOAT = 6,
    DOUBLE = 7,
    STR = 8
};

char getLevelName(LogLevel level);

std::string formatTimespec(uint64_t ts);

std::string_view getBaseFilename(std::string_view path);

size_t encodeLEB128(uint64_t value, uint8_t *output);

void encodeLEB128(uint64_t value, std::vector<uint8_t> &output);

uint64_t decodeLEB128(const uint8_t *input, size_t size, size_t *bytesRead);

uint64_t decodeLEB128(std::ifstream &inFile);

}// namespace qslog
#endif//QSLOG_COMMON_H