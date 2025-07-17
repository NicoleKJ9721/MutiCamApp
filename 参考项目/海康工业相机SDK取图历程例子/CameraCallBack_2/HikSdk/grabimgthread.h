#ifndef GRABIMGTHREAD_H
#define GRABIMGTHREAD_H

#include <QThread>
#include <QImage>
#include <QDateTime>
#include "HikSdk/cmvcamera.h"

class GrabImgThread : public QThread
{
    Q_OBJECT

public:
    explicit GrabImgThread(int cameraId);
    ~GrabImgThread();

    void setCameraPtr(CMvCamera *camera);

    void run();

signals:
    void signal_imageReady(const QImage &image,int cameraId);

private:
    static void __stdcall ImageCallback(unsigned char * pData,MV_FRAME_OUT_INFO_EX* pFrameInfo,void* pUser);

private:
    int m_cameraId;
    CMvCamera *m_cameraPtr;

};

#endif // GRABIMGTHREAD_H
