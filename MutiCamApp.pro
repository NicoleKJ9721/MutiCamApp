QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    cameramanager.cpp \
    cmvcamera.cpp \
    drawingmanager.cpp \
    grabimgthread.cpp \
    main.cpp \
    mainwindow.cpp \
    settingsmanager.cpp


HEADERS += \
    SDK/include/CameraParams.h \
    SDK/include/MvCameraControl.h \
    SDK/include/MvErrorDefine.h \
    SDK/include/MvISPErrorDefine.h \
    SDK/include/MvObsoleteInterfaces.h \
    SDK/include/MvSdkExport.h \
    SDK/include/ObsoleteCamParams.h \
    SDK/include/PixelType.h \
    cameramanager.h \
    cmvcamera.h \
    cvmattype.h \
    drawingmanager.h \
    grabimgthread.h \
    mainwindow.h \
    settingsmanager.h

FORMS += \
    mainwindow.ui

# Hik SDK
# $$PWD表示的意思就是pro文件所在的目录。
INCLUDEPATH += $$PWD/SDK/include
DEPENDPATH += $$PWD/SDK/include
LIBS += -L$$PWD/SDK/Lib -lMvCameraControl

# OpenCV
win32: {
    INCLUDEPATH += D:/ProgramData/opencv/build/include
    DEPENDPATH += D:/ProgramData/opencv/build/include

    # 对于OpenCV 4.x (使用world模块)
    CONFIG(debug, debug|release) {
        LIBS += -LD:/ProgramData/opencv/build/x64/vc16/lib \
                -lopencv_world4110d  # Debug版本
    } else {
        LIBS += -LD:/ProgramData/opencv/build/x64/vc16/lib \
                -lopencv_world4110   # Release版本
    }
}


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Settings/settings.qrc
