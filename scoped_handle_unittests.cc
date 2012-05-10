#include <gtest/gtest.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include "scoped_handle.h"

struct fake_fd_policy : public handle_traits_base<int> {
    static int null_handle() {
        return -1;
    }

    static void dispose(int) {
    }
};

scoped_handle<fake_fd_policy> mk_handle() {
    return scoped_handle<fake_fd_policy>(3);
}

class ScopedHandleTestFixture : public ::testing::Test {
protected :
    scoped_handle<fake_fd_policy> fd1_;
    scoped_handle<fake_fd_policy> fd2_;

public :
    ScopedHandleTestFixture() : fd1_(), fd2_(2) {}

};

TEST_F(ScopedHandleTestFixture, eq_operators) {
    EXPECT_FALSE(fd1_);
    EXPECT_TRUE(!fd1_);
    EXPECT_TRUE(fd2_);
    EXPECT_TRUE(2 == fd2_);
    EXPECT_TRUE(fd2_ == 2);
    EXPECT_TRUE(2 != fd1_);
    EXPECT_TRUE(fd1_ != 2);
}

TEST_F(ScopedHandleTestFixture, default_init) {
    EXPECT_EQ(fake_fd_policy::null_handle(), scoped_handle_get(fd1_));
    EXPECT_EQ(2, scoped_handle_get(fd2_));
}

TEST_F(ScopedHandleTestFixture, move_constructors) {
    fd1_ = mk_handle();
    EXPECT_EQ(3, scoped_handle_get(fd1_));
}

TEST_F(ScopedHandleTestFixture, release_reset) {
    int fd = scoped_handle_release(fd2_);
    EXPECT_FALSE(fd2_);
    scoped_handle_reset(fd2_, fd);
    EXPECT_TRUE(fd == fd2_);
}

TEST_F(ScopedHandleTestFixture, swap) {
    const int old1 = scoped_handle_get(fd1_);
    const int old2 = scoped_handle_get(fd2_);

    swap(fd1_, fd2_);

    EXPECT_EQ(old1, fd2_);
    EXPECT_EQ(old2, fd1_);
}
