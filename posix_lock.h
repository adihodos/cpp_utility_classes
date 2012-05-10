//
//Copyright (c) 2011,2012, Adrian Hodos
//All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//    1. Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//    2. Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//    3. All advertising materials mentioning features or use of this software
//       must display the following acknowledgement:
//       This product includes software developed by the <authors>.
//    4. Neither the name of the author(s) nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ''AS IS'' AND ANY
//  EXPRESS OR IMPLIED WARRANTIES,
//  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
//  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#pragma once

#include <pthread.h>

struct posix_mutex_traits {
    typedef     pthread_mutex_t lock_t;

    static bool initialize(lock_t& pt) {
        return pthread_mutex_init(&pt, nullptr) == 0;
    }

    static void dispose(lock_t& mtx) {
        pthread_mutex_destroy(&mtx);
    }

    static void acquire(lock_t& mtx) {
        pthread_mutex_lock(&mtx);
    }

    static void release(lock_t& mtx) {
        pthread_mutex_unlock(&mtx);
    }

    static void try_acquire(lock_t& mtx) {
        pthread_mutex_trylock(&mtx);
    }
};


struct posix_rwlock_traits {
    typedef pthread_rwlock_t    mutex_t;

    static bool initialize(mutex_t& rwlock) {
        return pthread_rwlock_init(&rwlock, nullptr);
    }

    static bool destroy(mutex_t& rwlock) {
        return pthread_rwlock_destroy(&rwlock);
    }

    static void acquire_rd(mutex_t& rwlock) {
        pthread_rwlock_rdlock(&rwlock);
    }

    static void acquire_wr(mutex_t& rwlock) {
        pthread_rwlock_wrlock(&rwlock);
    }

    static bool try_acquire_rd(mutex_t& rwlock) {
        return pthread_rwlock_tryrdlock(&rwlock) == 0;
    }

    static bool try_acquire_wr(mutex_t& rwlock) {
        return pthread_rwlock_trywrlock(&rwlock) == 0;
    }
};
