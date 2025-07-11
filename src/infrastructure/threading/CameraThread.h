#ifndef CAMERA_THREAD_H
#define CAMERA_THREAD_H

#include <QThread>
#include <QMutex>
#include <QTimer>
#include <QImage>
#include <memory>
#include "../camera/HikVisionCamera.h"

class CameraThread : public QThread
{
    Q_OBJECT

public:
    explicit CameraThread(const QString& serialNumber, QObject* parent = nullptr);
    ~CameraThread();

    // 控制方法
    void startCapture();
    void stopCapture();
    bool isCapturing() const;
    
    // 参数设置
    void setTargetFPS(int fps);
    int getTargetFPS() const;
    
    // 相机信息
    QString getSerialNumber() const;
    bool isCameraConnected() const;

protected:
    void run() override;

signals:
    void frameReady(const QImage& image);
    void errorOccurred(const QString& error);
    void cameraConnected(const QString& serialNumber);
    void cameraDisconnected(const QString& serialNumber);

private:
    void captureFrame();

private:
    // 相机相关
    std::unique_ptr<HikVisionCamera> m_camera;
    QString m_serialNumber;
    
    // 线程控制
    bool m_running;
    bool m_capturing;
    mutable QMutex m_mutex;
    
    // 采集控制
    QTimer* m_captureTimer;
    int m_targetFPS;
    int m_frameInterval; // 毫秒
    
    // 图像处理
    QImage convertToQImage(unsigned char* pImageBuf, unsigned int width, 
                          unsigned int height, unsigned int imageSize);
    QImage convertGrayToPseudoColor(const QImage& grayImage);
    bool initializeCamera();
    void cleanupCamera();
    
    // 性能优化
    unsigned char* m_frameBuffer;
    unsigned int m_bufferSize;
};

#endif // CAMERA_THREAD_H 