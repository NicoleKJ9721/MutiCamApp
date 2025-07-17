#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include "cmvcamera.h"
#include "myThread.h"

QT_BEGIN_NAMESPACE
namespace Ui {class MainWindow;}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void init();
    void display(const Mat* imagePtr);
    void saveImage(QString format);

private slots:


    void on_btnFindDev_clicked();

    void on_btnOpen_clicked();

    void on_btnClose_clicked();

    void on_btnOnce_clicked();

    void on_btnContinus_clicked();

    void on_btnStop_clicked();

    void on_btnJPG_clicked();

    void on_btnPNG_clicked();

    void slot_displayImage(QImage image);





private:
    Ui::MainWindow *ui;
    bool m_bOpenDevice;
    CMvCamera *camera;
    MV_CC_DEVICE_INFO_LIST m_stDevList;
    cv::Mat *image = nullptr;
    MyThread *myThread = nullptr;

};
#endif // MAINWINDOW_H
