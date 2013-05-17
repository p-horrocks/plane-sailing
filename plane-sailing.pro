#-------------------------------------------------
#
# Project created by QtCreator 2013-04-27T21:52:51
#
#-------------------------------------------------

QT       += core gui xml

TARGET = plane-sailing
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++0x
SOURCES += main.cpp \
    util.cpp \
    kmlfile.cpp \
    pointset.cpp \
    track3d.cpp \
    point3d.cpp \
    point2d.cpp \
    distribution.cpp \
    units.cpp \
    thread.cpp \
    mainwnd.cpp

HEADERS += \
    util.h \
    kmlfile.h \
    pointset.h \
    track3d.h \
    point3d.h \
    point2d.h \
    distribution.h \
    units.h \
    thread.h \
    mainwnd.h

LIBS += -lgdal
