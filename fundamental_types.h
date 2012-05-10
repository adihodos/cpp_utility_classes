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

template<typename T, typename U>
struct types_eq {
    enum {
        result = 0
    };
};

template<typename T>
struct types_eq<T, T> {
    enum {
        result = 1
    };
};

template<typename T>
struct is_builtin_type {
    enum {
        Yes = 0,
        No = 1
    };
};

#ifndef MAKE_BUILTIN_TYPE
#define MAKE_BUILTIN_TYPE(type) \
    template<> \
    struct is_builtin_type<type> { \
        enum { \
            Yes = 1, \
            No = 0 \
        }; \
    }
#endif

MAKE_BUILTIN_TYPE(bool);
MAKE_BUILTIN_TYPE(signed char);
MAKE_BUILTIN_TYPE(unsigned char);
MAKE_BUILTIN_TYPE(char);
MAKE_BUILTIN_TYPE(signed short);
MAKE_BUILTIN_TYPE(unsigned short);
MAKE_BUILTIN_TYPE(signed int);
MAKE_BUILTIN_TYPE(unsigned int);
MAKE_BUILTIN_TYPE(signed long);
MAKE_BUILTIN_TYPE(unsigned long);
MAKE_BUILTIN_TYPE(signed long long);
MAKE_BUILTIN_TYPE(unsigned long long);
MAKE_BUILTIN_TYPE(float);
MAKE_BUILTIN_TYPE(double);
MAKE_BUILTIN_TYPE(long double);
