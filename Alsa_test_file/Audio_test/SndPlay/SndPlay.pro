#-------------------------------------------------
#
# Project created by QtCreator 2018-01-04T15:02:04
#
#-------------------------------------------------

QT       += core

QT       -= gui

LIBS     += -lasound

TARGET = SndPlay
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    alsa_sound_test.cpp \
    fifo.cpp \
    list_lib.cpp \
    soundjni.cpp

HEADERS += \
    fifo.h \
    list_lib.h \
    soundjni.h \
    soundJNI_c.h
