QT       -= gui

TARGET = opts
TEMPLATE = lib

release {
    CONFIG+= staticlib
    QMAKE_CXXFLAGS_RELEASE -= -g
}


QMAKE_EXT_CPP += .c

DESTDIR = $$PWD/../lib
BUILDDIR = $$PWD/../build/options
OBJECTS_DIR = $$BUILDDIR/obj
MOC_DIR     = $$BUILDDIR/moc
RCC_DIR     = $$BUILDDIR/rcc
UI_DIR      = $$BUILDDIR/ui

SOURCES += \
    dougstring.cpp \
    dougnode.cpp \
    doughost.cpp \
    dougcolor.cpp \
    options.cpp \
    opts.cpp \
    dougbool.cpp

HEADERS += \
    option_types.h \
    dougstring.h \
    dougnode.h \
    doughost.h \
    dougcolor.h \
    dougbool.h \
    options.h \
    opts.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
