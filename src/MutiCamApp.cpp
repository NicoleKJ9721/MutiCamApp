#include "MutiCamApp.h"
#include <QMessageBox>
#include <QDebug>
#include <QPixmap>
#include <QApplication>
#include <QMouseEvent>
#include <QEvent>
#include <QResizeEvent>
#include <QCursor>
#include <QStackedLayout>
#include <algorithm>
#define _USE_MATH_DEFINES
#include <cmath>
#include <map>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

MutiCamApp::MutiCamApp(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui_MutiCamApp)
    , m_cameraManager(nullptr)
    , m_isMeasuring(false)
    // {{ AURA-X: Delete - 移除残留的绘图模式和活动视图成员变量. Approval: 寸止(ID:cleanup). }}
    // m_isDrawingMode和m_activeView已移除，绘图状态现在由VideoDisplayWidget管理
    , m_frameCache(MAX_CACHED_FRAMES)  // 初始化缓存，设置最大缓存数量
    , m_verticalDisplayWidget(nullptr)
    , m_leftDisplayWidget(nullptr)
    , m_frontDisplayWidget(nullptr)
    , m_verticalDisplayWidget2(nullptr)
    , m_leftDisplayWidget2(nullptr)
    , m_frontDisplayWidget2(nullptr)
    , m_lastActivePaintingOverlay(nullptr)
{
    ui->setupUi(this);
    
    // 初始化硬件加速显示控件
    initializeVideoDisplayWidgets();
    
    // 初始化相机系统
    initializeCameraSystem();
    
    // 连接信号和槽
    connectSignalsAndSlots();
    
    // 设置初始状态
    ui->btnStartMeasure->setEnabled(true);
    ui->btnStopMeasure->setEnabled(false);
    
    // 安装鼠标事件过滤器
    installMouseEventFilters();
}

MutiCamApp::~MutiCamApp()
{
    qDebug() << "MutiCamApp destructor called";
    
    // 停止测量
    if (m_isMeasuring) {
        onStopMeasureClicked();
    }
    
    // 清理相机管理器 - 确保完全释放资源
    if (m_cameraManager) {
        qDebug() << "Cleaning up camera manager...";
        
        // 停止所有相机采集
        m_cameraManager->stopAllCameras();
        
        // 断开所有相机连接
        m_cameraManager->disconnectAllCameras();
        
        // 重置相机管理器指针，触发其析构函数
        m_cameraManager.reset();
        
        qDebug() << "Camera manager cleaned up";
    }
    
    // 清理 VideoDisplayWidget 资源
    if (m_verticalDisplayWidget) {
        delete m_verticalDisplayWidget;
        m_verticalDisplayWidget = nullptr;
    }
    if (m_leftDisplayWidget) {
        delete m_leftDisplayWidget;
        m_leftDisplayWidget = nullptr;
    }
    if (m_frontDisplayWidget) {
        delete m_frontDisplayWidget;
        m_frontDisplayWidget = nullptr;
    }
    
    delete ui; 
    
    qDebug() << "MutiCamApp destructor completed";
}

void MutiCamApp::initializeCameraSystem()
{
    try {
        // 创建相机管理器
        m_cameraManager = std::make_unique<MutiCam::Camera::CameraManager>(this);
        
        // 枚举可用设备
        auto devices = MutiCam::Camera::CameraManager::enumerateDevices();
        
        if (devices.empty()) {
            qDebug() << "No cameras found";
            return;
        }
        
        // 添加找到的相机（最多添加3个相机对应3个视图）
        int cameraCount = std::min(static_cast<int>(devices.size()), 3);
        
        for (int i = 0; i < cameraCount; ++i) {
            std::string cameraId;
            switch (i) {
                case 0: cameraId = "vertical"; break;
                case 1: cameraId = "left"; break;
                case 2: cameraId = "front"; break;
            }
            
            if (m_cameraManager->addCamera(cameraId, devices[i])) {
                qDebug() << "Added camera:" << cameraId.c_str() << "Serial:" << devices[i].c_str();
            } else {
                qDebug() << "Failed to add camera:" << cameraId.c_str();
            }
        }
        
    } catch (const std::exception& e) {
        qDebug() << "Error initializing camera system:" << e.what();
        QMessageBox::warning(this, "相机初始化错误", 
                           QString("相机系统初始化失败: %1").arg(e.what()));
    }
 }

// calculateLineIntersection 方法已迁移到 VideoDisplayWidget

// extendLineToImageBounds 方法已迁移到 VideoDisplayWidget

// {{ AURA-X: Delete - drawDashedLine 方法已迁移到 VideoDisplayWidget. }}

// 计算字体大小
// calculateFontScale 方法已迁移到 VideoDisplayWidget

// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// drawSingleTwoLines方法已迁移到VideoDisplayWidget

// {{ AURA-X: Delete - calculateExtendedLine 方法已迁移到 VideoDisplayWidget. }}

// drawTextWithBackground 方法已迁移到 VideoDisplayWidget

// calculateTextBackgroundSize 方法已迁移到 VideoDisplayWidget

// {{ AURA-X: Delete - drawSingleParallel 方法已迁移到 VideoDisplayWidget. }}


// calculateFontScale 重载方法已迁移到 VideoDisplayWidget

void MutiCamApp::connectSignalsAndSlots()
{
    // 连接按钮信号
    connect(ui->btnStartMeasure, &QPushButton::clicked, 
            this, &MutiCamApp::onStartMeasureClicked);
    connect(ui->btnStopMeasure, &QPushButton::clicked, 
            this, &MutiCamApp::onStopMeasureClicked);
    
    // 连接画点按钮信号
    connect(ui->btnDrawPoint, &QPushButton::clicked, 
            this, &MutiCamApp::onDrawPointClicked);
    connect(ui->btnDrawPointVertical, &QPushButton::clicked, 
            this, &MutiCamApp::onDrawPointVerticalClicked);
    connect(ui->btnDrawPointLeft, &QPushButton::clicked, 
            this, &MutiCamApp::onDrawPointLeftClicked);
    connect(ui->btnDrawPointFront, &QPushButton::clicked, 
            this, &MutiCamApp::onDrawPointFrontClicked);
    
    // 连接画直线按钮信号
    connect(ui->btnDrawStraight, &QPushButton::clicked,
            this, &MutiCamApp::onDrawLineClicked);
    connect(ui->btnDrawStraightVertical, &QPushButton::clicked,
            this, &MutiCamApp::onDrawLineVerticalClicked);
    connect(ui->btnDrawStraightLeft, &QPushButton::clicked,
            this, &MutiCamApp::onDrawLineLeftClicked);
    connect(ui->btnDrawStraightFront, &QPushButton::clicked,
            this, &MutiCamApp::onDrawLineFrontClicked);
    
    // 连接画圆形按钮信号
    connect(ui->btnDrawSimpleCircle, &QPushButton::clicked,
            this, &MutiCamApp::onDrawSimpleCircleClicked);
    connect(ui->btnDrawSimpleCircleVertical, &QPushButton::clicked,
            this, &MutiCamApp::onDrawSimpleCircleVerticalClicked);
    connect(ui->btnDrawSimpleCircleLeft, &QPushButton::clicked,
            this, &MutiCamApp::onDrawSimpleCircleLeftClicked);
    connect(ui->btnDrawSimpleCircleFront, &QPushButton::clicked,
            this, &MutiCamApp::onDrawSimpleCircleFrontClicked);
    
    // 连接精细圆按钮信号
    connect(ui->btnDrawFineCircle, &QPushButton::clicked,
            this, &MutiCamApp::onDrawFineCircleClicked);
    connect(ui->btnDrawFineCircleVertical, &QPushButton::clicked,
            this, &MutiCamApp::onDrawFineCircleVerticalClicked);
    connect(ui->btnDrawFineCircleLeft, &QPushButton::clicked,
            this, &MutiCamApp::onDrawFineCircleLeftClicked);
    connect(ui->btnDrawFineCircleFront, &QPushButton::clicked,
            this, &MutiCamApp::onDrawFineCircleFrontClicked);
    
    // 连接平行线按钮信号
    connect(ui->btnDrawParallel, &QPushButton::clicked,
            this, &MutiCamApp::onDrawParallelClicked);
    connect(ui->btnDrawParallelVertical, &QPushButton::clicked,
            this, &MutiCamApp::onDrawParallelVerticalClicked);
    connect(ui->btnDrawParallelLeft, &QPushButton::clicked,
            this, &MutiCamApp::onDrawParallelLeftClicked);
    connect(ui->btnDrawParallelFront, &QPushButton::clicked,
            this, &MutiCamApp::onDrawParallelFrontClicked);
    
    // 连接线与线绘制按钮
    connect(ui->btnDraw2Line, &QPushButton::clicked,
            this, &MutiCamApp::onDrawTwoLinesClicked);
    connect(ui->btnDraw2LineVertical, &QPushButton::clicked,
            this, &MutiCamApp::onDrawTwoLinesClicked);
    connect(ui->btnDraw2LineLeft, &QPushButton::clicked,
            this, &MutiCamApp::onDrawTwoLinesClicked);
    connect(ui->btnDraw2LineFront, &QPushButton::clicked,
            this, &MutiCamApp::onDrawTwoLinesClicked);
    
    // 连接清空按钮信号
    connect(ui->btnClearDrawings, &QPushButton::clicked,
            this, &MutiCamApp::onClearDrawingsClicked);
    connect(ui->btnClearDrawingsVertical, &QPushButton::clicked,
            this, &MutiCamApp::onClearDrawingsVerticalClicked);
    connect(ui->btnClearDrawingsLeft, &QPushButton::clicked,
            this, &MutiCamApp::onClearDrawingsLeftClicked);
    connect(ui->btnClearDrawingsFront, &QPushButton::clicked,
            this, &MutiCamApp::onClearDrawingsFrontClicked);

    // 连接撤销按钮信号
    connect(ui->btnCan1StepDraw, &QPushButton::clicked,
            this, &MutiCamApp::onUndoDrawingClicked);
    connect(ui->btnCan1StepDrawVertical, &QPushButton::clicked,
            this, &MutiCamApp::onUndoDrawingVerticalClicked);
    connect(ui->btnCan1StepDrawLeft, &QPushButton::clicked,
            this, &MutiCamApp::onUndoDrawingLeftClicked);
    connect(ui->btnCan1StepDrawFront, &QPushButton::clicked,
            this, &MutiCamApp::onUndoDrawingFrontClicked);

    // 连接网格相关信号
    connect(ui->leGridDensity, &QLineEdit::textChanged,
            this, &MutiCamApp::onGridDensityChanged);
    connect(ui->leGridDensVertical, &QLineEdit::textChanged,
            this, &MutiCamApp::onGridDensityVerticalChanged);
    connect(ui->leGridDensLeft, &QLineEdit::textChanged,
            this, &MutiCamApp::onGridDensityLeftChanged);
    connect(ui->leGridDensFront, &QLineEdit::textChanged,
            this, &MutiCamApp::onGridDensityFrontChanged);

    connect(ui->btnCancelGrids, &QPushButton::clicked,
            this, &MutiCamApp::onCancelGridsClicked);
    connect(ui->btnCancelGridsVertical, &QPushButton::clicked,
            this, &MutiCamApp::onCancelGridsVerticalClicked);
    connect(ui->btnCancelGridsLeft, &QPushButton::clicked,
            this, &MutiCamApp::onCancelGridsLeftClicked);
    connect(ui->btnCancelGridsFront, &QPushButton::clicked,
            this, &MutiCamApp::onCancelGridsFrontClicked);

    // 连接选项卡切换信号
    connect(ui->tabWidget, &QTabWidget::currentChanged,
            this, &MutiCamApp::onTabChanged);
    
    // 连接相机管理器信号
    if (m_cameraManager) {
        connect(m_cameraManager.get(), &MutiCam::Camera::CameraManager::cameraFrameReady,
                this, &MutiCamApp::onCameraFrameReady);
        connect(m_cameraManager.get(), &MutiCam::Camera::CameraManager::cameraStateChanged,
                this, &MutiCamApp::onCameraStateChanged);
        connect(m_cameraManager.get(), &MutiCam::Camera::CameraManager::cameraError,
                this, &MutiCamApp::onCameraError);
    }
    
    // 连接PaintingOverlay的测量结果信号和绘图同步信号
    if (m_verticalPaintingOverlay) {
        connect(m_verticalPaintingOverlay, &PaintingOverlay::measurementCompleted,
                this, &MutiCamApp::onMeasurementResult);
        connect(m_verticalPaintingOverlay, &PaintingOverlay::selectionChanged,
                this, &MutiCamApp::onSelectionChanged);
        connect(m_verticalPaintingOverlay, &PaintingOverlay::drawingCompleted,
                this, &MutiCamApp::onDrawingSync);
        connect(m_verticalPaintingOverlay, &PaintingOverlay::drawingDataChanged,
                this, &MutiCamApp::onDrawingSync);
        connect(m_verticalPaintingOverlay, &PaintingOverlay::overlayActivated,
                this, &MutiCamApp::onOverlayActivated);
    }
    if (m_leftPaintingOverlay) {
        connect(m_leftPaintingOverlay, &PaintingOverlay::measurementCompleted,
                this, &MutiCamApp::onMeasurementResult);
        connect(m_leftPaintingOverlay, &PaintingOverlay::selectionChanged,
                this, &MutiCamApp::onSelectionChanged);
        connect(m_leftPaintingOverlay, &PaintingOverlay::drawingCompleted,
                this, &MutiCamApp::onDrawingSync);
        connect(m_leftPaintingOverlay, &PaintingOverlay::drawingDataChanged,
                this, &MutiCamApp::onDrawingSync);
        connect(m_leftPaintingOverlay, &PaintingOverlay::overlayActivated,
                this, &MutiCamApp::onOverlayActivated);
    }
    if (m_frontPaintingOverlay) {
        connect(m_frontPaintingOverlay, &PaintingOverlay::measurementCompleted,
                this, &MutiCamApp::onMeasurementResult);
        connect(m_frontPaintingOverlay, &PaintingOverlay::selectionChanged,
                this, &MutiCamApp::onSelectionChanged);
        connect(m_frontPaintingOverlay, &PaintingOverlay::drawingCompleted,
                this, &MutiCamApp::onDrawingSync);
        connect(m_frontPaintingOverlay, &PaintingOverlay::drawingDataChanged,
                this, &MutiCamApp::onDrawingSync);
        connect(m_frontPaintingOverlay, &PaintingOverlay::overlayActivated,
                this, &MutiCamApp::onOverlayActivated);
    }
    if (m_verticalPaintingOverlay2) {
        connect(m_verticalPaintingOverlay2, &PaintingOverlay::measurementCompleted,
                this, &MutiCamApp::onMeasurementResult);
        connect(m_verticalPaintingOverlay2, &PaintingOverlay::selectionChanged,
                this, &MutiCamApp::onSelectionChanged);
        connect(m_verticalPaintingOverlay2, &PaintingOverlay::drawingCompleted,
                this, &MutiCamApp::onDrawingSync);
        connect(m_verticalPaintingOverlay2, &PaintingOverlay::drawingDataChanged,
                this, &MutiCamApp::onDrawingSync);
        connect(m_verticalPaintingOverlay2, &PaintingOverlay::overlayActivated,
                this, &MutiCamApp::onOverlayActivated);
    }
    if (m_leftPaintingOverlay2) {
        connect(m_leftPaintingOverlay2, &PaintingOverlay::measurementCompleted,
                this, &MutiCamApp::onMeasurementResult);
        connect(m_leftPaintingOverlay2, &PaintingOverlay::selectionChanged,
                this, &MutiCamApp::onSelectionChanged);
        connect(m_leftPaintingOverlay2, &PaintingOverlay::drawingCompleted,
                this, &MutiCamApp::onDrawingSync);
        connect(m_leftPaintingOverlay2, &PaintingOverlay::drawingDataChanged,
                this, &MutiCamApp::onDrawingSync);
        connect(m_leftPaintingOverlay2, &PaintingOverlay::overlayActivated,
                this, &MutiCamApp::onOverlayActivated);
    }
    if (m_frontPaintingOverlay2) {
        connect(m_frontPaintingOverlay2, &PaintingOverlay::measurementCompleted,
                this, &MutiCamApp::onMeasurementResult);
        connect(m_frontPaintingOverlay2, &PaintingOverlay::selectionChanged,
                this, &MutiCamApp::onSelectionChanged);
        connect(m_frontPaintingOverlay2, &PaintingOverlay::drawingCompleted,
                this, &MutiCamApp::onDrawingSync);
        connect(m_frontPaintingOverlay2, &PaintingOverlay::drawingDataChanged,
                this, &MutiCamApp::onDrawingSync);
        connect(m_frontPaintingOverlay2, &PaintingOverlay::overlayActivated,
                this, &MutiCamApp::onOverlayActivated);
    }
}

void MutiCamApp::onStartMeasureClicked()
{
    if (!m_cameraManager) {
        QMessageBox::warning(this, "错误", "相机管理器未初始化");
        return;
    }
    
    try {
        qDebug() << "Starting measurement...";
        
        // 启动所有相机
        if (m_cameraManager->startAllCameras()) {
            m_isMeasuring = true;
            
            // 更新按钮状态
            ui->btnStartMeasure->setEnabled(false);
            ui->btnStopMeasure->setEnabled(true);
            
            // 更新状态栏或显示信息
            statusBar()->showMessage("测量已开始 - 图像采集中...");
            
            qDebug() << "Measurement started successfully";
        } else {
            QMessageBox::warning(this, "启动失败", "无法启动相机，请检查相机连接");
        }
        
    } catch (const std::exception& e) {
        qDebug() << "Error starting measurement:" << e.what();
        QMessageBox::critical(this, "启动错误", 
                            QString("启动测量时发生错误: %1").arg(e.what()));
    }
}

void MutiCamApp::onStopMeasureClicked()
{
    if (!m_cameraManager) {
        return;
    }
    
    try {
        qDebug() << "Stopping measurement...";
        
        // 停止所有相机
        m_cameraManager->stopAllCameras();
        m_isMeasuring = false;
        
        // 更新按钮状态
        ui->btnStartMeasure->setEnabled(true);
        ui->btnStopMeasure->setEnabled(false);
        
        // 更新状态栏
        statusBar()->showMessage("测量已停止");
        
        qDebug() << "Measurement stopped successfully";
        
    } catch (const std::exception& e) {
        qDebug() << "Error stopping measurement:" << e.what();
    }
}

void MutiCamApp::onCameraFrameReady(const QString& cameraId, const cv::Mat& frame)
{
    if (frame.empty()) return;

    VideoDisplayWidget* mainWidget = getVideoDisplayWidget(cameraId);
    VideoDisplayWidget* tabWidget = nullptr;

    if (cameraId == "vertical") tabWidget = m_verticalDisplayWidget2;
    else if (cameraId == "left") tabWidget = m_leftDisplayWidget2;
    else if (cameraId == "front") tabWidget = m_frontDisplayWidget2;

    // 只有当主界面Tab可见时才更新主视图，以节省性能
    if (mainWidget && ui->tabWidget->currentIndex() == 0) {
        mainWidget->setVideoFrame(matToQPixmap(frame));
        // 同步主界面视图的坐标变换
        syncOverlayTransforms(cameraId);
    }
    
    // 只有当对应的Tab页可见时才更新，以节省性能
    if (tabWidget && tabWidget->isVisible()) {
        tabWidget->setVideoFrame(matToQPixmap(frame));
        // 同步选项卡视图的坐标变换
        syncOverlayTransforms(cameraId + "2");
    }
}

void MutiCamApp::onCameraStateChanged(const QString& cameraId, MutiCam::Camera::CameraState state)
{
    qDebug() << "Camera" << cameraId << "state changed to:" << static_cast<int>(state);
    
    // 可以在这里添加状态显示逻辑
    QString stateText;
    switch (state) {
        case MutiCam::Camera::CameraState::Disconnected:
            stateText = "断开连接";
            break;
        case MutiCam::Camera::CameraState::Connected:
            stateText = "已连接";
            break;
        case MutiCam::Camera::CameraState::Streaming:
            stateText = "采集中";
            break;
    }
    
    // 更新状态栏显示
    statusBar()->showMessage(QString("相机 %1: %2").arg(cameraId, stateText), 2000);
}

void MutiCamApp::onCameraError(const QString& cameraId, const QString& error)
{
    qDebug() << "Camera error for" << cameraId << ":" << error;
    
    // 显示错误信息
    QMessageBox::warning(this, "相机错误", 
                        QString("相机 %1 发生错误: %2").arg(cameraId, error));
}

void MutiCamApp::onMeasurementResult(const QString& viewName, const QString& result)
{
    // 在这里处理测量结果，例如：
    // 1. 显示到UI的某个文本框中
    // 2. 记录到日志文件
    qDebug() << "测量结果来自 [" << viewName << "]:" << result;
    statusBar()->showMessage(QString("新测量结果 [%1]: %2").arg(viewName, result), 5000);
}

void MutiCamApp::onSelectionChanged(const QString& info)
{
    // 在状态栏显示选择的图元信息
    if (info.isEmpty()) {
        statusBar()->showMessage("就绪");
    } else {
        statusBar()->showMessage(QString("选中了: %1").arg(info));
    }
}



QPixmap MutiCamApp::matToQPixmap(const cv::Mat& mat, bool setDevicePixelRatio)
{
    if (mat.empty()) {
        return QPixmap();
    }

    try {
        QImage qimg;

        // 性能优化：减少内存拷贝的图像转换
        // 优化策略：1)预分配缓冲区 2)避免不必要的copy() 3)直接数据指针访问
        if (mat.channels() == 1) {
            // 灰度图像：检查是否需要拷贝
            if (mat.isContinuous()) {
                // 连续内存：可以安全地直接使用数据指针
                qimg = QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
                // 立即转换为QPixmap以确保数据安全
            } else {
                // 非连续内存：需要拷贝
                qimg = QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8).copy();
            }
        } else if (mat.channels() == 3) {
            // BGR图像：使用预分配缓冲区优化转换
            // 检查并调整转换缓冲区大小
            if (m_rgbConvertBuffer.rows != mat.rows ||
                m_rgbConvertBuffer.cols != mat.cols ||
                m_rgbConvertBuffer.type() != CV_8UC3) {
                m_rgbConvertBuffer.create(mat.rows, mat.cols, CV_8UC3);
            }

            // 使用预分配缓冲区进行颜色转换，避免内存分配
            cv::cvtColor(mat, m_rgbConvertBuffer, cv::COLOR_BGR2RGB);

            // 创建QImage，直接使用缓冲区数据（缓冲区在对象生命周期内有效）
            qimg = QImage(m_rgbConvertBuffer.data, m_rgbConvertBuffer.cols, m_rgbConvertBuffer.rows,
                         m_rgbConvertBuffer.step, QImage::Format_RGB888);
        } else if (mat.channels() == 4) {
            // RGBA图像：优化处理
            if (mat.isContinuous()) {
                // 连续内存：直接使用
                qimg = QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGBA8888);
            } else {
                // 非连续内存：需要拷贝
                qimg = QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGBA8888).copy();
            }
        } else {
            // 不支持的格式：转换为灰度
            cv::Mat grayMat;
            if (mat.channels() > 4) {
                // 多通道图像，取第一个通道
                cv::extractChannel(mat, grayMat, 0);
            } else {
                cv::cvtColor(mat, grayMat, cv::COLOR_BGR2GRAY);
            }
            qimg = QImage(grayMat.data, grayMat.cols, grayMat.rows,
                         grayMat.step, QImage::Format_Grayscale8).copy();
        }

        // 立即转换为QPixmap以确保数据安全和性能
        // QPixmap::fromImage会进行必要的数据拷贝，确保数据独立性
        QPixmap pixmap = QPixmap::fromImage(qimg);

        // 根据参数决定是否设置设备像素比
        if (setDevicePixelRatio) {
            qreal devicePixelRatio = qApp->devicePixelRatio();
            pixmap.setDevicePixelRatio(devicePixelRatio);
        }

        return pixmap;
        
    } catch (const std::exception& e) {
        qDebug() << "Error converting Mat to QPixmap:" << e.what();
        return QPixmap();
    }
}

// 画点功能实现
void MutiCamApp::onDrawPointClicked()
{
    // 命令所有视图进入"画点"模式
    if (m_verticalPaintingOverlay) m_verticalPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::Point);
    if (m_leftPaintingOverlay) m_leftPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::Point);
    if (m_frontPaintingOverlay) m_frontPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::Point);
    if (m_verticalPaintingOverlay2) m_verticalPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::Point);
    if (m_leftPaintingOverlay2) m_leftPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::Point);
    if (m_frontPaintingOverlay2) m_frontPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::Point);
}

void MutiCamApp::onDrawPointVerticalClicked()
{
    // 只命令垂直视图进入"画点"模式
    if (m_verticalPaintingOverlay) m_verticalPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::Point);
    if (m_verticalPaintingOverlay2) m_verticalPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::Point);
}

void MutiCamApp::onDrawPointLeftClicked()
{
    // 只命令左视图进入"画点"模式
    if (m_leftPaintingOverlay) m_leftPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::Point);
    if (m_leftPaintingOverlay2) m_leftPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::Point);
}

void MutiCamApp::onDrawPointFrontClicked()
{
    // 只命令前视图进入"画点"模式
    if (m_frontPaintingOverlay) m_frontPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::Point);
    if (m_frontPaintingOverlay2) m_frontPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::Point);
}

// setDrawingMode和exitDrawingMode方法已移除 - 现在直接通过按钮槽函数调用VideoDisplayWidget的startDrawing方法

// drawPointsOnImage 方法已迁移到 VideoDisplayWidget

void MutiCamApp::installMouseEventFilters()
{
    // 为主界面VideoDisplayWidget安装事件过滤器并启用鼠标追踪
    if (m_verticalDisplayWidget) {
        m_verticalDisplayWidget->installEventFilter(this);
        m_verticalDisplayWidget->setMouseTracking(true);
    }
    if (m_leftDisplayWidget) {
        m_leftDisplayWidget->installEventFilter(this);
        m_leftDisplayWidget->setMouseTracking(true);
    }
    if (m_frontDisplayWidget) {
        m_frontDisplayWidget->installEventFilter(this);
        m_frontDisplayWidget->setMouseTracking(true);
    }
    
    // 为选项卡VideoDisplayWidget安装事件过滤器并启用鼠标追踪
    if (m_verticalDisplayWidget2) {
        m_verticalDisplayWidget2->installEventFilter(this);
        m_verticalDisplayWidget2->setMouseTracking(true);
    }
    if (m_leftDisplayWidget2) {
        m_leftDisplayWidget2->installEventFilter(this);
        m_leftDisplayWidget2->setMouseTracking(true);
    }
    if (m_frontDisplayWidget2) {
        m_frontDisplayWidget2->installEventFilter(this);
        m_frontDisplayWidget2->setMouseTracking(true);
    }
}

bool MutiCamApp::eventFilter(QObject* obj, QEvent* event)
{
    // {{ AURA-X: Delete - 事件处理已迁移到VideoDisplayWidget. Approval: 寸止(ID:cleanup). }}
    // 所有绘图相关的事件处理已完全迁移到VideoDisplayWidget
    // 这里只保留基本的事件过滤功能
    
    return QMainWindow::eventFilter(obj, event);
}

// {{ AURA-X: Delete - 残留的标签点击处理方法. Approval: 寸止(ID:cleanup). }}
// handleLabelClick方法已删除，现在直接使用VideoDisplayWidget

QString MutiCamApp::getViewName(QLabel* label)
{
    if (label == ui->lbVerticalView) {
        return "vertical";
    } else if (label == ui->lbLeftView) {
        return "left";
    } else if (label == ui->lbFrontView) {
        return "front";
    }
    return "";
}

QString MutiCamApp::getViewName(VideoDisplayWidget* widget)
{
    if (widget == m_verticalDisplayWidget || widget == m_verticalDisplayWidget2) {
        return "vertical";
    } else if (widget == m_leftDisplayWidget || widget == m_leftDisplayWidget2) {
        return "left";
    } else if (widget == m_frontDisplayWidget || widget == m_frontDisplayWidget2) {
        return "front";
    }
    return "";
}

// handleVideoWidgetClick方法已迁移到VideoDisplayWidget中

// handleVideoWidgetMouseMove方法已迁移到VideoDisplayWidget中

QPointF MutiCamApp::windowToImageCoordinates(VideoDisplayWidget* widget, const QPoint& windowPos)
{
    // 坐标转换方法已移到PaintingOverlay中，这里使用简化的转换逻辑
    QPointF offset = widget->getImageOffset();
    double scale = widget->getScaleFactor();
    
    // 转换为图像坐标
    double imageX = (windowPos.x() - offset.x()) / scale;
    double imageY = (windowPos.y() - offset.y()) / scale;
    
    return QPointF(imageX, imageY);
}

// {{ AURA-X: Delete - 残留的鼠标移动处理方法. Approval: 寸止(ID:cleanup). }}
// handleMouseMove方法已删除，现在直接使用VideoDisplayWidget

void MutiCamApp::updateViewDisplay(const QString& viewName)
{
    // 获取对应的标签和帧
    QLabel* mainLabel = nullptr;
    VideoDisplayWidget* tabWidget = nullptr;
    cv::Mat* currentFrame = nullptr;
    
    if (viewName == "vertical") {
        mainLabel = ui->lbVerticalView;
        tabWidget = m_verticalDisplayWidget2;
        currentFrame = &m_currentFrameVertical;
    } else if (viewName == "left") {
        mainLabel = ui->lbLeftView;
        tabWidget = m_leftDisplayWidget2;
        currentFrame = &m_currentFrameLeft;
    } else if (viewName == "front") {
        mainLabel = ui->lbFrontView;
        tabWidget = m_frontDisplayWidget2;
        currentFrame = &m_currentFrameFront;
    }
    
    if (!mainLabel || !tabWidget || !currentFrame || currentFrame->empty()) {
        qDebug() << "无法更新" << viewName << "视图显示 - 缺少必要数据";
        return;
    }
    
    // {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
    // displayImageWithHardwareAcceleration方法已迁移到VideoDisplayWidget
    // 使用VideoDisplayWidget更新主界面视图
    updateVideoDisplayWidget(viewName, *currentFrame);
    
    // 强制更新选项卡视图，确保绘画数据持久显示
    // {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
    // renderDrawingsOnFrame方法已迁移到VideoDisplayWidget
    cv::Mat renderedFrame = *currentFrame; // 直接使用原始帧
    
    // 将cv::Mat转换为QPixmap并设置到VideoDisplayWidget
    QImage qimg;
    if (renderedFrame.channels() == 3) {
        qimg = QImage(renderedFrame.data, renderedFrame.cols, renderedFrame.rows, renderedFrame.step, QImage::Format_RGB888).rgbSwapped();
    } else if (renderedFrame.channels() == 1) {
        qimg = QImage(renderedFrame.data, renderedFrame.cols, renderedFrame.rows, renderedFrame.step, QImage::Format_Grayscale8);
    }
    
    if (!qimg.isNull()) {
        tabWidget->setVideoFrame(QPixmap::fromImage(qimg));
    }
    
    qDebug() << "已更新" << viewName << "视图显示";
}

QPointF MutiCamApp::windowToImageCoordinates(QLabel* label, const QPoint& windowPos)
{
    // 获取对应的原始图像尺寸
    cv::Mat* currentFrame = nullptr;
    QString viewName = getViewName(label);
    
    if (viewName == "vertical") {
        currentFrame = &m_currentFrameVertical;
    } else if (viewName == "left") {
        currentFrame = &m_currentFrameLeft;
    } else if (viewName == "front") {
        currentFrame = &m_currentFrameFront;
    }
    
    if (!currentFrame || currentFrame->empty()) {
        return QPointF(windowPos);
    }
    
    // 获取原始图像尺寸
    int imageWidth = currentFrame->cols;
    int imageHeight = currentFrame->rows;
    
    // 获取标签尺寸
    QSize labelSize = label->size();
    if (labelSize.width() <= 0 || labelSize.height() <= 0) {
        return QPointF(windowPos);
    }
    
    // 计算缩放比例，保持宽高比
    double scaleX = static_cast<double>(labelSize.width()) / imageWidth;
    double scaleY = static_cast<double>(labelSize.height()) / imageHeight;
    double scale = std::min(scaleX, scaleY);
    
    // 计算缩放后的图像尺寸
    int scaledWidth = static_cast<int>(imageWidth * scale);
    int scaledHeight = static_cast<int>(imageHeight * scale);
    
    // 计算图像在标签中的偏移（居中显示）
    int offsetX = (labelSize.width() - scaledWidth) / 2;
    int offsetY = (labelSize.height() - scaledHeight) / 2;
    
    // 检查点击是否在图像区域内
    if (windowPos.x() < offsetX || windowPos.x() >= offsetX + scaledWidth ||
        windowPos.y() < offsetY || windowPos.y() >= offsetY + scaledHeight) {
        return QPointF(-1, -1); // 无效坐标
    }
    
    // 转换为图像坐标
    double imageX = (windowPos.x() - offsetX) / scale;
    double imageY = (windowPos.y() - offsetY) / scale;
    
    return QPointF(imageX, imageY);
}

// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// shouldUpdate方法已迁移到VideoDisplayWidget

size_t MutiCamApp::calculatePointsHash(const QString& viewName)
{
    // {{ AURA-X: Delete - 绘图数据已迁移到VideoDisplayWidget. Approval: 寸止(ID:cleanup). }}
    // 绘图数据哈希计算已迁移到VideoDisplayWidget，这里返回固定值
    Q_UNUSED(viewName);
    return 0;
}

// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// getCachedFrame方法已迁移到VideoDisplayWidget

// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// cacheFrame方法已迁移到VideoDisplayWidget

void MutiCamApp::invalidateCache(const QString& viewName)
{
    if (viewName.isEmpty()) {
        // 清空所有缓存
        m_frameCache.clear();
        m_renderBuffers.clear();  // 同时清理渲染缓冲区
        qDebug() << "已清空所有帧缓存和渲染缓冲区";
    } else {
        // 清空指定视图的缓存
        m_frameCache.remove(viewName);
        m_renderBuffers.remove(viewName);  // 清理对应的渲染缓冲区
        qDebug() << "已清空" << viewName << "视图的帧缓存和渲染缓冲区";
    }
}

// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// hasDrawingData方法已迁移到VideoDisplayWidget

// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// displayFrameWithHighDPI方法已迁移到VideoDisplayWidget

// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// displayImageOnLabel方法已迁移到VideoDisplayWidget

// 直线绘制槽函数实现
void MutiCamApp::onDrawLineClicked()
{
    // 命令所有视图进入"画线"模式
    if (m_verticalPaintingOverlay) m_verticalPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::Line);
    if (m_leftPaintingOverlay) m_leftPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::Line);
    if (m_frontPaintingOverlay) m_frontPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::Line);
    if (m_verticalPaintingOverlay2) m_verticalPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::Line);
    if (m_leftPaintingOverlay2) m_leftPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::Line);
    if (m_frontPaintingOverlay2) m_frontPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::Line);
}

void MutiCamApp::onDrawLineVerticalClicked()
{
    // 只命令垂直视图进入"画线"模式
    if (m_verticalPaintingOverlay) m_verticalPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::Line);
    if (m_verticalPaintingOverlay2) m_verticalPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::Line);
}

void MutiCamApp::onDrawLineLeftClicked()
{
    // 只命令左视图进入"画线"模式
    if (m_leftPaintingOverlay) m_leftPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::Line);
    if (m_leftPaintingOverlay2) m_leftPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::Line);
}

void MutiCamApp::onDrawLineFrontClicked()
{
    // 只命令前视图进入"画线"模式
    if (m_frontPaintingOverlay) m_frontPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::Line);
    if (m_frontPaintingOverlay2) m_frontPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::Line);
}

// 直线绘制相关方法实现
// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// handleLineDrawingClick方法已迁移到VideoDisplayWidget::handleLineDrawingClick

// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// drawLinesOnImage方法已迁移到VideoDisplayWidget

// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// drawSingleLine、drawDashedLine、calculateLineAngle方法已迁移到VideoDisplayWidget

// 圆形绘制槽函数实现
void MutiCamApp::onDrawSimpleCircleClicked()
{
    // 命令所有视图进入"画圆"模式
    if (m_verticalPaintingOverlay) m_verticalPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::Circle);
    if (m_leftPaintingOverlay) m_leftPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::Circle);
    if (m_frontPaintingOverlay) m_frontPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::Circle);
    if (m_verticalPaintingOverlay2) m_verticalPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::Circle);
    if (m_leftPaintingOverlay2) m_leftPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::Circle);
    if (m_frontPaintingOverlay2) m_frontPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::Circle);
}

void MutiCamApp::onDrawSimpleCircleVerticalClicked()
{
    // 只命令垂直视图进入"画圆"模式
    if (m_verticalPaintingOverlay) m_verticalPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::Circle);
    if (m_verticalPaintingOverlay2) m_verticalPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::Circle);
}

void MutiCamApp::onDrawSimpleCircleLeftClicked()
{
    // 只命令左视图进入"画圆"模式
    if (m_leftPaintingOverlay) m_leftPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::Circle);
    if (m_leftPaintingOverlay2) m_leftPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::Circle);
}

void MutiCamApp::onDrawSimpleCircleFrontClicked()
{
    // 只命令前视图进入"画圆"模式
    if (m_frontPaintingOverlay) m_frontPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::Circle);
    if (m_frontPaintingOverlay2) m_frontPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::Circle);
}

void MutiCamApp::onDrawFineCircleClicked()
{
    // 命令所有视图进入"精细圆"模式
    if (m_verticalPaintingOverlay) m_verticalPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::FineCircle);
    if (m_leftPaintingOverlay) m_leftPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::FineCircle);
    if (m_frontPaintingOverlay) m_frontPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::FineCircle);
    if (m_verticalPaintingOverlay2) m_verticalPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::FineCircle);
    if (m_leftPaintingOverlay2) m_leftPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::FineCircle);
    if (m_frontPaintingOverlay2) m_frontPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::FineCircle);
}

void MutiCamApp::onDrawFineCircleVerticalClicked()
{
    // 只命令垂直视图进入"精细圆"模式
    if (m_verticalPaintingOverlay) m_verticalPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::FineCircle);
    if (m_verticalPaintingOverlay2) m_verticalPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::FineCircle);
}

void MutiCamApp::onDrawFineCircleLeftClicked()
{
    // 只命令左视图进入"精细圆"模式
    if (m_leftPaintingOverlay) m_leftPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::FineCircle);
    if (m_leftPaintingOverlay2) m_leftPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::FineCircle);
}

void MutiCamApp::onDrawFineCircleFrontClicked()
{
    // 只命令前视图进入"精细圆"模式
    if (m_frontPaintingOverlay) m_frontPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::FineCircle);
    if (m_frontPaintingOverlay2) m_frontPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::FineCircle);
}

// 平行线绘制按钮点击事件处理
void MutiCamApp::onDrawParallelClicked()
{
    // 命令所有视图进入"平行线"模式
    if (m_verticalPaintingOverlay) m_verticalPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::Parallel);
    if (m_leftPaintingOverlay) m_leftPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::Parallel);
    if (m_frontPaintingOverlay) m_frontPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::Parallel);
    if (m_verticalPaintingOverlay2) m_verticalPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::Parallel);
    if (m_leftPaintingOverlay2) m_leftPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::Parallel);
    if (m_frontPaintingOverlay2) m_frontPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::Parallel);
}

void MutiCamApp::onDrawParallelVerticalClicked()
{
    // 只命令垂直视图进入"平行线"模式
    if (m_verticalPaintingOverlay) m_verticalPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::Parallel);
    if (m_verticalPaintingOverlay2) m_verticalPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::Parallel);
}

void MutiCamApp::onDrawParallelLeftClicked()
{
    // 只命令左视图进入"平行线"模式
    if (m_leftPaintingOverlay) m_leftPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::Parallel);
    if (m_leftPaintingOverlay2) m_leftPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::Parallel);
}

void MutiCamApp::onDrawParallelFrontClicked()
{
    // 只命令前视图进入"平行线"模式
    if (m_frontPaintingOverlay) m_frontPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::Parallel);
    if (m_frontPaintingOverlay2) m_frontPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::Parallel);
}

// 线与线绘制按钮点击事件处理
void MutiCamApp::onDrawTwoLinesClicked()
{
    // 命令所有视图进入"两线测量"模式
    if (m_verticalPaintingOverlay) m_verticalPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::TwoLines);
    if (m_leftPaintingOverlay) m_leftPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::TwoLines);
    if (m_frontPaintingOverlay) m_frontPaintingOverlay->startDrawing(PaintingOverlay::DrawingTool::TwoLines);
    if (m_verticalPaintingOverlay2) m_verticalPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::TwoLines);
    if (m_leftPaintingOverlay2) m_leftPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::TwoLines);
    if (m_frontPaintingOverlay2) m_frontPaintingOverlay2->startDrawing(PaintingOverlay::DrawingTool::TwoLines);
}

// 线与线绘制相关方法实现
// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// handleTwoLinesDrawingClick方法已迁移到VideoDisplayWidget::handleTwoLinesDrawingClick

// 平行线绘制相关方法实现
// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// handleParallelDrawingClick方法已迁移到VideoDisplayWidget::handleParallelDrawingClick

// 圆形绘制相关方法实现
// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// handleCircleDrawingClick方法已迁移到VideoDisplayWidget::handleCircleDrawingClick

// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// drawCirclesOnImage、drawSingleCircle方法已迁移到VideoDisplayWidget

// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// calculateCircleFromThreePoints方法已迁移到VideoDisplayWidget

// 精细圆绘制相关方法实现
// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// handleFineCircleDrawingClick方法已迁移到VideoDisplayWidget::handleFineCircleDrawingClick

// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// drawFineCirclesOnImage、drawSingleFineCircle方法已迁移到VideoDisplayWidget

// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// calculateCircleFromFivePoints方法已迁移到VideoDisplayWidget

void MutiCamApp::onTabChanged(int index)
{
    // 根据选项卡索引更新对应的视图
    // 0: 主界面, 1: 垂直视图, 2: 左视图, 3: 前视图, 4: 设置

    if (index == 0) { // 主界面
        // 切换回主界面时，更新所有主视图以显示最新帧
        if (!m_currentFrameVertical.empty()) {
            updateVideoDisplayWidget("vertical", m_currentFrameVertical);
        }
        if (!m_currentFrameLeft.empty()) {
            updateVideoDisplayWidget("left", m_currentFrameLeft);
        }
        if (!m_currentFrameFront.empty()) {
            updateVideoDisplayWidget("front", m_currentFrameFront);
        }
    } else if (index == 1) { // 垂直视图选项卡
        if (!m_currentFrameVertical.empty()) {
            // 使用VideoDisplayWidget显示，自动包含绘画数据
            updateVideoDisplayWidget("vertical2", m_currentFrameVertical);
        }
    } else if (index == 2) { // 左视图选项卡
        if (!m_currentFrameLeft.empty()) {
            // 使用VideoDisplayWidget显示，自动包含绘画数据
            updateVideoDisplayWidget("left2", m_currentFrameLeft);
        }
    } else if (index == 3) { // 前视图选项卡
        if (!m_currentFrameFront.empty()) {
            // 使用VideoDisplayWidget显示，自动包含绘画数据
            updateVideoDisplayWidget("front2", m_currentFrameFront);
        }
    }

    qDebug() << "Tab changed to index:" << index;
}

// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// renderDrawingsOnFrame方法已迁移到VideoDisplayWidget

// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// drawParallelLinesOnImage方法已迁移到VideoDisplayWidget

// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// drawTwoLinesOnImage方法已迁移到VideoDisplayWidget

// VideoDisplayWidget 相关函数实现
void MutiCamApp::initializeVideoDisplayWidgets()
{
    // 创建硬件加速显示控件
    m_verticalDisplayWidget = new VideoDisplayWidget(this);
    m_leftDisplayWidget = new VideoDisplayWidget(this);
    m_frontDisplayWidget = new VideoDisplayWidget(this);
    
    // 创建选项卡VideoDisplayWidget
    m_verticalDisplayWidget2 = new VideoDisplayWidget(this);
    m_leftDisplayWidget2 = new VideoDisplayWidget(this);
    m_frontDisplayWidget2 = new VideoDisplayWidget(this);
    
    // 创建PaintingOverlay控件
    m_verticalPaintingOverlay = new PaintingOverlay(this);
    m_leftPaintingOverlay = new PaintingOverlay(this);
    m_frontPaintingOverlay = new PaintingOverlay(this);
    
    // 创建选项卡PaintingOverlay
    m_verticalPaintingOverlay2 = new PaintingOverlay(this);
    m_leftPaintingOverlay2 = new PaintingOverlay(this);
    m_frontPaintingOverlay2 = new PaintingOverlay(this);
    
    // 设置PaintingOverlay的视图名称（用于measurementCompleted信号）
    m_verticalPaintingOverlay->setViewName("Vertical");
    m_leftPaintingOverlay->setViewName("Left");
    m_frontPaintingOverlay->setViewName("Front");
    
    m_verticalPaintingOverlay2->setViewName("Vertical2");
    m_leftPaintingOverlay2->setViewName("Left2");
    m_frontPaintingOverlay2->setViewName("Front2");
    
    // 创建叠加容器并使用QStackedLayout
    QWidget* verticalContainer = new QWidget(this);
    QWidget* leftContainer = new QWidget(this);
    QWidget* frontContainer = new QWidget(this);
    
    QWidget* verticalContainer2 = new QWidget(this);
    QWidget* leftContainer2 = new QWidget(this);
    QWidget* frontContainer2 = new QWidget(this);
    
    // 【关键修复】: 为主界面视图创建 QGridLayout 以实现重叠
    QGridLayout* verticalGrid = new QGridLayout(verticalContainer);
    QGridLayout* leftGrid = new QGridLayout(leftContainer);
    QGridLayout* frontGrid = new QGridLayout(frontContainer);
    
    // 【关键修复】: 为选项卡视图创建 QGridLayout
    QGridLayout* verticalGrid2 = new QGridLayout(verticalContainer2);
    QGridLayout* leftGrid2 = new QGridLayout(leftContainer2);
    QGridLayout* frontGrid2 = new QGridLayout(frontContainer2);
    
    // 【关键修复】: 将两个控件添加到同一个网格单元(0,0)以实现重叠
    verticalGrid->setContentsMargins(0, 0, 0, 0);
    verticalGrid->addWidget(m_verticalDisplayWidget, 0, 0);
    verticalGrid->addWidget(m_verticalPaintingOverlay, 0, 0);
    
    leftGrid->setContentsMargins(0, 0, 0, 0);
    leftGrid->addWidget(m_leftDisplayWidget, 0, 0);
    leftGrid->addWidget(m_leftPaintingOverlay, 0, 0);
    
    frontGrid->setContentsMargins(0, 0, 0, 0);
    frontGrid->addWidget(m_frontDisplayWidget, 0, 0);
    frontGrid->addWidget(m_frontPaintingOverlay, 0, 0);
    
    // 将选项卡VideoDisplayWidget和PaintingOverlay添加到同一个网格单元
    verticalGrid2->setContentsMargins(0, 0, 0, 0);
    verticalGrid2->addWidget(m_verticalDisplayWidget2, 0, 0);
    verticalGrid2->addWidget(m_verticalPaintingOverlay2, 0, 0);
    
    leftGrid2->setContentsMargins(0, 0, 0, 0);
    leftGrid2->addWidget(m_leftDisplayWidget2, 0, 0);
    leftGrid2->addWidget(m_leftPaintingOverlay2, 0, 0);
    
    frontGrid2->setContentsMargins(0, 0, 0, 0);
    frontGrid2->addWidget(m_frontDisplayWidget2, 0, 0);
    frontGrid2->addWidget(m_frontPaintingOverlay2, 0, 0);
    

    
    // 设置PaintingOverlay为透明背景，使其叠加在VideoDisplayWidget之上
    m_verticalPaintingOverlay->setAttribute(Qt::WA_TranslucentBackground);
    m_leftPaintingOverlay->setAttribute(Qt::WA_TranslucentBackground);
    m_frontPaintingOverlay->setAttribute(Qt::WA_TranslucentBackground);
    
    m_verticalPaintingOverlay2->setAttribute(Qt::WA_TranslucentBackground);
    m_leftPaintingOverlay2->setAttribute(Qt::WA_TranslucentBackground);
    m_frontPaintingOverlay2->setAttribute(Qt::WA_TranslucentBackground);
    
    // {{ AURA-X: Modify - 使用布局管理器API正确替换控件. Approval: 寸止(ID:fix_layout_replacement). }}
    // 使用 QLayout::replaceWidget 正确替换控件
    
    // 获取各个 QLabel 所在的布局
    QGridLayout* verticalLayout = ui->gridLayout_vertical;
    QGridLayout* leftLayout = ui->gridLayout_left;
    QGridLayout* frontLayout = ui->gridLayout_front;
    
    // 获取选项卡 QLabel 所在的布局
    QVBoxLayout* verticalLayout2 = ui->verticalLayout_verticalDisplay;
    QVBoxLayout* leftLayout2 = ui->verticalLayout_leftDisplay;
    QVBoxLayout* frontLayout2 = ui->verticalLayout_frontDisplay;
    
    if (verticalLayout && leftLayout && frontLayout && verticalLayout2 && leftLayout2 && frontLayout2) {
        qDebug() << "开始使用布局管理器替换控件";
        
        // 设置硬件加速控件的属性
        m_verticalDisplayWidget->setScaledContents(true);
        m_leftDisplayWidget->setScaledContents(true);
        m_frontDisplayWidget->setScaledContents(true);
        
        // 设置选项卡VideoDisplayWidget的属性
        m_verticalDisplayWidget2->setScaledContents(true);
        m_leftDisplayWidget2->setScaledContents(true);
        m_frontDisplayWidget2->setScaledContents(true);
        
        // 设置最小尺寸策略
        m_verticalDisplayWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        m_leftDisplayWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        m_frontDisplayWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        
        // 设置选项卡VideoDisplayWidget的尺寸策略
        m_verticalDisplayWidget2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        m_leftDisplayWidget2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        m_frontDisplayWidget2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        
        // 使用 replaceWidget 替换控件为叠加容器，这会自动处理布局管理
        QLayoutItem* verticalItem = verticalLayout->replaceWidget(ui->lbVerticalView, verticalContainer);
        QLayoutItem* leftItem = leftLayout->replaceWidget(ui->lbLeftView, leftContainer);
        QLayoutItem* frontItem = frontLayout->replaceWidget(ui->lbFrontView, frontContainer);
        
        // 替换选项卡QLabel为叠加容器
        QLayoutItem* verticalItem2 = verticalLayout2->replaceWidget(ui->lbVerticalView2, verticalContainer2);
        QLayoutItem* leftItem2 = leftLayout2->replaceWidget(ui->lbLeftView2, leftContainer2);
        QLayoutItem* frontItem2 = frontLayout2->replaceWidget(ui->lbFrontView2, frontContainer2);
        
        // 删除被替换的控件和布局项
        if (verticalItem) {
            delete verticalItem;
            ui->lbVerticalView->deleteLater();
        }
        if (leftItem) {
            delete leftItem;
            ui->lbLeftView->deleteLater();
        }
        if (frontItem) {
            delete frontItem;
            ui->lbFrontView->deleteLater();
        }
        
        // 删除被替换的选项卡控件和布局项
        if (verticalItem2) {
            delete verticalItem2;
            ui->lbVerticalView2->deleteLater();
        }
        if (leftItem2) {
            delete leftItem2;
            ui->lbLeftView2->deleteLater();
        }
        if (frontItem2) {
            delete frontItem2;
            ui->lbFrontView2->deleteLater();
        }
        
        // 启用PaintingOverlay的选择功能
        m_verticalPaintingOverlay->enableSelection(true);
        m_leftPaintingOverlay->enableSelection(true);
        m_frontPaintingOverlay->enableSelection(true);
        
        m_verticalPaintingOverlay2->enableSelection(true);
        m_leftPaintingOverlay2->enableSelection(true);
        m_frontPaintingOverlay2->enableSelection(true);
        
        // 显示叠加容器
        verticalContainer->show();
        leftContainer->show();
        frontContainer->show();
        
        // 显示选项卡叠加容器
        verticalContainer2->show();
        leftContainer2->show();
        frontContainer2->show();
        
        // 注意：PaintingOverlay的信号连接已在前面的代码中完成，这里不需要重复连接
        
        qDebug() << "硬件加速显示控件已通过布局管理器正确替换 QLabel";
        qDebug() << "硬件加速控件几何信息:";
        qDebug() << "Vertical Widget:" << m_verticalDisplayWidget->geometry();
        qDebug() << "Left Widget:" << m_leftDisplayWidget->geometry();
        qDebug() << "Front Widget:" << m_frontDisplayWidget->geometry();
    } else {
        qDebug() << "无法获取 QLabel 的布局管理器，回退到手动替换方式";
        
        // 回退方案：手动设置父控件和几何信息
        QWidget* verticalParent = ui->lbVerticalView->parentWidget();
        QWidget* leftParent = ui->lbLeftView->parentWidget();
        QWidget* frontParent = ui->lbFrontView->parentWidget();
        
        if (verticalParent && leftParent && frontParent) {
            QRect verticalGeometry = ui->lbVerticalView->geometry();
            QRect leftGeometry = ui->lbLeftView->geometry();
            QRect frontGeometry = ui->lbFrontView->geometry();
            
            ui->lbVerticalView->hide();
            ui->lbLeftView->hide();
            ui->lbFrontView->hide();
            
            m_verticalDisplayWidget->setParent(verticalParent);
            m_leftDisplayWidget->setParent(leftParent);
            m_frontDisplayWidget->setParent(frontParent);
            
            m_verticalDisplayWidget->setGeometry(verticalGeometry);
            m_leftDisplayWidget->setGeometry(leftGeometry);
            m_frontDisplayWidget->setGeometry(frontGeometry);
            
            m_verticalDisplayWidget->setScaledContents(true);
            m_leftDisplayWidget->setScaledContents(true);
            m_frontDisplayWidget->setScaledContents(true);
            
            m_verticalDisplayWidget->show();
            m_leftDisplayWidget->show();
            m_frontDisplayWidget->show();
            
            qDebug() << "使用回退方案完成控件替换";
        }
    }
    
    qDebug() << "VideoDisplayWidget 硬件加速显示控件初始化完成";
}

void MutiCamApp::updateVideoDisplayWidget(const QString& viewName, const cv::Mat& frame)
{
    VideoDisplayWidget* widget = getVideoDisplayWidget(viewName);
    if (!widget || frame.empty()) {
        return;
    }
    
    // 将 cv::Mat 转换为 QPixmap
    QPixmap pixmap = matToQPixmap(frame);
    widget->setVideoFrame(pixmap);
    
    // 注意：几何图形数据的更新已移除，现在只在用户完成绘制操作时才更新
    // 这大幅减少了每帧的数据传输开销，提升了性能
}

// 这些函数已移除，绘图数据管理现在完全由VideoDisplayWidget内部处理

VideoDisplayWidget* MutiCamApp::getVideoDisplayWidget(const QString& viewName)
{
    if (viewName == "vertical") {
        return m_verticalDisplayWidget;
    } else if (viewName == "left") {
        return m_leftDisplayWidget;
    } else if (viewName == "front") {
        return m_frontDisplayWidget;
    } else if (viewName == "vertical2") {
        return m_verticalDisplayWidget2;
    } else if (viewName == "left2") {
        return m_leftDisplayWidget2;
    } else if (viewName == "front2") {
        return m_frontDisplayWidget2;
    }
    return nullptr;
}

PaintingOverlay* MutiCamApp::getPaintingOverlay(const QString& viewName)
{
    if (viewName == "vertical") {
        return m_verticalPaintingOverlay;
    } else if (viewName == "left") {
        return m_leftPaintingOverlay;
    } else if (viewName == "front") {
        return m_frontPaintingOverlay;
    } else if (viewName == "vertical2") {
        return m_verticalPaintingOverlay2;
    } else if (viewName == "left2") {
        return m_leftPaintingOverlay2;
    } else if (viewName == "front2") {
        return m_frontPaintingOverlay2;
    }
    return nullptr;
}

PaintingOverlay* MutiCamApp::getActivePaintingOverlay()
{
    // 首先尝试获取当前焦点的PaintingOverlay
    QWidget* focusWidget = QApplication::focusWidget();
    if (auto* overlay = qobject_cast<PaintingOverlay*>(focusWidget)) {
        return overlay;
    }

    // 如果没有焦点overlay，使用记录的最后活动overlay
    int currentTab = ui->tabWidget->currentIndex();
    if (currentTab == 0) {
        // 主界面，优先使用记录的最后活动overlay
        if (m_lastActivePaintingOverlay) {
            // 确保最后活动的overlay是主界面的overlay
            if (m_lastActivePaintingOverlay == m_verticalPaintingOverlay ||
                m_lastActivePaintingOverlay == m_leftPaintingOverlay ||
                m_lastActivePaintingOverlay == m_frontPaintingOverlay) {
                return m_lastActivePaintingOverlay;
            }
        }
        // 如果没有记录或记录的不是主界面overlay，返回垂直视图作为默认
        return m_verticalPaintingOverlay;
    } else if (currentTab == 1) {
        return m_verticalPaintingOverlay2;
    } else if (currentTab == 2) {
        return m_leftPaintingOverlay2;
    } else if (currentTab == 3) {
        return m_frontPaintingOverlay2;
    }
    return nullptr;
}

// {{ AURA-X: Add - 清空绘图按钮槽函数实现. Approval: 寸止(ID:clear_buttons). }}
void MutiCamApp::onClearDrawingsClicked()
{
    PaintingOverlay* overlay = getActivePaintingOverlay();
    if (overlay) {
        overlay->clearAllDrawings();
    }
}

void MutiCamApp::onClearDrawingsVerticalClicked()
{
    if (m_verticalPaintingOverlay) {
        m_verticalPaintingOverlay->clearAllDrawings();
    }
}

void MutiCamApp::onClearDrawingsLeftClicked()
{
    if (m_leftPaintingOverlay) {
        m_leftPaintingOverlay->clearAllDrawings();
    }
}

void MutiCamApp::onClearDrawingsFrontClicked()
{
    if (m_frontPaintingOverlay) {
        m_frontPaintingOverlay->clearAllDrawings();
    }
}

// {{ AURA-X: Add - 撤销绘图按钮槽函数实现. Approval: 寸止(ID:undo_buttons). }}
void MutiCamApp::onUndoDrawingClicked()
{
    PaintingOverlay* overlay = getActivePaintingOverlay();
    if (overlay) {
        overlay->undoLastDrawing();
    }
}

void MutiCamApp::onUndoDrawingVerticalClicked()
{
    if (m_verticalPaintingOverlay) {
        m_verticalPaintingOverlay->undoLastDrawing();
    }
}

void MutiCamApp::onUndoDrawingLeftClicked()
{
    if (m_leftPaintingOverlay) {
        m_leftPaintingOverlay->undoLastDrawing();
    }
}

void MutiCamApp::onUndoDrawingFrontClicked()
{
    if (m_frontPaintingOverlay) {
        m_frontPaintingOverlay->undoLastDrawing();
    }
}

// 网格相关槽函数实现
void MutiCamApp::onGridDensityChanged()
{
    QString text = ui->leGridDensity->text();
    bool ok;
    int spacing = text.toInt(&ok);

    if (ok && spacing >= 0) {
        // 确保网格间距至少为10（如果不为0）
        if (spacing > 0 && spacing < 10) {
            spacing = 10;
            ui->leGridDensity->setText(QString::number(spacing));
        }

        // 设置主界面所有视图的网格
        if (m_verticalPaintingOverlay) m_verticalPaintingOverlay->setGridSpacing(spacing);
        if (m_leftPaintingOverlay) m_leftPaintingOverlay->setGridSpacing(spacing);
        if (m_frontPaintingOverlay) m_frontPaintingOverlay->setGridSpacing(spacing);
    }
}

void MutiCamApp::onGridDensityVerticalChanged()
{
    QString text = ui->leGridDensVertical->text();
    bool ok;
    int spacing = text.toInt(&ok);

    if (ok && spacing >= 0) {
        // 确保网格间距至少为10（如果不为0）
        if (spacing > 0 && spacing < 10) {
            spacing = 10;
            ui->leGridDensVertical->setText(QString::number(spacing));
        }

        // 设置垂直视图的网格
        if (m_verticalPaintingOverlay2) m_verticalPaintingOverlay2->setGridSpacing(spacing);
    }
}

void MutiCamApp::onGridDensityLeftChanged()
{
    QString text = ui->leGridDensLeft->text();
    bool ok;
    int spacing = text.toInt(&ok);

    if (ok && spacing >= 0) {
        // 确保网格间距至少为10（如果不为0）
        if (spacing > 0 && spacing < 10) {
            spacing = 10;
            ui->leGridDensLeft->setText(QString::number(spacing));
        }

        // 设置左侧视图的网格
        if (m_leftPaintingOverlay2) m_leftPaintingOverlay2->setGridSpacing(spacing);
    }
}

void MutiCamApp::onGridDensityFrontChanged()
{
    QString text = ui->leGridDensFront->text();
    bool ok;
    int spacing = text.toInt(&ok);

    if (ok && spacing >= 0) {
        // 确保网格间距至少为10（如果不为0）
        if (spacing > 0 && spacing < 10) {
            spacing = 10;
            ui->leGridDensFront->setText(QString::number(spacing));
        }

        // 设置对向视图的网格
        if (m_frontPaintingOverlay2) m_frontPaintingOverlay2->setGridSpacing(spacing);
    }
}

void MutiCamApp::onCancelGridsClicked()
{
    // 清空主界面网格密度输入框
    ui->leGridDensity->clear();

    // 取消主界面所有视图的网格
    if (m_verticalPaintingOverlay) m_verticalPaintingOverlay->setGridSpacing(0);
    if (m_leftPaintingOverlay) m_leftPaintingOverlay->setGridSpacing(0);
    if (m_frontPaintingOverlay) m_frontPaintingOverlay->setGridSpacing(0);
}

void MutiCamApp::onCancelGridsVerticalClicked()
{
    // 清空垂直视图网格密度输入框
    ui->leGridDensVertical->clear();

    // 取消垂直视图的网格
    if (m_verticalPaintingOverlay2) m_verticalPaintingOverlay2->setGridSpacing(0);
}

void MutiCamApp::onCancelGridsLeftClicked()
{
    // 清空左侧视图网格密度输入框
    ui->leGridDensLeft->clear();

    // 取消左侧视图的网格
    if (m_leftPaintingOverlay2) m_leftPaintingOverlay2->setGridSpacing(0);
}

void MutiCamApp::onCancelGridsFrontClicked()
{
    // 清空对向视图网格密度输入框
    ui->leGridDensFront->clear();

    // 取消对向视图的网格
    if (m_frontPaintingOverlay2) m_frontPaintingOverlay2->setGridSpacing(0);
}

// {{ AURA-X: Add - 记录最后活动overlay槽函数实现. Approval: 寸止(ID:last_active_overlay). }}
void MutiCamApp::onOverlayActivated(PaintingOverlay* overlay)
{
    m_lastActivePaintingOverlay = overlay;
}

// {{ AURA-X: Add - 绘图同步槽函数实现. Approval: 寸止(ID:drawing_sync). }}
void MutiCamApp::onDrawingSync(const QString& viewName)
{
    // 获取发送信号的PaintingOverlay
    PaintingOverlay* sourceOverlay = qobject_cast<PaintingOverlay*>(sender());
    if (!sourceOverlay) {
        return;
    }
    
    // 获取源overlay的绘图状态
    PaintingOverlay::DrawingState state = sourceOverlay->getDrawingState();
    
    // 根据视图名称确定同步目标：主界面视图和选项卡视图分别同步
    QList<PaintingOverlay*> targetOverlays;
    
    // 判断源视图是主界面视图还是选项卡视图
    if (viewName == "Vertical" || viewName == "Left" || viewName == "Front") {
        // 主界面视图：同步到对应的选项卡视图
        if (viewName == "Vertical" && m_verticalPaintingOverlay2) {
            targetOverlays.append(m_verticalPaintingOverlay2);
        } else if (viewName == "Left" && m_leftPaintingOverlay2) {
            targetOverlays.append(m_leftPaintingOverlay2);
        } else if (viewName == "Front" && m_frontPaintingOverlay2) {
            targetOverlays.append(m_frontPaintingOverlay2);
        }
    } else if (viewName == "Vertical2" || viewName == "Left2" || viewName == "Front2") {
        // 选项卡视图：同步到对应的主界面视图
        if (viewName == "Vertical2" && m_verticalPaintingOverlay) {
            targetOverlays.append(m_verticalPaintingOverlay);
        } else if (viewName == "Left2" && m_leftPaintingOverlay) {
            targetOverlays.append(m_leftPaintingOverlay);
        } else if (viewName == "Front2" && m_frontPaintingOverlay) {
            targetOverlays.append(m_frontPaintingOverlay);
        }
    }
    
    // 同步到目标视图
    for (PaintingOverlay* overlay : targetOverlays) {
        if (overlay && overlay != sourceOverlay) {
            // 临时断开信号连接，避免循环触发
            disconnect(overlay, &PaintingOverlay::drawingCompleted, 
                      this, &MutiCamApp::onDrawingSync);
            
            // 设置绘图状态
            overlay->setDrawingState(state);
            
            // 重新连接信号
            connect(overlay, &PaintingOverlay::drawingCompleted,
                   this, &MutiCamApp::onDrawingSync);
        }
    }
}

// {{ AURA-X: Add - 获取当前活动VideoDisplayWidget辅助函数. Approval: 寸止(ID:active_widget_helper). }}
VideoDisplayWidget* MutiCamApp::getActiveVideoWidget()
{
    int currentTab = ui->tabWidget->currentIndex();
    
    // 根据当前选项卡返回对应的VideoDisplayWidget
    switch (currentTab) {
        case 0: // 主界面
            // 可以根据需要返回主界面的某个默认widget，这里返回垂直视图
            return m_verticalDisplayWidget;
        case 1: // 垂直视图选项卡
            return m_verticalDisplayWidget2;
        case 2: // 左视图选项卡
            return m_leftDisplayWidget2;
        case 3: // 前视图选项卡
            return m_frontDisplayWidget2;
        default:
            return m_verticalDisplayWidget; // 默认返回垂直视图
    }
}

void MutiCamApp::syncOverlayTransforms(const QString& viewName)
{
    VideoDisplayWidget* videoWidget = getVideoDisplayWidget(viewName);
    PaintingOverlay* paintingOverlay = getPaintingOverlay(viewName);

    if (videoWidget && paintingOverlay) {
        // 从视频显示控件获取当前的偏移和缩放比例
        QPointF offset = videoWidget->getImageOffset();
        double scale = videoWidget->getScaleFactor();
        QSize imageSize = videoWidget->getImageSize();
        
        // 将这些变换信息设置到绘画层
        paintingOverlay->setTransforms(offset, scale, imageSize);
    }
}

void MutiCamApp::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    
    // 同步所有视图的坐标变换
    syncOverlayTransforms("vertical");
    syncOverlayTransforms("left");
    syncOverlayTransforms("front");
    syncOverlayTransforms("vertical2");
    syncOverlayTransforms("left2");
    syncOverlayTransforms("front2");
}

// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// displayImageWithHardwareAcceleration方法已迁移到VideoDisplayWidget

// 类型转换函数已移除，VideoDisplayWidget现在使用自己的数据类型