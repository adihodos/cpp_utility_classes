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

#include "shared_handle.h"

/**
 *\brief RAII class for different OS handles.
 *  A handle is an "obfuscated pointer" to a resource allocated by a system
 *  (socket descriptors, file descriptors, etc...).
 *
 */
template<typename management_policy>
class scoped_handle {
public :
    typedef management_policy                           mpolicy_t;
    typedef typename mpolicy_t::handle_t                handle_t;
    typedef typename mpolicy_t::handle_ptr_t            handle_ptr_t;
    typedef typename mpolicy_t::handle_ref_t            handle_ref_t;
    typedef typename mpolicy_t::handle_const_ref_t      handle_const_ref_t;
    typedef scoped_handle<management_policy>            self_t;

private :
    /*!< Owned handle */
    handle_t    handle_;

    struct helper_t {
        int member;
    };

    handle_t get() const {
        return handle_;
    }

    handle_ptr_t get_impl_ptr() {
        return &handle_;
    }

    handle_t release() {
        handle_t tmpHandle = handle_;
        handle_ = mpolicy_t::null_handle();
        return tmpHandle;
    }

    void release(handle_t newValue) {
        if (handle_ != newValue) {
            mpolicy_t::dispose(handle_);
            handle_ = newValue;
        }
    }

    void swap(self_t& rhs) {
        std::swap(handle_, rhs.handle_);
    }

public :
    /**
     * \brief Default constructor. Initializes object with a non valid handle of
     * the corresponding type.
     */
    scoped_handle() : handle_(mpolicy_t::null_handle()) {}

    /**
     * \brief Initialize with an existing handle. The object assumes ownership
     * of the handle.
     */
    explicit scoped_handle(handle_t newHandle) : handle_(newHandle) {}

    scoped_handle(self_t&& right) {
        handle_ = right.release();
    }

    scoped_handle(const self_t&) = delete;

    ~scoped_handle() {
        mpolicy_t::dispose(handle_);
    }

    operator int helper_t::*() const {
        return handle_ == mpolicy_t::null_handle() ? nullptr :
            &helper_t::member;
    }

    bool operator!() const {
        return handle_ == mpolicy_t::null_handle();
    }

    friend inline handle_t scoped_handle_get(const self_t& sh) {
        return sh.get();
    }

    /**
     * \brief Release ownership of the handle to the caller.
     */
    friend inline handle_t scoped_handle_release(self_t& sh) {
        return sh.release();
    }

    friend inline void scoped_handle_reset(
        self_t& sh, 
        typename self_t::handle_t newValue = self_t::mpolicy_t::null_handle()
        )
    {
        sh.release(newValue);
    }

    /**
     * \brief Returns a pointer to the owned handle.
     */
    friend inline handle_ptr_t scoped_handle_get_impl(self_t& sh) {
        return sh.get_impl();
    }

    /**
     * \brief Swaps contents with another object of this type.
     */
    friend inline void swap(self_t& left, self_t& right) {
        return left.swap(right);
    }

    self_t& operator=(const self_t&) = delete;

    self_t& operator=(self_t&& right) {
        if (this != &right) {
            mpolicy_t::dispose(handle_);
            handle_ = right.release();
        }
        return *this;
    }
};

template<typename T>
inline bool operator==(const typename T::handle_t& left,
                       const scoped_handle<T>& right) {
    return left == scoped_handle_get(right);
}

template<typename T>
inline bool operator==(const scoped_handle<T>& left,
                       const typename T::handle_t& right) {
    return right == left;
}

template<typename T>
inline bool operator!=(const typename T::handle_t& left,
                       const scoped_handle<T>& right) {
    return !(left == right);
}

template<typename T>
inline bool operator!=(const scoped_handle<T>& left,
                       const typename T::handle_t& right) {
    return right != left;
}
