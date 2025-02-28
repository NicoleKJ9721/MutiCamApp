#ifndef GRABIMGTHREAD_H
#define GRABIMGTHREAD_H

#include <QThread>
#include <QImage>
#include <QMutex>
#include "cmvcamera.h"

class GrabImgThread : public QThread
{
    Q_OBJECT

public:
    explicit GrabImgThread(int cameraId, QObject *parent = nullptr);
    ~GrabImgThread();

    void setCameraPtr(CMvCamera* cameraPtr);
    int getImageSaveCount() const { return m_imageSaveCount; }
    
    // 获取最后一帧
    cv::Mat getLastFrame();

signals:
    void signal_imageReady(QImage image, int cameraId);
    void signal_imageSaved(int cameraId);

protected:
    void run() override;

private:
    static void __stdcall ImageCallback(unsigned char *pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser);

    CMvCamera* m_cameraPtr;
    int m_cameraId;
    int m_imageSaveCount = 0;
    
    // 最后一帧
    cv::Mat m_lastFrame;
    QMutex m_frameMutex;
};

#endif // GRABIMGTHREAD_H 