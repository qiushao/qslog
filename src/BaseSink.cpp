//
// Created by jingle on 2025/3/13.
//

#include "qslog/BaseSink.h"

namespace qslog {

BaseSink::BaseSink(std::string_view sinkName) : sinkName_(sinkName) {
}
std::string_view BaseSink::getName() {
    return sinkName_;
}

}// namespace qslog
