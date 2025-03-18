#include "OSUtils.h"
#include <cstdint>
#include <pthread.h>
#include <unistd.h>

namespace qslog {

int32_t OSUtils::getPid() {
    static int32_t cachedPid = getpid();
    return cachedPid;
}

static int32_t _getTid() {
#if defined(__APPLE__) || defined(__IOS__)
    uint64_t tid;
    pthread_threadid_np(NULL, &tid);
    return tid;
#elif defined(__linux__) || defined(__ANDROID__)
    return gettid();
#else
    return 0;
#endif
}

int32_t OSUtils::getTid() {
    static thread_local int32_t cachedTid = _getTid();
    return cachedTid;
}

}// namespace qslog