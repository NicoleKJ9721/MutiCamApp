@echo off

if exist %Systemroot%\SysWOW64 (
    regsvr32 /u /s "%MVCAM_GENICAM_CLPROTOCOL%\\..\\Win32_i86\\MvDSS.ax
) else (
    regsvr32 /u /s "%MVCAM_GENICAM_CLPROTOCOL%\\..\\Win32_i86\\MvDSS.ax
)

echo MvDSS unregister ok
@ping 127.0.0.1 -n 3 >nul
