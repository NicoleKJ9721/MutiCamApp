#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->initWidget();
}

MainWindow::~MainWindow()
{
    delete ui;

    for(int i=0;i<2;i++)
    {
        if(m_myCamera[i])
        {
            m_myCamera[i]->Close();
            delete m_myCamera[i];
            m_myCamera[i] = NULL;
        }
    }
}

void MainWindow::initWidget()
{
    for(int i=0;i<2;i++)
    {
        //相机对象
        m_myCamera[i] = new CMvCamera;

        //线程对象
        m_grabThread[i] = new GrabImgThread(i);
        connect(m_grabThread[i],SIGNAL(signal_imageReady(QImage,int)),this,SLOT(slot_imageReady(QImage,int)),Qt::BlockingQueuedConnection);
    }
}

//显示图像
void MainWindow::slot_imageReady(const QImage &image,int cameraId)
{
    QPixmap showPixmap = QPixmap::fromImage(image).scaled(QSize(250,200),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    if(cameraId == 0)
    {
        ui->lb_time_1->setText("相机1：" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz"));
        ui->lb_image_1->setPixmap(showPixmap);
    }
    else
    {
        ui->lb_time_2->setText("相机2：" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz"));
        ui->lb_image_2->setPixmap(showPixmap);
    }
}

//初始化
void MainWindow::on_pb_init_clicked()
{
    //枚举子网内所有设备
    memset(&m_stDevList,0,sizeof(MV_CC_DEVICE_INFO_LIST));
    int nRet = CMvCamera::EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE,&m_stDevList);
    if(MV_OK != nRet)
    {
        LOGDEBUG<<"枚举相机设备失败！";
        return;
    }

    int deviceNum = m_stDevList.nDeviceNum;
    LOGDEBUG<<"相机连接数量:"<<deviceNum;
    if(deviceNum > 2)
    {
        //我这里只定义了两个相机对象,所以限制为2,实际情况需要根据相机数量定义头文件中的对象数
        deviceNum = 2;
    }
    for(int i=0;i<deviceNum;i++)
    {
        MV_CC_DEVICE_INFO *pDeviceInfo = m_stDevList.pDeviceInfo[i];
        QString strSerialNumber = "";
        if(pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)
        {
            strSerialNumber = (char*)pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber;
        }
        else if(pDeviceInfo->nTLayerType == MV_USB_DEVICE)
        {
            strSerialNumber = (char*)pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber;
        }
        else
        {
            LOGDEBUG<<"警告,未知设备枚举！";
            return;
        }
        LOGDEBUG<<"i:"<<i<<"   strSerialNumber:"<<strSerialNumber;
        if(i == 0)
        {
            ui->lb_name_1->setText(strSerialNumber);
        }
        else if(i == 1)
        {
            ui->lb_name_2->setText(strSerialNumber);
        }

        //根据相机序列号指定相机对象,就可以指定某个窗口进行显示
        //if(strSerialNumber == "DA0333897")
        //{
        //    m_deviceInfo[0] = pDeviceInfo;
        //}
        //else if(strSerialNumber == "DA0424312")
        //{
        //    m_deviceInfo[1] = pDeviceInfo;
        //}

        //不指定
        m_deviceInfo[i] = pDeviceInfo;

        //打开相机
        int nRet = m_myCamera[i]->Open(m_deviceInfo[i]);
        if(MV_OK != nRet)
        {
            LOGDEBUG<<"i:"<<i<<"打开相机失败！";
            return;
        }
        else
        {
            //关闭触发模式
            nRet = m_myCamera[i]->SetEnumValue("TriggerMode",0);
            if(MV_OK != nRet)
            {
                LOGDEBUG<<"i:"<<i<<"关闭触发模式失败！";
                return;
            }

            //设置线程对象中的回调函数
            m_grabThread[i]->setCameraPtr(m_myCamera[i]);
        }
    }
}

//开始取图
void MainWindow::on_pb_start_clicked()
{
    for(int i=0;i<2;i++)
    {
        int nRet = m_myCamera[i]->StartGrabbing();
        if (MV_OK != nRet)
        {
            LOGDEBUG<<"i:"<<i<<"开始取图失败！";
            return;
        }
    }
}

//停止取图
void MainWindow::on_pb_stop_clicked()
{
    for(int i=0;i<2;i++)
    {
        int nRet = m_myCamera[i]->StopGrabbing();
        if (MV_OK != nRet)
        {
            LOGDEBUG<<"i:"<<i<<"停止取图失败！";
            return;
        }
    }
}

