@echo off
echo Serial Test Program Runner
echo ========================

if exist "build_serial\bin\Release\serial_test.exe" (
    echo Found executable: build_serial\bin\Release\serial_test.exe
    echo Starting serial test program...
    echo Make sure COM5 port is available
    echo.
    build_serial\bin\Release\serial_test.exe
) else (
    echo Error: Executable not found!
    echo Please run build_and_run.bat first to compile the program.
    pause
) 