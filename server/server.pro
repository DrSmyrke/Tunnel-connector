QT += core
QT += network
QT -= gui

TARGET = tunnelConnector
CONFIG += console c++11
CONFIG -= app_bundle

CONFIG(debug, debug|release):CONFIGURATION=debug
CONFIG(release, debug|release):CONFIGURATION=release

OBJECTS_DIR         = ../build/server/obj/$${CONFIGURATION}
MOC_DIR             = ../build/server/$${CONFIGURATION}
RCC_DIR             = ../build/server
UI_DIR              = ../build/server/ui
DESTDIR             = ../bin/server

win32|win64{
    RC_FILE=  ../index.rc
    OTHER_FILES+= ../index.rc
    DISTFILES += ../index.rc
}

QMAKE_CXXFLAGS += "-std=c++11"

TEMPLATE = app

SOURCES += main.cpp \
    global.cpp \
    myfunctions.cpp \
    myproto.cpp \
    server.cpp

exists(./gitversion.pri):include(./gitversion.pri)
exists(./myLibs.pri):include(./myLibs.pri)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    global.h \
    myfunctions.h \
    myproto.h \
    server.h
	
DISTFILES += \
    gitversion.pri

