#include "grabimgthread.h"
#include <QDebug>
#include <QDateTime>

GrabImgThread::GrabImgThread(int cameraId, QObject *parent)
    : QThread(parent)
    , m_cameraPtr(nullptr)
    , m_cameraId(cameraId)
{
}

GrabImgThread::~GrabImgThread()
{
    // 确保线程停止
    if (isRunning())
    {
        terminate();
        wait();
    }
}

void GrabImgThread::setCameraPtr(CMvCamera* cameraPtr)
{
    m_cameraPtr = cameraPtr;
}

cv::Mat GrabImgThread::getLastFrame()
{
    QMutexLocker locker(&m_frameMutex);
    return m_lastFrame.clone();
}

void GrabImgThread::run()
{
    if (!m_cameraPtr)
    {
        qDebug() << "相机指针为空，无法启动图像抓取线程";
        return;
    }

    // 不使用回调方式，而是直接循环读取图像
    // 线程循环，直到被终止
    int errorCount = 0;
    const int MAX_ERROR_COUNT = 10; // 最大连续错误次数
    
    while (!isInterruptionRequested())
    {
        // 使用OpenCV读取图像
        Mat cvImage;
        int nRet = m_cameraPtr->ReadBuffer(cvImage);
        if (MV_OK == nRet && !cvImage.empty())
        {
            // 重置错误计数
            errorCount = 0;
            
            // 保存最后一帧
            {
                QMutexLocker locker(&m_frameMutex);
                m_lastFrame = cvImage.clone();
            }
            
            // 转换为QImage
            QImage qImage;
            if (cvImage.channels() == 1)
            {
                // 灰度图像
                qImage = QImage(cvImage.data, cvImage.cols, cvImage.rows, cvImage.step, QImage::Format_Grayscale8).copy();
            }
            else if (cvImage.channels() == 3)
            {
                // 彩色图像
                // OpenCV使用BGR格式，需要转换为RGB
                cvtColor(cvImage, cvImage, COLOR_BGR2RGB);
                qImage = QImage(cvImage.data, cvImage.cols, cvImage.rows, cvImage.step, QImage::Format_RGB888).copy();
            }
            else
            {
                qDebug() << "不支持的图像格式，通道数:" << cvImage.channels();
                continue;
            }

            // 发送图像就绪信号
            emit signal_imageReady(qImage, m_cameraId);
        }
        else if (nRet != MV_OK)
        {
            errorCount++;
            qDebug() << "读取图像失败，错误码:" << nRet << "，连续错误次数:" << errorCount;
            
            // 如果连续错误次数过多，尝试重新开始采集
            if (errorCount >= MAX_ERROR_COUNT)
            {
                qDebug() << "连续错误次数过多，尝试重新开始采集";
                // 停止采集
                m_cameraPtr->StopGrabbing();
                // 等待一段时间
                msleep(500);
                // 重新开始采集
                m_cameraPtr->StartGrabbing();
                // 重置错误计数
                errorCount = 0;
                // 等待相机稳定
                msleep(500);
            }
            
            // 错误后增加等待时间
            msleep(100);
        }
        else
        {
            // 图像为空但没有错误，可能是正常情况
            qDebug() << "获取到空图像";
            msleep(50);
        }

        // 休眠一段时间，避免CPU占用过高
        msleep(30);
    }
}

void __stdcall GrabImgThread::ImageCallback(unsigned char *pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
{
    if (pData == NULL || pFrameInfo == NULL || pUser == NULL)
    {
        return;
    }

    GrabImgThread* pThread = static_cast<GrabImgThread*>(pUser);
    if (pThread == NULL)
    {
        return;
    }

    // 转换为QImage
    QImage qImage;
    if (pFrameInfo->enPixelType == PixelType_Gvsp_Mono8)
    {
        // 灰度图像
        qImage = QImage(pData, pFrameInfo->nWidth, pFrameInfo->nHeight, pFrameInfo->nWidth, QImage::Format_Grayscale8);
    }
    else if (pFrameInfo->enPixelType == PixelType_Gvsp_RGB8_Packed)
    {
        // RGB图像
        qImage = QImage(pData, pFrameInfo->nWidth, pFrameInfo->nHeight, pFrameInfo->nWidth * 3, QImage::Format_RGB888);
    }
    else
    {
        // 不支持的格式
        qDebug() << "不支持的图像格式，像素类型:" << pFrameInfo->enPixelType;
        return;
    }

    // 发送图像就绪信号
    emit pThread->signal_imageReady(qImage.copy(), pThread->m_cameraId);
} 