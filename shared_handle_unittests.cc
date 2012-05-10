#include <gtest/gtest.h>
#include "shared_handle.h"

struct fake_fd_policy : public handle_traits_base<int> {
    static int null_handle() {
        return -1;
    }

    static void dispose(int) {
    }
};

struct tracer {
    int dummy_;
    static unsigned int constructed_;
    static unsigned int destroyed_;

    tracer(int val) : dummy_(val) {
    }

    ~tracer() {}

    static unsigned int get_constructed_count() {
        return constructed_;
    }

    static unsigned int get_destroyed_count() {
        return destroyed_;
    }

    bool operator==(const tracer& right) const {
        return dummy_ == right.dummy_;
    }

    bool operator!=(const tracer& right) const {
        return !(*this == right);
    }

    static tracer mk_tracer(int value) {
        ++tracer::constructed_;
        return tracer(value);
    }

    static void rm_tracer(int) {
        ++tracer::destroyed_;
    }

    static void reset_statitstics() {
        constructed_ = 0;
        destroyed_ = 0;
    }
};

unsigned int tracer::constructed_ = 0;
unsigned int tracer::destroyed_ = 0;

tracer g_null_tracer(-1);

struct tracer_policy : public handle_traits_base<tracer> {
    static tracer null_handle() {
        return g_null_tracer;
    }

    static void dispose(const tracer& tr) {
        if (tr != g_null_tracer)
            tracer::rm_tracer(tr.dummy_);
    }
};

typedef shared_handle<fake_fd_policy> shared_handle_t;

shared_handle<fake_fd_policy> mkFakeFD() {
    return shared_handle<fake_fd_policy>(2);
}

shared_handle<fake_fd_policy> mkFakeFD(const shared_handle<fake_fd_policy>& orig) {
    return shared_handle<fake_fd_policy>(orig);
}

TEST(shared_handle_test, one) {
    shared_handle<fake_fd_policy> sh1(1);

    EXPECT_EQ(1, sh1.refcount());
    EXPECT_TRUE(sh1.shouldBeErased());

    {
        shared_handle<fake_fd_policy> sh2(sh1);
        EXPECT_EQ(2, sh1.refcount());
        EXPECT_EQ(2, sh2.refcount());
        EXPECT_FALSE(sh1.shouldBeErased());

        {
            shared_handle<fake_fd_policy> sh3;
            sh3 = sh1;
            EXPECT_EQ(3, sh1.refcount());
            EXPECT_EQ(3, sh2.refcount());
            EXPECT_EQ(3, sh3.refcount());
            EXPECT_FALSE(sh1.shouldBeErased());
            EXPECT_FALSE(sh2.shouldBeErased());
            EXPECT_FALSE(sh3.shouldBeErased());
        }
    }

    EXPECT_EQ(1, sh1.refcount());
    EXPECT_TRUE(sh1.shouldBeErased());
}

TEST(shared_handle, two) {
    shared_handle<fake_fd_policy> sh;
    shared_handle<fake_fd_policy> sh2(3);

    sh = mkFakeFD();
    EXPECT_EQ(1, sh.refcount());
    EXPECT_TRUE(sh.shouldBeErased());

    sh = mkFakeFD(sh2);
    EXPECT_EQ(2, sh.refcount());
    EXPECT_FALSE(sh.shouldBeErased());
}

TEST(shared_handle, swap_single) {
    shared_handle<fake_fd_policy> shA(1);
    shared_handle<fake_fd_policy> shB(2);

    swap(shA, shB);
    EXPECT_EQ(1, shA.refcount());
    EXPECT_EQ(1, shB.refcount());
    EXPECT_EQ(2, shared_handle_get(shA));
    EXPECT_EQ(1, shared_handle_get(shB));
}

TEST(shared_handle, swap_multiple) {
    shared_handle<fake_fd_policy> shA(1);
    shared_handle_t sha2, sha3, sha4;
    sha2 = sha3 = sha4 = shA;

    shared_handle_t shB(2);
    shared_handle_t shb2, shb3, shb4;
    shb2 = shb3 = shb4 = shB;

    swap(sha3, shb2);
    EXPECT_EQ(4, sha3.refcount());
    EXPECT_EQ(4, shb2.refcount());
    EXPECT_EQ(2, shared_handle_get(sha3));
    EXPECT_EQ(1, shared_handle_get(shb2));
}

typedef shared_handle<tracer_policy> shared_tracer_t;

TEST(shared_handle, correctness) {
    {
        shared_tracer_t st(tracer::mk_tracer(1));
        shared_tracer_t st1(st);
        shared_tracer_t st2;
        st2 = st1;
    }

    EXPECT_EQ(1, tracer::get_constructed_count());
    EXPECT_EQ(1, tracer::get_destroyed_count());
}

TEST(shared_handle, correctness_assign) {
    tracer::reset_statitstics();
    {
        shared_tracer_t st1(tracer::mk_tracer(1));
        shared_tracer_t st2, st5, st6;
        st2 = st5 = st6 = st1;
        shared_tracer_t st3(tracer::mk_tracer(2));
        st1 = st3;
        st2 = st3;
    }

    EXPECT_EQ(2, tracer::get_constructed_count());
    EXPECT_EQ(2, tracer::get_destroyed_count());
}
