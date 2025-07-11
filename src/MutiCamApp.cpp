#include "MutiCamApp.h"
#include "infrastructure/threading/CameraThread.h"
#include "utils/DebugUtils.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QPixmap>
#include <iostream>

MutiCamApp::MutiCamApp(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui_MutiCamApp)
    , lbVerticalView(nullptr)
    , lbLeftView(nullptr)
    , lbFrontView(nullptr)
    , lbVerticalView2(nullptr)
    , lbLeftView2(nullptr)
    , lbFrontView2(nullptr)
    , leGridDensity(nullptr)
    , leGridDensVertical(nullptr)
    , leGridDensLeft(nullptr)
    , leGridDensFront(nullptr)
    , m_drawingManager(nullptr)
    , m_verticalCameraThread(nullptr)
    , m_leftCameraThread(nullptr)
    , m_frontCameraThread(nullptr)
    , m_verticalCameraSN("Vir21084717")
    , m_leftCameraSN("Vir21128616")
    , m_frontCameraSN("Vir21128647")
    , m_camerasRunning(false)
{
    DEBUG_LOG("开始初始化MutiCamApp...");
    
    try {
        ui->setupUi(this);
        DEBUG_LOG("UI设置完成");
        
        setupUI();
        DEBUG_LOG("setupUI完成");
        
        setupDrawingManager();
        DEBUG_LOG("绘图管理器设置完成");
        
        connectSignals();
        DEBUG_LOG("信号连接完成");
        
        initializeViews();
        DEBUG_LOG("视图初始化完成");
        
        setupMouseEventFilters();
        DEBUG_LOG("鼠标事件过滤器设置完成");
        
        initializeCameraThreads();
        DEBUG_LOG("相机线程初始化完成");
        
        DEBUG_LOG("MutiCamApp初始化完成");
    } catch (const std::exception& e) {
        qCritical() << "MutiCamApp初始化异常:" << e.what();
    } catch (...) {
        qCritical() << "MutiCamApp初始化发生未知异常";
    }
}

MutiCamApp::~MutiCamApp()
{
    cleanupCameraThreads();
    delete ui; 
}

// ======================= 调试控制方法 =======================

void MutiCamApp::enableDebugOutput()
{
    DebugConfig::enableDebug();
    DEBUG_LOG("调试输出已开启");
}

void MutiCamApp::disableDebugOutput()
{
    DEBUG_LOG("调试输出即将关闭");
    DebugConfig::disableDebug();
}

bool MutiCamApp::isDebugOutputEnabled() const
{
    return DebugConfig::isDebugEnabled();
}

void MutiCamApp::setupUI()
{
    // 获取UI控件的指针引用
    lbVerticalView = ui->lbVerticalView;
    lbLeftView = ui->lbLeftView;
    lbFrontView = ui->lbFrontView;
    lbVerticalView2 = ui->lbVerticalView2;
    lbLeftView2 = ui->lbLeftView2;
    lbFrontView2 = ui->lbFrontView2;
    
    leGridDensity = ui->leGridDensity;
    leGridDensVertical = ui->leGridDensVertical;
    leGridDensLeft = ui->leGridDensLeft;
    leGridDensFront = ui->leGridDensFront;
    
    // 设置默认网格密度值
    leGridDensity->setText("50");
    leGridDensVertical->setText("50");
    leGridDensLeft->setText("50");
    leGridDensFront->setText("50");
}

void MutiCamApp::connectSignals()
{
    // 主界面绘图工具信号连接
    connect(ui->btnDrawPoint, &QPushButton::clicked, this, &MutiCamApp::onDrawPointClicked);
    connect(ui->btnDrawStraight, &QPushButton::clicked, this, &MutiCamApp::onDrawStraightClicked);
    connect(ui->btnDrawSimpleCircle, &QPushButton::clicked, this, &MutiCamApp::onDrawSimpleCircleClicked);
    connect(ui->btnDrawParallel, &QPushButton::clicked, this, &MutiCamApp::onDrawParallelClicked);
    connect(ui->btnDraw2Line, &QPushButton::clicked, this, &MutiCamApp::onDraw2LineClicked);
    connect(ui->btnDrawFineCircle, &QPushButton::clicked, this, &MutiCamApp::onDrawFineCircleClicked);
    connect(ui->btnCan1StepDraw, &QPushButton::clicked, this, &MutiCamApp::onCan1StepDrawClicked);
    connect(ui->btnClearDrawings, &QPushButton::clicked, this, &MutiCamApp::onClearDrawingsClicked);
    connect(ui->btnSaveImage, &QPushButton::clicked, this, &MutiCamApp::onSaveImageClicked);
    
    // 网格控制信号连接
    connect(ui->btnCancelGrids, &QPushButton::clicked, this, &MutiCamApp::onCancelGridsClicked);
    
    // 自动检测信号连接
    connect(ui->btnLineDet, &QPushButton::clicked, this, &MutiCamApp::onLineDetClicked);
    connect(ui->btnCircleDet, &QPushButton::clicked, this, &MutiCamApp::onCircleDetClicked);
    connect(ui->btnCan1StepDet, &QPushButton::clicked, this, &MutiCamApp::onCan1StepDetClicked);
    
    // 测量控制信号连接
    connect(ui->btnStartMeasure, &QPushButton::clicked, this, &MutiCamApp::onStartMeasureClicked);
    connect(ui->btnStopMeasure, &QPushButton::clicked, this, &MutiCamApp::onStopMeasureClicked);
    
    // 垂直视图信号连接
    connect(ui->btnDrawParallelVertical, &QPushButton::clicked, this, &MutiCamApp::onVerticalDrawParallelClicked);
    connect(ui->btnCalibrationVertical, &QPushButton::clicked, this, &MutiCamApp::onVerticalCalibrationClicked);
    connect(ui->btnDrawFineCircleVertical, &QPushButton::clicked, this, &MutiCamApp::onVerticalDrawFineCircleClicked);
    connect(ui->btnCan1StepDrawVertical, &QPushButton::clicked, this, &MutiCamApp::onVerticalCan1StepDrawClicked);
    connect(ui->btnDrawStraightVertical, &QPushButton::clicked, this, &MutiCamApp::onVerticalDrawStraightClicked);
    connect(ui->btnClearDrawingsVertical, &QPushButton::clicked, this, &MutiCamApp::onVerticalClearDrawingsClicked);
    connect(ui->btnDrawPointVertical, &QPushButton::clicked, this, &MutiCamApp::onVerticalDrawPointClicked);
    connect(ui->btnDraw2LineVertical, &QPushButton::clicked, this, &MutiCamApp::onVerticalDraw2LineClicked);
    connect(ui->btnDrawSimpleCircleVertical, &QPushButton::clicked, this, &MutiCamApp::onVerticalDrawSimpleCircleClicked);
    connect(ui->btnLineDetVertical, &QPushButton::clicked, this, &MutiCamApp::onVerticalLineDetClicked);
    connect(ui->btnCircleDetVertical, &QPushButton::clicked, this, &MutiCamApp::onVerticalCircleDetClicked);
    connect(ui->btnCan1StepDetVertical, &QPushButton::clicked, this, &MutiCamApp::onVerticalCan1StepDetClicked);
    connect(ui->btnCancelGridsVertical, &QPushButton::clicked, this, &MutiCamApp::onVerticalCancelGridsClicked);
    connect(ui->btnSaveImageVertical, &QPushButton::clicked, this, &MutiCamApp::onVerticalSaveImageClicked);
    
    // 左侧视图信号连接
    connect(ui->btnClearDrawingsLeft, &QPushButton::clicked, this, &MutiCamApp::onLeftClearDrawingsClicked);
    connect(ui->btnDrawFineCircleLeft, &QPushButton::clicked, this, &MutiCamApp::onLeftDrawFineCircleClicked);
    connect(ui->btnDrawParallelLeft, &QPushButton::clicked, this, &MutiCamApp::onLeftDrawParallelClicked);
    connect(ui->btnCan1StepDrawLeft, &QPushButton::clicked, this, &MutiCamApp::onLeftCan1StepDrawClicked);
    connect(ui->btnDrawStraightLeft, &QPushButton::clicked, this, &MutiCamApp::onLeftDrawStraightClicked);
    connect(ui->btnDrawPointLeft, &QPushButton::clicked, this, &MutiCamApp::onLeftDrawPointClicked);
    connect(ui->btnDrawSimpleCircleLeft, &QPushButton::clicked, this, &MutiCamApp::onLeftDrawSimpleCircleClicked);
    connect(ui->btnDraw2LineLeft, &QPushButton::clicked, this, &MutiCamApp::onLeftDraw2LineClicked);
    connect(ui->btnCalibrationLeft, &QPushButton::clicked, this, &MutiCamApp::onLeftCalibrationClicked);
    connect(ui->btnLineDetLeft, &QPushButton::clicked, this, &MutiCamApp::onLeftLineDetClicked);
    connect(ui->btnCircleDetLeft, &QPushButton::clicked, this, &MutiCamApp::onLeftCircleDetClicked);
    connect(ui->btnCan1StepDetLeft, &QPushButton::clicked, this, &MutiCamApp::onLeftCan1StepDetClicked);
    connect(ui->btnCancelGridsLeft, &QPushButton::clicked, this, &MutiCamApp::onLeftCancelGridsClicked);
    connect(ui->btnSaveImageLeft, &QPushButton::clicked, this, &MutiCamApp::onLeftSaveImageClicked);
    
    // 对向视图信号连接
    connect(ui->btnDrawPointFront, &QPushButton::clicked, this, &MutiCamApp::onFrontDrawPointClicked);
    connect(ui->btnDrawStraightFront, &QPushButton::clicked, this, &MutiCamApp::onFrontDrawStraightClicked);
    connect(ui->btnDrawSimpleCircleFront, &QPushButton::clicked, this, &MutiCamApp::onFrontDrawSimpleCircleClicked);
    connect(ui->btnDrawParallelFront, &QPushButton::clicked, this, &MutiCamApp::onFrontDrawParallelClicked);
    connect(ui->btnDraw2LineFront, &QPushButton::clicked, this, &MutiCamApp::onFrontDraw2LineClicked);
    connect(ui->btnDrawFineCircleFront, &QPushButton::clicked, this, &MutiCamApp::onFrontDrawFineCircleClicked);
    connect(ui->btnCan1StepDrawFront, &QPushButton::clicked, this, &MutiCamApp::onFrontCan1StepDrawClicked);
    connect(ui->btnClearDrawingsFront, &QPushButton::clicked, this, &MutiCamApp::onFrontClearDrawingsClicked);
    connect(ui->btnCalibrationFront, &QPushButton::clicked, this, &MutiCamApp::onFrontCalibrationClicked);
    connect(ui->btnLineDetFront, &QPushButton::clicked, this, &MutiCamApp::onFrontLineDetClicked);
    connect(ui->btnCircleDetFront, &QPushButton::clicked, this, &MutiCamApp::onFrontCircleDetClicked);
    connect(ui->btnCan1StepDetFront, &QPushButton::clicked, this, &MutiCamApp::onFrontCan1StepDetClicked);
    connect(ui->btnCancelGridsFront, &QPushButton::clicked, this, &MutiCamApp::onFrontCancelGridsClicked);
    connect(ui->btnSaveImageFront, &QPushButton::clicked, this, &MutiCamApp::onFrontSaveImageClicked);
}

void MutiCamApp::initializeViews()
{
    // 设置图像显示标签的样式
    QString imageStyle = "QLabel { border: 2px solid gray; background-color: #f0f0f0; }";
    
    if (lbVerticalView) {
        lbVerticalView->setStyleSheet(imageStyle);
        lbVerticalView->setText("垂直相机视图");
    }
    if (lbLeftView) {
        lbLeftView->setStyleSheet(imageStyle);
        lbLeftView->setText("左侧相机视图");
    }
    if (lbFrontView) {
        lbFrontView->setStyleSheet(imageStyle);
        lbFrontView->setText("对向相机视图");
    }
    if (lbVerticalView2) {
        lbVerticalView2->setStyleSheet(imageStyle);
        lbVerticalView2->setText("垂直相机视图");
    }
    if (lbLeftView2) {
        lbLeftView2->setStyleSheet(imageStyle);
        lbLeftView2->setText("左侧相机视图");
    }
    if (lbFrontView2) {
        lbFrontView2->setStyleSheet(imageStyle);
        lbFrontView2->setText("对向相机视图");
    }
    
    // 设置窗口标题和状态栏
    setWindowTitle("靶装配对接测量软件 - C++版本");
    statusBar()->showMessage("准备就绪", 3000);
}

// 主界面绘图工具槽函数实现
void MutiCamApp::onDrawPointClicked() {
    qDebug() << "绘制点工具被点击";
    statusBar()->showMessage("绘制点工具已激活", 2000);
    
    if (m_drawingManager) {
        m_drawingManager->startPointMeasurement();
    }
}

void MutiCamApp::onDrawStraightClicked() {
    qDebug() << "绘制直线工具被点击";
    statusBar()->showMessage("绘制直线工具已激活", 2000);
}

void MutiCamApp::onDrawSimpleCircleClicked() {
    qDebug() << "绘制圆工具被点击";
    statusBar()->showMessage("绘制圆工具已激活", 2000);
}

void MutiCamApp::onDrawParallelClicked() {
    qDebug() << "绘制平行线工具被点击";
    statusBar()->showMessage("绘制平行线工具已激活", 2000);
}

void MutiCamApp::onDraw2LineClicked() {
    qDebug() << "绘制角度线工具被点击";
    statusBar()->showMessage("绘制角度线工具已激活", 2000);
}

void MutiCamApp::onDrawFineCircleClicked() {
    qDebug() << "绘制精确圆工具被点击";
    statusBar()->showMessage("绘制精确圆工具已激活", 2000);
}

void MutiCamApp::onCan1StepDrawClicked() {
    qDebug() << "撤销绘制操作被点击";
    statusBar()->showMessage("已撤销上一步绘制操作", 2000);
}

void MutiCamApp::onClearDrawingsClicked() {
    qDebug() << "清除绘制被点击";
    statusBar()->showMessage("已清除所有绘制内容", 2000);
}

void MutiCamApp::onSaveImageClicked() {
    QString fileName = QFileDialog::getSaveFileName(this, "保存图像", "", "Images (*.png *.jpg *.bmp)");
    if (!fileName.isEmpty()) {
        qDebug() << "保存图像到:" << fileName;
        statusBar()->showMessage("图像已保存到: " + fileName, 3000);
    }
}

// 网格控制槽函数
void MutiCamApp::onCancelGridsClicked() {
    qDebug() << "取消网格被点击";
    statusBar()->showMessage("网格已取消", 2000);
}

// 自动检测槽函数
void MutiCamApp::onLineDetClicked() {
    qDebug() << "直线查找被点击";
    statusBar()->showMessage("正在进行直线查找...", 2000);
}

void MutiCamApp::onCircleDetClicked() {
    qDebug() << "圆查找被点击";
    statusBar()->showMessage("正在进行圆查找...", 2000);
}

void MutiCamApp::onCan1StepDetClicked() {
    qDebug() << "撤销识别被点击";
    statusBar()->showMessage("已撤销上一步识别操作", 2000);
}

// 测量控制槽函数
void MutiCamApp::onStartMeasureClicked() {
    qDebug() << "开始测量被点击";
    startCameras();
}

void MutiCamApp::onStopMeasureClicked() {
    qDebug() << "停止测量被点击";
    stopCameras();
}

// 垂直视图槽函数实现（简化实现，显示调试信息）
void MutiCamApp::onVerticalDrawParallelClicked() { qDebug() << "垂直视图 - 绘制平行线"; }
void MutiCamApp::onVerticalCalibrationClicked() { qDebug() << "垂直视图 - 标定"; }
void MutiCamApp::onVerticalDrawFineCircleClicked() { qDebug() << "垂直视图 - 绘制精确圆"; }
void MutiCamApp::onVerticalCan1StepDrawClicked() { qDebug() << "垂直视图 - 撤销绘制"; }
void MutiCamApp::onVerticalDrawStraightClicked() { qDebug() << "垂直视图 - 绘制直线"; }
void MutiCamApp::onVerticalClearDrawingsClicked() { qDebug() << "垂直视图 - 清除绘制"; }
void MutiCamApp::onVerticalDrawPointClicked() { 
    qDebug() << "垂直视图 - 绘制点"; 
    statusBar()->showMessage("垂直视图绘制点工具已激活", 2000);
    
    if (m_drawingManager) {
        m_drawingManager->startPointMeasurement();
    }
}
void MutiCamApp::onVerticalDraw2LineClicked() { qDebug() << "垂直视图 - 绘制角度线"; }
void MutiCamApp::onVerticalDrawSimpleCircleClicked() { qDebug() << "垂直视图 - 绘制圆"; }
void MutiCamApp::onVerticalLineDetClicked() { qDebug() << "垂直视图 - 直线查找"; }
void MutiCamApp::onVerticalCircleDetClicked() { qDebug() << "垂直视图 - 圆查找"; }
void MutiCamApp::onVerticalCan1StepDetClicked() { qDebug() << "垂直视图 - 撤销识别"; }
void MutiCamApp::onVerticalCancelGridsClicked() { qDebug() << "垂直视图 - 取消网格"; }
void MutiCamApp::onVerticalSaveImageClicked() { qDebug() << "垂直视图 - 保存图像"; }

// 左侧视图槽函数实现（简化实现，显示调试信息）
void MutiCamApp::onLeftClearDrawingsClicked() { qDebug() << "左侧视图 - 清除绘制"; }
void MutiCamApp::onLeftDrawFineCircleClicked() { qDebug() << "左侧视图 - 绘制精确圆"; }
void MutiCamApp::onLeftDrawParallelClicked() { qDebug() << "左侧视图 - 绘制平行线"; }
void MutiCamApp::onLeftCan1StepDrawClicked() { qDebug() << "左侧视图 - 撤销绘制"; }
void MutiCamApp::onLeftDrawStraightClicked() { qDebug() << "左侧视图 - 绘制直线"; }
void MutiCamApp::onLeftDrawPointClicked() { 
    qDebug() << "左侧视图 - 绘制点"; 
    statusBar()->showMessage("左侧视图绘制点工具已激活", 2000);
    
    if (m_drawingManager) {
        m_drawingManager->startPointMeasurement();
    }
}
void MutiCamApp::onLeftDrawSimpleCircleClicked() { qDebug() << "左侧视图 - 绘制圆"; }
void MutiCamApp::onLeftDraw2LineClicked() { qDebug() << "左侧视图 - 绘制角度线"; }
void MutiCamApp::onLeftCalibrationClicked() { qDebug() << "左侧视图 - 标定"; }
void MutiCamApp::onLeftLineDetClicked() { qDebug() << "左侧视图 - 直线查找"; }
void MutiCamApp::onLeftCircleDetClicked() { qDebug() << "左侧视图 - 圆查找"; }
void MutiCamApp::onLeftCan1StepDetClicked() { qDebug() << "左侧视图 - 撤销识别"; }
void MutiCamApp::onLeftCancelGridsClicked() { qDebug() << "左侧视图 - 取消网格"; }
void MutiCamApp::onLeftSaveImageClicked() { qDebug() << "左侧视图 - 保存图像"; }

// 对向视图槽函数实现（简化实现，显示调试信息）
void MutiCamApp::onFrontDrawPointClicked() { 
    qDebug() << "对向视图 - 绘制点"; 
    statusBar()->showMessage("对向视图绘制点工具已激活", 2000);
    
    if (m_drawingManager) {
        m_drawingManager->startPointMeasurement();
    }
}
void MutiCamApp::onFrontDrawStraightClicked() { qDebug() << "对向视图 - 绘制直线"; }
void MutiCamApp::onFrontDrawSimpleCircleClicked() { qDebug() << "对向视图 - 绘制圆"; }
void MutiCamApp::onFrontDrawParallelClicked() { qDebug() << "对向视图 - 绘制平行线"; }
void MutiCamApp::onFrontDraw2LineClicked() { qDebug() << "对向视图 - 绘制角度线"; }
void MutiCamApp::onFrontDrawFineCircleClicked() { qDebug() << "对向视图 - 绘制精确圆"; }
void MutiCamApp::onFrontCan1StepDrawClicked() { qDebug() << "对向视图 - 撤销绘制"; }
void MutiCamApp::onFrontClearDrawingsClicked() { qDebug() << "对向视图 - 清除绘制"; }
void MutiCamApp::onFrontCalibrationClicked() { qDebug() << "对向视图 - 标定"; }
void MutiCamApp::onFrontLineDetClicked() { qDebug() << "对向视图 - 直线查找"; }
void MutiCamApp::onFrontCircleDetClicked() { qDebug() << "对向视图 - 圆查找"; }
void MutiCamApp::onFrontCan1StepDetClicked() { qDebug() << "对向视图 - 撤销识别"; }
void MutiCamApp::onFrontCancelGridsClicked() { qDebug() << "对向视图 - 取消网格"; }
void MutiCamApp::onFrontSaveImageClicked() { qDebug() << "对向视图 - 保存图像"; }

// ======================= 相机控制方法实现 =======================

void MutiCamApp::initializeCameraThreads()
{
    qDebug() << "初始化相机线程...";
    
    // 创建垂直视图相机线程
    m_verticalCameraThread = std::make_unique<CameraThread>(m_verticalCameraSN, this);
    connect(m_verticalCameraThread.get(), &CameraThread::frameReady, 
            this, &MutiCamApp::onVerticalFrameReady);
    connect(m_verticalCameraThread.get(), &CameraThread::errorOccurred, 
            this, &MutiCamApp::onCameraError);
    connect(m_verticalCameraThread.get(), &CameraThread::cameraConnected, 
            this, &MutiCamApp::onCameraConnected);
    connect(m_verticalCameraThread.get(), &CameraThread::cameraDisconnected, 
            this, &MutiCamApp::onCameraDisconnected);
    
    // 创建左侧视图相机线程
    m_leftCameraThread = std::make_unique<CameraThread>(m_leftCameraSN, this);
    connect(m_leftCameraThread.get(), &CameraThread::frameReady, 
            this, &MutiCamApp::onLeftFrameReady);
    connect(m_leftCameraThread.get(), &CameraThread::errorOccurred, 
            this, &MutiCamApp::onCameraError);
    connect(m_leftCameraThread.get(), &CameraThread::cameraConnected, 
            this, &MutiCamApp::onCameraConnected);
    connect(m_leftCameraThread.get(), &CameraThread::cameraDisconnected, 
            this, &MutiCamApp::onCameraDisconnected);
    
    // 创建对向视图相机线程
    m_frontCameraThread = std::make_unique<CameraThread>(m_frontCameraSN, this);
    connect(m_frontCameraThread.get(), &CameraThread::frameReady, 
            this, &MutiCamApp::onFrontFrameReady);
    connect(m_frontCameraThread.get(), &CameraThread::errorOccurred, 
            this, &MutiCamApp::onCameraError);
    connect(m_frontCameraThread.get(), &CameraThread::cameraConnected, 
            this, &MutiCamApp::onCameraConnected);
    connect(m_frontCameraThread.get(), &CameraThread::cameraDisconnected, 
            this, &MutiCamApp::onCameraDisconnected);
    
    qDebug() << "相机线程初始化完成";
}

void MutiCamApp::cleanupCameraThreads()
{
    qDebug() << "清理相机线程...";
    
    stopCameras();
    
    if (m_verticalCameraThread) {
        m_verticalCameraThread.reset();
    }
    
    if (m_leftCameraThread) {
        m_leftCameraThread.reset();
    }
    
    if (m_frontCameraThread) {
        m_frontCameraThread.reset();
    }
    
    qDebug() << "相机线程清理完成";
}

void MutiCamApp::startCameras()
{
    if (m_camerasRunning) {
        qDebug() << "相机已在运行中";
        return;
    }
    
    qDebug() << "启动相机采集...";
    statusBar()->showMessage("正在启动相机...", 3000);
    
    // 枚举设备
    if (!HikVisionCamera::enumerateDevices()) {
        QString errorMsg = "未找到任何相机设备，请检查相机连接和驱动安装";
        QMessageBox::warning(this, "相机错误", errorMsg);
        statusBar()->showMessage(errorMsg, 5000);
        return;
    }
    
    // 启动垂直视图相机
    if (m_verticalCameraThread) {
        qDebug() << "启动垂直相机线程，序列号:" << m_verticalCameraSN;
        m_verticalCameraThread->startCapture();
        qDebug() << "垂直相机线程是否在采集:" << m_verticalCameraThread->isCapturing();
    }
    
    // 启动左侧视图相机
    if (m_leftCameraThread) {
        qDebug() << "启动左侧相机线程，序列号:" << m_leftCameraSN;
        m_leftCameraThread->startCapture();
        qDebug() << "左侧相机线程是否在采集:" << m_leftCameraThread->isCapturing();
    }
    
    // 启动对向视图相机
    if (m_frontCameraThread) {
        qDebug() << "启动对向相机线程，序列号:" << m_frontCameraSN;
        m_frontCameraThread->startCapture();
        qDebug() << "对向相机线程是否在采集:" << m_frontCameraThread->isCapturing();
    }
    
    m_camerasRunning = true;
    
    // 更新UI状态
    ui->btnStartMeasure->setEnabled(false);
    ui->btnStopMeasure->setEnabled(true);
    
    statusBar()->showMessage("相机启动完成", 3000);
    qDebug() << "相机启动流程完成";
}

void MutiCamApp::stopCameras()
{
    if (!m_camerasRunning) {
        return;
    }
    
    qDebug() << "停止相机采集...";
    statusBar()->showMessage("正在停止相机...", 3000);
    
    // 停止所有相机线程
    if (m_verticalCameraThread) {
        m_verticalCameraThread->stopCapture();
    }
    
    if (m_leftCameraThread) {
        m_leftCameraThread->stopCapture();
    }
    
    if (m_frontCameraThread) {
        m_frontCameraThread->stopCapture();
    }
    
    m_camerasRunning = false;
    
    // 更新UI状态
    ui->btnStartMeasure->setEnabled(true);
    ui->btnStopMeasure->setEnabled(false);
    
    statusBar()->showMessage("相机已停止", 3000);
    qDebug() << "相机停止完成";
}

void MutiCamApp::displayImage(const QImage& image, QLabel* label)
{
    if (image.isNull() || !label) {
        ERROR_OUT("ERROR: displayImage failed - image null: " << image.isNull() << ", label null: " << (!label));
        return;
    }
    
    // 使用高质量显示方法
    displayImageHighQuality(image, label, 1.0);
}

void MutiCamApp::displayImageHighQuality(const QImage& image, QLabel* label, double zoomFactor)
{
    if (image.isNull() || !label) {
        return;
    }
    
    // 检查标签是否可见，不可见则跳过渲染
    if (!label->isVisible()) {
        return;
    }
    
    // 获取原始图像和标签尺寸
    int imgWidth = image.width();
    int imgHeight = image.height();
    QSize labelSize = label->size();
    
    DEBUG_OUT("Optimized display - Original: " << imgWidth << "x" << imgHeight 
              << ", Label: " << labelSize.width() << "x" << labelSize.height()
              << ", Zoom: " << zoomFactor);
    
    // 计算基础缩放比例（适应标签大小）
    double baseScale = std::min(static_cast<double>(labelSize.width()) / imgWidth, 
                               static_cast<double>(labelSize.height()) / imgHeight);
    
    // 应用用户缩放因子
    double finalScale = baseScale * zoomFactor;
    
    // 添加缩放阈值判断，避免不必要的缩放操作
    const double SCALE_THRESHOLD = 0.01;
    if (std::abs(finalScale - 1.0) < SCALE_THRESHOLD) {
        // 缩放比例接近1.0时，直接使用原图像，避免不必要的缩放
        QPixmap pixmap = QPixmap::fromImage(image);
        label->setPixmap(pixmap);
        label->setAlignment(Qt::AlignCenter);
        return;
    }
    
    // 计算目标尺寸
    int targetWidth = static_cast<int>(imgWidth * finalScale);
    int targetHeight = static_cast<int>(imgHeight * finalScale);
    
    // 确保缩放后的图像不会超出标签范围
    if (targetWidth > labelSize.width() || targetHeight > labelSize.height()) {
        double adjustedScale = std::min(static_cast<double>(labelSize.width()) / imgWidth,
                                       static_cast<double>(labelSize.height()) / imgHeight);
        finalScale = adjustedScale;
        targetWidth = static_cast<int>(imgWidth * finalScale);
        targetHeight = static_cast<int>(imgHeight * finalScale);
        DEBUG_OUT("Scale adjusted to fit label: " << finalScale);
    }
    
    // 使用优化的缩放算法（参考Python版本）
    Qt::TransformationMode transformMode;
    if (finalScale > 1.0) {
        // 放大时使用平滑变换以保持质量
        transformMode = Qt::SmoothTransformation;
    } else {
        // 缩小时使用快速变换以提升性能（最近邻插值）
        transformMode = Qt::FastTransformation;
    }
    
    // 执行单次缩放，避免多级缩放的性能损失
    QImage scaledImage = image.scaled(targetWidth, targetHeight, Qt::KeepAspectRatio, transformMode);
    
    DEBUG_OUT("Scaled image size: " << scaledImage.width() << "x" << scaledImage.height());
    
    // 创建QPixmap并设置（这里仍然需要复制，但已经优化了缩放部分）
    QPixmap pixmap = QPixmap::fromImage(scaledImage);
    label->setPixmap(pixmap);
    label->setAlignment(Qt::AlignCenter);
    
    // 强制更新显示（参考Python版本）
    label->update();
    
    // 计算实际的缩放比例用于调试
    double actualScale = static_cast<double>(scaledImage.width()) / imgWidth;
    DEBUG_OUT("Actual scale applied: " << actualScale << " (target: " << finalScale << ")");
}

// ======================= 相机信号处理槽函数 =======================

void MutiCamApp::onVerticalFrameReady(const QImage& image)
{
    DEBUG_OUT("=== 收到垂直相机图像 ===");
    DEBUG_OUT("图像尺寸: " << image.width() << "x" << image.height());
    DEBUG_OUT("图像格式: " << (int)image.format());
    
    // 保存当前帧（直接赋值，避免深拷贝）
    m_currentVerticalFrame = image;
    
    // 只更新可见的视图，减少不必要的渲染
    if (lbVerticalView && lbVerticalView->isVisible()) {
        displayImageWithDrawings(image, lbVerticalView);
    }
    
    // 检查Tab视图是否可见（只有当Tab页被选中时才更新）
    if (lbVerticalView2 && lbVerticalView2->isVisible() && 
        ui->tabWidget->currentWidget()->findChild<QLabel*>("lbVerticalView2") == lbVerticalView2) {
        displayImageWithDrawings(image, lbVerticalView2);
    }
    
    DEBUG_OUT("垂直相机图像处理完成");
}

void MutiCamApp::onLeftFrameReady(const QImage& image)
{
    DEBUG_LOG("收到左侧相机图像 - 尺寸:" << image.size() << "格式:" << image.format());
    
    // 保存当前帧（避免深拷贝）
    m_currentLeftFrame = image;
    
    // 只更新可见的视图
    if (lbLeftView && lbLeftView->isVisible()) {
        displayImageWithDrawings(image, lbLeftView);
    }
    
    // 检查Tab视图是否可见
    if (lbLeftView2 && lbLeftView2->isVisible() && 
        ui->tabWidget->currentWidget()->findChild<QLabel*>("lbLeftView2") == lbLeftView2) {
        displayImageWithDrawings(image, lbLeftView2);
    }
}

void MutiCamApp::onFrontFrameReady(const QImage& image)
{
    DEBUG_LOG("收到对向相机图像 - 尺寸:" << image.size() << "格式:" << image.format());
    
    // 保存当前帧（避免深拷贝）
    m_currentFrontFrame = image;
    
    // 只更新可见的视图
    if (lbFrontView && lbFrontView->isVisible()) {
        displayImageWithDrawings(image, lbFrontView);
    }
    
    // 检查Tab视图是否可见
    if (lbFrontView2 && lbFrontView2->isVisible() && 
        ui->tabWidget->currentWidget()->findChild<QLabel*>("lbFrontView2") == lbFrontView2) {
        displayImageWithDrawings(image, lbFrontView2);
    }
}

void MutiCamApp::onCameraError(const QString& error)
{
    qWarning() << "相机错误:" << error;
    statusBar()->showMessage("相机错误: " + error, 5000);
    
    // 显示错误对话框
    QMessageBox::warning(this, "相机错误", error);
}

void MutiCamApp::onCameraConnected(const QString& serialNumber)
{
    qDebug() << "相机连接成功:" << serialNumber;
    
    QString message;
    if (serialNumber == m_verticalCameraSN) {
        message = "垂直相机连接成功";
    } else if (serialNumber == m_leftCameraSN) {
        message = "左侧相机连接成功";
    } else if (serialNumber == m_frontCameraSN) {
        message = "对向相机连接成功";
    } else {
        message = QString("相机连接成功: %1").arg(serialNumber);
    }
    
    statusBar()->showMessage(message, 3000);
}

void MutiCamApp::onCameraDisconnected(const QString& serialNumber)
{
    qDebug() << "相机断开连接:" << serialNumber;
    
    QString message;
    if (serialNumber == m_verticalCameraSN) {
        message = "垂直相机断开连接";
    } else if (serialNumber == m_leftCameraSN) {
        message = "左侧相机断开连接";
    } else if (serialNumber == m_frontCameraSN) {
        message = "对向相机断开连接";
    } else {
        message = QString("相机断开连接: %1").arg(serialNumber);
    }
    
    statusBar()->showMessage(message, 3000);
}

// ======================= 绘图管理器相关方法 =======================

void MutiCamApp::setupDrawingManager()
{
    // 创建绘图管理器
    m_drawingManager = std::make_unique<DrawingManager>(this);
    
    // 设置所有视图
    if (lbVerticalView) {
        m_drawingManager->setupView(lbVerticalView, "vertical");
    }
    if (lbLeftView) {
        m_drawingManager->setupView(lbLeftView, "left");
    }
    if (lbFrontView) {
        m_drawingManager->setupView(lbFrontView, "front");
    }
    if (lbVerticalView2) {
        m_drawingManager->setupView(lbVerticalView2, "vertical_2");
    }
    if (lbLeftView2) {
        m_drawingManager->setupView(lbLeftView2, "left_2");
    }
    if (lbFrontView2) {
        m_drawingManager->setupView(lbFrontView2, "front_2");
    }
    
    // 设置视图配对关系（主界面 <-> Tab页面）
    if (lbVerticalView && lbVerticalView2) {
        m_drawingManager->setPairedViews(lbVerticalView, lbVerticalView2);
    }
    if (lbLeftView && lbLeftView2) {
        m_drawingManager->setPairedViews(lbLeftView, lbLeftView2);
    }
    if (lbFrontView && lbFrontView2) {
        m_drawingManager->setPairedViews(lbFrontView, lbFrontView2);
    }
    
    qDebug() << "绘图管理器设置完成";
}

void MutiCamApp::setupMouseEventFilters()
{
    // 为所有图像显示标签安装事件过滤器
    if (lbVerticalView) {
        lbVerticalView->installEventFilter(this);
    }
    if (lbLeftView) {
        lbLeftView->installEventFilter(this);
    }
    if (lbFrontView) {
        lbFrontView->installEventFilter(this);
    }
    if (lbVerticalView2) {
        lbVerticalView2->installEventFilter(this);
    }
    if (lbLeftView2) {
        lbLeftView2->installEventFilter(this);
    }
    if (lbFrontView2) {
        lbFrontView2->installEventFilter(this);
    }
    
    qDebug() << "鼠标事件过滤器设置完成";
}

bool MutiCamApp::eventFilter(QObject* obj, QEvent* event)
{
    // 检查是否是我们感兴趣的事件
    if (event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonRelease ||
        event->type() == QEvent::MouseMove ||
        event->type() == QEvent::MouseButtonDblClick ||
        event->type() == QEvent::Wheel) {
        
        // 检查是否是图像显示标签
        QLabel* label = qobject_cast<QLabel*>(obj);
        if (label && (label == lbVerticalView || label == lbLeftView || label == lbFrontView ||
                      label == lbVerticalView2 || label == lbLeftView2 || label == lbFrontView2)) {
            
            if (event->type() == QEvent::Wheel) {
                // 处理鼠标滚轮事件进行缩放
                QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
                handleLabelWheelEvent(label, wheelEvent);
                return true;
            } else {
                QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
                handleLabelMouseEvent(label, mouseEvent);
                return true; // 事件已处理
            }
        }
    }
    
    // 调用基类的事件过滤器
    return QMainWindow::eventFilter(obj, event);
}

void MutiCamApp::handleLabelMouseEvent(QLabel* label, QMouseEvent* event)
{
    if (!m_drawingManager || !label || !event) {
        return;
    }
    
    // 获取当前视图的图像帧
    QImage currentFrame = getCurrentFrameForView(label);
    
    // 根据事件类型调用相应的处理函数（使用带Frame参数的版本）
    switch (event->type()) {
        case QEvent::MouseButtonPress:
            m_drawingManager->handleMousePressWithFrame(event, label, currentFrame);
            break;
        case QEvent::MouseMove:
            m_drawingManager->handleMouseMoveWithFrame(event, label, currentFrame);
            break;
        case QEvent::MouseButtonRelease:
            m_drawingManager->handleMouseReleaseWithFrame(event, label, currentFrame);
            break;
        case QEvent::MouseButtonDblClick:
            m_drawingManager->handleMouseDoubleClick(event, label);
            break;
        default:
            break;
    }
    
    // 在鼠标事件后重新显示带绘制内容的图像并同步到配对视图
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease) {
        if (!currentFrame.isNull()) {
            // 更新当前视图
            displayImageWithDrawings(currentFrame, label);
            
            // 同步到配对视图
            m_drawingManager->syncToAllPairedViews(label, currentFrame);
            
            // 更新配对视图的显示
            QLabel* pairedView = m_drawingManager->getPairedView(label);
            if (pairedView) {
                QImage pairedFrame = getCurrentFrameForView(pairedView);
                if (!pairedFrame.isNull()) {
                    displayImageWithDrawings(pairedFrame, pairedView);
                }
            }
        }
    }
}

void MutiCamApp::handleLabelWheelEvent(QLabel* label, QWheelEvent* event)
{
    if (!label || !event) {
        return;
    }
    
    // 获取当前缩放因子
    auto it = m_viewZoomFactors.find(label);
    double currentZoom = (it != m_viewZoomFactors.end()) ? it->second : 1.0;
    
    // 计算缩放变化
    double zoomDelta = 0.1;  // 每次滚轮滚动的缩放步长
    double newZoom = currentZoom;
    
    if (event->angleDelta().y() > 0) {
        // 向上滚动，放大
        newZoom = std::min(currentZoom + zoomDelta, 3.0);  // 最大3倍缩放
    } else {
        // 向下滚动，缩小
        newZoom = std::max(currentZoom - zoomDelta, 0.5);  // 最小0.5倍缩放
    }
    
    // 如果缩放因子有变化，更新显示
    if (std::abs(newZoom - currentZoom) > 0.01) {
        m_viewZoomFactors[label] = newZoom;
        
        // 获取当前图像
        QImage currentFrame = getCurrentFrameForView(label);
        if (!currentFrame.isNull()) {
            // 显示带绘制内容的缩放图像
            displayImageWithDrawings(currentFrame, label);
            
            // 更新状态栏显示缩放信息
            QString zoomInfo = QString("缩放: %1%").arg(static_cast<int>(newZoom * 100));
            statusBar()->showMessage(zoomInfo, 2000);
            
            qDebug() << "视图缩放:" << label->objectName() << "缩放因子:" << newZoom;
        }
    }
    
    event->accept();
}

QImage MutiCamApp::getCurrentFrameForView(QLabel* label)
{
    if (label == lbVerticalView || label == lbVerticalView2) {
        return m_currentVerticalFrame;
    } else if (label == lbLeftView || label == lbLeftView2) {
        return m_currentLeftFrame;
    } else if (label == lbFrontView || label == lbFrontView2) {
        return m_currentFrontFrame;
    }
    
    // 如果没有对应的帧，创建一个测试图像
    if (label) {
        QSize labelSize = label->size();
        QImage testFrame(labelSize.width(), labelSize.height(), QImage::Format_RGB888);
        testFrame.fill(QColor(60, 60, 60)); // 深灰色背景
        return testFrame;
    }
    
    return QImage();
}

void MutiCamApp::displayImageWithDrawings(const QImage& image, QLabel* label)
{
    if (image.isNull() || !label || !m_drawingManager) {
        return;
    }
    
    // 获取当前视图的缩放因子
    auto it = m_viewZoomFactors.find(label);
    double zoomFactor = (it != m_viewZoomFactors.end()) ? it->second : 1.0;
    
    // 获取对应的图层管理器
    LayerManager* layerManager = m_drawingManager->getLayerManager(label);
    if (layerManager) {
        // 在原始图像上绘制所有对象
        QImage frameWithDrawings = layerManager->renderFrame(image);
        
        // 显示带绘制的图像，使用缩放因子
        displayImageHighQuality(frameWithDrawings, label, zoomFactor);
    } else {
        // 如果没有图层管理器，直接显示原始图像
        displayImageHighQuality(image, label, zoomFactor);
    }
}