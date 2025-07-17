#include "MutiCamApp.h"
#include <QMessageBox>
#include <QDebug>
#include <QPixmap>
#include <QApplication>
#include <QMouseEvent>
#include <QEvent>
#include <QCursor>
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
    , m_isDrawingMode(false)
    , m_activeView("")
    , m_frameCache(MAX_CACHED_FRAMES)  // 初始化缓存，设置最大缓存数量
    , m_lastUpdateTime(std::chrono::steady_clock::now())  // 初始化更新时间
    , m_verticalDisplayWidget(nullptr)
    , m_leftDisplayWidget(nullptr)
    , m_frontDisplayWidget(nullptr)
    , m_verticalDisplayWidget2(nullptr)
    , m_leftDisplayWidget2(nullptr)
    , m_frontDisplayWidget2(nullptr)
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
    
    // 连接VideoDisplayWidget的绘制数据变化信号
    if (m_verticalDisplayWidget) {
        connect(m_verticalDisplayWidget, &VideoDisplayWidget::drawingDataChanged,
                this, &MutiCamApp::onDrawingDataChanged);
    }
    if (m_leftDisplayWidget) {
        connect(m_leftDisplayWidget, &VideoDisplayWidget::drawingDataChanged,
                this, &MutiCamApp::onDrawingDataChanged);
    }
    if (m_frontDisplayWidget) {
        connect(m_frontDisplayWidget, &VideoDisplayWidget::drawingDataChanged,
                this, &MutiCamApp::onDrawingDataChanged);
    }
    if (m_verticalDisplayWidget2) {
        connect(m_verticalDisplayWidget2, &VideoDisplayWidget::drawingDataChanged,
                this, &MutiCamApp::onDrawingDataChanged);
    }
    if (m_leftDisplayWidget2) {
        connect(m_leftDisplayWidget2, &VideoDisplayWidget::drawingDataChanged,
                this, &MutiCamApp::onDrawingDataChanged);
    }
    if (m_frontDisplayWidget2) {
        connect(m_frontDisplayWidget2, &VideoDisplayWidget::drawingDataChanged,
                this, &MutiCamApp::onDrawingDataChanged);
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
    if (frame.empty()) {
        return;
    }
    
    try {
        // 保存当前帧的副本
        cv::Mat frameCopy = frame.clone();
        
        // 根据相机ID保存到对应的成员变量
        if (cameraId == "vertical") {
            m_currentFrameVertical = frameCopy.clone();
            // 直接更新VideoDisplayWidget的视频帧，不再频繁更新几何图形数据
            if (m_verticalDisplayWidget) {
                QPixmap pixmap = matToQPixmap(frameCopy);
                m_verticalDisplayWidget->setVideoFrame(pixmap);
            }
            if (ui->tabWidget->currentIndex() == 1 && m_verticalDisplayWidget2) {
                QPixmap pixmap = matToQPixmap(frameCopy);
                m_verticalDisplayWidget2->setVideoFrame(pixmap);
            }
        } else if (cameraId == "left") {
            m_currentFrameLeft = frameCopy.clone();
            // 直接更新VideoDisplayWidget的视频帧，不再频繁更新几何图形数据
            if (m_leftDisplayWidget) {
                QPixmap pixmap = matToQPixmap(frameCopy);
                m_leftDisplayWidget->setVideoFrame(pixmap);
            }
            if (ui->tabWidget->currentIndex() == 2 && m_leftDisplayWidget2) {
                QPixmap pixmap = matToQPixmap(frameCopy);
                m_leftDisplayWidget2->setVideoFrame(pixmap);
            }
        } else if (cameraId == "front") {
            m_currentFrameFront = frameCopy.clone();
            // 直接更新VideoDisplayWidget的视频帧，不再频繁更新几何图形数据
            if (m_frontDisplayWidget) {
                QPixmap pixmap = matToQPixmap(frameCopy);
                m_frontDisplayWidget->setVideoFrame(pixmap);
            }
            if (ui->tabWidget->currentIndex() == 3 && m_frontDisplayWidget2) {
                QPixmap pixmap = matToQPixmap(frameCopy);
                m_frontDisplayWidget2->setVideoFrame(pixmap);
            }
        }
        
    } catch (const std::exception& e) {
        qDebug() << "Error displaying frame for camera" << cameraId << ":" << e.what();
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

void MutiCamApp::onDrawingDataChanged(const QString& viewName)
{
    qDebug() << "Drawing data changed for view:" << viewName;
    
    // 当VideoDisplayWidget的绘制数据发生变化时，同步更新MutiCamApp的数据
    // 这里可以根据需要从VideoDisplayWidget获取最新的绘制数据
    // 并更新到m_viewData、m_lineData等成员变量中
    
    // 使缓存失效，确保下次更新时重新渲染
    invalidateCache(viewName);
    
    // 可选：触发其他相关视图的更新
    // updateDrawingDataForView(viewName);
}



QPixmap MutiCamApp::matToQPixmap(const cv::Mat& mat, bool setDevicePixelRatio)
{
    if (mat.empty()) {
        return QPixmap();
    }
    
    try {
        cv::Mat rgbMat;
        
        // 根据通道数进行转换
        if (mat.channels() == 1) {
            // 灰度图像转RGB
            cv::cvtColor(mat, rgbMat, cv::COLOR_GRAY2RGB);
        } else if (mat.channels() == 3) {
            // BGR转RGB
            cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);
        } else {
            // 其他格式直接使用
            rgbMat = mat;
        }
        
        // 创建QImage
        QImage qimg(rgbMat.data, rgbMat.cols, rgbMat.rows, 
                   rgbMat.step, QImage::Format_RGB888);
        
        // 转换为QPixmap
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
    setDrawingMode("point", "general");
}

void MutiCamApp::onDrawPointVerticalClicked()
{
    setDrawingMode("point", "vertical");
}

void MutiCamApp::onDrawPointLeftClicked()
{
    setDrawingMode("point", "left");
}

void MutiCamApp::onDrawPointFrontClicked()
{
    setDrawingMode("point", "front");
}

void MutiCamApp::setDrawingMode(const QString& drawingType, const QString& viewName)
{
    // 先退出之前的绘制模式
    exitDrawingMode();
    
    m_isDrawingMode = true;
    m_currentDrawingType = drawingType;
    m_activeView = viewName;
    
    // 将绘图类型转换为VideoDisplayWidget的DrawingTool枚举
    VideoDisplayWidget::DrawingTool tool = VideoDisplayWidget::DrawingTool::None;
    if (drawingType == "point") {
        tool = VideoDisplayWidget::DrawingTool::Point;
    } else if (drawingType == "line") {
        tool = VideoDisplayWidget::DrawingTool::Line;
    } else if (drawingType == "circle") {
        tool = VideoDisplayWidget::DrawingTool::Circle;
    } else if (drawingType == "fine_circle") {
        tool = VideoDisplayWidget::DrawingTool::FineCircle;
    } else if (drawingType == "parallel") {
        tool = VideoDisplayWidget::DrawingTool::Parallel;
    } else if (drawingType == "two_lines") {
        tool = VideoDisplayWidget::DrawingTool::TwoLines;
    }
    
    // 启动所有相关视图的绘制模式
    if (viewName == "general" || viewName == "vertical") {
        if (m_verticalDisplayWidget) {
            m_verticalDisplayWidget->startDrawing(tool);
        }
        if (m_verticalDisplayWidget2) {
            m_verticalDisplayWidget2->startDrawing(tool);
        }
    }
    if (viewName == "general" || viewName == "left") {
        if (m_leftDisplayWidget) {
            m_leftDisplayWidget->startDrawing(tool);
        }
        if (m_leftDisplayWidget2) {
            m_leftDisplayWidget2->startDrawing(tool);
        }
    }
    if (viewName == "general" || viewName == "front") {
        if (m_frontDisplayWidget) {
            m_frontDisplayWidget->startDrawing(tool);
        }
        if (m_frontDisplayWidget2) {
            m_frontDisplayWidget2->startDrawing(tool);
        }
    }
    
    qDebug() << "进入" << drawingType << "绘制模式:" << viewName;
}

void MutiCamApp::exitDrawingMode()
{
    m_isDrawingMode = false;
    m_currentDrawingType = "";
    m_activeView = "";
    
    // 清理当前正在绘制的直线、圆形、精细圆和平行线
    m_currentLines.clear();
    m_currentCircles.clear();
    m_currentFineCircles.clear();
    m_currentParallels.clear();
    
    // 停止所有VideoDisplayWidget的绘制模式
    if (m_verticalDisplayWidget) {
        m_verticalDisplayWidget->stopDrawing();
    }
    if (m_leftDisplayWidget) {
        m_leftDisplayWidget->stopDrawing();
    }
    if (m_frontDisplayWidget) {
        m_frontDisplayWidget->stopDrawing();
    }
    if (m_verticalDisplayWidget2) {
        m_verticalDisplayWidget2->stopDrawing();
    }
    if (m_leftDisplayWidget2) {
        m_leftDisplayWidget2->stopDrawing();
    }
    if (m_frontDisplayWidget2) {
        m_frontDisplayWidget2->stopDrawing();
    }
    
    qDebug() << "退出绘制模式";
}

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
    if (!m_isDrawingMode) {
        return QMainWindow::eventFilter(obj, event);
    }
    
    // 检查是否是VideoDisplayWidget
    VideoDisplayWidget* videoWidget = qobject_cast<VideoDisplayWidget*>(obj);
    if (videoWidget) {
        // 只处理右键退出绘制模式，其他事件让VideoDisplayWidget自己处理
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::RightButton) {
                exitDrawingMode();
                return true;
            }
        }
        // 让VideoDisplayWidget处理其他所有鼠标事件
        return false;
    }
    
    // 检查是否是QLabel（保留对QLabel的支持，仅用于点绘制）
    QLabel* label = qobject_cast<QLabel*>(obj);
    
    if (label && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        
        // 右键退出绘制模式
        if (mouseEvent->button() == Qt::RightButton) {
            exitDrawingMode();
            return true;
        }
        
        // 左键开始绘制（对于点测量，我们在释放时处理）
        if (mouseEvent->button() == Qt::LeftButton) {
            return true; // 消费事件但不处理
        }
    }
    else if (label && event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        
        // 左键释放时处理点测量（仅限点绘制）
        if (mouseEvent->button() == Qt::LeftButton && m_currentDrawingType == "point") {
            handleLabelClick(label, mouseEvent->pos());
            return true;
        }
    }
    else if (label && event->type() == QEvent::MouseMove) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        
        // 处理鼠标移动事件（仅限点绘制的实时显示）
        if (m_currentDrawingType == "point") {
            handleMouseMove(label, mouseEvent->pos());
            return true;
        }
    }
    
    // 其他事件正常处理
    return QMainWindow::eventFilter(obj, event);
}

void MutiCamApp::handleLabelClick(QLabel* label, const QPoint& pos)
{
    QString viewName = getViewName(label);
    if (viewName.isEmpty()) {
        return;
    }
    
    // 检查是否是有效的绘制目标
    bool isValidTarget = false;
    if (m_activeView == "general") {
        isValidTarget = true; // 通用模式下所有视图都可以绘制
    } else if (m_activeView == viewName) {
        isValidTarget = true; // 特定视图模式下只能在对应视图绘制
    }
    
    if (!isValidTarget) {
        return;
    }
    
    // 将窗口坐标转换为图像坐标
    QPointF imagePos = windowToImageCoordinates(label, pos);
    
    // 检查坐标是否有效
    if (imagePos.x() < 0 || imagePos.y() < 0) {
        qDebug() << "点击位置超出图像范围，忽略";
        return;
    }
    
    // 根据绘制类型处理点击
    bool dataChanged = false;
    if (m_currentDrawingType == "point") {
        // 添加点到对应视图
        m_viewData[viewName].append(imagePos);
        qDebug() << "在" << viewName << "视图添加点:" << imagePos;
        dataChanged = true;
    } else if (m_currentDrawingType == "line") {
        // {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
        // handleLineDrawingClick方法已迁移到VideoDisplayWidget
        qDebug() << "直线绘制功能已迁移到VideoDisplayWidget";
    } else if (m_currentDrawingType == "circle") {
        // {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
        // handleCircleDrawingClick方法已迁移到VideoDisplayWidget
        qDebug() << "圆形绘制功能已迁移到VideoDisplayWidget";
    } else if (m_currentDrawingType == "fine_circle") {
        // {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
        // handleFineCircleDrawingClick方法已迁移到VideoDisplayWidget
        qDebug() << "精细圆绘制功能已迁移到VideoDisplayWidget";
    } else if (m_currentDrawingType == "parallel") {
        // {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
        // handleParallelDrawingClick方法已迁移到VideoDisplayWidget
        qDebug() << "平行线绘制功能已迁移到VideoDisplayWidget";
    } else if (m_currentDrawingType == "two_lines") {
        // {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
        // handleTwoLinesDrawingClick方法已迁移到VideoDisplayWidget
        qDebug() << "线与线绘制功能已迁移到VideoDisplayWidget";
    }
    
    // 如果数据发生变化，使用事件驱动的方式更新绘制数据
    if (dataChanged) {
        // 使该视图的缓存失效（因为绘制数据已改变）
        invalidateCache(viewName);
        
        // 事件驱动更新：只在绘制操作完成时更新几何图形数据
        updateDrawingDataForView(viewName);
    }
    
    // 立即更新显示
    updateViewDisplay(viewName);
}

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
    // 使用VideoDisplayWidget的内置坐标转换方法
    return widget->widgetToImage(windowPos);
}

void MutiCamApp::handleMouseMove(QLabel* label, const QPoint& pos)
{
    QString viewName = getViewName(label);
    if (viewName.isEmpty()) {
        return;
    }
    
    // 检查是否是有效的绘制目标
    bool isValidTarget = false;
    if (m_activeView == "general") {
        isValidTarget = true; // 通用模式下所有视图都可以绘制
    } else if (m_activeView == viewName) {
        isValidTarget = true; // 特定视图模式下只能在对应视图绘制
    }
    
    if (!isValidTarget) {
        return;
    }
    
    // 将窗口坐标转换为图像坐标
    QPointF imagePos = windowToImageCoordinates(label, pos);
    
    // 检查坐标是否有效
    if (imagePos.x() < 0 || imagePos.y() < 0) {
        return;
    }
    
    // 根据绘制类型处理鼠标移动（主要用于实时预览）
    if (m_currentDrawingType == "line" && m_currentLines.contains(viewName)) {
        // 直线绘制：如果已有一个点，更新第二个点进行实时预览
        LineObject& currentLine = m_currentLines[viewName];
        if (currentLine.points.size() == 1) {
            // 添加第二个点用于实时预览
            currentLine.points.append(imagePos);
            
            // 使该视图的缓存失效并更新显示
            invalidateCache(viewName);
            updateViewDisplay(viewName);
        } else if (currentLine.points.size() == 2) {
            // 更新第二个点的位置
            currentLine.points[1] = imagePos;
            
            // 使该视图的缓存失效并更新显示
            invalidateCache(viewName);
            updateViewDisplay(viewName);
        }
    }
    // {{ AURA-X: Add - 添加平行线实时预览逻辑（QLabel版本）. Approval: 寸止(ID:8). }}
    else if (m_currentDrawingType == "parallel" && m_currentParallels.contains(viewName)) {
        // 平行线绘制：如果已有一个点，创建临时预览数据
        ParallelObject& currentParallel = m_currentParallels[viewName];
        if (currentParallel.points.size() == 1) {
            // 创建临时预览数据，不修改原始points数组
            m_tempParallelPreview[viewName] = currentParallel;
            m_tempParallelPreview[viewName].points.append(imagePos); // 只在预览数据中添加第二个点
            
            // 使该视图的缓存失效并更新显示
            invalidateCache(viewName);
            updateViewDisplay(viewName);
        }
    }
    // 注意：圆形和精细圆绘制通常不需要实时预览，因为需要多个点确定
}

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
    
    qDebug() << "已更新" << viewName << "视图显示，包含" << m_viewData[viewName].size() << "个点";
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
    if (!m_viewData.contains(viewName)) {
        return 0;
    }
    
    const QVector<QPointF>& points = m_viewData[viewName];
    size_t hash = 0;
    
    // 使用简单的哈希算法计算点数据的哈希值
    for (const QPointF& point : points) {
        // 将坐标转换为整数进行哈希计算
        int x = static_cast<int>(point.x() * 100);  // 保留两位小数精度
        int y = static_cast<int>(point.y() * 100);
        hash ^= std::hash<int>{}(x) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= std::hash<int>{}(y) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    }
    
    return hash;
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
    qDebug() << "开始通用直线绘制模式";
    setDrawingMode("line", "general");
}

void MutiCamApp::onDrawLineVerticalClicked()
{
    qDebug() << "开始垂直视图直线绘制模式";
    setDrawingMode("line", "vertical");
}

void MutiCamApp::onDrawLineLeftClicked()
{
    qDebug() << "开始左视图直线绘制模式";
    setDrawingMode("line", "left");
}

void MutiCamApp::onDrawLineFrontClicked()
{
    qDebug() << "开始前视图直线绘制模式";
    setDrawingMode("line", "front");
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
    qDebug() << "开始通用圆形绘制模式";
    setDrawingMode("circle", "general");
}

void MutiCamApp::onDrawSimpleCircleVerticalClicked()
{
    qDebug() << "开始垂直视图圆形绘制模式";
    setDrawingMode("circle", "vertical");
}

void MutiCamApp::onDrawSimpleCircleLeftClicked()
{
    qDebug() << "开始左视图圆形绘制模式";
    setDrawingMode("circle", "left");
}

void MutiCamApp::onDrawSimpleCircleFrontClicked()
{
    qDebug() << "开始前视图圆形绘制模式";
    setDrawingMode("circle", "front");
}

void MutiCamApp::onDrawFineCircleClicked()
{
    qDebug() << "开始通用精细圆绘制模式";
    setDrawingMode("fine_circle", "general");
}

void MutiCamApp::onDrawFineCircleVerticalClicked()
{
    qDebug() << "开始垂直视图精细圆绘制模式";
    setDrawingMode("fine_circle", "vertical");
}

void MutiCamApp::onDrawFineCircleLeftClicked()
{
    qDebug() << "开始左视图精细圆绘制模式";
    setDrawingMode("fine_circle", "left");
}

void MutiCamApp::onDrawFineCircleFrontClicked()
{
    qDebug() << "开始前视图精细圆绘制模式";
    setDrawingMode("fine_circle", "front");
}

// 平行线绘制按钮点击事件处理
void MutiCamApp::onDrawParallelClicked()
{
    qDebug() << "开始通用平行线绘制模式";
    setDrawingMode("parallel", "general");
}

void MutiCamApp::onDrawParallelVerticalClicked()
{
    qDebug() << "开始垂直视图平行线绘制模式";
    setDrawingMode("parallel", "vertical");
}

void MutiCamApp::onDrawParallelLeftClicked()
{
    qDebug() << "开始左视图平行线绘制模式";
    setDrawingMode("parallel", "left");
}

void MutiCamApp::onDrawParallelFrontClicked()
{
    qDebug() << "开始前视图平行线绘制模式";
    setDrawingMode("parallel", "front");
}

// 线与线绘制按钮点击事件处理
void MutiCamApp::onDrawTwoLinesClicked()
{
    qDebug() << "开始线与线绘制模式";
    setDrawingMode("two_lines", "general");
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
    
    if (index == 1) { // 垂直视图选项卡
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
    
    // 设置视图名称
    m_verticalDisplayWidget->setViewName("vertical");
    m_leftDisplayWidget->setViewName("left");
    m_frontDisplayWidget->setViewName("front");
    
    // 设置选项卡视图名称
    m_verticalDisplayWidget2->setViewName("vertical2");
    m_leftDisplayWidget2->setViewName("left2");
    m_frontDisplayWidget2->setViewName("front2");
    
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
        
        // 使用 replaceWidget 替换控件，这会自动处理布局管理
        QLayoutItem* verticalItem = verticalLayout->replaceWidget(ui->lbVerticalView, m_verticalDisplayWidget);
        QLayoutItem* leftItem = leftLayout->replaceWidget(ui->lbLeftView, m_leftDisplayWidget);
        QLayoutItem* frontItem = frontLayout->replaceWidget(ui->lbFrontView, m_frontDisplayWidget);
        
        // 替换选项卡QLabel
        QLayoutItem* verticalItem2 = verticalLayout2->replaceWidget(ui->lbVerticalView2, m_verticalDisplayWidget2);
        QLayoutItem* leftItem2 = leftLayout2->replaceWidget(ui->lbLeftView2, m_leftDisplayWidget2);
        QLayoutItem* frontItem2 = frontLayout2->replaceWidget(ui->lbFrontView2, m_frontDisplayWidget2);
        
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
        
        // 显示硬件加速控件
        m_verticalDisplayWidget->show();
        m_leftDisplayWidget->show();
        m_frontDisplayWidget->show();
        
        // 显示选项卡VideoDisplayWidget
        m_verticalDisplayWidget2->show();
        m_leftDisplayWidget2->show();
        m_frontDisplayWidget2->show();
        
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

// 新增：专门用于更新几何图形数据的函数（事件驱动）
void MutiCamApp::updateDrawingDataForView(const QString& viewName)
{
    // 获取主界面和选项卡的控件
    VideoDisplayWidget* mainWidget = getVideoDisplayWidget(viewName);
    VideoDisplayWidget* tabWidget = nullptr;
    
    if (viewName == "vertical") {
        tabWidget = m_verticalDisplayWidget2;
    } else if (viewName == "left") {
        tabWidget = m_leftDisplayWidget2;
    } else if (viewName == "front") {
        tabWidget = m_frontDisplayWidget2;
    }
    
    // 更新主界面控件的绘制数据
    if (mainWidget) {
        updateWidgetDrawingData(mainWidget, viewName);
    }
    
    // 更新选项卡控件的绘制数据
    if (tabWidget) {
        updateWidgetDrawingData(tabWidget, viewName);
    }
}

// 辅助函数：更新单个控件的绘制数据
void MutiCamApp::updateWidgetDrawingData(VideoDisplayWidget* widget, const QString& viewName)
{
    if (!widget) {
        return;
    }
    
    // 更新绘制数据
    if (m_viewData.contains(viewName)) {
        widget->setPointsData(m_viewData.value(viewName));
    }
    
    if (m_lineData.contains(viewName)) {
        widget->setLinesData(convertToWidgetLineObjects(m_lineData.value(viewName)));
    }
    
    if (m_circleData.contains(viewName)) {
        widget->setCirclesData(convertToWidgetCircleObjects(m_circleData.value(viewName)));
    }
    
    if (m_fineCircleData.contains(viewName)) {
        widget->setFineCirclesData(convertToWidgetFineCircleObjects(m_fineCircleData.value(viewName)));
    }
    
    if (m_parallelData.contains(viewName)) {
        widget->setParallelLinesData(convertToWidgetParallelObjects(m_parallelData.value(viewName)));
    }
    
    if (m_twoLinesData.contains(viewName)) {
        widget->setTwoLinesData(convertToWidgetTwoLinesObjects(m_twoLinesData.value(viewName)));
    }
    
    // 触发重绘
    widget->update();
}

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

// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// displayImageWithHardwareAcceleration方法已迁移到VideoDisplayWidget

// 类型转换函数实现
QVector<::LineObject> MutiCamApp::convertToWidgetLineObjects(const QVector<LineObject>& lines)
{
    QVector<::LineObject> result;
    for (const auto& line : lines) {
        ::LineObject widgetLine;
        widgetLine.points = line.points;
        widgetLine.isCompleted = line.isCompleted;
        widgetLine.color = line.color;
        widgetLine.thickness = line.thickness;
        widgetLine.isDashed = line.isDashed;
        result.append(widgetLine);
    }
    return result;
}

QVector<::CircleObject> MutiCamApp::convertToWidgetCircleObjects(const QVector<CircleObject>& circles)
{
    QVector<::CircleObject> result;
    for (const auto& circle : circles) {
        ::CircleObject widgetCircle;
        widgetCircle.points = circle.points;
        widgetCircle.isCompleted = circle.isCompleted;
        widgetCircle.color = circle.color;
        widgetCircle.thickness = circle.thickness;
        widgetCircle.center = circle.center;
        widgetCircle.radius = circle.radius;
        result.append(widgetCircle);
    }
    return result;
}

QVector<::FineCircleObject> MutiCamApp::convertToWidgetFineCircleObjects(const QVector<FineCircleObject>& fineCircles)
{
    QVector<::FineCircleObject> result;
    for (const auto& fineCircle : fineCircles) {
        ::FineCircleObject widgetFineCircle;
        widgetFineCircle.points = fineCircle.points;
        widgetFineCircle.isCompleted = fineCircle.isCompleted;
        widgetFineCircle.color = fineCircle.color;
        widgetFineCircle.thickness = fineCircle.thickness;
        widgetFineCircle.center = fineCircle.center;
        widgetFineCircle.radius = fineCircle.radius;
        result.append(widgetFineCircle);
    }
    return result;
}

QVector<::ParallelObject> MutiCamApp::convertToWidgetParallelObjects(const QVector<ParallelObject>& parallels)
{
    QVector<::ParallelObject> result;
    for (const auto& parallel : parallels) {
        ::ParallelObject widgetParallel;
        widgetParallel.points = parallel.points;
        widgetParallel.isCompleted = parallel.isCompleted;
        widgetParallel.color = parallel.color;
        widgetParallel.thickness = parallel.thickness;
        widgetParallel.distance = parallel.distance;
        widgetParallel.angle = parallel.angle;
        result.append(widgetParallel);
    }
    return result;
}

QVector<::TwoLinesObject> MutiCamApp::convertToWidgetTwoLinesObjects(const QVector<TwoLinesObject>& twoLines)
{
    QVector<::TwoLinesObject> result;
    for (const auto& twoLine : twoLines) {
        ::TwoLinesObject widgetTwoLines;
        widgetTwoLines.points = twoLine.points;
        widgetTwoLines.isCompleted = twoLine.isCompleted;
        widgetTwoLines.isPreview = twoLine.isPreview;
        widgetTwoLines.color = twoLine.color;
        widgetTwoLines.thickness = twoLine.thickness;
        widgetTwoLines.angle = twoLine.angle;
        widgetTwoLines.intersection = twoLine.intersection;
        result.append(widgetTwoLines);
    }
    return result;
}