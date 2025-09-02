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
            
            // 显示每个设备的详细信息
            for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) {
                MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
                if (nullptr == pDeviceInfo) {
                    continue;
                }
                
                std::cout << "\n设备 " << (i + 1) << ":" << std::endl;
                
                if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE) {
                    std::cout << "  类型: GigE相机" << std::endl;
                    
                    // 获取序列号
                    if (pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber[0] != '\0') {
                        std::string serialNumber = std::string((char*)pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber);
                        std::cout << "  序列号: " << serialNumber << std::endl;
                    }
                    
                    // 获取型号名称
                    if (pDeviceInfo->SpecialInfo.stGigEInfo.chModelName[0] != '\0') {
                        std::string modelName = std::string((char*)pDeviceInfo->SpecialInfo.stGigEInfo.chModelName);
                        std::cout << "  型号: " << modelName << std::endl;
                    }
                    
                    // 获取IP地址
                    unsigned int nIp = pDeviceInfo->SpecialInfo.stGigEInfo.nCurrentIp;
                    std::cout << "  IP地址: " << ((nIp & 0xff000000) >> 24) << "."
                              << ((nIp & 0x00ff0000) >> 16) << "."
                              << ((nIp & 0x0000ff00) >> 8) << "."
                              << (nIp & 0x000000ff) << std::endl;
                              
                } else if (pDeviceInfo->nTLayerType == MV_USB_DEVICE) {
                    std::cout << "  类型: USB相机" << std::endl;
                    
                    // 获取序列号
                    if (pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber[0] != '\0') {
                        std::string serialNumber = std::string((char*)pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
                        std::cout << "  序列号: " << serialNumber << std::endl;
                    }
                    
                    // 获取型号名称
                    if (pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName[0] != '\0') {
                        std::string modelName = std::string((char*)pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName);
                        std::cout << "  型号: " << modelName << std::endl;
                    }
                    
                    // 获取厂商名称
                    if (pDeviceInfo->SpecialInfo.stUsb3VInfo.chVendorName[0] != '\0') {
                        std::string vendorName = std::string((char*)pDeviceInfo->SpecialInfo.stUsb3VInfo.chVendorName);
                        std::cout << "  厂商: " << vendorName << std::endl;
                    }
                }
            }
            
            if (stDeviceList.nDeviceNum == 0) {
                std::cout << "\n未找到任何设备，请检查：" << std::endl;
                std::cout << "1. 设备是否正确连接" << std::endl;
                std::cout << "2. 设备驱动是否正确安装" << std::endl;
                std::cout << "3. 网络设置是否正确（对于GigE设备）" << std::endl;
            }
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