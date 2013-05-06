#-------------------------------------------------
#
# Project created by QtCreator 2013-04-27T21:52:51
#
#-------------------------------------------------

QT       += core

QT       -= gui

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
    point3d.cpp

HEADERS += \
    util.h \
    kmlfile.h \
    pointset.h \
    track3d.h \
    point3d.h
