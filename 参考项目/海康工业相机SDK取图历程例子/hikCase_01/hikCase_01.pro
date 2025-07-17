QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

DEFINES += QT_DEPRECATED_WARNINGS

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    cmvcamera.cpp \
    main.cpp \
    mainwindow.cpp \
    myThread.cpp

HEADERS += \
    SDK/include/CameraParams.h \
    SDK/include/MvCameraControl.h \
    SDK/include/MvErrorDefine.h \
    SDK/include/MvISPErrorDefine.h \
    SDK/include/MvObsoleteInterfaces.h \
    SDK/include/MvSdkExport.h \
    SDK/include/ObsoleteCamParams.h \
    SDK/include/PixelType.h \
    cmvcamera.h \
    mainwindow.h \
    myThread.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


# Hik SDK
INCLUDEPATH += E:\QTProject\hikCase_01\SDK\include
DEPENDPATH += E:\QTProject\hikCase_01\SDK\include
LIBS += E:\QTProject\hikCase_01\SDK\Lib\MvCameraControl.lib

# OpenCV
INCLUDEPATH += D:\QT\opencv\rebuild_for_qt\install\include
LIBS += D:\QT\opencv\rebuild_for_qt\lib\libopencv_*.a
