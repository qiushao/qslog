#include "qslog/common.h"

namespace qslog {

char getLevelName(qslog::LogLevel level) {
    static char levelChar[] = {'A', 'V', 'D', 'I', 'W', 'E', 'F', 'S'};
    return levelChar[static_cast<size_t>(level)];
}

}// namespace qslog