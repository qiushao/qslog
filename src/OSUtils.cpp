#include "OSUtils.h"
#include <pthread.h>
#include <unistd.h>

namespace qslog {

size_t OSUtils::getPid() {
    static size_t cachedPid = getpid();
    return cachedPid;
}

static size_t _getTid() {
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

size_t OSUtils::getTid() {
    static thread_local size_t cachedTid = _getTid();
    return cachedTid;
}

}// namespace qslog