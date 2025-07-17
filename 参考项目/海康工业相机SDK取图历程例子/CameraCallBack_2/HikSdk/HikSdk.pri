HEADERS += \
    $$PWD/cmvcamera.h \
    $$PWD/grabimgthread.h

SOURCES += \
    $$PWD/cmvcamera.cpp \
    $$PWD/grabimgthread.cpp

msvc:
{
    #OpenCV
    LIBS += -L$$PWD/OpenCV/OpenCV_Msvc/lib/ -lopencv_world455
    INCLUDEPATH += $$PWD/OpenCV/OpenCV_Msvc/Includes
    DEPENDPATH += $$PWD/OpenCV/OpenCV_Msvc/Includes

    #SDK
    LIBS += -L$$PWD/SDK/Lib/ -lMvCameraControl
    INCLUDEPATH += $$PWD/SDK/Includes
    DEPENDPATH += $$PWD/SDK/Includes
}
