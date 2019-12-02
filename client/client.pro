QT += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = tunnelConnector
CONFIG += c++11
CONFIG -= app_bundle

CONFIG(debug, debug|release):CONFIGURATION=debug
CONFIG(release, debug|release):CONFIGURATION=release

OBJECTS_DIR         = ../build/obj/$${CONFIGURATION}
MOC_DIR             = ../build/$${CONFIGURATION}
RCC_DIR             = ../build
UI_DIR              = ../build/ui
DESTDIR             = ../bin/client

win32|win64{
    RC_FILE=  index.rc
    OTHER_FILES+= index.rc
    DISTFILES += index.rc
}

QMAKE_CXXFLAGS += "-std=c++11"

TEMPLATE = app

SOURCES += main.cpp \
    global.cpp \
    mainwindow.cpp \
    myfunctions.cpp

exists(./gitversion.pri):include(./gitversion.pri)
exists(./myLibs.pri):include(./myLibs.pri)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    global.h \
    mainwindow.h \
    myfunctions.h

DISTFILES += \
    gitversion.pri

FORMS += \
    mainwindow.ui
