@echo off

if exist %Systemroot%\SysWOW64 (
    regsvr32 /u /s "%MVCAM_GENICAM_CLPROTOCOL%\\..\\Win64_x64\\MvDSS.ax"
) else (
    echo System not support...
)

echo MvDSS unregister ok
@ping 127.0.0.1 -n 3 >nul
