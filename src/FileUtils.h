#ifndef QSLOG_FILEUTILS_H
#define QSLOG_FILEUTILS_H

#include <cstddef>
#include <string_view>
namespace qslog {

class FileUtils {
public:
    static size_t getFileSize(std::string_view path);

    static int openFileToRead(std::string_view path);

    static int openFileToWrite(std::string_view path, bool truncate = false);

    static uint8_t *mapFileToWrite(int fd, long offset, size_t len);
};

}// namespace qslog

#endif//QSLOG_FILEUTILS_H
