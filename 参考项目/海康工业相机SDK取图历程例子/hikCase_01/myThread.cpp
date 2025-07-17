#include "mythread.h"

MyThread::MyThread(QObject *parent)
    : QThread{parent}
{
    image = new QImage();

}

MyThread::~MyThread()
{
    delete image;
    if(cameraPtr == NULL)
    {
        delete cameraPtr;
    }
    if(imagePtr == NULL)
    {
        delete imagePtr;
    }
}

void MyThread::getCameraPtr(CMvCamera *camera)
{
    cameraPtr = camera;
}

void MyThread::getImagePtr(cv::Mat *image)
{
    imagePtr = image;
}

void MyThread::run()
{
    if(cameraPtr == NULL)
    {
        return;
    }
    if(imagePtr == NULL)
    {
        return;
    }

    while(!isInterruptionRequested())
    {
        qDebug()<<"SoftTrigger:"<<cameraPtr->CommandExecute("TriggerSoftware");
        qDebug()<<"ReadBuffer:"<<cameraPtr->ReadBuffer(*imagePtr);

        //先处理好再发送
        if(imagePtr->channels()>1)
        {
            *image = QImage((const unsigned char*)(imagePtr->data),imagePtr->cols,imagePtr->rows,QImage::Format_RGB888);
        }
        else
        {
            *image = QImage((const unsigned char*)(imagePtr->data),imagePtr->cols,imagePtr->rows,QImage::Format_Indexed8);
        }
        emit signal_messImage(*image);
        msleep(10);

        //耗时操作，需要放到线程中保存
        //QImage保存图像
        //QString savePath = QDir::currentPath() + "/myImage/";
        //QString curDate = QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss.zzz");
        //QString saveName = savePath + curDate + ".png";
        //qDebug()<<"saveName:"<<saveName;
        //myImage->save(saveName);
    }
}
