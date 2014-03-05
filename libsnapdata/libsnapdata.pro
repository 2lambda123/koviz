QT       += gui

TARGET = snapdata
TEMPLATE = lib

release {
    CONFIG += staticlib
    QMAKE_CXXFLAGS_RELEASE -= -g
}

DEFINES += SNAPDATA_LIBRARY

QMAKE_EXT_CPP += .c

DESTDIR = $$PWD/../lib
BUILDDIR = $$PWD/../build/libdata
OBJECTS_DIR = $$BUILDDIR/obj
MOC_DIR     = $$BUILDDIR/moc
RCC_DIR     = $$BUILDDIR/rcc
UI_DIR      = $$BUILDDIR/ui


SOURCES += \
    montemodel.cpp \
    snaptable.cpp \
    trickmodel.cpp \
    monte.cpp \
    trickcurvemodel.cpp \
    timeit.cpp \
    timeit_linux.cpp \
    timeit_win32.cpp 


HEADERS += \
    monte.h \ 
    role.h \ 
    trick_types.h \ 
    snaptable.h \ 
    montemodel.h \ 
    trickcurvemodel.h \ 
    trickmodel.h \ 
    roleparam.h \ 
    numsortitem.h \
    timeit.h \
    timeit_linux.h \
    timeit_win32.h 


unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
