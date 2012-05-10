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

template<typename T>
struct default_storage {
    static void dispose(T* ptr) {
        delete ptr;
    }

    enum {
        is_array_ptr = 0
    };
};

template<typename T>
struct default_array_storage {
    static void dispose(T* ptr) {
        delete[] ptr;
    }

    enum {
        is_array_ptr = 1
    };
};

template<typename T>
struct intrusive_refcount {
    static void add_ref(const T* obj) {
        if (obj)
            obj->add_ref();
    }

    static bool dec_ref(const T* obj) {
        if (obj)
            obj->dec_ref();
    }
};

template<typename T>
struct com_refcount {
    static void add_ref(const T* obj) {
        if (obj)
            obj->AddRef();
    }

    static bool dec_ref(const T* obj) {
        if (obj)
            obj->Release();
    }
};

template<typename T>
struct assert_check {
    static void check_ptr(const T* ptr) {
        assert(ptr != nullptr);
    }
};

template<typename T>
struct no_checking {
    static void check_ptr(const T*) {}
};
