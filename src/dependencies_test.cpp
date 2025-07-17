#include "dependencies_test.h"
#include <iostream>
#include <string>

bool DependenciesTest::testOpenCV() {
    try {
        // 创建一个简单的Mat对象测试OpenCV
        cv::Mat testImage = cv::Mat::zeros(100, 100, CV_8UC3);
        if (testImage.empty()) {
            std::cout << "OpenCV测试失败：无法创建Mat对象" << std::endl;
            return false;
        }
        
        // 测试基本的图像操作
        cv::circle(testImage, cv::Point(50, 50), 20, cv::Scalar(0, 255, 0), 2);
        
        std::cout << "OpenCV测试成功：版本 " << getOpenCVVersion() << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cout << "OpenCV测试异常：" << e.what() << std::endl;
        return false;
    }
}

bool DependenciesTest::testHikvisionSDK() {
    try {
        // 测试海康SDK初始化
        int nRet = MV_CC_Initialize();
        if (MV_OK != nRet) {
            std::cout << "海康SDK初始化失败，错误码：" << nRet << std::endl;
            return false;
        }
        
        // 枚举设备
        MV_CC_DEVICE_INFO_LIST stDeviceList;
        memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
        nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
        
        if (MV_OK == nRet) {
            std::cout << "海康SDK测试成功：找到 " << stDeviceList.nDeviceNum << " 个设备" << std::endl;
        } else {
            std::cout << "海康SDK设备枚举失败，错误码：" << nRet << std::endl;
        }
        
        // 清理SDK
        MV_CC_Finalize();
        return true;
    }
    catch (const std::exception& e) {
        std::cout << "海康SDK测试异常：" << e.what() << std::endl;
        return false;
    }
}

std::string DependenciesTest::getOpenCVVersion() {
    return cv::getVersionString();
}

std::string DependenciesTest::getHikvisionSDKVersion() {
    // 海康SDK没有直接的版本获取API，返回固定字符串
    return "MVS SDK (版本信息需要通过其他方式获取)";
}