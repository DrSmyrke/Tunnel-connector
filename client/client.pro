QT += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = tunnelConnector
CONFIG += c++11
CONFIG -= app_bundle

CONFIG(debug, debug|release):CONFIGURATION=debug
CONFIG(release, debug|release):CONFIGURATION=release

OBJECTS_DIR         = ../build/client/obj/$${CONFIGURATION}
MOC_DIR             = ../build/client/$${CONFIGURATION}
RCC_DIR             = ../build/client
UI_DIR              = ../build/client/ui
DESTDIR             = ../bin/client

win32|win64{
    RC_FILE=  ../index.rc
    OTHER_FILES+= ../index.rc
    DISTFILES += ../index.rc
}

QMAKE_CXXFLAGS += "-std=c++11"

TEMPLATE = app

SOURCES += main.cpp \
    connector.cpp \
    global.cpp \
    localserver.cpp \
    mainwindow.cpp \
    myfunctions.cpp \
    myproto.cpp

exists(./gitversion.pri):include(./gitversion.pri)
exists(./myLibs.pri):include(./myLibs.pri)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

build_pass:CONFIG(debug, debug|release) {
    unix: TARGET = $$join(TARGET,,,_debug)
    else: TARGET = $$join(TARGET,,,d)
}

HEADERS += \
    connector.h \
    global.h \
    localserver.h \
    mainwindow.h \
    myfunctions.h \
    myproto.h

DISTFILES += \
    gitversion.pri

FORMS += \
    mainwindow.ui
