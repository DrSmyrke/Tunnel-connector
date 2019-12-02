QT += core
QT += network
QT -= gui

TARGET = tunnelConnector
CONFIG += console c++11
CONFIG -= app_bundle

CONFIG(debug, debug|release):CONFIGURATION=debug
CONFIG(release, debug|release):CONFIGURATION=release

OBJECTS_DIR         = ../build/obj/$${CONFIGURATION}
MOC_DIR             = ../build/$${CONFIGURATION}
RCC_DIR             = ../build
UI_DIR              = ../build/ui
DESTDIR             = ../bin/server

QMAKE_CXXFLAGS += "-std=c++11"

TEMPLATE = app

SOURCES += main.cpp \
    global.cpp \
    myfunctions.cpp \
    server.cpp

# Check if the git version file exists
! include(./gitversion.pri) {
        error("Couldn't find the gitversion.pri file!")
}
! include(./myLibs.pri) {
        error("Couldn't find the gitversion.pri file!")
}

HEADERS += \
    global.h \
    myfunctions.h \
    server.h
