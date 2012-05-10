TEMPLATE = app
CONFIG += console thread
CONFIG -= qt

unix {
    QMAKE_CXXFLAGS += -std=c++0x -Wall -Wextra -ggdb3
    DEFINES += UNIT_TEST_PASS
    LIBS += -lgtest
}

SOURCES += main.cpp \
    scoped_handle_unittests.cc \
    shared_handle_unittests.cc

HEADERS += \
    scoped_handle.h \
    fundamental_types.h \
    compound_types.h \
    shared_handle.h \
    handle_traits.h \
    scoped_pointer.h \
    shared_pointer.h \
    pointer_policies.h \
    intrusive_refcount_impl.h \
    function_types.h \
    auto_lock.h \
    posix_lock.h \
    scoped_lock.h

