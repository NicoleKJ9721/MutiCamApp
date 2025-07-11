#ifndef DEBUGUTILS_H
#define DEBUGUTILS_H

#include <QDebug>
#include <iostream>

// 全局调试开关
class DebugConfig {
public:
    static bool debugEnabled;
    
    // 启用调试输出
    static void enableDebug() {
        debugEnabled = true;
    }
    
    // 禁用调试输出
    static void disableDebug() {
        debugEnabled = false;
    }
    
    // 检查调试状态
    static bool isDebugEnabled() {
        return debugEnabled;
    }
};

// 条件调试宏 - 只在调试开启时输出
#define DEBUG_OUT(x) \
    do { \
        if (DebugConfig::isDebugEnabled()) { \
            std::cout << x << std::endl; \
        } \
    } while(0)

#define DEBUG_LOG(x) \
    do { \
        if (DebugConfig::isDebugEnabled()) { \
            qDebug() << x; \
        } \
    } while(0)

// 重要信息总是输出（错误、警告等）
#define INFO_OUT(x) std::cout << x << std::endl
#define ERROR_OUT(x) std::cerr << x << std::endl

#endif // DEBUGUTILS_H 