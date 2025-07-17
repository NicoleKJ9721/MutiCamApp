msvc:
{
    #SDK
    LIBS += -L$$PWD/SDK/Lib/ -lMvCameraControl
    INCLUDEPATH += $$PWD/SDK/Includes
    DEPENDPATH += $$PWD/SDK/Includes
}
mingw:
{
    #SDK
    LIBS += -L$$PWD/SDK/Lib/MvCameraControl.lib
    INCLUDEPATH += $$PWD/SDK/Includes
    DEPENDPATH += $$PWD/SDK/Includes
}