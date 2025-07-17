QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS

#设置字符
contains( CONFIG,"msvc" ):QMAKE_CXXFLAGS += /source-charset:utf-8 /execution-charset:utf-8
contains( CONFIG,"msvc" ):QMAKE_CFLAGS +=/source-charset:utf-8 /execution-charset:utf-8

#海康SDK
include (./HikSdk/HikSdk.pri)

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
