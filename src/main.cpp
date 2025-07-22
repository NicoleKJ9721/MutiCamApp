#include "MutiCamApp.h"
#include "dependencies_test.h"

#include <QApplication>
#include <QMessageBox>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif
#pragma comment(lib, "user32.lib")

int main(int argc, char *argv[])
{
#ifdef _WIN32
    // 设置控制台编码为UTF-8，修复中文乱码
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // 注意：不使用_O_U8TEXT模式，因为会与qDebug冲突
    // _setmode(_fileno(stdout), _O_U8TEXT);
    // _setmode(_fileno(stderr), _O_U8TEXT);
#endif

    // Qt 5.14+推荐的高DPI策略
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QApplication a(argc, argv);

    // 测试依赖库是否正确配置
    std::cout << "=== 依赖库测试开始 ===" << std::endl;
    
    bool opencvOk = DependenciesTest::testOpenCV();
    bool hikvisionOk = DependenciesTest::testHikvisionSDK();
    
    if (!opencvOk || !hikvisionOk) {
        QString errorMsg = "依赖库配置检查失败：\n";
        if (!opencvOk) errorMsg += "- OpenCV配置异常\n";
        if (!hikvisionOk) errorMsg += "- 海康SDK配置异常\n";
        errorMsg += "\n请检查第三方库配置是否正确。";
        
        QMessageBox::warning(nullptr, "依赖库检查", errorMsg);
    } else {
        std::cout << "所有依赖库配置正常" << std::endl;
    }
    
    std::cout << "=== 依赖库测试结束 ===" << std::endl;
    
    MutiCamApp w;
    w.show();
    return a.exec();
}