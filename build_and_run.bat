@echo off
echo 串口测试程序编译和运行脚本
echo ================================

REM 创建构建目录
if not exist "build_serial" mkdir build_serial
cd build_serial

REM 配置CMake
echo 配置CMake...
cmake -G "Visual Studio 17 2022" -A x64 ..\CMakeLists.txt
if errorlevel 1 (
    echo CMake配置失败！
    pause
    exit /b 1
)

REM 编译项目
echo 编译项目...
cmake --build . --config Release
if errorlevel 1 (
    echo 编译失败！
    pause
    exit /b 1
)

REM 检查可执行文件是否存在
if not exist "bin\Release\serial_test.exe" (
    echo 可执行文件未找到！
    pause
    exit /b 1
)

echo 编译成功！
echo ================================
echo 正在运行串口测试程序...
echo 请确保COM5端口可用
echo 按任意键开始运行...
pause

REM 运行程序
bin\Release\serial_test.exe

echo 程序已退出
pause 