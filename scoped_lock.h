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

/**
 * \brief RAII class for an OS specific synchronization primitive
 *      (critical section, mutex, etc). This class is best used in conjunction
 *      with the auto_lock class.
 * \see auto_lock
 */
template<typename LockTypeTraits>
class scoped_lock {
public :
    /*!< The primitive type */
    typedef typename LockTypeTraits::lock_t    lock_t;
    typedef scoped_lock<LockTypeTraits>        self_t;

private :
    /*!< Owned primitive */
    lock_t  lock_;

public :
    scoped_lock() {
        LockTypeTraits::initialize(lock_);
    }

    scoped_lock(const self_t&) = delete;
    self_t& operator=(const self_t&) = delete;

    /**
     * \brief Initialize with a handle to an existing primitive. The class
     *          assumes ownership of the primitive and will destroy it upon
     *          expiring.
     * \param lock OS-specific handle to an existing synchronization primitive.
     */
    explicit scoped_lock(lock_t& lock) : lock_(lock) {}

    ~scoped_lock() {
        LockTypeTraits::dispose(lock_);
    }

    /**
     * \brief Acquire the lock.
     * \remarks This call will block the calling thread, until the lock becomes
     *          available.
     */
    void acquire() {
        LockTypeTraits::acquire(lock_);
    }

    /**
     * \brief If owning the lock, release it.
     * \remarks Must be called after a call to acquire() or
     *          if try_acquire() returned true.
     *          Calling this function without actually owning the lock results
     *          in undefined behaviour (OS dependent).
     */
    void release() {
        LockTypeTraits::release(lock_);
    }

    /**
     * \brief Try acquiring the lock, but do not block waiting for it to become
     *          available.
     * \return True if the lock was acquired, false if not.
     */
    void try_acquire() {
        LockTypeTraits::try_acquire(lock_);
    }
};
