#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include "HikSdk/grabimgthread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void initWidget();

    void showImage(QImage showImage,int index);

private slots:
    void slot_imageReady(const QImage &image,int cameraId);

    void on_pb_init_clicked();
    void on_pb_start_clicked();
    void on_pb_stop_clicked();

private:
    Ui::MainWindow *ui;

    CMvCamera *m_myCamera[2];             //相机对象
    GrabImgThread *m_grabThread[2];       //捕获图像线程
    MV_CC_DEVICE_INFO *m_deviceInfo[2];   //设备信息
    MV_CC_DEVICE_INFO_LIST m_stDevList;   //设备信息列表
};
#endif // MAINWINDOW_H
