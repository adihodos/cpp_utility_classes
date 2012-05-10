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

/**
 * \brief A unique owning smart pointer class. It will release the memory
 *  that the pointer points to, using the policy of deallocation specified
 *  by the storage_policy<T> policy. Also supports various degrees
 *  of checking (when accessing the pointer) via the checking_policy<T> policy.
 *  The default storage policy releases the pointer by calling delete. The
 *  default checking policy is to use assertions in debug mode, but no checking
 *  in release mode. This class does not offer implicit conversion to the
 *  underlying pointer type, since they can be pretty dangerous. Also it does
 *  not use any members functions, except those needed to mimic built-in pointer
 *  behaviour.
 * \remarks Use the default policies in thee pointer_policies.h file as a guide
 *  when implementing a custom policy.
 */
template<
        typename T,
        template<typename> class storage_policy = default_storage,
        template<typename> class checking_policy = assert_check
> class scoped_ptr {
public :
    typedef storage_policy<T>                                   spolicy_t;
    typedef checking_policy<T>                                  checkpolicy_t;
    typedef scoped_ptr<T, storage_policy, checking_policy>      self_t;
    typedef T*                                                  pointer_t;
    typedef const T*                                            const_pointer_t;
    typedef T&                                                  ref_t;
    typedef const T&                                            const_ref_t;

private :
    /*!< Pointer to the allocated memory. */
    T*  pointee_;

    struct helper_t {
        int member;
    };

    T* get() const {
        checkpolicy_t::check_ptr(pointee_);
        return pointee_;
    }

    T* release() {
        T*  old_val = pointee_;
        pointee_ = nullptr;
        return old_val;
    }

    void reset(T* other) {
        if (pointee_ != other) {
            spolicy_t::dispose(pointee_);
            pointee_ = other;
        }
    }

    T** get_impl() {
        return &pointee_;
    }

    void swap(self_t& other) {
        std::swap(pointee_, other.pointee_);
    }

public :
    /**
     * \brief Default initialize using the null pointer.
     */
    scoped_ptr() : pointee_(nullptr) {}

    explicit scoped_ptr(T* ptr) : pointee_(ptr) {}

    scoped_ptr(self_t&& right) : pointee_(right.pointee_) {
        right.pointee_ = nullptr;
    }

    ~scoped_ptr() {
        spolicy_t::dispose(pointee_);
    }

    /**
     * \brief Construct from an rvalue object with a convertible pointer type.
     */
    template<typename U>
    scoped_ptr(scoped_ptr<U, management_policy, checking_policy>&& right)
        : pointee_(right.pointee_) {
        right.pointee_ = nullptr;
    }

    scoped_ptr(const self_t&) = delete;
    self_t& operator=(const self_t&) = delete;

    self_t& operator=(self_t&& right) {
        if (this != &right) {
            spolicy_t::dispose(pointee_);
            pointee_ = right.pointee_;
            right.pointee_ = nullptr;
        }
        return *this;
    }

    template<typename U>
    self_t& operator=(
            scoped_ptr<U, management_policy, checking_policy>&& right
            )
    {
        spolicy_t::dispose(pointee_);
        pointee_ = right.pointee_;
        right.pointee_ = nullptr;
        return *this;
    }

    /**
     * \brief Test if the owned pointer is null.
     */
    bool operator!() const {
        return pointee_ == nullptr;
    }

    /**
     * \brief Test if the owned pointer is not null.
     */
    operator int helper_t::*() const {
        return pointee_ == nullptr ? nullptr : &helper_t::member;
    }

    T*  operator->() const {
        checkpolicy_t::check_ptr(pointee_);
        return pointee_;
    }

    T&  operator*() {
        checkpolicy_t::check_ptr(pointee_);
        return *pointee_;
    }

    const T& operator*() const {
        checkpolicy_t::check_ptr(pointee_);
        return *pointee_;
    }

    /**
     * \brief Implements subscripting, if the pointer points to an array.
     */
    T&  operator[](int index) {
        static_assert(spolicy_t::is_array_ptr,
                      "Subscripting only applies to pointer to array!");
        checkpolicy_t::check_ptr(pointee_);
        return ptr_[index];
    }

    /**
     * \brief Implements subscripting, if the pointer points to an array.
     */
    const T& operator[](int index) const {
        static_assert(spolicy_t::is_array_ptr,
                      "Subscripting only applies to pointer to array!");
        checkpolicy_t::check_ptr(pointee_);
        return ptr_[index];
    }

    /**
     * \brief Explicit access to the owned pointer of a scoped_ptr object.
     * \param Reference to a scoped_ptr object.
     * \return The raw pointer that the scoped_ptr owns.
     */
    friend inline T* scoped_pointer_get(const self_t& sp) {
        return sp.get();
    }

    /**
     * \brief Releases ownerhip of the raw pointer to the caller, who now has the
     *  responsability of freeing the memory, when no longer needed.
     * \param sp Reference to a scoped_ptr object.
     * \return The raw pointer.
     */
    friend inline T* scoped_pointer_release(self_t& sp) {
        return sp.release();
    }

    /**
     * \brief Reset the owned pointer of a scoped_ptr to the new value.
     * \param sp Reference to a scoped_ptr object.
     * \param other The new pointer that the scoped_ptr object will own. This
     *        pointer must have been allocated using the same method as the
     *        original owned pointer.
     * \remarks The old raw pointer owned by the scoped_ptr will be destroyed.
     */
    friend inline void scoped_pointer_reset(self_t& sp, T* other = nullptr) {
        sp.reset(other);
    }

    /**
     * \brief Convenience function to get a pointer to the raw pointer
     *        owned by the scoped_ptr object.
     * \param sp Reference to a scoped_ptr object.
     * \remarks This function should be used with care, since it can cause
     *          memory leaks. Make sure that the scoped_ptr owns a null pointer
     *          if you are going to use the return value to reset the owned
     *          pointer of the scoped_ptr object.
     */
    friend inline T** scoped_pointer_get_impl(self_t& sp) {
        return sp.get_impl();
    }

    /**
     * \brief Swap contents with another scoped_ptr object.
     */
    friend inline void swap(self_t& left, self_t& right) {
        return left.swap(right);
    }
};

template<typename T, typename U, typename W>
inline bool operator==(const T* left, scoped_ptr<T, U, W>& right) {
    return left == scoped_pointer_get(right);
}

template<typename T, typename U, typename W>
inline bool operator!=(const T* left, scoped_ptr<T, U, W>& right) {
    return !(left == right);
}

template<typename T, typename U, typename W>
inline bool operator==(scoped_ptr<T, U, W>& left, const T* right) {
    return right == left;
}

template<typename T, typename U, typename W>
inline bool operator!=(scoped_ptr<T, U, W>& left, const T* right) {
    return !(right == left);
}
