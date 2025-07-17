#include "grabimgthread.h"

GrabImgThread::GrabImgThread(int cameraId)
    : m_cameraId(cameraId)
{

}

GrabImgThread::~GrabImgThread()
{

}

//设置相机指针
void GrabImgThread::setCameraPtr(CMvCamera *camera)
{
    m_cameraPtr = camera;

    //注册回调函数
    //nRet = m_myCamera[i]->RegisterImageCallBack(ImageCallback,this);    //单色相机
    int nRet = m_cameraPtr->RegisterImageCallBackRGB(ImageCallback,this);   //彩色相机
    if(MV_OK != nRet)
    {
        LOGDEBUG<<"m_cameraId:"<<m_cameraId<<"注册回调函数失败！";
        return;
    }
}

//线程运行
void GrabImgThread::run()
{

}

//回调函数
void __stdcall GrabImgThread::ImageCallback(unsigned char * pData,MV_FRAME_OUT_INFO_EX* pFrameInfo,void* pUser)
{
    LOGDEBUG<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz ")<<"回调函数执行了";
    GrabImgThread* pThread = static_cast<GrabImgThread *>(pUser);

    //创建QImage对象
    QImage showImage = QImage(pData,pFrameInfo->nWidth,pFrameInfo->nHeight,QImage::Format_RGB888);

    //发送信号,通知主界面更新图像
    emit pThread->signal_imageReady(showImage,pThread->m_cameraId);
}

