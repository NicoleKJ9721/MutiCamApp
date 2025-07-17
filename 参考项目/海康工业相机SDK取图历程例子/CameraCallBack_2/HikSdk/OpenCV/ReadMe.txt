msvc:
{
    #OpenCV
    LIBS += -L$$PWD/OpenCV/OpenCV_Msvc/lib/ -lopencv_world451d
    INCLUDEPATH += $$PWD/OpenCV/OpenCV_Msvc/include
    DEPENDPATH += $$PWD/OpenCV/OpenCV_Msvc/include
}
mingw:
{
    #OpenCV
    LIBS += -L $$PWD/OpenCV/OpenCV_MinGw/Lib/libopencv_*.a
    INCLUDEPATH += $$PWD/OpenCV/OpenCV_MinGw/Includes
}