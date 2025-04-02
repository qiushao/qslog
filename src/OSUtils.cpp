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

uint64_t OSUtils::realTimeMillisecond() {
    struct timespec ts {};
    clock_gettime(CLOCK_REALTIME_COARSE, &ts);
    return (uint64_t(ts.tv_sec) * 1000000000LL + ts.tv_nsec) / 1000000;
}

}// namespace qslog