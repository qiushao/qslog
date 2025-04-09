#include "qslog/FileUtils.h"
#include <fcntl.h>
#include <fstream>
#include <sys/mman.h>
#include <sys/stat.h>

namespace qslog {

size_t FileUtils::getFileSize(std::string_view path) {
    struct stat st {};
    if (stat(path.data(), &st) == 0) {
        return st.st_size;
    }
    return 0;
}

int FileUtils::openFileToWrite(std::string_view path, bool truncate) {
    int flags = O_RDWR | O_CREAT;
    if (truncate) {
        flags |= O_TRUNC;
    }
    int fd = open(path.data(), flags, 0666);
    if (fd == -1) {
        perror("openFileToWrite");
    }
    return fd;
}

uint8_t *FileUtils::mapFileToWrite(int fd, long offset, size_t len) {
    auto *map = static_cast<uint8_t *>(mmap(nullptr, len, PROT_WRITE, MAP_SHARED, fd, offset));
    if (map == MAP_FAILED) {
        perror("mapFileToWrite");
        return nullptr;
    }
    return map;
}

int FileUtils::openFileToRead(std::string_view path) {
    int fd = open(path.data(), O_RDONLY);
    if (fd == -1) {
        perror("openFileToRead");
    }
    return fd;
}

}// namespace qslog