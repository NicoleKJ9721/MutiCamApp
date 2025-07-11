#include "MutiCamApp.h"
#include "utils/DebugUtils.h"

#include <QApplication>
#include <QDebug>
#include <QLoggingCategory>
#include <iostream>
#include <cstring>
#include <cstdlib>
#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#include <fcntl.h>
#endif
#pragma comment(lib, "user32.lib")

int main(int argc, char *argv[])
{
    // 默认关闭调试输出，需要时可以调用 DebugConfig::enableDebug() 开启
    DebugConfig::disableDebug();
    
    // 检查是否需要控制台（命令行参数或环境变量）
    bool needConsole = false;
    
    // 1. 检查命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "--console") == 0 || strcmp(argv[i], "-d") == 0) {
            needConsole = true;
            break;
        }
    }
    
    // 2. 检查环境变量
    const char* debugEnv = getenv("MUTICAM_DEBUG");
    if (debugEnv && (strcmp(debugEnv, "1") == 0 || strcmp(debugEnv, "true") == 0)) {
        needConsole = true;
    }
    
#ifdef _WIN32
    if (needConsole) {
        // 在Windows上分配控制台窗口（仅在需要时）
        AllocConsole();
        
        // 重定向stdout, stdin, stderr到控制台
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
        freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
        freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
        
        // 设置控制台代码页为936(GBK)，而不是UTF-8
        SetConsoleOutputCP(936);
        
        // 同时开启调试输出
        DebugConfig::enableDebug();
    }
#endif
    
    DEBUG_OUT("程序开始启动...");
    DEBUG_LOG("程序开始启动...");
    
    QApplication a(argc, argv);
    DEBUG_OUT("QApplication创建成功");
    DEBUG_LOG("QApplication创建成功");
    
    try {
        DEBUG_OUT("创建主窗口...");
        DEBUG_LOG("创建主窗口...");
        MutiCamApp w;
        DEBUG_OUT("主窗口创建成功");
        DEBUG_LOG("主窗口创建成功");
        
        DEBUG_OUT("显示主窗口...");
        DEBUG_LOG("显示主窗口...");
        w.show();
        DEBUG_OUT("主窗口显示成功");
        DEBUG_LOG("主窗口显示成功");
        
        DEBUG_OUT("进入事件循环...");
        DEBUG_LOG("进入事件循环...");
        int result = a.exec();
        DEBUG_OUT("事件循环结束，返回值:" << result);
        DEBUG_LOG("事件循环结束，返回值:" << result);
        return result;
    } catch (const std::exception& e) {
        ERROR_OUT("main函数异常:" << e.what());
        qCritical() << "main函数异常:" << e.what();
        return -1;
    } catch (...) {
        ERROR_OUT("main函数发生未知异常");
        qCritical() << "main函数发生未知异常";
        return -1;
    }
}