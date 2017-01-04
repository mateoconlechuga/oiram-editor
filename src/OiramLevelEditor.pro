#-------------------------------------------------
#
# Project created by QtCreator 2016-12-24T02:20:19
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OiramLevelEditor
TEMPLATE = app
CONFIG += static

SOURCES += main.cpp\
        mainwindow.cpp \
    tilemapview.cpp \
    tile.cpp \
    element.cpp \
    tileview.cpp \
    utils.cpp \
    level.c

HEADERS  += mainwindow.h \
    tilemapview.h \
    tile.h \
    element.h \
    tileview.h \
    utils.h \
    level.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc
