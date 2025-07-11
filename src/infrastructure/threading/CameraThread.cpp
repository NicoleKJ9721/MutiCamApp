#include "CameraThread.h"
#include "utils/DebugUtils.h"
#include <QDebug>
#include <QMutexLocker>
#include <QCoreApplication>
#include <QPainter>
#include <QFont>
#include <QElapsedTimer>
#include <iostream>
#include <algorithm>

CameraThread::CameraThread(const QString& serialNumber, QObject* parent)
    : QThread(parent)
    , m_camera(nullptr)
    , m_serialNumber(serialNumber)
    , m_running(false)
    , m_capturing(false)
    , m_captureTimer(nullptr)
    , m_targetFPS(15)   // 降低默认帧率到15fps以减少CPU占用
    , m_frameInterval(67) // 15fps = 67ms间隔 (1000/15 ≈ 67)
    , m_frameBuffer(nullptr)
    , m_bufferSize(0)
{
    qDebug() << "创建相机线程，序列号:" << serialNumber << "，帧率：15fps";
    
    // 不在构造函数中创建定时器，改用循环方式
}

CameraThread::~CameraThread()
{
    stopCapture();
    
    cleanupCamera();
    
    if (m_frameBuffer) {
        delete[] m_frameBuffer;
        m_frameBuffer = nullptr;
    }
}

void CameraThread::startCapture()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_capturing) {
        qDebug() << "相机已在采集中，序列号:" << m_serialNumber;
        return;
    }
    
    qDebug() << "启动相机采集，序列号:" << m_serialNumber;
    
    if (!isRunning()) {
        start(); // 启动线程
    } else {
        // 线程已运行，直接开始采集
        m_capturing = true;
        if (m_camera && m_camera->isConnected()) {
            m_camera->startGrabbing();
        }
    }
}

void CameraThread::stopCapture()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_capturing) {
        return;
    }
    
    qDebug() << "停止相机采集，序列号:" << m_serialNumber;
    
    m_capturing = false;
    m_running = false;
    
    if (m_camera) {
        m_camera->stopGrabbing();
    }
    
    // 等待线程结束
    if (isRunning()) {
        wait(3000); // 等待3秒
    }
}

bool CameraThread::isCapturing() const
{
    QMutexLocker locker(&m_mutex);
    return m_capturing;
}

void CameraThread::setTargetFPS(int fps)
{
    QMutexLocker locker(&m_mutex);
    
    if (fps <= 0 || fps > 60) {
        qWarning() << "无效的帧率设置:" << fps << "，使用默认值15";
        fps = 15; // 改为15fps默认值
    }
    
    m_targetFPS = fps;
    m_frameInterval = 1000 / fps; // 转换为毫秒
    
    qDebug() << "设置相机帧率:" << fps << "fps，间隔:" << m_frameInterval << "ms";
}

int CameraThread::getTargetFPS() const
{
    QMutexLocker locker(&m_mutex);
    return m_targetFPS;
}

QString CameraThread::getSerialNumber() const
{
    return m_serialNumber;
}

bool CameraThread::isCameraConnected() const
{
    QMutexLocker locker(&m_mutex);
    return m_camera && m_camera->isConnected();
}

void CameraThread::run()
{
    qDebug() << "相机线程开始运行，序列号:" << m_serialNumber;
    
    m_running = true;
    m_capturing = true;
    
    // 初始化相机
    if (!initializeCamera()) {
        emit errorOccurred(QString("初始化相机失败: %1").arg(m_serialNumber));
        return;
    }
    
    emit cameraConnected(m_serialNumber);
    
    // 开始采集
    if (m_camera && m_camera->isConnected() && !m_camera->startGrabbing()) {
        emit errorOccurred(QString("开始采集失败: %1").arg(m_serialNumber));
        return;
    }
    
    DEBUG_OUT("Start optimized image capture loop - SN: " << m_serialNumber.toStdString() 
              << ", Interval: " << m_frameInterval << "ms, Target FPS: " << m_targetFPS);
    
    // 使用循环方式获取图像，并添加更精确的帧率控制
    QElapsedTimer frameTimer;
    frameTimer.start();
    qint64 lastFrameTime = 0;
    
    while (m_running && m_capturing) {
        qint64 currentTime = frameTimer.elapsed();
        
        // 检查是否到了下一帧的时间
        if (currentTime - lastFrameTime >= m_frameInterval) {
            captureFrame();
            lastFrameTime = currentTime;
        }
        
        // 动态休眠时间，提高精度并减少CPU占用
        qint64 nextFrameTime = lastFrameTime + m_frameInterval;
        qint64 sleepTime = nextFrameTime - frameTimer.elapsed();
        
        if (sleepTime > 0) {
            msleep(std::min(sleepTime, static_cast<qint64>(10))); // 最多休眠10ms
        } else {
            msleep(1); // 最小休眠1ms，让出CPU时间
        }
    }
    
    DEBUG_OUT("Image capture loop ended - SN: " << m_serialNumber.toStdString());
    
    // 清理
    cleanupCamera();
    emit cameraDisconnected(m_serialNumber);
    
    qDebug() << "相机线程结束，序列号:" << m_serialNumber;
}

void CameraThread::captureFrame()
{
    if (!m_capturing) {
        DEBUG_LOG("跳过采集 - capturing:" << m_capturing);
        return;
    }
    
    // 添加跳帧机制以减少CPU负载
    static int frameSkipCounter = 0;
    frameSkipCounter++;
    
    // 每3帧处理1帧（进一步降低实际处理帧率）
    if (frameSkipCounter % 3 != 0) {
        return;
    }
    
    QImage image;
    
    // 强制使用测试模式来演示彩色效果（可以改为 true）
    bool forceTestMode = false;
    
    // 检查是否有真实相机
    if (!forceTestMode && m_camera && m_camera->isConnected()) {
        // 真实相机模式
        unsigned char* pImageBuf = nullptr;
        unsigned int nImageSize = 0;
        unsigned int nWidth = 0;
        unsigned int nHeight = 0;
        
        // 获取图像数据
        if (!m_camera->getFrame(pImageBuf, nImageSize, nWidth, nHeight)) {
            qDebug() << "获取图像失败，相机序列号:" << m_serialNumber;
            return; // 获取失败，继续下次采集
        }
        
        qDebug() << "获取图像成功 - 序列号:" << m_serialNumber 
                 << "尺寸:" << nWidth << "x" << nHeight 
                 << "大小:" << nImageSize << "字节";
        
        try {
            // 转换为QImage
            image = convertToQImage(pImageBuf, nWidth, nHeight, nImageSize);
        } catch (const std::exception& e) {
            qWarning() << "图像转换异常:" << e.what();
        }
        
        // 释放图像缓冲区
        if (pImageBuf) {
            delete[] pImageBuf;
            pImageBuf = nullptr;
        }
    } else {
        // 测试模式 - 生成简化的测试图像以减少CPU占用
        static int frameCounter = 0;
        static QImage baseImage; // 缓存基础图像
        frameCounter++;
        
        // 如果是第一次或每100帧重新生成基础图像
        if (baseImage.isNull() || frameCounter % 100 == 1) {
            baseImage = QImage(640, 480, QImage::Format_RGB888);
            
            // 使用更简单的填充模式减少计算量
            int blockSize = 80; // 分块大小
            for (int by = 0; by < baseImage.height(); by += blockSize) {
                for (int bx = 0; bx < baseImage.width(); bx += blockSize) {
                    int r = (bx * 255) / baseImage.width();
                    int g = (by * 255) / baseImage.height();
                    int b = 128; // 固定蓝色分量
                    
                    // 填充整个块
                    for (int y = by; y < std::min(by + blockSize, baseImage.height()); y++) {
                        for (int x = bx; x < std::min(bx + blockSize, baseImage.width()); x++) {
                            baseImage.setPixel(x, y, qRgb(r, g, b));
                        }
                    }
                }
            }
        }
        
        // 复制基础图像并添加简单的动态效果
        image = baseImage.copy();
        
        // 添加简单的动态颜色变化（不重新绘制整个图像）
        int dynamicColor = (frameCounter * 15) % 255;
        
        // 在图像上绘制相机标识文字（优化：减少字体大小和绘制复杂度）
        QPainter painter(&image);
        painter.setPen(QColor(255, 255, 255));
        painter.setFont(QFont("Arial", 16, QFont::Bold)); // 减小字体
        QString text = QString("测试相机: %1\n帧号: %2").arg(m_serialNumber).arg(frameCounter);
        painter.drawText(QRect(10, 10, 250, 60), Qt::AlignLeft | Qt::AlignTop, text);
        
        DEBUG_OUT("Generated optimized test image - SN: " << m_serialNumber.toStdString() << ", Frame: " << frameCounter);
    }
    
    // 发送图像信号
    if (!image.isNull()) {
        DEBUG_OUT("*** Image ready, emitting frameReady signal - SN: " << m_serialNumber.toStdString() 
                  << ", Size: " << image.width() << "x" << image.height() << " ***");
        emit frameReady(image);
    } else {
        ERROR_OUT("ERROR: Image is null, cannot send - SN: " << m_serialNumber.toStdString());
    }
}

QImage CameraThread::convertToQImage(unsigned char* pImageBuf, unsigned int width, 
                                    unsigned int height, unsigned int imageSize)
{
    if (!pImageBuf || width == 0 || height == 0) {
        return QImage();
    }
    
    try {
        // 获取相机像素格式
        QString pixelFormat = m_camera->getPixelFormat();
        
        if (pixelFormat == "Mono8") {
            // 灰度图像 - 转换为伪彩色
            QImage grayImage(pImageBuf, width, height, QImage::Format_Grayscale8);
            QImage colorImage = convertGrayToPseudoColor(grayImage);
            return colorImage;
        } 
        else if (pixelFormat == "RGB8") {
            // RGB图像 - 8位三通道 - 减少不必要的拷贝
            return QImage(pImageBuf, width, height, width * 3, QImage::Format_RGB888).copy();
        }
        else {
            // 未知格式，尝试作为灰度图像处理并转换为伪彩色
            std::cout << "Unknown pixel format: " << pixelFormat.toStdString() 
                     << ", treating as grayscale and converting to pseudo color" << std::endl;
            QImage grayImage(pImageBuf, width, height, QImage::Format_Grayscale8);
            QImage colorImage = convertGrayToPseudoColor(grayImage);
            return colorImage;
        }
    } catch (const std::exception& e) {
        std::cout << "Image format conversion failed: " << e.what() << std::endl;
        return QImage();
    }
}

QImage CameraThread::convertGrayToPseudoColor(const QImage& grayImage)
{
    if (grayImage.isNull()) {
        return QImage();
    }
    
    QImage colorImage(grayImage.size(), QImage::Format_RGB888);
    
    for (int y = 0; y < grayImage.height(); y++) {
        for (int x = 0; x < grayImage.width(); x++) {
            // 获取灰度值
            QRgb gray = grayImage.pixel(x, y);
            int grayLevel = qGray(gray);
            
            // 将灰度值转换为伪彩色（热力图风格）
            int r, g, b;
            
            if (grayLevel < 64) {
                // 蓝色到青色
                r = 0;
                g = grayLevel * 4;
                b = 255;
            } else if (grayLevel < 128) {
                // 青色到绿色
                r = 0;
                g = 255;
                b = 255 - (grayLevel - 64) * 4;
            } else if (grayLevel < 192) {
                // 绿色到黄色
                r = (grayLevel - 128) * 4;
                g = 255;
                b = 0;
            } else {
                // 黄色到红色
                r = 255;
                g = 255 - (grayLevel - 192) * 4;
                b = 0;
            }
            
            colorImage.setPixel(x, y, qRgb(r, g, b));
        }
    }
    
    return colorImage;
}

bool CameraThread::initializeCamera()
{
    qDebug() << "初始化相机，序列号:" << m_serialNumber;
    
    try {
        // 创建相机实例
        m_camera = std::make_unique<HikVisionCamera>(m_serialNumber);
        
        if (!m_camera->isConnected()) {
            std::cout << "Camera connection failed, using TEST MODE - SN: " << m_serialNumber.toStdString() << std::endl;
            // 不返回false，而是继续使用测试模式
            return true;
        }
        
        // 尝试设置RGB格式而不是灰度格式
        if (m_camera->setPixelFormat("RGB8")) {
            std::cout << "Set camera to RGB8 format - SN: " << m_serialNumber.toStdString() << std::endl;
        } else {
            std::cout << "Failed to set RGB8, using Mono8 - SN: " << m_serialNumber.toStdString() << std::endl;
            m_camera->setPixelFormat("Mono8");
        }
        
        // 设置曝光时间（可以根据需要调整）
        m_camera->setExposureTime(30000.0f); // 30ms
        
        std::cout << "Camera initialized successfully - SN: " << m_serialNumber.toStdString() 
                  << ", Format: " << m_camera->getPixelFormat().toStdString()
                  << ", Exposure: " << m_camera->getExposureTime() << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cout << "Camera initialization exception: " << e.what() 
                  << ", using TEST MODE - SN: " << m_serialNumber.toStdString() << std::endl;
        return true; // 继续使用测试模式
    }
}

void CameraThread::cleanupCamera()
{
    if (m_camera) {
        m_camera->stopGrabbing();
        m_camera->closeCamera();
        m_camera.reset();
    }
} 