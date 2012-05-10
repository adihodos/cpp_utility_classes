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

#include "function_types.h"

template<typename T>
struct compound_type {
    enum {
        is_pointer = 0,
        is_reference = 0,
        is_array = 0,
        is_class = 0,
        is_function = is_function_type<T>::Yes,
        is_enum = 0,
        is_pointer_to_member = 0
    };
    
    typedef T                           base_t;
    typedef T                           bottom_t;
    typedef compound_type<void>         class_t;
};

template<typename T>
struct compound_type<T*> {
    enum {
        is_pointer = 1,
        is_reference = 0,
        is_array = 0,
        is_class = 0,
        is_function = 0,
        is_enum = 0,
        is_pointer_to_member = 0
    };
    
    typedef T                                       base_t;
    typedef typename compound_type<T>::bottom_t     bottom_t;
    typedef compound_type<void>                     class_t;
};

template<typename T>
struct compound_type<T&> {
    enum {
        is_pointer = 0,
        is_reference = 1,
        is_array = 0,
        is_class = 0,
        is_function = 0,
        is_enum = 0,
        is_pointer_to_member = 0
    };

    typedef T                                       base_t;
    typedef typename compound_type<T>::bottom_t     bottom_t;
    typedef compound_type<void>                     class_t;
};

template<typename T, size_t N>
struct compound_type<T [N]> {
    enum {
        is_pointer = 0,
        is_reference = 0,
        is_array = 1,
        is_class = 0,
        is_function = 0,
        is_enum = 0,
        is_pointer_to_member = 0
    };

    typedef T                                       base_t;
    typedef typename compound_type<T>::bottom_t     bottom_t;
    typedef compound_type<void>                     class_t;
};

template<typename T>
struct compound_type<T []> {
    enum {
        is_pointer = 0,
        is_reference = 0,
        is_array = 1,
        is_class = 0,
        is_function = 0,
        is_enum = 0,
        is_pointer_to_member = 0
    };

    typedef T                                       base_t;
    typedef typename compound_type<T>::bottom_t     bottom_t;
    typedef compound_type<void>                     class_t;
};

template<typename T, typename U>
struct compound_type<T U::*> {    
    enum {
        is_pointer = 0,
        is_reference = 0,
        is_array = 0,
        is_class = 0,
        is_function = 0,
        is_enum = 0,
        is_pointer_to_member = 1
    };

    typedef T                                       base_t;
    typedef typename compound_type<T>::bottom_t     bottom_t;
    typedef compound_type<U>                        class_t;
};
