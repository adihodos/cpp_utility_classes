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

#include "handle_traits.h"

/**
 *\brief Allows automatic management and sharing of a resource (socket, file
 *  descriptor, mutex, etc.). This class implements a form of reference counting
 *  using a circular double linked list. Each shared_handle object is a node in
 *  the circular double linked list, all nodes in a list referring to the same
 *  resource. When the last node in the list is deleted, the resource is also
 *  destroyed (since there are no more outstanding references to it).
 *\remarks The management_policy template parameter must define the following :
 *  - null_handle() which returns a value representing an invalid handle (for
 *      example -1 for posix file descriptor/socket or INVALID_HANDLE_VALUE for
 *      windows file descriptors)
 *  - dispose() which has the task of destroying the resource (
 *      close() for posix file/sockets descriptors,
 *      CloseHandle() for windows file descriptors, etc)
 *  - a series of typedefs for the type of a resource, pointer to a resource,
 *      reference to a resource. The handle_traits_base class can be used to
 *      help defining those members, by inheriting from it and overriding the
 *      default template arguments, where necessary.
 *\sa handle_traits_base
 */
template<typename management_policy>
class shared_handle {
public :
    typedef management_policy                           mpolicy_t;
    typedef typename mpolicy_t::handle_t                handle_t;
    typedef typename mpolicy_t::handle_ptr_t            handle_ptr_t;
    typedef typename mpolicy_t::handle_ref_t            handle_ref_t;
    typedef typename mpolicy_t::handle_const_ref_t      handle_const_ref_t;
    typedef shared_handle<management_policy>            self_t;
private :
    /**
     * \brief Pointer to the next node in the list.
     */
    mutable self_t*     next_;
    /**
     * \brief Pointer to the previous node in the list.
     */
    mutable self_t*     prev_;
    /**
     * \brief The shared resource.
     */
    handle_t            handle_;

    struct helper_t {
        int member;
    };

    /**
     * \brief Initialize by making a circular reference to this node.
     */
    void initialize() {
        this->next_ = this->prev_ = this;
    }

    /**
     * \brief Adds this object to the list of objects that share the handle.
     * \param other Pointer to the previous node in the object list.
     */
    void initialize(self_t* other) {
        other->next_->prev_ = this;
        this->next_ = other->next_;
        other->next_ = this;
        this->prev_ = other;
    }

    /**
     * \brief Remove this node from the list of nodes that share
     *  the same handle.
     * \return True if this node is the only node in the list (in which case
     *  the owned resource will be destructed when this object expires).
     * \remarks The next_ and prev_ pointers are not updated, since erase()
     *  is called either at the node's destruction or at assignment.
     */
    bool erase() {
        this->prev_->next_ = this->next_;
        this->next_->prev_ = this->prev_;
        return (next_ == this) && (prev_ == this);
    }

    /**
     * \brief Take ownership of a resource from a rvalue (temporary).
     */
    void steal_from_rvalue(self_t&& rval) {
        //
        // Steal the handle and sink rval.
        handle_ = rval.handle_;
        rval.handle_ = mpolicy_t::null_handle();
        //
        // Replace the rval node in the existing list. We save the next_ pointer,
        // since in the case when it points to rval it becomes an alias for
        // the this pointer, after the first assignment is executed.
        self_t* tmpVal = rval.next_;
        //
        // Replace the pointers to the nodes that where linked to rval with
        // pointers to ourselves.
        rval.prev_->next_ = this;
        tmpVal->prev_ = this;

        //
        // Update our links and make rval to refer to itself.
        next_ = rval.next_;
        prev_ = rval.prev_;
        rval.prev_ = rval.next_ = &rval;
    }

    bool has_one_ref() const {
        return (prev_ == this) && (next_ == this);
    }

    handle_t get() const {
        return handle_;
    }

    void reset(handle_t newHandle) {
        if (newHandle != handle_) {
            if (erase()) {
                mpolicy_t::dispose(handle_);
            } else {
                //
                // Make this node refer to itself since erase() does not update
                // the previous and next links.
                initialize();
            }
            handle_ = newHandle;
        }
    }

    handle_ptr_t get_impl() {
        if (!has_one_ref()) {
            erase();
            initialize();
        } else {
            mpolicy_t::dispose(handle_);
        }
        return &handle_;
    }

    void swap(self_t& other) {
        //
        // Save the next pointer, to avoid becoming an alias for &other.
        // This happens when this node references itself.
        self_t* saveRegOne = this->next_;
        this->prev_->next_ = &other;
        saveRegOne->prev_ = &other;

        //
        // See remarks for saveRegOne.
        self_t* saveRegTwo = other.next_;
        other.prev_->next_ = this;
        saveRegTwo->prev_ = this;

        //
        // Save old links since we need them to fix the other node.
        saveRegOne = this->prev_;
        saveRegTwo = this->next_;

        this->prev_ = other.prev_;
        this->next_ = other.next_;

        other.prev_ = saveRegOne;
        other.next_ = saveRegTwo;

        std::swap(this->handle_, other.handle_);
    }

public :
    /**
     * \brief Initialize with a null handle.
     */
    shared_handle() : handle_(mpolicy_t::null_handle()) {
        initialize();
    }

    /**
     * \brief Take ownership of an existing handle.
     * \param Existing handle.
     */
    explicit shared_handle(handle_t newHandle) : handle_(newHandle) {
        initialize();
    }

    /**
     * \brief Share ownership with another shared_handle object.
     */
    shared_handle(const self_t& other) : handle_(other.handle_) {
        initialize(const_cast<self_t*>(&other));
    }

    /**
     * \brief Construct from a rvalue of this type.
     */
    shared_handle(self_t&& other) {
        steal_from_rvalue(std::forward<self_t&&>(other));
    }

    ~shared_handle() {
        if (erase())
            mpolicy_t::dispose(handle_);
    }

    /**
     * \brief operator int helper_t::* allows testing of the handle's validity
     *  in the form if(handle).
     * \return True if handle is valid, false otherwise.
     */
    operator int helper_t::*() const {
        return handle_ == mpolicy_t::null_handle() ? nullptr : &helper_t::member;
    }

    /**
     * \brief Test if the handle is invalid.
     * \return True if the owned handle is not valid, false otherwise.
     */
    bool operator!() const {
        return handle_ == mpolicy_t::null_handle();
    }

    /**
     * \brief Get access to the raw handle wrapped by a shared_handle object.
     * \param sh reference to a shared_handle object
     * \return The raw handle.
     */
    friend inline handle_t shared_handle_get(const self_t& sh) {
        return sh.get();
    }

    /**
     * \brief Reset the raw handle referenced by this object to a new one.
     * \param sh reference to a shared_handle object
     * \param newHandle The new handle that the object will reference.
     * \remarks The previous owned handle will be destroyed it the shared_handle
     *  object is the only one that refers it.
     */
    friend inline void shared_handle_reset(
            self_t& sh,
            handle_t newHandle = mpolicy_t::null_handle()
            ) {
        return sh.reset(newHandle);
    }

    /**
     * \brief Returns a pointer to the raw handle.
     * \param sh reference to a shared handle object.
     * \remarks This function should be used with maximum care.
     *  If there is more then one reference to the raw handle,
     *  it will remove sh from the list of sharers. If sh is the only object
     *  referring to the raw handle, it will destroy it before returning a
     *  pointer to it.
     * \return Pointer to the raw handle.
     */
    friend inline handle_ptr_t shared_handle_get_impl(self_t& sh) {
        return sh.get_impl();
    }

    /**
     * \brief Swap the contents of two shared_handle objects.
     */
    friend inline void swap(self_t& left, self_t& right) {
        left.swap(right);
    }

#if defined(UNIT_TEST_PASS)

    bool shouldBeErased() const {
        return has_one_ref();
    }

    size_t refcount() const {
        const self_t* head = next_;
        size_t count = 1;
        while (head != this) {
            head = head->next_;
            ++count;
        }
        return count;
    }

#endif

    self_t& operator=(const self_t& other) {
        if (this != &other) {
            if (erase())
                mpolicy_t::dispose(handle_);
            handle_ = other.handle_;
            initialize(const_cast<self_t*>(&other));
        }
        return *this;
    }

    self_t& operator=(self_t&& other) {
        if (this != &other) {
            if (erase())
                mpolicy_t::dispose(handle_);
            steal_from_rvalue(std::forward<self_t&&>(other));
        }
        return *this;
    }
};

template<typename T>
inline bool operator==(const shared_handle<T>& left,
                       const shared_handle<T>& right) {
    return shared_handle_get(left) == shared_handle_get(right);
}

template<typename T>
inline bool operator!=(const shared_handle<T>& left,
                       const shared_handle<T>& right) {
    return !(left == right);
}

template<typename T>
inline bool operator==(const typename T::handle_t& left,
                       const shared_handle<T>& right) {
    return left == shared_handle_get(right);
}

template<typename T>
inline bool operator!=(const typename T::handle_t& left,
                       const shared_handle<T>& right) {
    return !(left == right);
}

template<typename T>
inline bool operator==(const shared_handle<T>& left,
                       const typename T::handle_t& right) {
    return right == left;
}

template<typename T>
inline bool operator!=(const shared_handle<T>& left,
                       const typename T::handle_t& right) {
    return !(right == left);
}
