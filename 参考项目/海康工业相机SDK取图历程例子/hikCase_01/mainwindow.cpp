#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cmvcamera.h"
#include<QDebug>
#include<QImage>
#include<QPixmap>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete image;
    if(camera)
    {
        camera->Close();
        delete camera;
        camera = nullptr;
    }
    if(myThread->isRunning())
    {
        myThread->requestInterruption();
        myThread->wait();
        delete myThread;
    }
}

void MainWindow::init()
{
    this->setWindowTitle("海康相机功能测试");
    this->setWindowFlags(Qt::WindowCloseButtonHint | Qt::Dialog);
    this->setWindowModality(Qt::ApplicationModal);
    this->setFixedSize(this->width(), this->height());

    m_bOpenDevice = false;

    // 图像指针对象
    image = new Mat();
    //线程对象实例化
    myThread = new MyThread();
    // connect(myThread, SIGNAL(signal_message()), this, SLOT(slot_display()));
    connect(myThread, SIGNAL(signal_messImage(QImage)), this, SLOT(slot_displayImage(QImage)));


    qDebug()<<"init done";

}

void MainWindow::slot_displayImage(QImage qimage)
{
    //显示图像，因为myThread里已经处理好了图像的格式
    qimage = (qimage).scaled(ui->labDisplay->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    ui->labDisplay->setPixmap(QPixmap::fromImage(qimage));
}


void MainWindow::on_btnFindDev_clicked()
{
    ui->cbxSel->clear();
    memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    // enum all devices
    int nRet = CMvCamera::EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &m_stDevList);
    // std::cout<<"nRet:"<<nRet<<std::endl;
    if(MV_OK != nRet)
    {
        QMessageBox::warning(this, "警告", "枚举设备失败!");
        return;
    }
    for(unsigned int i = 0; i < m_stDevList.nDeviceNum; i++)
    {
        MV_CC_DEVICE_INFO* pDeviceInfo = m_stDevList.pDeviceInfo[i];
        QString strModelName = "";
        QString strSerialName = "";
        QString strDeviceName = "";
        if(pDeviceInfo->nTLayerType == MV_USB_DEVICE)
        {
            strModelName = (char*)pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName;
            strSerialName = (char*)pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber;
            strDeviceName = strModelName + " , " + strSerialName;
        }
        else if(pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)
        {
            strModelName = (char*)pDeviceInfo->SpecialInfo.stGigEInfo.chModelName;
            strSerialName = (char*)pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber;
            strDeviceName = strModelName + " , " + strSerialName;
        }
        else
        {
            QMessageBox::warning(this,"警告","未知设备枚举！");
            return;
        }
        qDebug()<<"strDeviceName:"<<strDeviceName;
        ui->cbxSel->addItem(strDeviceName);
    }
}


void MainWindow::on_btnOpen_clicked()
{
    if(m_bOpenDevice)
    {
        QMessageBox::warning(this, "警告","相机已打开！");
        return;
    }
    QString deviceModel = ui->cbxSel->currentText();
    if(deviceModel == "")
    {
        QMessageBox::warning(this,"警告","请选择设备！");
        return;
    }
    camera = new CMvCamera;
    if(NULL == camera)
    {
        return;
    }
    int index = ui->cbxSel->currentIndex();
    // 打开设备
    int nRet = camera->Open(m_stDevList.pDeviceInfo[index]);
    qDebug()<<"Connect:"<<nRet;
    if(MV_OK != nRet)
    {
        camera = NULL;
        QMessageBox::warning(this,"警告","打开设备失败！");
        return;
    }
    // 设置为触发模式
    qDebug()<<"TriggerMode:"<<camera->SetEnumValue("TriggerMode", 1);
    // 设置触发源为软触发
    qDebug()<<"TriggerSource:"<<camera->SetEnumValue("TriggerSource",7);
    //设置曝光时间
    qDebug()<<"SetExposureTime:"<<camera->SetFloatValue("ExposureTime",5000);
    //开启相机采集
    qDebug()<<"StartCamera:"<<camera->StartGrabbing();

    myThread->getCameraPtr(camera);
    myThread->getImagePtr(image);
    m_bOpenDevice = true;
}


void MainWindow::on_btnClose_clicked()
{
    if(ui->cbxSel->currentText() == "")
    {
        QMessageBox::warning(this, "警告", "设备未检索或未打开！");
        return;
    }
    if(camera)
    {
        camera->Close();
        delete camera;
        camera = nullptr;
        m_bOpenDevice = false;
    }
}


void MainWindow::on_btnOnce_clicked()
{
    if(!m_bOpenDevice)
    {
        QMessageBox::warning(this, "警告", "采集失败，请打开设备！");
        return;
    }
    Mat *image = new Mat;
    // 发送软触发
    qDebug()<<"single SoftTrigger:"<<camera->CommandExecute("TriggerSoftware");
    // 读取Mat格式的图像
    qDebug()<<"single ReadBuffer:"<<camera->ReadBuffer(*image);

    display(image);
    delete image;

}


void MainWindow::on_btnContinus_clicked()
{
    if(!m_bOpenDevice)
    {
        QMessageBox::warning(this, "警告", "采集失败，请打开设备！");
        return;
    }
    if(!myThread->isRunning())
    {
        myThread->start();
        qDebug()<<"Thread Start。";
    }

}


void MainWindow::on_btnStop_clicked()
{
    if(myThread->isRunning())
    {
        myThread->requestInterruption();
        myThread->wait();
    }
}


void MainWindow::on_btnJPG_clicked()
{
    saveImage(".jpg");
}


void MainWindow::on_btnPNG_clicked()
{
    saveImage(".png");
}

void MainWindow::display(const Mat* imagePtr)
{
    qDebug()<<"single display ok";
    // 判断是黑白、彩色图像
    QImage *qimage = new QImage();
    if(imagePtr->channels()>1)
    {
        // 将Mat格式的图像转为QImage
        *qimage = QImage((const unsigned char*)(imagePtr->data),imagePtr->cols,imagePtr->rows,QImage::Format_RGB888);
    }
    else
    {
        *qimage = QImage((const unsigned char*)(imagePtr->data),imagePtr->cols,imagePtr->rows,QImage::Format_Indexed8);
    }
    // 缩放图像
    *qimage = (*qimage).scaled(ui->labDisplay->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    // 显示图像
    ui->labDisplay->setPixmap(QPixmap::fromImage(*qimage));
    delete qimage;
}

void MainWindow::saveImage(QString format)
{
    if(ui->labDisplay->pixmap().isNull())
    {
        QMessageBox::warning(this,"警告","保存失败,未采集到图像！");
        return;
    }
    //QPixmap方法保存在程序运行目录
    //format: .bmp .tif .png .jpg
    QString savePath = QDir::currentPath() + "/IMAGES/";
    QDir().mkpath(savePath);
    QString curDate = QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss");
    QString saveName = savePath + curDate + format;
    QPixmap curImage = ui->labDisplay->pixmap();
    qDebug()<<"saveName:"<<saveName;
    if(curImage.save(saveName))
    {
        qDebug()<<"保存成功！";
    }
    else
    {
        qDebug()<<"保存失败！";
    }
}


























