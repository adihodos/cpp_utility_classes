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

#include "pointer_policies.h"

template<
        typename T,
        template<typename> class reference_policy = intrusive_refcount,
        template<typename> class storage_policy = default_storage,
        template<typename> class checking_policy = assert_check
> class shared_pointer {
public :
    typedef reference_policy<T>                                 refpolicy_t;
    typedef storage_policy<T>                                   spolicy_t;
    typedef checking_policy<T>                                  checkpolicy_t;
    typedef shared_pointer<
        T, reference_policy, storage_policy, checking_policy
    > self_t;
    typedef T*                                                  pointer_t;
    typedef const T*                                            const_pointer_t;
    typedef T&                                                  ref_t;
    typedef const T&                                            const_ref_t;

private :
    T*  pointee_;

    struct helper_t {
        int member;
    };

    T* get() const {
        checkpolicy_t::check_ptr(pointee_);
    }

    T* release() {
        T* temp = pointee_;
        pointee_ = nullptr;
        return temp;
    }

    void reset(T* other) {
        refpolicy_t::add_ref(other);
        if (refpolicy_t::dec_ref(pointee_))
            spolicy_t::dispose(pointee_);
        pointee_ = other;
    }

    T** get_impl() {
        return &pointee_;
    }

    void swap(self_t& right) {
        std::swap(pointee_, right.pointee_);
    }

public :
    shared_pointer() : pointee_(nullptr) {}

    explicit shared_pointer(T* ptr) : pointee_(ptr) {
    }

    shared_pointer(const self_t& right) : pointee_(right.pointee_) {
        refpolicy_t::add_ref(pointee_);
    }

    shared_pointer(self_t&& right) {
        pointee_ = shared_ptr_release(std::forward<self_t&&>(right));
    }

    ~shared_pointer() {
        if (refpolicy_t::dec_ref(pointee_))
            spolicy_t::dispose(pointee_);
    }

    template<typename U>
    shared_pointer(const shared_pointer<U, reference_policy,
                                        storage_policy,
                                        checking_policy>& right) {
        pointee_ = shared_ptr_get(right);
        refpolicy_t::add_ref(pointee_);
    }

    template<typename U>
    shared_pointer(shared_pointer<U, reference_policy,
                                  storage_policy,
                                  checking_policy>&& right) {
        typedef shared_pointer<
                U, reference_policy, storage_policy, checking_policy
                > convertible_rval_t;
        pointee_ = shared_ptr_release(
                    std::forward<convertible_rval_t&&>(right));
    }

    self_t& operator=(const self_t& right) {
        refpolicy_t::add_ref(right.pointee_);
        if (refpolicy_t::dec_ref(pointee_))
            spolicy_t::dispose(pointee_);
        pointee_ = right.pointee_;
        return *this;
    }

    self_t& operator=(self_t&& right) {
        if (this != &right) {
            if (refpolicy_t::dec_ref(pointee_))
                spolicy_t::dispose(pointee_);
            pointee_ = shared_ptr_release(std::forward<self_t&&>(right));
        }
        return *this;
    }

    template<typename U>
    self_t& operator=(const shared_pointer<U, reference_policy,
                                           storage_policy,
                                           checking_policy>& right) {
        refpolicy_t::add_ref(shared_ptr_get(right));
        if (refpolicy_t::dec_ref(pointee_))
            spolicy_t::dispose(pointee_);
        pointee_ = shared_ptr_get(right);
        return *this;
    }

    template<typename U>
    self_t& operator=(shared_pointer<U, reference_policy,
                                     storage_policy, checking_policy>&& right) {
        typedef shared_pointer<
                U, reference_policy, storage_policy, checking_policy
                > convertible_rval_t;

        if (refpolicy_t::dec_ref(pointee_))
            spolicy_t::dispose(pointee_);
        pointee_ = shared_ptr_release(
                    std::forward<convertible_rval_t&&>(right));
        return *this;
    }

    bool operator!() const {
        return pointee_ == nullptr;
    }

    operator int helper_t::*() const {
        return pointee_ == nullptr ? nullptr : &helper_t::member;
    }

    T* operator->() const {
        checkpolicy_t::check_ptr(pointee_);
        return pointee_;
    }

    T& operator*() {
        checkpolicy_t::check_ptr(pointee_);
        return *pointee_;
    }

    const T& operator*() const {
        checkpolicy_t::check_ptr(pointee_);
        return *pointee_;
    }

    friend inline T* shared_ptr_get(const self_t& sp) {
        return sp.get();
    }

    friend inline T* shared_ptr_release(self_t&& sp) {
        return sp.release();
    }

    friend inline void shared_ptr_reset(self_t& sp, T* other = nullptr) {
        return sp.reset(other);
    }

    friend inline T** shared_ptr_get_impl(self_t& sp) {
        return sp.get_impl();
    }

    friend inline void swap(self_t& left, self_t& right) {
        left.swap(right);
    }
};

template<typename T, typename RP, typename SP, typename CP>
inline bool operator==(
        const shared_pointer<T, RP, SP, CP>& left,
        const shared_pointer<T, RP, SP, CP>& right
        )
{
    return shared_ptr_get(left) == shared_ptr_get(right);
}

template<typename T, typename RP, typename SP, typename CP>
inline bool operator!=(
        const shared_pointer<T, RP, SP, CP>& left,
        const shared_pointer<T, RP, SP, CP>& right
        )
{
    return !(left == right);
}

template<typename T, typename RP, typename SP, typename CP>
inline bool operator==(
        const shared_pointer<T, RP, SP, CP>& left,
        const T* right
        )
{
    return shared_ptr_get(left) == right;
}

template<typename T, typename RP, typename SP, typename CP>
inline bool operator==(
        const T* left,
        const shared_pointer<T, RP, SP, CP>& right
        )
{
    return right == left;
}

template<typename T, typename RP, typename SP, typename CP>
inline bool operator!=(
        const shared_pointer<T, RP, SP, CP>& left,
        const T* right
        )
{
    return !(left == right);
}

template<typename T, typename RP, typename SP, typename CP>
inline bool operator!=(
        const T* left,
        const shared_pointer<T, RP, SP, CP>& right
        )
{
    return !(right == left);
}

template<typename T, typename U, typename RP, typename SP, typename CP>
inline bool operator==(
        const shared_pointer<T, RP, SP, CP>& left,
        const shared_pointer<U, RP, SP, CP>& right
        )
{
    return shared_ptr_get(left) == shared_ptr_get(right);
}

template<typename T, typename U, typename RP, typename SP, typename CP>
inline bool operator==(
        const shared_pointer<U, RP, SP, CP>& left,
        const shared_pointer<T, RP, SP, CP>& right
        )
{
    return right == left;
}

template<typename T, typename U, typename RP, typename SP, typename CP>
inline bool operator!=(
        const shared_pointer<T, RP, SP, CP>& left,
        const shared_pointer<U, RP, SP, CP>& right
        )
{
    return !(left == right);
}

template<typename T, typename U, typename RP, typename SP, typename CP>
inline bool operator!=(
        const shared_pointer<U, RP, SP, CP>& left,
        const shared_pointer<T, RP, SP, CP>& right
        )
{
    return !(right == left);
}

template<typename T, typename U, typename RP, typename SP, typename CP>
inline bool operator==(
        const shared_pointer<U, RP, SP, CP>& left,
        const U* right
        )
{
    return shared_ptr_get(left) == right;
}

template<typename T, typename U, typename RP, typename SP, typename CP>
inline bool operator==(
        const U* left,
        const shared_pointer<U, RP, SP, CP>& right
        )
{
    return right == left;
}

template<typename T, typename U, typename RP, typename SP, typename CP>
inline bool operator!=(
        const shared_pointer<U, RP, SP, CP>& left,
        const U* right
        )
{
    return !(left == right);
}

template<typename T, typename U, typename RP, typename SP, typename CP>
inline bool operator!=(
        const U* left,
        const shared_pointer<U, RP, SP, CP>& right
        )
{
    return !(right == left);
}
