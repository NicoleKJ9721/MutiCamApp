#pragma once

// OpenCV头文件测试
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

// 海康威视SDK头文件测试
#include "MvCameraControl.h"
#include "CameraParams.h"
#include "PixelType.h"

// Qt头文件
#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QTabWidget>

/**
 * @brief 依赖库测试类
 * 用于验证OpenCV和海康SDK是否正确配置
 */
class DependenciesTest {
public:
    // 测试OpenCV是否可用
    static bool testOpenCV();
    
    // 测试海康SDK是否可用
    static bool testHikvisionSDK();
    
    // 获取OpenCV版本信息
    static std::string getOpenCVVersion();
    
    // 获取海康SDK版本信息
    static std::string getHikvisionSDKVersion();
};