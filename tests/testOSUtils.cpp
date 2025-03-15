#include <gtest/gtest.h>
#include "../src/OSUtils.h"
#include <thread>

TEST(OSUtilsTest, GetPid) {
    size_t pid = qslog::OSUtils::getPid();
    EXPECT_TRUE(pid > 0);
}

TEST(OSUtilsTest, GetTid) {
    size_t tid = qslog::OSUtils::getTid();
    EXPECT_TRUE(tid > 0);
}

TEST(OSUtilsTest, GetTidSameThreadConsistent) {
    size_t tid1 = qslog::OSUtils::getTid();
    size_t tid2 = qslog::OSUtils::getTid();
    EXPECT_EQ(tid1, tid2);
}

TEST(OSUtilsTest, GetTidDifferentThreadsDifferent) {
    size_t main_tid = qslog::OSUtils::getTid();
    size_t child_tid;
    std::thread child_thread([&child_tid]() {
        child_tid = qslog::OSUtils::getTid();
    });
    child_thread.join();
    EXPECT_NE(main_tid, child_tid);
}