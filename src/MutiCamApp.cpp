#include "MutiCamApp.h"
#include "ui_MutiCamApp.h"
#include "ZoomPanWidget.h"
#include <QMessageBox>
#include <QDebug>
#include <QPixmap>
#include <QApplication>
#include <QMouseEvent>
#include <QEvent>
#include <QResizeEvent>
#include <QCursor>
#include <QStackedLayout>
#include <QDir>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
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
    , m_statusUpdateTimer(nullptr)
    , m_isMeasuring(false)
    // {{ AURA-X: Delete - 移除残留的绘图模式和活动视图成员变量. Approval: 寸止(ID:cleanup). }}
    // m_isDrawingMode和m_activeView已移除，绘图状态现在由ZoomPanWidget管理
    , m_frameCache(MAX_CACHED_FRAMES)  // 初始化缓存，设置最大缓存数量
    , m_verticalZoomPanWidget(nullptr)
    , m_leftZoomPanWidget(nullptr)
    , m_frontZoomPanWidget(nullptr)
    , m_verticalZoomPanWidget2(nullptr)
    , m_leftZoomPanWidget2(nullptr)
    , m_frontZoomPanWidget2(nullptr)
    , m_lastActivePaintingOverlay(nullptr)
    , m_saveProgressDialog(nullptr)
    , m_saveWatcher(nullptr)
    , m_settingsManager(nullptr)
    , m_logManager(nullptr)
    , m_serialController(nullptr)
    , m_isUpdatingUISize(false)
    , m_currentX(0.0)
    , m_currentY(0.0)
    , m_currentZ(0.0)
{
    ui->setupUi(this);

    // 初始化缩放平移显示控件
    initializeZoomPanWidgets();

    // 初始化日志管理器（优先初始化，其他模块可能需要使用）
    initializeLogManager();

    // 初始化设置管理器
    initializeSettingsManager();

    // 初始化轨迹记录器
    initializeTrajectoryRecorder();

    // 初始化串口控制器
    initializeSerialController();

    // 初始化拍照参数预设
    initializeCapturePresets();

    // 注意：相机系统将在点击"开始测量"时初始化

    // 初始化按钮映射
    initializeButtonMappings();

    // 连接信号和槽
    connectSignalsAndSlots();

    // 初始化相机状态监控
    initializeCameraStatusMonitoring();

    // 设置初始状态
    ui->btnStartMeasure->setEnabled(true);
    ui->btnStopMeasure->setEnabled(false);
    
    // 安装鼠标事件过滤器
    installMouseEventFilters();

    // 初始化异步保存组件
    m_saveWatcher = new QFutureWatcher<void>(this);
    connect(m_saveWatcher, &QFutureWatcher<void>::finished,
            this, &MutiCamApp::onAsyncSaveFinished);
}

MutiCamApp::~MutiCamApp()
{
    qDebug() << "MutiCamApp destructor called";

    // 停止状态更新定时器
    if (m_statusUpdateTimer) {
        m_statusUpdateTimer->stop();
        delete m_statusUpdateTimer;
        m_statusUpdateTimer = nullptr;
    }

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
    
    // 清理 ZoomPanWidget 资源（PaintingOverlay会作为子控件自动清理）
    if (m_verticalZoomPanWidget) {
        delete m_verticalZoomPanWidget;
        m_verticalZoomPanWidget = nullptr;
    }
    if (m_leftZoomPanWidget) {
        delete m_leftZoomPanWidget;
        m_leftZoomPanWidget = nullptr;
    }
    if (m_frontZoomPanWidget) {
        delete m_frontZoomPanWidget;
        m_frontZoomPanWidget = nullptr;
    }
    if (m_verticalZoomPanWidget2) {
        delete m_verticalZoomPanWidget2;
        m_verticalZoomPanWidget2 = nullptr;
    }
    if (m_leftZoomPanWidget2) {
        delete m_leftZoomPanWidget2;
        m_leftZoomPanWidget2 = nullptr;
    }
    if (m_frontZoomPanWidget2) {
        delete m_frontZoomPanWidget2;
        m_frontZoomPanWidget2 = nullptr;
    }

    // 清理设置管理器
    if (m_settingsManager) {
        delete m_settingsManager;
        m_settingsManager = nullptr;
    }

    // 清理串口控制器
    if (m_serialController) {
        m_serialController->closePort();
        delete m_serialController;
        m_serialController = nullptr;
    }

    delete ui;
    
    qDebug() << "MutiCamApp destructor completed";
}

void MutiCamApp::initializeCameraSystem()
{
    try {
        // 创建相机管理器
        m_cameraManager = std::make_unique<MutiCam::Camera::CameraManager>(this);

        // 从参数设置中获取相机序列号
        if (!m_settingsManager) {
            qDebug() << "Settings manager not initialized, cannot get camera serial numbers";
            return;
        }

        const auto& settings = m_settingsManager->getCurrentSettings();

        // 定义相机ID和对应的序列号
        std::vector<std::pair<std::string, std::string>> cameraConfigs = {
            {"vertical", settings.verCamSN.toStdString()},
            {"left", settings.leftCamSN.toStdString()},
            {"front", settings.frontCamSN.toStdString()}
        };

        // 枚举可用设备（用于验证序列号是否存在）
        auto availableDevices = MutiCam::Camera::CameraManager::enumerateDevices();

        if (availableDevices.empty()) {
            qDebug() << "No cameras found in system";
            statusBar()->showMessage("系统中未发现任何相机设备", 5000);
            return;
        }

        qDebug() << "Available camera devices:";
        for (const auto& device : availableDevices) {
            qDebug() << "  -" << QString::fromStdString(device);
        }

        // 根据参数设置中的序列号添加相机
        for (const auto& config : cameraConfigs) {
            const std::string& cameraId = config.first;
            const std::string& serialNumber = config.second;

            // 检查序列号是否为空
            if (serialNumber.empty()) {
                qDebug() << "Camera serial number not set for:" << cameraId.c_str();
                continue;
            }

            // 检查序列号是否在可用设备列表中
            bool deviceFound = std::find(availableDevices.begin(), availableDevices.end(), serialNumber) != availableDevices.end();

            if (!deviceFound) {
                qDebug() << "Camera with serial number" << serialNumber.c_str() << "not found for" << cameraId.c_str();
                QString statusMsg = QString("未找到序列号为 %1 的相机（%2视图）")
                                  .arg(QString::fromStdString(serialNumber))
                                  .arg(QString::fromStdString(cameraId));
                statusBar()->showMessage(statusMsg, 5000);
                continue;
            }

            // 尝试添加相机
            if (m_cameraManager->addCamera(cameraId, serialNumber)) {
                qDebug() << "Successfully added camera:" << cameraId.c_str() << "Serial:" << serialNumber.c_str();
            } else {
                qDebug() << "Failed to add camera:" << cameraId.c_str() << "Serial:" << serialNumber.c_str();
                QString statusMsg = QString("无法连接序列号为 %1 的相机（%2视图）")
                                  .arg(QString::fromStdString(serialNumber))
                                  .arg(QString::fromStdString(cameraId));
                statusBar()->showMessage(statusMsg, 5000);
            }
        }

    } catch (const std::exception& e) {
        qDebug() << "Error initializing camera system:" << e.what();
        QString statusMsg = QString("相机系统初始化失败: %1").arg(e.what());
        statusBar()->showMessage(statusMsg, 5000);
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
    // 连接基本控制按钮信号（不在映射表中的按钮）
    connect(ui->btnStartMeasure, &QPushButton::clicked,
            this, &MutiCamApp::onStartMeasureClicked);
    connect(ui->btnStopMeasure, &QPushButton::clicked,
            this, &MutiCamApp::onStopMeasureClicked);

    // 连接相机状态监控按钮
    connect(ui->btnClearAlerts, &QPushButton::clicked,
            this, &MutiCamApp::onClearAlertsClicked);
    connect(ui->btnRefreshStatus, &QPushButton::clicked,
            this, &MutiCamApp::onRefreshStatusClicked);

    // 连接载物台控制按钮
    connect(ui->btnMoveXLeft, &QPushButton::clicked,
            this, &MutiCamApp::onMoveXLeftClicked);
    connect(ui->btnMoveXRight, &QPushButton::clicked,
            this, &MutiCamApp::onMoveXRightClicked);
    connect(ui->btnMoveYUp, &QPushButton::clicked,
            this, &MutiCamApp::onMoveYUpClicked);
    connect(ui->btnMoveYDown, &QPushButton::clicked,
            this, &MutiCamApp::onMoveYDownClicked);
    connect(ui->btnMoveZUp, &QPushButton::clicked,
            this, &MutiCamApp::onMoveZUpClicked);
    connect(ui->btnMoveZDown, &QPushButton::clicked,
            this, &MutiCamApp::onMoveZDownClicked);
    connect(ui->btnStageHome, &QPushButton::clicked,
            this, &MutiCamApp::onStageHomeClicked);
    connect(ui->btnStageStop, &QPushButton::clicked,
            this, &MutiCamApp::onStageStopClicked);

    // 连接轨迹记录按钮
    connect(ui->btnTrajectoryStart, &QPushButton::clicked,
            this, &MutiCamApp::onTrajectoryStartClicked);
    connect(ui->btnTrajectoryStop, &QPushButton::clicked,
            this, &MutiCamApp::onTrajectoryStopClicked);
    connect(ui->btnTrajectoryClear, &QPushButton::clicked,
            this, &MutiCamApp::onTrajectoryClearClicked);
    connect(ui->btnTrajectoryExport, &QPushButton::clicked,
            this, &MutiCamApp::onTrajectoryExportClicked);

    // 使用新的按钮映射系统连接所有其他按钮
    connectButtonSignals();
    // 所有绘图、清空、撤销、保存按钮的连接已移至按钮映射系统

    // 连接网格相关信号（LineEdit信号不在按钮映射中）
    connect(ui->leGridDensity, &QLineEdit::textChanged,
            this, &MutiCamApp::onGridDensityChanged);
    connect(ui->leGridDensVertical, &QLineEdit::textChanged,
            this, &MutiCamApp::onGridDensityVerticalChanged);
    connect(ui->leGridDensLeft, &QLineEdit::textChanged,
            this, &MutiCamApp::onGridDensityLeftChanged);
    connect(ui->leGridDensFront, &QLineEdit::textChanged,
            this, &MutiCamApp::onGridDensityFrontChanged);

    // 网格取消按钮和自动检测按钮的连接已移至按钮映射系统

    // 连接选项卡切换信号
    connect(ui->tabWidget, &QTabWidget::currentChanged,
            this, &MutiCamApp::onTabChanged);

    // 连接参数输入框的实时保存信号
    connectSettingsSignals();

    // 注意：相机管理器信号将在相机系统初始化时连接
    
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
        connect(m_verticalPaintingOverlay, &PaintingOverlay::viewDoubleClicked,
                this, &MutiCamApp::onViewDoubleClicked);
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
        connect(m_leftPaintingOverlay, &PaintingOverlay::viewDoubleClicked,
                this, &MutiCamApp::onViewDoubleClicked);
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
        connect(m_frontPaintingOverlay, &PaintingOverlay::viewDoubleClicked,
                this, &MutiCamApp::onViewDoubleClicked);
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

    // 连接视图控制按钮信号
    connect(ui->btnResetZoomVertical, &QPushButton::clicked,
            this, &MutiCamApp::onResetZoomVertical);
    connect(ui->btnResetZoomLeft, &QPushButton::clicked,
            this, &MutiCamApp::onResetZoomLeft);
    connect(ui->btnResetZoomFront, &QPushButton::clicked,
            this, &MutiCamApp::onResetZoomFront);

    // 连接ZoomPanWidget的视图变换信号
    if (m_verticalZoomPanWidget2) {
        connect(m_verticalZoomPanWidget2, &ZoomPanWidget::viewTransformChanged,
                this, &MutiCamApp::onViewTransformChanged);
    }
    if (m_leftZoomPanWidget2) {
        connect(m_leftZoomPanWidget2, &ZoomPanWidget::viewTransformChanged,
                this, &MutiCamApp::onViewTransformChanged);
    }
    if (m_frontZoomPanWidget2) {
        connect(m_frontZoomPanWidget2, &ZoomPanWidget::viewTransformChanged,
                this, &MutiCamApp::onViewTransformChanged);
    }

    // 连接ZoomPanWidget的鼠标事件信号（如果需要在MutiCamApp中处理）
    // 注意：由于PaintingOverlay现在是ZoomPanWidget的子控件，
    // 大部分鼠标事件会直接被PaintingOverlay处理，这里的连接主要用于调试或特殊处理

    // 设置自动检测参数的默认值
    initializeDetectionParameters();
}

void MutiCamApp::onStartMeasureClicked()
{
    try {
        qDebug() << "Starting measurement...";

        // 记录测量开始操作
        if (m_logManager) {
            m_logManager->logMeasurementOperation("开始测量按钮被点击");
        }

        // 如果相机管理器未初始化，先初始化相机系统
        if (!m_cameraManager) {
            qDebug() << "相机管理器未初始化，正在初始化相机系统...";
            statusBar()->showMessage("正在连接相机...", 3000);

            // 记录相机初始化开始
            if (m_logManager) {
                m_logManager->logCameraOperation("开始初始化相机系统");
            }

            // 初始化相机系统
            initializeCameraSystem();

            // 连接相机管理器信号（必须在初始化后立即连接）
            if (m_cameraManager) {
                connectCameraManagerSignals();
                if (m_logManager) {
                    m_logManager->logCameraOperation("相机管理器信号连接完成");
                }
            }

            // 检查是否成功初始化
            if (!m_cameraManager) {
                QMessageBox::warning(this, "相机连接失败",
                                   "无法初始化相机系统，请检查：\n"
                                   "1. 相机设备是否正确连接\n"
                                   "2. 参数设置中的相机序列号是否正确\n"
                                   "3. 相机驱动是否正常安装");
                return;
            }
        }

        // 启动所有相机
        if (m_cameraManager->startAllCameras()) {
            m_isMeasuring = true;

            // 更新按钮状态
            ui->btnStartMeasure->setEnabled(false);
            ui->btnStopMeasure->setEnabled(true);

            // 更新状态栏或显示信息
            statusBar()->showMessage("测量已开始 - 图像采集中...");

            // 记录成功启动日志
            if (m_logManager) {
                m_logManager->logCameraOperation("所有相机启动成功，开始测量");
            }

            qDebug() << "Measurement started successfully";
        } else {
            // 记录启动失败日志
            if (m_logManager) {
                m_logManager->logError("相机启动失败", "无法启动相机采集");
            }

            QMessageBox::warning(this, "启动失败",
                               "无法启动相机采集，请检查：\n"
                               "1. 相机是否正确连接\n"
                               "2. 相机是否被其他程序占用\n"
                               "3. 参数设置中的序列号是否正确");
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

        // 记录停止测量操作
        if (m_logManager) {
            m_logManager->logMeasurementOperation("停止测量按钮被点击");
        }

        // 停止所有相机
        m_cameraManager->stopAllCameras();
        m_isMeasuring = false;

        // 更新按钮状态
        ui->btnStartMeasure->setEnabled(true);
        ui->btnStopMeasure->setEnabled(false);

        // 更新状态栏
        statusBar()->showMessage("测量已停止");

        // 记录成功停止日志
        if (m_logManager) {
            m_logManager->logMeasurementOperation("测量已成功停止");
        }

        qDebug() << "Measurement stopped successfully";

    } catch (const std::exception& e) {
        qDebug() << "Error stopping measurement:" << e.what();

        // 记录停止失败日志
        if (m_logManager) {
            m_logManager->logError("停止测量失败", e.what());
        }
    }
}

void MutiCamApp::onCameraFrameReady(const QString& cameraId, const cv::Mat& frame)
{
    static int frameCount = 0;
    frameCount++;
    if (frameCount % 30 == 0) { // 每30帧打印一次，避免日志过多
        qDebug() << "接收到相机帧：" << cameraId << "帧大小：" << frame.cols << "x" << frame.rows;
    }

    if (frame.empty()) return;

    // 更新帧率计算
    updateFrameRate(cameraId);

    // 存储当前帧供自动检测使用
    if (cameraId == "vertical") {
        m_currentFrameVertical = frame.clone();
    } else if (cameraId == "left") {
        m_currentFrameLeft = frame.clone();
    } else if (cameraId == "front") {
        m_currentFrameFront = frame.clone();
    }

    ZoomPanWidget* mainWidget = getZoomPanWidget(cameraId);
    ZoomPanWidget* tabWidget = nullptr;

    if (cameraId == "vertical") tabWidget = m_verticalZoomPanWidget2;
    else if (cameraId == "left") tabWidget = m_leftZoomPanWidget2;
    else if (cameraId == "front") tabWidget = m_frontZoomPanWidget2;

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
    QString alertLevel = "info";

    switch (state) {
        case MutiCam::Camera::CameraState::Disconnected:
            stateText = "断开连接";
            alertLevel = "warning";
            break;
        case MutiCam::Camera::CameraState::Connected:
            stateText = "已连接";
            alertLevel = "info";
            break;
        case MutiCam::Camera::CameraState::Streaming:
            stateText = "采集中";
            alertLevel = "info";
            break;
    }

    // 更新状态栏显示
    statusBar()->showMessage(QString("相机 %1: %2").arg(cameraId, stateText), 2000);

    // 添加状态变化报警
    QString alertMessage = QString("%1相机%2").arg(cameraId).arg(stateText);
    addAlertMessage(alertMessage, alertLevel);
}

void MutiCamApp::onCameraError(const QString& cameraId, const QString& error)
{
    qDebug() << "Camera error for" << cameraId << ":" << error;

    // 添加错误报警
    QString alertMessage = QString("%1相机错误: %2").arg(cameraId).arg(error);
    addAlertMessage(alertMessage, "error");

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

// 绘图功能槽函数已被通用方法替代，通过按钮映射系统调用

// setDrawingMode和exitDrawingMode方法已移除 - 现在直接通过按钮槽函数调用VideoDisplayWidget的startDrawing方法

// drawPointsOnImage 方法已迁移到 VideoDisplayWidget

void MutiCamApp::installMouseEventFilters()
{
    // 为主界面ZoomPanWidget安装事件过滤器并启用鼠标追踪
    if (m_verticalZoomPanWidget) {
        m_verticalZoomPanWidget->installEventFilter(this);
        m_verticalZoomPanWidget->setMouseTracking(true);
    }
    if (m_leftZoomPanWidget) {
        m_leftZoomPanWidget->installEventFilter(this);
        m_leftZoomPanWidget->setMouseTracking(true);
    }
    if (m_frontZoomPanWidget) {
        m_frontZoomPanWidget->installEventFilter(this);
        m_frontZoomPanWidget->setMouseTracking(true);
    }

    // 为选项卡ZoomPanWidget安装事件过滤器并启用鼠标追踪
    if (m_verticalZoomPanWidget2) {
        m_verticalZoomPanWidget2->installEventFilter(this);
        m_verticalZoomPanWidget2->setMouseTracking(true);
    }
    if (m_leftZoomPanWidget2) {
        m_leftZoomPanWidget2->installEventFilter(this);
        m_leftZoomPanWidget2->setMouseTracking(true);
    }
    if (m_frontZoomPanWidget2) {
        m_frontZoomPanWidget2->installEventFilter(this);
        m_frontZoomPanWidget2->setMouseTracking(true);
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

QString MutiCamApp::getViewName(ZoomPanWidget* widget)
{
    if (widget == m_verticalZoomPanWidget || widget == m_verticalZoomPanWidget2) {
        return "vertical";
    } else if (widget == m_leftZoomPanWidget || widget == m_leftZoomPanWidget2) {
        return "left";
    } else if (widget == m_frontZoomPanWidget || widget == m_frontZoomPanWidget2) {
        return "front";
    }
    return "";
}

// handleZoomPanWidgetClick方法已迁移到ZoomPanWidget中

// handleZoomPanWidgetMouseMove方法已迁移到ZoomPanWidget中

QPointF MutiCamApp::windowToImageCoordinates(ZoomPanWidget* widget, const QPoint& windowPos)
{
    // 使用ZoomPanWidget的坐标转换方法
    if (widget) {
        return widget->windowToImageCoordinates(windowPos);
    }

    return QPointF(windowPos);
}

// {{ AURA-X: Delete - 残留的鼠标移动处理方法. Approval: 寸止(ID:cleanup). }}
// handleMouseMove方法已删除，现在直接使用VideoDisplayWidget

void MutiCamApp::updateViewDisplay(const QString& viewName)
{
    // 获取对应的ZoomPanWidget和帧
    ZoomPanWidget* tabWidget = nullptr;
    cv::Mat* currentFrame = nullptr;

    if (viewName == "vertical") {
        tabWidget = m_verticalZoomPanWidget2;
        currentFrame = &m_currentFrameVertical;
    } else if (viewName == "left") {
        tabWidget = m_leftZoomPanWidget2;
        currentFrame = &m_currentFrameLeft;
    } else if (viewName == "front") {
        tabWidget = m_frontZoomPanWidget2;
        currentFrame = &m_currentFrameFront;
    }

    if (!tabWidget || !currentFrame || currentFrame->empty()) {
        // 静默返回，避免频繁的调试输出
        return;
    }
    
    // {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
    // displayImageWithHardwareAcceleration方法已迁移到ZoomPanWidget
    // 使用ZoomPanWidget更新主界面视图
    updateZoomPanWidget(viewName, *currentFrame);
    
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

    // 移除频繁的更新日志，避免控制台输出过多
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
    double scale = (std::min)(scaleX, scaleY);
    
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
        // 静默清理，避免频繁日志输出
    } else {
        // 清空指定视图的缓存
        m_frameCache.remove(viewName);
        m_renderBuffers.remove(viewName);  // 清理对应的渲染缓冲区
        // 静默清理，避免频繁日志输出
    }
}

// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// hasDrawingData方法已迁移到VideoDisplayWidget

// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// displayFrameWithHighDPI方法已迁移到VideoDisplayWidget

// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// displayImageOnLabel方法已迁移到VideoDisplayWidget

// 直线绘制槽函数已被通用方法替代

// 直线绘制相关方法实现
// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// handleLineDrawingClick方法已迁移到VideoDisplayWidget::handleLineDrawingClick

// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// drawLinesOnImage方法已迁移到VideoDisplayWidget

// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// drawSingleLine、drawDashedLine、calculateLineAngle方法已迁移到VideoDisplayWidget

// 圆形绘制槽函数已被通用方法替代

// 平行线和两线测量槽函数已被通用方法替代

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
            updateZoomPanWidget("vertical", m_currentFrameVertical);
        }
        if (!m_currentFrameLeft.empty()) {
            updateZoomPanWidget("left", m_currentFrameLeft);
        }
        if (!m_currentFrameFront.empty()) {
            updateZoomPanWidget("front", m_currentFrameFront);
        }
    } else if (index == 1) { // 垂直视图选项卡
        if (!m_currentFrameVertical.empty()) {
            // 使用ZoomPanWidget显示，自动包含绘画数据
            updateZoomPanWidget("vertical2", m_currentFrameVertical);
        }
    } else if (index == 2) { // 左视图选项卡
        if (!m_currentFrameLeft.empty()) {
            // 使用ZoomPanWidget显示，自动包含绘画数据
            updateZoomPanWidget("left2", m_currentFrameLeft);
        }
    } else if (index == 3) { // 前视图选项卡
        if (!m_currentFrameFront.empty()) {
            // 使用ZoomPanWidget显示，自动包含绘画数据
            updateZoomPanWidget("front2", m_currentFrameFront);
        }
    }

    qDebug() << "Tab changed to index:" << index;
}

void MutiCamApp::onViewDoubleClicked(const QString& viewName)
{
    // 根据视图名称跳转到对应的选项卡
    if (viewName == "Vertical") {
        ui->tabWidget->setCurrentIndex(1); // 垂直视图选项卡
        qDebug() << "双击垂直视图，跳转到选项卡1";
    } else if (viewName == "Left") {
        ui->tabWidget->setCurrentIndex(2); // 左侧视图选项卡
        qDebug() << "双击左侧视图，跳转到选项卡2";
    } else if (viewName == "Front") {
        ui->tabWidget->setCurrentIndex(3); // 对向视图选项卡
        qDebug() << "双击对向视图，跳转到选项卡3";
    }
}

// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// renderDrawingsOnFrame方法已迁移到VideoDisplayWidget

// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// drawParallelLinesOnImage方法已迁移到VideoDisplayWidget

// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// drawTwoLinesOnImage方法已迁移到VideoDisplayWidget

// ZoomPanWidget 相关函数实现
void MutiCamApp::initializeZoomPanWidgets()
{
    // 创建主界面显示控件（禁用缩放平移功能）
    m_verticalZoomPanWidget = new ZoomPanWidget(this);
    m_leftZoomPanWidget = new ZoomPanWidget(this);
    m_frontZoomPanWidget = new ZoomPanWidget(this);

    // 禁用主界面的缩放平移功能
    m_verticalZoomPanWidget->setZoomRange(1.0, 1.0); // 固定缩放为1.0
    m_leftZoomPanWidget->setZoomRange(1.0, 1.0);
    m_frontZoomPanWidget->setZoomRange(1.0, 1.0);

    // 禁用主界面的平移功能（空格+左键）
    m_verticalZoomPanWidget->setPanEnabled(false);
    m_leftZoomPanWidget->setPanEnabled(false);
    m_frontZoomPanWidget->setPanEnabled(false);

    // 创建选项卡ZoomPanWidget（启用缩放平移功能）
    m_verticalZoomPanWidget2 = new ZoomPanWidget(this);
    m_leftZoomPanWidget2 = new ZoomPanWidget(this);
    m_frontZoomPanWidget2 = new ZoomPanWidget(this);
    
    // 创建PaintingOverlay控件（作为ZoomPanWidget的子控件）
    m_verticalPaintingOverlay = new PaintingOverlay(m_verticalZoomPanWidget);
    m_leftPaintingOverlay = new PaintingOverlay(m_leftZoomPanWidget);
    m_frontPaintingOverlay = new PaintingOverlay(m_frontZoomPanWidget);

    // 创建选项卡PaintingOverlay
    m_verticalPaintingOverlay2 = new PaintingOverlay(m_verticalZoomPanWidget2);
    m_leftPaintingOverlay2 = new PaintingOverlay(m_leftZoomPanWidget2);
    m_frontPaintingOverlay2 = new PaintingOverlay(m_frontZoomPanWidget2);
    
    // 设置PaintingOverlay的视图名称（用于measurementCompleted信号）
    m_verticalPaintingOverlay->setViewName("Vertical");
    m_leftPaintingOverlay->setViewName("Left");
    m_frontPaintingOverlay->setViewName("Front");

    m_verticalPaintingOverlay2->setViewName("Vertical2");
    m_leftPaintingOverlay2->setViewName("Left2");
    m_frontPaintingOverlay2->setViewName("Front2");

    // 将PaintingOverlay关联到ZoomPanWidget
    m_verticalZoomPanWidget->setPaintingOverlay(m_verticalPaintingOverlay);
    m_leftZoomPanWidget->setPaintingOverlay(m_leftPaintingOverlay);
    m_frontZoomPanWidget->setPaintingOverlay(m_frontPaintingOverlay);

    m_verticalZoomPanWidget2->setPaintingOverlay(m_verticalPaintingOverlay2);
    m_leftZoomPanWidget2->setPaintingOverlay(m_leftPaintingOverlay2);
    m_frontZoomPanWidget2->setPaintingOverlay(m_frontPaintingOverlay2);
    
    // ZoomPanWidget已经内部管理VideoDisplayWidget，不需要复杂的叠加容器
    // PaintingOverlay将作为ZoomPanWidget的子控件进行管理
    
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
        qDebug() << "开始使用布局管理器替换控件为ZoomPanWidget";

        // 设置ZoomPanWidget的尺寸策略
        m_verticalZoomPanWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        m_leftZoomPanWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        m_frontZoomPanWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

        // 设置选项卡ZoomPanWidget的尺寸策略
        m_verticalZoomPanWidget2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        m_leftZoomPanWidget2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        m_frontZoomPanWidget2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        // 使用 replaceWidget 替换控件为ZoomPanWidget，这会自动处理布局管理
        QLayoutItem* verticalItem = verticalLayout->replaceWidget(ui->lbVerticalView, m_verticalZoomPanWidget);
        QLayoutItem* leftItem = leftLayout->replaceWidget(ui->lbLeftView, m_leftZoomPanWidget);
        QLayoutItem* frontItem = frontLayout->replaceWidget(ui->lbFrontView, m_frontZoomPanWidget);

        // 替换选项卡QLabel为ZoomPanWidget
        QLayoutItem* verticalItem2 = verticalLayout2->replaceWidget(ui->lbVerticalView2, m_verticalZoomPanWidget2);
        QLayoutItem* leftItem2 = leftLayout2->replaceWidget(ui->lbLeftView2, m_leftZoomPanWidget2);
        QLayoutItem* frontItem2 = frontLayout2->replaceWidget(ui->lbFrontView2, m_frontZoomPanWidget2);
        
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
        
        // 显示ZoomPanWidget
        m_verticalZoomPanWidget->show();
        m_leftZoomPanWidget->show();
        m_frontZoomPanWidget->show();

        // 显示选项卡ZoomPanWidget
        m_verticalZoomPanWidget2->show();
        m_leftZoomPanWidget2->show();
        m_frontZoomPanWidget2->show();

        // 注意：PaintingOverlay的信号连接已在前面的代码中完成，这里不需要重复连接

        qDebug() << "ZoomPanWidget已通过布局管理器正确替换 QLabel";
        qDebug() << "ZoomPanWidget几何信息:";
        qDebug() << "Vertical Widget:" << m_verticalZoomPanWidget->geometry();
        qDebug() << "Left Widget:" << m_leftZoomPanWidget->geometry();
        qDebug() << "Front Widget:" << m_frontZoomPanWidget->geometry();
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

            m_verticalZoomPanWidget->setParent(verticalParent);
            m_leftZoomPanWidget->setParent(leftParent);
            m_frontZoomPanWidget->setParent(frontParent);

            m_verticalZoomPanWidget->setGeometry(verticalGeometry);
            m_leftZoomPanWidget->setGeometry(leftGeometry);
            m_frontZoomPanWidget->setGeometry(frontGeometry);

            m_verticalZoomPanWidget->show();
            m_leftZoomPanWidget->show();
            m_frontZoomPanWidget->show();

            qDebug() << "使用回退方案完成ZoomPanWidget替换";
        }
    }
    
    qDebug() << "ZoomPanWidget 缩放平移显示控件初始化完成";
}

void MutiCamApp::updateZoomPanWidget(const QString& viewName, const cv::Mat& frame)
{
    ZoomPanWidget* widget = getZoomPanWidget(viewName);
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

ZoomPanWidget* MutiCamApp::getZoomPanWidget(const QString& viewName)
{
    if (viewName == "vertical") {
        return m_verticalZoomPanWidget;
    } else if (viewName == "left") {
        return m_leftZoomPanWidget;
    } else if (viewName == "front") {
        return m_frontZoomPanWidget;
    } else if (viewName == "vertical2") {
        return m_verticalZoomPanWidget2;
    } else if (viewName == "left2") {
        return m_leftZoomPanWidget2;
    } else if (viewName == "front2") {
        return m_frontZoomPanWidget2;
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

    // 只在找不到匹配视图时输出警告
    qDebug() << "Warning: 未找到匹配的视图名称：" << viewName;
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

// 清空、撤销绘图和撤销检测槽函数已被通用方法替代

// {{ AURA-X: Add - 保存图像按钮槽函数实现. Approval: 寸止(ID:save_image_buttons). }}
void MutiCamApp::onSaveImageClicked()
{
    // 主界面保存图像按钮 - 异步保存所有视图的图像
    qDebug() << "主界面保存图像按钮被点击 - 异步保存所有视图";

    // 记录保存图像操作
    if (m_logManager) {
        m_logManager->logUIOperation("主界面保存图像按钮被点击", "异步保存所有视图");
    }

    // 检查是否已有保存任务在进行
    if (m_saveWatcher && m_saveWatcher->isRunning()) {
        qDebug() << "保存任务正在进行中，忽略重复点击";
        if (m_logManager) {
            m_logManager->logWarning("保存任务重复点击", "保存任务正在进行中");
        }
        return;
    }

    // 异步保存所有三个视图的图像
    QStringList viewTypes = {"vertical", "left", "front"};
    saveImagesAsync(viewTypes);
}

void MutiCamApp::onSaveImageVerticalClicked()
{
    qDebug() << "垂直视图保存图像按钮被点击 - 异步保存";

    // 检查是否已有保存任务在进行
    if (m_saveWatcher && m_saveWatcher->isRunning()) {
        qDebug() << "保存任务正在进行中，忽略重复点击";
        return;
    }

    // 异步保存垂直视图
    QStringList viewTypes = {"vertical"};
    saveImagesAsync(viewTypes);
}

void MutiCamApp::onSaveImageLeftClicked()
{
    qDebug() << "左侧视图保存图像按钮被点击 - 异步保存";

    // 检查是否已有保存任务在进行
    if (m_saveWatcher && m_saveWatcher->isRunning()) {
        qDebug() << "保存任务正在进行中，忽略重复点击";
        return;
    }

    // 异步保存左侧视图
    QStringList viewTypes = {"left"};
    saveImagesAsync(viewTypes);
}

void MutiCamApp::onSaveImageFrontClicked()
{
    qDebug() << "对向视图保存图像按钮被点击 - 异步保存";

    // 检查是否已有保存任务在进行
    if (m_saveWatcher && m_saveWatcher->isRunning()) {
        qDebug() << "保存任务正在进行中，忽略重复点击";
        return;
    }

    // 异步保存对向视图
    QStringList viewTypes = {"front"};
    saveImagesAsync(viewTypes);
}

void MutiCamApp::saveImages(const QString& viewType)
{
    try {
        qDebug() << "开始保存" << viewType << "视图图像";

        // 输出OpenCV版本和编码器信息（只在第一次保存时输出）
        static bool infoShown = false;
        if (!infoShown) {
            qDebug() << "OpenCV版本：" << CV_VERSION;
            qDebug() << "OpenCV主版本：" << CV_MAJOR_VERSION;
            qDebug() << "OpenCV次版本：" << CV_MINOR_VERSION;

            // 检查JPEG编码器支持
            std::vector<int> params;
            params.push_back(cv::IMWRITE_JPEG_QUALITY);
            params.push_back(100);  // 100%质量，最高画质

            // 创建一个小的测试图像
            cv::Mat testImg = cv::Mat::zeros(10, 10, CV_8UC3);
            QString testJpgPath = "D:/opencv_test.jpg";
            bool jpegSupported = cv::imwrite(testJpgPath.toLocal8Bit().toStdString(), testImg, params);
            qDebug() << "JPEG编码器支持：" << (jpegSupported ? "是" : "否");
            if (jpegSupported && QFile::exists(testJpgPath)) {
                QFile::remove(testJpgPath);
            }

            infoShown = true;
        }

        // 获取对应视图的绘制覆盖层
        PaintingOverlay* overlay = nullptr;
        QString viewName;

        // 【关键修复】主界面保存应该使用主界面的overlay，不是标签页的overlay
        if (viewType == "vertical") {
            overlay = m_verticalPaintingOverlay;  // 使用主界面的overlay
            viewName = "垂直";
            qDebug() << "使用主界面垂直视图overlay：" << overlay;
        } else if (viewType == "left") {
            overlay = m_leftPaintingOverlay;      // 使用主界面的overlay
            viewName = "左侧";
            qDebug() << "使用主界面左侧视图overlay：" << overlay;
        } else if (viewType == "front") {
            overlay = m_frontPaintingOverlay;     // 使用主界面的overlay
            viewName = "对向";
            qDebug() << "使用主界面对向视图overlay：" << overlay;
        } else {
            qDebug() << "未知的视图类型：" << viewType;
            return;
        }

        // 获取当前帧
        cv::Mat currentFrame = getCurrentFrame(viewType);
        qDebug() << "获取到的帧信息：";
        qDebug() << "  - 是否为空：" << (currentFrame.empty() ? "是" : "否");
        if (!currentFrame.empty()) {
            qDebug() << "  - 尺寸：" << currentFrame.cols << "x" << currentFrame.rows;
            qDebug() << "  - 通道数：" << currentFrame.channels();
            qDebug() << "  - 数据类型：" << currentFrame.type();
        }

        if (currentFrame.empty()) {
            QString errorMsg = QString("%1视图没有可用的图像帧").arg(viewName);
            qDebug() << errorMsg;
            qDebug() << "可能的原因：";
            qDebug() << "  1. 相机未启动或未连接";
            qDebug() << "  2. 相机数据流中断";
            qDebug() << "  3. onCameraFrameReady未被调用";
            QMessageBox::warning(this, "保存失败", errorMsg + "\n\n请检查相机是否正常工作。");
            return;
        }

        // 创建保存目录
        QString saveDir = createSaveDirectory(viewName);
        if (saveDir.isEmpty()) {
            QString errorMsg = "创建保存目录失败";
            qDebug() << errorMsg;
            QMessageBox::warning(this, "保存失败", errorMsg);
            return;
        }

        // 获取用户配置的图像格式和质量
        QString imageFormat = ui->comboBoxCaptureFormat->currentText().toLower();
        QString qualitySetting = ui->comboBoxImageQuality->currentText();
        QString fileExtension = QString(".%1").arg(imageFormat);

        // 生成文件名
        QString currentTime = QDateTime::currentDateTime().toString("hh-mm-ss");
        QString originFileName = QString("%1_origin%2").arg(currentTime).arg(fileExtension);
        QString visualFileName = QString("%1_visual%2").arg(currentTime).arg(fileExtension);
        QString originPath = QDir(saveDir).filePath(originFileName);
        QString visualPath = QDir(saveDir).filePath(visualFileName);

        // 保存原始图像
        qDebug() << "尝试保存原始图像：";
        qDebug() << "  - 文件路径：" << originPath;
        qDebug() << "  - 图像尺寸：" << currentFrame.cols << "x" << currentFrame.rows;
        qDebug() << "  - 图像格式：" << currentFrame.type();
        qDebug() << "  - 数据是否连续：" << (currentFrame.isContinuous() ? "是" : "否");
        qDebug() << "  - 步长：" << currentFrame.step;

        // 检查目录是否存在
        QFileInfo fileInfo(originPath);
        QDir parentDir = fileInfo.absoluteDir();
        if (!parentDir.exists()) {
            qDebug() << "错误：父目录不存在：" << parentDir.absolutePath();
        } else {
            qDebug() << "父目录存在：" << parentDir.absolutePath();
        }

        // 检查文件路径长度
        if (originPath.length() > 260) {
            qDebug() << "警告：文件路径可能过长：" << originPath.length() << "字符";
        }

        // 确保图像数据是连续的
        cv::Mat frameToSave;
        if (currentFrame.isContinuous()) {
            frameToSave = currentFrame;
            qDebug() << "图像数据已连续，直接保存";
        } else {
            frameToSave = currentFrame.clone();
            qDebug() << "图像数据不连续，创建连续副本";
        }

        // 添加OpenCV保存诊断
        qDebug() << "OpenCV保存诊断：";
        qDebug() << "  - 图像类型：" << frameToSave.type() << "(期望：16=CV_8UC3, 0=CV_8UC1)";
        qDebug() << "  - 图像深度：" << frameToSave.depth() << "(期望：0=CV_8U)";
        qDebug() << "  - 图像通道：" << frameToSave.channels();
        qDebug() << "  - 图像为空：" << (frameToSave.empty() ? "是" : "否");
        qDebug() << "  - 数据指针：" << (frameToSave.data ? "有效" : "无效");

        // 尝试使用英文路径测试OpenCV
        QString testPath = QString("D:/test_%1.jpg").arg(QDateTime::currentDateTime().toString("hhmmss"));
        qDebug() << "  - 测试英文路径：" << testPath;
        bool testResult = cv::imwrite(testPath.toLocal8Bit().toStdString(), frameToSave);
        qDebug() << "  - 英文路径保存结果：" << (testResult ? "成功" : "失败");
        if (testResult && QFile::exists(testPath)) {
            QFile::remove(testPath); // 清理测试文件
            qDebug() << "  - 确认：OpenCV功能正常，问题可能是中文路径";
        }

        // 测试不同的路径编码方式
        qDebug() << "  - 测试路径编码方式：";
        qDebug() << "    * toStdString():" << QString::fromStdString(originPath.toStdString());
        qDebug() << "    * toLocal8Bit():" << QString::fromLocal8Bit(originPath.toLocal8Bit());
        qDebug() << "    * toUtf8():" << QString::fromUtf8(originPath.toUtf8());

        // 现在主要保存方法已经使用Local8Bit编码，应该能成功
        qDebug() << "  - 注意：主要保存方法现在使用Local8Bit编码";

        // 根据图像格式和质量设置压缩参数
        std::vector<int> compression_params;

        // 根据质量设置确定参数
        int jpegQuality = 100;  // 默认最高质量
        int pngCompression = 0; // 默认无压缩

        if (qualitySetting == "无损最高质量") {
            jpegQuality = 100;
            pngCompression = 0;
        } else if (qualitySetting == "高质量") {
            jpegQuality = 95;
            pngCompression = 1;
        } else if (qualitySetting == "标准质量") {
            jpegQuality = 85;
            pngCompression = 3;
        }

        if (imageFormat == "jpg" || imageFormat == "jpeg") {
            compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
            compression_params.push_back(jpegQuality);
            qDebug() << "使用OpenCV保存JPEG，质量：" << jpegQuality << "%";
            if (jpegQuality == 100) {
                qDebug() << "注意：JPEG即使100%质量仍是有损压缩，建议使用PNG获得真正无损质量";
            }
        } else if (imageFormat == "png") {
            compression_params.push_back(cv::IMWRITE_PNG_COMPRESSION);
            compression_params.push_back(pngCompression);
            qDebug() << "使用OpenCV保存PNG，压缩级别：" << pngCompression << "（PNG无损格式）";
        } else if (imageFormat == "bmp") {
            // BMP格式不需要压缩参数，完全无损
            qDebug() << "使用OpenCV保存BMP，完全无损无压缩";
        } else {
            // 默认使用JPEG参数
            compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
            compression_params.push_back(jpegQuality);
            qDebug() << "未知格式，使用默认JPEG参数，质量：" << jpegQuality << "%";
        }

        bool saveResult = cv::imwrite(originPath.toLocal8Bit().toStdString(), frameToSave, compression_params);
        if (saveResult) {
            qDebug() << "原始图像保存成功：" << originPath;

            // 验证文件是否真的存在
            if (QFile::exists(originPath)) {
                QFileInfo savedFile(originPath);
                qDebug() << "文件确认存在，大小：" << savedFile.size() << "字节";
            } else {
                qDebug() << "警告：cv::imwrite返回成功但文件不存在";
            }
        } else {
            qDebug() << "原始图像保存失败：" << originPath;
            qDebug() << "可能的原因：";
            qDebug() << "  1. 磁盘空间不足";
            qDebug() << "  2. 文件路径包含非法字符";
            qDebug() << "  3. 权限不足";
            qDebug() << "  4. 图像数据格式不支持";

            // 尝试备用保存方法：使用QImage保存
            qDebug() << "尝试使用QImage备用保存方法...";
            try {
                QImage qimg;
                if (frameToSave.channels() == 3) {
                    cv::Mat rgbFrame;
                    cv::cvtColor(frameToSave, rgbFrame, cv::COLOR_BGR2RGB);
                    // 使用copy构造，避免引用临时数据
                    qimg = QImage(rgbFrame.data, rgbFrame.cols, rgbFrame.rows,
                                 rgbFrame.step, QImage::Format_RGB888).copy();
                } else if (frameToSave.channels() == 1) {
                    // 使用copy构造，避免引用临时数据
                    qimg = QImage(frameToSave.data, frameToSave.cols, frameToSave.rows,
                                 frameToSave.step, QImage::Format_Grayscale8).copy();
                }

                // 根据格式设置QImage保存参数
                QString qimageFormat = imageFormat.toUpper();
                int quality = -1;  // 默认质量
                if (qimageFormat == "JPG") qimageFormat = "JPEG";
                if (qimageFormat == "JPEG") quality = jpegQuality;  // 使用相同的JPEG质量

                qDebug() << "使用QImage备用保存，格式：" << qimageFormat << "质量：" << quality;
                if (!qimg.isNull() && qimg.save(originPath, qimageFormat.toLocal8Bit().constData(), quality)) {
                    qDebug() << "使用QImage备用方法保存成功：" << originPath;
                } else {
                    qDebug() << "QImage备用方法也保存失败";
                }
            } catch (const std::exception& e) {
                qDebug() << "QImage备用保存方法出错：" << e.what();
            } catch (...) {
                qDebug() << "QImage备用保存方法出现未知错误";
            }
        }

        // 保存可视化图像
        if (overlay) {
            qDebug() << "开始渲染可视化图像...";
            cv::Mat visualFrame = renderVisualizedImage(currentFrame, overlay);

            qDebug() << "可视化图像渲染结果：";
            qDebug() << "  - 是否为空：" << (visualFrame.empty() ? "是" : "否");
            if (!visualFrame.empty()) {
                qDebug() << "  - 尺寸：" << visualFrame.cols << "x" << visualFrame.rows;
                qDebug() << "  - 通道数：" << visualFrame.channels();
                qDebug() << "  - 数据是否连续：" << (visualFrame.isContinuous() ? "是" : "否");

                // 确保可视化图像数据是连续的
                cv::Mat visualFrameToSave;
                if (visualFrame.isContinuous()) {
                    visualFrameToSave = visualFrame;
                    qDebug() << "可视化图像数据已连续，直接保存";
                } else {
                    visualFrameToSave = visualFrame.clone();
                    qDebug() << "可视化图像数据不连续，创建连续副本";
                }

                // 添加可视化图像OpenCV保存诊断
                qDebug() << "可视化图像OpenCV保存诊断：";
                qDebug() << "  - 图像类型：" << visualFrameToSave.type();
                qDebug() << "  - 图像深度：" << visualFrameToSave.depth();
                qDebug() << "  - 图像通道：" << visualFrameToSave.channels();
                qDebug() << "  - 图像为空：" << (visualFrameToSave.empty() ? "是" : "否");

                // 使用与原始图像相同的压缩参数
                std::vector<int> visual_compression_params = compression_params;

                bool visualSaveResult = cv::imwrite(visualPath.toLocal8Bit().toStdString(), visualFrameToSave, visual_compression_params);
                if (visualSaveResult) {
                    qDebug() << "可视化图像保存成功：" << visualPath;

                    // 验证文件是否真的存在
                    if (QFile::exists(visualPath)) {
                        QFileInfo savedFile(visualPath);
                        qDebug() << "可视化文件确认存在，大小：" << savedFile.size() << "字节";
                    } else {
                        qDebug() << "警告：可视化图像cv::imwrite返回成功但文件不存在";
                    }
                } else {
                    qDebug() << "可视化图像保存失败：" << visualPath;

                    // 尝试备用保存方法：使用QImage保存
                    qDebug() << "尝试使用QImage备用保存可视化图像...";
                    try {
                        QImage qimg;
                        if (visualFrameToSave.channels() == 3) {
                            cv::Mat rgbFrame;
                            cv::cvtColor(visualFrameToSave, rgbFrame, cv::COLOR_BGR2RGB);
                            // 使用copy构造，避免引用临时数据
                            qimg = QImage(rgbFrame.data, rgbFrame.cols, rgbFrame.rows,
                                         rgbFrame.step, QImage::Format_RGB888).copy();
                        } else if (visualFrameToSave.channels() == 1) {
                            // 使用copy构造，避免引用临时数据
                            qimg = QImage(visualFrameToSave.data, visualFrameToSave.cols, visualFrameToSave.rows,
                                         visualFrameToSave.step, QImage::Format_Grayscale8).copy();
                        }

                        // 使用与原始图像相同的格式和质量参数
                        QString qimageFormat = imageFormat.toUpper();
                        int quality = -1;
                        if (qimageFormat == "JPG") qimageFormat = "JPEG";
                        if (qimageFormat == "JPEG") quality = jpegQuality;

                        if (!qimg.isNull() && qimg.save(visualPath, qimageFormat.toLocal8Bit().constData(), quality)) {
                            qDebug() << "使用QImage备用方法保存可视化图像成功：" << visualPath;
                        } else {
                            qDebug() << "QImage备用方法保存可视化图像也失败";
                        }
                    } catch (const std::exception& e) {
                        qDebug() << "QImage备用保存可视化图像出错：" << e.what();
                    } catch (...) {
                        qDebug() << "QImage备用保存可视化图像出现未知错误";
                    }
                }
            } else {
                qDebug() << "可视化图像渲染失败，跳过保存";
            }
        } else {
            qDebug() << "警告：overlay为空，无法渲染可视化图像";
        }

        // 更新状态栏
        statusBar()->showMessage(QString("%1视图图像保存完成").arg(viewName), 2000);

    } catch (const std::exception& e) {
        QString errorMsg = QString("保存图像时出错：%1").arg(e.what());
        qDebug() << errorMsg;
        QMessageBox::critical(this, "保存错误", errorMsg);
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

// {{ AURA-X: Add - 获取当前活动ZoomPanWidget辅助函数. Approval: 寸止(ID:active_widget_helper). }}
ZoomPanWidget* MutiCamApp::getActiveZoomPanWidget()
{
    int currentTab = ui->tabWidget->currentIndex();

    // 根据当前选项卡返回对应的ZoomPanWidget
    switch (currentTab) {
        case 0: // 主界面
            // 可以根据需要返回主界面的某个默认widget，这里返回垂直视图
            return m_verticalZoomPanWidget;
        case 1: // 垂直视图选项卡
            return m_verticalZoomPanWidget2;
        case 2: // 左视图选项卡
            return m_leftZoomPanWidget2;
        case 3: // 前视图选项卡
            return m_frontZoomPanWidget2;
        default:
            return m_verticalZoomPanWidget; // 默认返回垂直视图
    }
}

void MutiCamApp::syncOverlayTransforms(const QString& viewName)
{
    ZoomPanWidget* zoomPanWidget = getZoomPanWidget(viewName);
    PaintingOverlay* paintingOverlay = getPaintingOverlay(viewName);

    if (zoomPanWidget && paintingOverlay) {
        // 从缩放平移控件获取当前的偏移和缩放比例
        QPointF offset = zoomPanWidget->getImageOffset();
        double scale = zoomPanWidget->getScaleFactor();
        QSize imageSize = zoomPanWidget->getImageSize();
        
        // 将这些变换信息设置到绘画层
        paintingOverlay->setTransforms(offset, scale, imageSize);
    }
}

cv::Mat MutiCamApp::getCurrentFrame(const QString& viewName) const
{
    // 移除视图名称中的数字后缀（如"vertical2" -> "vertical"）
    QString baseName = viewName;
    if (baseName.endsWith("2")) {
        baseName.chop(1);
    }

    // 转换为小写进行比较
    QString lowerBaseName = baseName.toLower();

    if (lowerBaseName == "vertical") {
        return m_currentFrameVertical;
    } else if (lowerBaseName == "left") {
        return m_currentFrameLeft;
    } else if (lowerBaseName == "front") {
        return m_currentFrameFront;
    }

    // 只在找不到匹配视图时输出警告
    qDebug() << "Warning: 未找到匹配的视图名称：" << viewName;
    return cv::Mat(); // 返回空Mat
}

// 自动检测槽函数实现
void MutiCamApp::onAutoLineDetectionClicked()
{
    // 主界面的直线检测按钮 - 同时在所有视图中启用检测模式
    qDebug() << "主界面直线检测按钮被点击 - 启用所有视图";

    // 在所有视图中启用直线检测模式
    startAutoDetection("vertical", PaintingOverlay::DrawingTool::ROI_LineDetect);
    startAutoDetection("left", PaintingOverlay::DrawingTool::ROI_LineDetect);
    startAutoDetection("front", PaintingOverlay::DrawingTool::ROI_LineDetect);

    // 更新状态栏提示
    statusBar()->showMessage("直线检测模式已启用 - 请在任意视图中框选ROI区域", 5000);
}

void MutiCamApp::onAutoCircleDetectionClicked()
{
    // 主界面的圆形检测按钮 - 同时在所有视图中启用检测模式
    qDebug() << "主界面圆形检测按钮被点击 - 启用所有视图";

    // 在所有视图中启用圆形检测模式
    startAutoDetection("vertical", PaintingOverlay::DrawingTool::ROI_CircleDetect);
    startAutoDetection("left", PaintingOverlay::DrawingTool::ROI_CircleDetect);
    startAutoDetection("front", PaintingOverlay::DrawingTool::ROI_CircleDetect);

    // 更新状态栏提示
    statusBar()->showMessage("圆形检测模式已启用 - 请在任意视图中框选ROI区域", 5000);
}

void MutiCamApp::onAutoLineDetectionVerticalClicked()
{
    startAutoDetection("vertical2", PaintingOverlay::DrawingTool::ROI_LineDetect);
}

void MutiCamApp::onAutoCircleDetectionVerticalClicked()
{
    startAutoDetection("vertical2", PaintingOverlay::DrawingTool::ROI_CircleDetect);
}

void MutiCamApp::onAutoLineDetectionLeftClicked()
{
    qDebug() << "左侧视图直线检测按钮被点击";
    startAutoDetection("left2", PaintingOverlay::DrawingTool::ROI_LineDetect);
}

void MutiCamApp::onAutoCircleDetectionLeftClicked()
{
    qDebug() << "左侧视图圆形检测按钮被点击";
    startAutoDetection("left2", PaintingOverlay::DrawingTool::ROI_CircleDetect);
}

void MutiCamApp::onAutoLineDetectionFrontClicked()
{
    qDebug() << "对向视图直线检测按钮被点击";
    startAutoDetection("front2", PaintingOverlay::DrawingTool::ROI_LineDetect);
}

void MutiCamApp::onAutoCircleDetectionFrontClicked()
{
    qDebug() << "对向视图圆形检测按钮被点击";
    startAutoDetection("front2", PaintingOverlay::DrawingTool::ROI_CircleDetect);
}

void MutiCamApp::startAutoDetection(const QString& viewName, PaintingOverlay::DrawingTool detectionType)
{
    // 确定目标视图
    QString targetView = viewName;
    if (targetView.isEmpty()) {
        // 如果没有指定视图，使用当前活动的视图
        targetView = getCurrentActiveViewName();
    }

    // 获取对应的PaintingOverlay
    PaintingOverlay* overlay = getPaintingOverlay(targetView);
    if (!overlay) {
        qDebug() << "无法获取视图的PaintingOverlay：" << targetView;
        QMessageBox::warning(this, "错误", QString("无法获取视图 %1 的PaintingOverlay").arg(targetView));
        return;
    }
    qDebug() << "成功获取PaintingOverlay：" << targetView;

    // 检查是否有当前帧
    cv::Mat currentFrame = getCurrentFrame(targetView);
    if (currentFrame.empty()) {
        qDebug() << "当前视图没有图像帧：" << targetView;
        QMessageBox::warning(this, "自动检测",
                            QString("当前视图 %1 没有图像，请先启动相机").arg(targetView));
        return;
    }
    qDebug() << "当前帧尺寸：" << currentFrame.cols << "x" << currentFrame.rows;

    // 配置检测参数
    configureDetectionParameters(overlay, detectionType);

    // 启动ROI绘制模式
    overlay->startDrawing(detectionType);

    // 给用户提示
    QString detectionTypeName = (detectionType == PaintingOverlay::DrawingTool::ROI_LineDetect) ? "直线" : "圆形";
    QString message = QString("请在 %1 视图中框选ROI区域进行%2检测").arg(targetView).arg(detectionTypeName);

    // 更新状态栏
    statusBar()->showMessage(message, 5000);

    qDebug() << "启动自动检测：" << targetView << detectionTypeName;
}

QString MutiCamApp::getCurrentActiveViewName() const
{
    // 根据当前Tab页确定活动视图
    int currentIndex = ui->tabWidget->currentIndex();
    switch (currentIndex) {
        case 0: // 主界面
            // 在主界面中，需要确定哪个视图是活动的
            // 这里简化处理，返回垂直视图
            return "vertical";
        case 1: // 垂直视图Tab
            return "vertical2";
        case 2: // 左侧视图Tab
            return "left2";
        case 3: // 对向视图Tab
            return "front2";
        default:
            return "vertical";
    }
}

EdgeDetector::EdgeDetectionParams MutiCamApp::getEdgeDetectionParams() const
{
    EdgeDetector::EdgeDetectionParams params;

    // 从UI获取Canny边缘检测参数
    bool ok;
    double lowThreshold = ui->ledCannyLineLow->text().toDouble(&ok);
    if (ok && lowThreshold >= 0 && lowThreshold <= 255) {
        params.lowThreshold = lowThreshold;
    } else {
        qDebug() << "无效的Canny低阈值，使用默认值50";
        params.lowThreshold = 50.0;
    }

    double highThreshold = ui->ledCannyLineHigh->text().toDouble(&ok);
    if (ok && highThreshold >= 0 && highThreshold <= 255) {
        params.highThreshold = highThreshold;
    } else {
        qDebug() << "无效的Canny高阈值，使用默认值150";
        params.highThreshold = 150.0;
    }

    // 确保低阈值小于高阈值
    if (params.lowThreshold >= params.highThreshold) {
        qDebug() << "Canny阈值关系错误，自动调整";
        params.lowThreshold = params.highThreshold * 0.5;
    }

    return params;
}

ShapeDetector::LineDetectionParams MutiCamApp::getLineDetectionParams() const
{
    ShapeDetector::LineDetectionParams params;

    // 从UI获取直线检测参数
    bool ok;
    int threshold = ui->ledLineDetThreshold->text().toInt(&ok);
    if (ok && threshold > 0 && threshold <= 1000) {
        params.threshold = threshold;
    } else {
        qDebug() << "无效的直线检测阈值，使用默认值50";
        params.threshold = 50;
    }

    double minLineLength = ui->ledLineDetMinLength->text().toDouble(&ok);
    if (ok && minLineLength >= 0 && minLineLength <= 1000) {
        params.minLineLength = minLineLength;
    } else {
        qDebug() << "无效的最小线段长度，使用默认值50";
        params.minLineLength = 50.0;
    }

    double maxLineGap = ui->ledLineDetMaxGap->text().toDouble(&ok);
    if (ok && maxLineGap >= 0 && maxLineGap <= 100) {
        params.maxLineGap = maxLineGap;
    } else {
        qDebug() << "无效的最大线段间隙，使用默认值10";
        params.maxLineGap = 10.0;
    }

    return params;
}

ShapeDetector::CircleDetectionParams MutiCamApp::getCircleDetectionParams() const
{
    ShapeDetector::CircleDetectionParams params;

    // 从UI获取圆形检测参数
    bool ok;
    double param1 = ui->ledCannyCircleHigh->text().toDouble(&ok);
    if (ok && param1 > 0 && param1 <= 500) {
        params.param1 = param1;
    } else {
        qDebug() << "无效的圆形检测Param1，使用默认值200";
        params.param1 = 200.0;
    }

    double param2 = ui->ledCircleDetParam2->text().toDouble(&ok);
    if (ok && param2 > 0 && param2 <= 200) {
        params.param2 = param2;
    } else {
        qDebug() << "无效的圆形检测Param2，使用默认值50";
        params.param2 = 50.0;
    }

    return params;
}

void MutiCamApp::configureDetectionParameters(PaintingOverlay* overlay, PaintingOverlay::DrawingTool detectionType)
{
    if (!overlay) return;

    // 获取EdgeDetector和ShapeDetector（通过PaintingOverlay的公共接口）
    // 由于PaintingOverlay的图像处理器是私有的，我们需要添加公共接口
    // 这里先记录参数，实际应用需要在PaintingOverlay中添加参数设置接口

    EdgeDetector::EdgeDetectionParams edgeParams = getEdgeDetectionParams();
    qDebug() << "边缘检测参数 - 低阈值:" << edgeParams.lowThreshold
             << "高阈值:" << edgeParams.highThreshold;

    if (detectionType == PaintingOverlay::DrawingTool::ROI_LineDetect) {
        ShapeDetector::LineDetectionParams lineParams = getLineDetectionParams();
        qDebug() << "直线检测参数 - 阈值:" << lineParams.threshold
                 << "最小长度:" << lineParams.minLineLength
                 << "最大间隙:" << lineParams.maxLineGap;
    } else if (detectionType == PaintingOverlay::DrawingTool::ROI_CircleDetect) {
        ShapeDetector::CircleDetectionParams circleParams = getCircleDetectionParams();
        qDebug() << "圆形检测参数 - Param1:" << circleParams.param1
                 << "Param2:" << circleParams.param2;
    }

    // 应用参数设置
    overlay->setEdgeDetectionParams(edgeParams);

    if (detectionType == PaintingOverlay::DrawingTool::ROI_LineDetect) {
        ShapeDetector::LineDetectionParams lineParams = getLineDetectionParams();
        overlay->setLineDetectionParams(lineParams);
    } else if (detectionType == PaintingOverlay::DrawingTool::ROI_CircleDetect) {
        ShapeDetector::CircleDetectionParams circleParams = getCircleDetectionParams();
        overlay->setCircleDetectionParams(circleParams);
    }
}

void MutiCamApp::initializeDetectionParameters()
{
    // 设置边缘检测默认参数
    ui->ledCannyLineLow->setText("50");      // Canny低阈值
    ui->ledCannyLineHigh->setText("150");    // Canny高阈值

    // 设置直线检测默认参数
    ui->ledLineDetThreshold->setText("50");     // 霍夫变换阈值
    ui->ledLineDetMinLength->setText("50");     // 最小线段长度
    ui->ledLineDetMaxGap->setText("10");        // 最大线段间隙

    // 设置圆形检测默认参数
    ui->ledCannyCircleHigh->setText("200");     // 圆形检测Canny高阈值
    ui->ledCircleDetParam2->setText("50");      // 圆形检测累加器阈值

    qDebug() << "自动检测参数默认值已设置";
}

// {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
// displayImageWithHardwareAcceleration方法已迁移到VideoDisplayWidget

// 类型转换函数已移除，VideoDisplayWidget现在使用自己的数据类型

QString MutiCamApp::createSaveDirectory(const QString& viewName)
{
    try {
        // 基础路径
        QString basePath = "D:/CamImage";
        QString today = QDateTime::currentDateTime().toString("yyyy.MM.dd");

        // 创建目录路径：D:/CamImage/年.月.日/视图名称/
        QString dateDir = QDir(basePath).filePath(today);
        QString viewDir = QDir(dateDir).filePath(viewName);

        qDebug() << "创建保存目录：" << viewDir;

        // 确保目录存在
        QDir dir;
        if (!dir.mkpath(viewDir)) {
            qDebug() << "创建保存目录失败：" << viewDir;
            return QString();
        }

        qDebug() << "保存目录创建成功：" << viewDir;
        return viewDir;

    } catch (const std::exception& e) {
        qDebug() << "创建保存目录时出错：" << e.what();
        return QString();
    }
}

cv::Mat MutiCamApp::renderVisualizedImage(const cv::Mat& originalFrame, PaintingOverlay* overlay)
{
    qDebug() << "renderVisualizedImage 开始";
    qDebug() << "  - 原始图像是否为空：" << (originalFrame.empty() ? "是" : "否");
    qDebug() << "  - overlay是否为空：" << (overlay ? "否" : "是");

    if (originalFrame.empty()) {
        qDebug() << "错误：原始图像为空";
        return cv::Mat();
    }

    if (!overlay) {
        qDebug() << "错误：overlay为空";
        return cv::Mat();
    }

    try {
        qDebug() << "开始创建可视化图像...";
        // 创建原始图像的副本作为基础
        cv::Mat visualFrame = originalFrame.clone();
        qDebug() << "原始图像副本创建成功，尺寸：" << visualFrame.cols << "x" << visualFrame.rows;

        // 创建一个QImage用于绘制
        qDebug() << "开始创建QImage，通道数：" << visualFrame.channels();
        QImage qimg;
        if (visualFrame.channels() == 3) {
            qDebug() << "处理3通道图像（BGR转RGB）";
            // BGR转RGB
            cv::cvtColor(visualFrame, visualFrame, cv::COLOR_BGR2RGB);
            // 使用copy构造，确保数据安全
            qimg = QImage(visualFrame.data, visualFrame.cols, visualFrame.rows,
                         visualFrame.step, QImage::Format_RGB888).copy();
        } else if (visualFrame.channels() == 1) {
            qDebug() << "处理1通道图像（灰度）";
            // 使用copy构造，确保数据安全
            qimg = QImage(visualFrame.data, visualFrame.cols, visualFrame.rows,
                         visualFrame.step, QImage::Format_Grayscale8).copy();
        } else {
            qDebug() << "错误：不支持的图像格式，通道数：" << visualFrame.channels();
            return cv::Mat();
        }

        qDebug() << "QImage创建成功，尺寸：" << qimg.width() << "x" << qimg.height();

        // 创建QPainter在QImage上绘制
        QPainter painter(&qimg);
        painter.setRenderHint(QPainter::Antialiasing, true);

        // 让overlay在painter上绘制所有图形
        qDebug() << "调用overlay->renderToImage()";
        overlay->renderToImage(painter, qimg.size());

        painter.end();
        qDebug() << "QPainter绘制完成";

        // 将QImage转换回cv::Mat
        qDebug() << "开始将QImage转换回cv::Mat";
        cv::Mat result;
        try {
            if (qimg.format() == QImage::Format_RGB888) {
                qDebug() << "转换RGB888格式";
                // 确保使用clone()创建独立的数据副本
                result = cv::Mat(qimg.height(), qimg.width(), CV_8UC3,
                               (void*)qimg.constBits(), qimg.bytesPerLine()).clone();
                cv::cvtColor(result, result, cv::COLOR_RGB2BGR);
            } else if (qimg.format() == QImage::Format_Grayscale8) {
                qDebug() << "转换灰度格式";
                // 确保使用clone()创建独立的数据副本
                result = cv::Mat(qimg.height(), qimg.width(), CV_8UC1,
                               (void*)qimg.constBits(), qimg.bytesPerLine()).clone();
            } else {
                qDebug() << "错误：不支持的QImage格式：" << qimg.format();
                return cv::Mat();
            }

            qDebug() << "可视化图像渲染完成，结果尺寸：" << result.cols << "x" << result.rows;
            return result;
        } catch (const std::exception& e) {
            qDebug() << "QImage转cv::Mat时出错：" << e.what();
            return cv::Mat();
        }

    } catch (const std::exception& e) {
        qDebug() << "渲染可视化图像时出错：" << e.what();
        return cv::Mat();
    }
}

// {{ AURA-X: Add - 异步保存图像方法实现. Approval: 寸止(ID:async_save). }}
void MutiCamApp::saveImagesAsync(const QStringList& viewTypes)
{
    // 创建进度对话框
    QString progressText = viewTypes.size() > 1 ?
        QString("正在保存 %1 个视图的图像...").arg(viewTypes.size()) :
        QString("正在保存%1视图图像...").arg(viewTypes.first() == "vertical" ? "垂直" :
                                        viewTypes.first() == "left" ? "左侧" : "对向");

    m_saveProgressDialog = new QProgressDialog(progressText, "取消", 0, viewTypes.size(), this);
    m_saveProgressDialog->setWindowModality(Qt::WindowModal);
    m_saveProgressDialog->setMinimumDuration(0);  // 立即显示
    m_saveProgressDialog->setValue(0);
    m_saveProgressDialog->show();

    // 禁用相关的保存按钮，防止重复点击
    if (viewTypes.size() > 1) {
        // 主界面保存所有视图
        ui->btnSaveImage->setEnabled(false);
    } else {
        // 单个视图保存，禁用对应的按钮
        QString viewType = viewTypes.first();
        if (viewType == "vertical") {
            ui->btnSaveImageVertical->setEnabled(false);
        } else if (viewType == "left") {
            ui->btnSaveImageLeft->setEnabled(false);
        } else if (viewType == "front") {
            ui->btnSaveImageFront->setEnabled(false);
        }
        // 同时也禁用主界面保存按钮，避免冲突
        ui->btnSaveImage->setEnabled(false);
    }

    // 启动异步任务
    QFuture<void> future = QtConcurrent::run([this, viewTypes]() {
        saveImagesSynchronous(viewTypes);
    });

    m_saveWatcher->setFuture(future);
}

void MutiCamApp::saveImagesSynchronous(const QStringList& viewTypes)
{
    try {
        for (int i = 0; i < viewTypes.size(); ++i) {
            const QString& viewType = viewTypes[i];

            // 在主线程中更新进度（线程安全）
            QMetaObject::invokeMethod(this, [this, i]() {
                if (m_saveProgressDialog) {
                    m_saveProgressDialog->setValue(i);
                    m_saveProgressDialog->setLabelText(QString("正在保存第 %1 个视图...").arg(i + 1));
                }
            }, Qt::QueuedConnection);

            // 检查是否被取消
            if (m_saveProgressDialog && m_saveProgressDialog->wasCanceled()) {
                qDebug() << "用户取消了保存操作";
                break;
            }

            // 执行实际的保存操作
            saveImages(viewType);

            qDebug() << "完成保存视图：" << viewType;
        }

        qDebug() << "所有视图保存完成";

    } catch (const std::exception& e) {
        qDebug() << "异步保存过程中出错：" << e.what();
    }
}

void MutiCamApp::onAsyncSaveFinished()
{
    qDebug() << "异步保存任务完成";

    // 关闭进度对话框
    if (m_saveProgressDialog) {
        m_saveProgressDialog->setValue(m_saveProgressDialog->maximum());
        m_saveProgressDialog->close();
        m_saveProgressDialog->deleteLater();
        m_saveProgressDialog = nullptr;
    }

    // 重新启用所有保存按钮
    ui->btnSaveImage->setEnabled(true);
    ui->btnSaveImageVertical->setEnabled(true);
    ui->btnSaveImageLeft->setEnabled(true);
    ui->btnSaveImageFront->setEnabled(true);

    // 更新状态栏提示
    statusBar()->showMessage("图像保存完成", 3000);
}

void MutiCamApp::initializeSettingsManager()
{
    // 创建设置管理器
    m_settingsManager = new SettingsManager("./Settings/settings.json", this);

    // 连接设置管理器信号
    connect(m_settingsManager, &SettingsManager::settingsLoaded,
            this, &MutiCamApp::onSettingsLoaded);

    // 加载设置到UI
    m_settingsManager->loadSettingsToUI(this);

    // 根据加载的UI尺寸参数调整窗口大小
    applyUISizeFromSettings();

    qDebug() << "设置管理器初始化完成（实时保存模式）";
}

void MutiCamApp::onSettingsLoaded(bool success)
{
    if (success) {
        qDebug() << "设置加载成功";
    } else {
        qDebug() << "设置加载失败";
    }
}

void MutiCamApp::onSettingsTextChanged()
{
    // 延迟保存设置到文件（避免频繁保存）
    if (m_settingsManager) {
        m_settingsManager->saveSettingsDelayed(this);
    }

    // 记录参数修改日志
    if (m_logManager) {
        m_logManager->logSettingsOperation("参数设置已修改", "实时保存到配置文件");
    }
}

void MutiCamApp::onCameraSerialChanged()
{
    // 保存设置
    if (m_settingsManager) {
        m_settingsManager->saveSettingsDelayed(this);
    }

    // 记录相机序列号修改日志
    if (m_logManager) {
        m_logManager->logParameterChange("相机序列号", "旧值", "新值");
    }

    // 如果相机系统已经初始化，则重新初始化
    if (m_cameraManager) {
        // 延迟重新初始化相机系统（避免用户快速输入时频繁重新初始化）
        static QTimer* reinitTimer = nullptr;
        if (!reinitTimer) {
            reinitTimer = new QTimer(this);
            reinitTimer->setSingleShot(true);
            reinitTimer->setInterval(2000); // 2秒延迟
            connect(reinitTimer, &QTimer::timeout, this, &MutiCamApp::reinitializeCameraSystem);
        }

        // 重启定时器
        reinitTimer->start();

        if (m_logManager) {
            m_logManager->logCameraOperation("相机序列号已修改", "", "将在2秒后重新初始化相机系统");
        }

        qDebug() << "相机序列号参数已改变，将在2秒后重新初始化相机系统";
    } else {
        if (m_logManager) {
            m_logManager->logCameraOperation("相机序列号已修改", "", "将在下次开始测量时使用新的序列号");
        }
        qDebug() << "相机序列号参数已改变，将在下次开始测量时使用新的序列号";
    }
}

void MutiCamApp::connectSettingsSignals()
{
    // 连接所有参数输入框的textChanged信号到实时保存槽函数

    // 相机参数（特殊处理：改变时重新初始化相机系统）
    connect(ui->ledVerCamSN, &QLineEdit::textChanged, this, &MutiCamApp::onCameraSerialChanged);
    connect(ui->ledLeftCamSN, &QLineEdit::textChanged, this, &MutiCamApp::onCameraSerialChanged);
    connect(ui->ledFrontCamSN, &QLineEdit::textChanged, this, &MutiCamApp::onCameraSerialChanged);

    // 直线查找参数
    connect(ui->ledCannyLineLow, &QLineEdit::textChanged, this, &MutiCamApp::onSettingsTextChanged);
    connect(ui->ledCannyLineHigh, &QLineEdit::textChanged, this, &MutiCamApp::onSettingsTextChanged);
    connect(ui->ledLineDetThreshold, &QLineEdit::textChanged, this, &MutiCamApp::onSettingsTextChanged);
    connect(ui->ledLineDetMinLength, &QLineEdit::textChanged, this, &MutiCamApp::onSettingsTextChanged);
    connect(ui->ledLineDetMaxGap, &QLineEdit::textChanged, this, &MutiCamApp::onSettingsTextChanged);

    // 圆查找参数
    connect(ui->ledCannyCircleLow, &QLineEdit::textChanged, this, &MutiCamApp::onSettingsTextChanged);
    connect(ui->ledCannyCircleHigh, &QLineEdit::textChanged, this, &MutiCamApp::onSettingsTextChanged);
    connect(ui->ledCircleDetParam2, &QLineEdit::textChanged, this, &MutiCamApp::onSettingsTextChanged);

    // UI尺寸参数（双向绑定）
    connect(ui->ledUIWidth, &QLineEdit::textChanged, this, &MutiCamApp::onUISizeChanged);
    connect(ui->ledUIHeight, &QLineEdit::textChanged, this, &MutiCamApp::onUISizeChanged);

    qDebug() << "参数输入框实时保存信号连接完成";
}

void MutiCamApp::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    // 同步所有视图的坐标变换
    syncOverlayTransforms("vertical");
    syncOverlayTransforms("left");
    syncOverlayTransforms("front");
    syncOverlayTransforms("vertical2");
    syncOverlayTransforms("left2");
    syncOverlayTransforms("front2");

    // 如果正在更新UI尺寸，避免循环触发
    if (m_isUpdatingUISize) {
        return;
    }

    // 更新UI尺寸参数
    QSize newSize = event->size();

    // 设置标志位，避免循环触发
    m_isUpdatingUISize = true;

    // 更新UI中的尺寸参数
    ui->ledUIWidth->setText(QString::number(newSize.width()));
    ui->ledUIHeight->setText(QString::number(newSize.height()));

    // 保存设置到文件
    if (m_settingsManager) {
        m_settingsManager->saveSettingsDelayed(this);
    }

    // 重置标志位
    m_isUpdatingUISize = false;

    qDebug() << "窗口大小改变：" << newSize.width() << "x" << newSize.height();
}

void MutiCamApp::onUISizeChanged()
{
    // 如果正在更新UI尺寸，避免循环触发
    if (m_isUpdatingUISize) {
        return;
    }

    // 从UI获取新的尺寸
    bool widthOk, heightOk;
    int newWidth = ui->ledUIWidth->text().toInt(&widthOk);
    int newHeight = ui->ledUIHeight->text().toInt(&heightOk);

    // 验证输入的有效性
    if (!widthOk || !heightOk || newWidth < 800 || newHeight < 600 ||
        newWidth > 4000 || newHeight > 3000) {
        qDebug() << "无效的UI尺寸参数，忽略调整";
        return;
    }

    // 设置标志位，避免循环触发
    m_isUpdatingUISize = true;

    // 调整窗口大小
    resize(newWidth, newHeight);

    // 保存设置到文件
    if (m_settingsManager) {
        m_settingsManager->saveSettingsDelayed(this);
    }

    // 重置标志位
    m_isUpdatingUISize = false;

    qDebug() << "根据参数调整窗口大小：" << newWidth << "x" << newHeight;
}

void MutiCamApp::reinitializeCameraSystem()
{
    qDebug() << "重新初始化相机系统...";

    // 停止当前的相机系统
    if (m_cameraManager) {
        // 停止所有相机采集
        m_cameraManager->stopAllCameras();

        // 断开所有相机连接
        m_cameraManager->disconnectAllCameras();

        // 重置相机管理器
        m_cameraManager.reset();
    }

    // 重新初始化相机系统
    initializeCameraSystem();

    // 重新连接相机管理器信号
    if (m_cameraManager) {
        connectCameraManagerSignals();
    }

    qDebug() << "相机系统重新初始化完成";
}

void MutiCamApp::connectCameraManagerSignals()
{
    if (!m_cameraManager) {
        qDebug() << "相机管理器未初始化，无法连接信号";
        return;
    }

    // 连接相机帧就绪信号
    connect(m_cameraManager.get(), &MutiCam::Camera::CameraManager::cameraFrameReady,
            this, &MutiCamApp::onCameraFrameReady);

    // 连接相机状态变化信号
    connect(m_cameraManager.get(), &MutiCam::Camera::CameraManager::cameraStateChanged,
            this, &MutiCamApp::onCameraStateChanged);

    // 连接相机错误信号
    connect(m_cameraManager.get(), &MutiCam::Camera::CameraManager::cameraError,
            this, &MutiCamApp::onCameraError);

    qDebug() << "相机管理器信号连接完成";
}

void MutiCamApp::initializeLogManager()
{
    try {
        // 创建日志管理器（暂时不绑定UI组件）
        m_logManager = new LogManager(nullptr, this);

        // 设置日志级别为INFO
        m_logManager->setLogLevel(LogLevel::INFO);

        // 启用文件日志
        m_logManager->setFileLoggingEnabled(true);

        // 暂时禁用UI日志显示（可以后续添加UI组件时启用）
        m_logManager->setUILoggingEnabled(false);

        // 记录系统启动日志
        m_logManager->logUIOperation("应用程序启动", "MutiCamApp C++版本启动");

        qDebug() << "日志管理器初始化完成";

    } catch (const std::exception& e) {
        qDebug() << "日志管理器初始化失败：" << e.what();
    }
}

void MutiCamApp::applyUISizeFromSettings()
{
    // 从UI参数获取尺寸
    bool widthOk, heightOk;
    int width = ui->ledUIWidth->text().toInt(&widthOk);
    int height = ui->ledUIHeight->text().toInt(&heightOk);

    // 验证参数有效性
    if (widthOk && heightOk && width >= 800 && height >= 600 &&
        width <= 4000 && height <= 3000) {

        // 设置标志位，避免触发resizeEvent中的参数更新
        m_isUpdatingUISize = true;

        // 调整窗口大小
        resize(width, height);

        // 重置标志位
        m_isUpdatingUISize = false;

        qDebug() << "程序启动时应用UI尺寸：" << width << "x" << height;
    } else {
        qDebug() << "UI尺寸参数无效，使用默认窗口大小";
    }
}

// ==================== 通用按钮处理方法实现 ====================

void MutiCamApp::startDrawingTool(PaintingOverlay::DrawingTool tool, const QString& viewName)
{
    // 记录绘制操作
    if (m_logManager) {
        QString toolName;
        switch (tool) {
            case PaintingOverlay::DrawingTool::Point: toolName = "点"; break;
            case PaintingOverlay::DrawingTool::Line: toolName = "直线"; break;
            case PaintingOverlay::DrawingTool::Circle: toolName = "圆形"; break;
            case PaintingOverlay::DrawingTool::FineCircle: toolName = "精细圆"; break;
            case PaintingOverlay::DrawingTool::Parallel: toolName = "平行线"; break;
            case PaintingOverlay::DrawingTool::TwoLines: toolName = "两线测量"; break;
            default: toolName = "未知工具"; break;
        }
        m_logManager->logDrawingOperation(QString("开始绘制%1").arg(toolName),
                                        QString("视图: %1").arg(viewName));
    }

    // 根据视图名称启动对应的绘图工具
    if (viewName == "all") {
        // 所有视图
        if (m_verticalPaintingOverlay) m_verticalPaintingOverlay->startDrawing(tool);
        if (m_leftPaintingOverlay) m_leftPaintingOverlay->startDrawing(tool);
        if (m_frontPaintingOverlay) m_frontPaintingOverlay->startDrawing(tool);
        if (m_verticalPaintingOverlay2) m_verticalPaintingOverlay2->startDrawing(tool);
        if (m_leftPaintingOverlay2) m_leftPaintingOverlay2->startDrawing(tool);
        if (m_frontPaintingOverlay2) m_frontPaintingOverlay2->startDrawing(tool);
    } else if (viewName == "vertical") {
        // 垂直视图
        if (m_verticalPaintingOverlay) m_verticalPaintingOverlay->startDrawing(tool);
        if (m_verticalPaintingOverlay2) m_verticalPaintingOverlay2->startDrawing(tool);
    } else if (viewName == "left") {
        // 左视图
        if (m_leftPaintingOverlay) m_leftPaintingOverlay->startDrawing(tool);
        if (m_leftPaintingOverlay2) m_leftPaintingOverlay2->startDrawing(tool);
    } else if (viewName == "front") {
        // 前视图
        if (m_frontPaintingOverlay) m_frontPaintingOverlay->startDrawing(tool);
        if (m_frontPaintingOverlay2) m_frontPaintingOverlay2->startDrawing(tool);
    }
}

void MutiCamApp::clearDrawings(const QString& viewName)
{
    if (viewName == "all") {
        // 清空所有视图
        PaintingOverlay* overlay = getActivePaintingOverlay();
        if (overlay) {
            overlay->clearAllDrawings();
        }
    } else if (viewName == "vertical") {
        if (m_verticalPaintingOverlay2) {
            m_verticalPaintingOverlay2->clearAllDrawings();
        }
    } else if (viewName == "left") {
        if (m_leftPaintingOverlay2) {
            m_leftPaintingOverlay2->clearAllDrawings();
        }
    } else if (viewName == "front") {
        if (m_frontPaintingOverlay2) {
            m_frontPaintingOverlay2->clearAllDrawings();
        }
    }
}

void MutiCamApp::undoOperation(const QString& viewName, bool isDetection)
{
    PaintingOverlay* overlay = nullptr;

    if (viewName == "all") {
        overlay = getActivePaintingOverlay();
    } else if (viewName == "vertical") {
        overlay = m_verticalPaintingOverlay2;
    } else if (viewName == "left") {
        overlay = m_leftPaintingOverlay2;
    } else if (viewName == "front") {
        overlay = m_frontPaintingOverlay2;
    }

    if (overlay) {
        if (isDetection) {
            overlay->undoLastDetection();
        } else {
            overlay->undoLastDrawing();
        }
    }
}

void MutiCamApp::saveImage(const QString& viewName)
{
    if (viewName == "all") {
        onSaveImageClicked();
    } else if (viewName == "vertical") {
        onSaveImageVerticalClicked();
    } else if (viewName == "left") {
        onSaveImageLeftClicked();
    } else if (viewName == "front") {
        onSaveImageFrontClicked();
    }
}

void MutiCamApp::initializeButtonMappings()
{
    m_buttonMappings.clear();

    // 绘图工具按钮映射
    // 点绘制按钮
    m_buttonMappings.emplace_back(ui->btnDrawPoint,
        [this]() { startDrawingTool(PaintingOverlay::DrawingTool::Point, "all"); },
        "drawing", "all", PaintingOverlay::DrawingTool::Point);
    m_buttonMappings.emplace_back(ui->btnDrawPointVertical,
        [this]() { startDrawingTool(PaintingOverlay::DrawingTool::Point, "vertical"); },
        "drawing", "vertical", PaintingOverlay::DrawingTool::Point);
    m_buttonMappings.emplace_back(ui->btnDrawPointLeft,
        [this]() { startDrawingTool(PaintingOverlay::DrawingTool::Point, "left"); },
        "drawing", "left", PaintingOverlay::DrawingTool::Point);
    m_buttonMappings.emplace_back(ui->btnDrawPointFront,
        [this]() { startDrawingTool(PaintingOverlay::DrawingTool::Point, "front"); },
        "drawing", "front", PaintingOverlay::DrawingTool::Point);

    // 直线绘制按钮
    m_buttonMappings.emplace_back(ui->btnDrawStraight,
        [this]() { startDrawingTool(PaintingOverlay::DrawingTool::Line, "all"); },
        "drawing", "all", PaintingOverlay::DrawingTool::Line);
    m_buttonMappings.emplace_back(ui->btnDrawStraightVertical,
        [this]() { startDrawingTool(PaintingOverlay::DrawingTool::Line, "vertical"); },
        "drawing", "vertical", PaintingOverlay::DrawingTool::Line);
    m_buttonMappings.emplace_back(ui->btnDrawStraightLeft,
        [this]() { startDrawingTool(PaintingOverlay::DrawingTool::Line, "left"); },
        "drawing", "left", PaintingOverlay::DrawingTool::Line);
    m_buttonMappings.emplace_back(ui->btnDrawStraightFront,
        [this]() { startDrawingTool(PaintingOverlay::DrawingTool::Line, "front"); },
        "drawing", "front", PaintingOverlay::DrawingTool::Line);

    // 圆形绘制按钮
    m_buttonMappings.emplace_back(ui->btnDrawSimpleCircle,
        [this]() { startDrawingTool(PaintingOverlay::DrawingTool::Circle, "all"); },
        "drawing", "all", PaintingOverlay::DrawingTool::Circle);
    m_buttonMappings.emplace_back(ui->btnDrawSimpleCircleVertical,
        [this]() { startDrawingTool(PaintingOverlay::DrawingTool::Circle, "vertical"); },
        "drawing", "vertical", PaintingOverlay::DrawingTool::Circle);
    m_buttonMappings.emplace_back(ui->btnDrawSimpleCircleLeft,
        [this]() { startDrawingTool(PaintingOverlay::DrawingTool::Circle, "left"); },
        "drawing", "left", PaintingOverlay::DrawingTool::Circle);
    m_buttonMappings.emplace_back(ui->btnDrawSimpleCircleFront,
        [this]() { startDrawingTool(PaintingOverlay::DrawingTool::Circle, "front"); },
        "drawing", "front", PaintingOverlay::DrawingTool::Circle);

    // 精细圆绘制按钮
    m_buttonMappings.emplace_back(ui->btnDrawFineCircle,
        [this]() { startDrawingTool(PaintingOverlay::DrawingTool::FineCircle, "all"); },
        "drawing", "all", PaintingOverlay::DrawingTool::FineCircle);
    m_buttonMappings.emplace_back(ui->btnDrawFineCircleVertical,
        [this]() { startDrawingTool(PaintingOverlay::DrawingTool::FineCircle, "vertical"); },
        "drawing", "vertical", PaintingOverlay::DrawingTool::FineCircle);
    m_buttonMappings.emplace_back(ui->btnDrawFineCircleLeft,
        [this]() { startDrawingTool(PaintingOverlay::DrawingTool::FineCircle, "left"); },
        "drawing", "left", PaintingOverlay::DrawingTool::FineCircle);
    m_buttonMappings.emplace_back(ui->btnDrawFineCircleFront,
        [this]() { startDrawingTool(PaintingOverlay::DrawingTool::FineCircle, "front"); },
        "drawing", "front", PaintingOverlay::DrawingTool::FineCircle);

    // 平行线绘制按钮
    m_buttonMappings.emplace_back(ui->btnDrawParallel,
        [this]() { startDrawingTool(PaintingOverlay::DrawingTool::Parallel, "all"); },
        "drawing", "all", PaintingOverlay::DrawingTool::Parallel);
    m_buttonMappings.emplace_back(ui->btnDrawParallelVertical,
        [this]() { startDrawingTool(PaintingOverlay::DrawingTool::Parallel, "vertical"); },
        "drawing", "vertical", PaintingOverlay::DrawingTool::Parallel);
    m_buttonMappings.emplace_back(ui->btnDrawParallelLeft,
        [this]() { startDrawingTool(PaintingOverlay::DrawingTool::Parallel, "left"); },
        "drawing", "left", PaintingOverlay::DrawingTool::Parallel);
    m_buttonMappings.emplace_back(ui->btnDrawParallelFront,
        [this]() { startDrawingTool(PaintingOverlay::DrawingTool::Parallel, "front"); },
        "drawing", "front", PaintingOverlay::DrawingTool::Parallel);

    // 两线测量按钮
    m_buttonMappings.emplace_back(ui->btnDraw2Line,
        [this]() { startDrawingTool(PaintingOverlay::DrawingTool::TwoLines, "all"); },
        "drawing", "all", PaintingOverlay::DrawingTool::TwoLines);
    m_buttonMappings.emplace_back(ui->btnDraw2LineVertical,
        [this]() { startDrawingTool(PaintingOverlay::DrawingTool::TwoLines, "vertical"); },
        "drawing", "vertical", PaintingOverlay::DrawingTool::TwoLines);
    m_buttonMappings.emplace_back(ui->btnDraw2LineLeft,
        [this]() { startDrawingTool(PaintingOverlay::DrawingTool::TwoLines, "left"); },
        "drawing", "left", PaintingOverlay::DrawingTool::TwoLines);
    m_buttonMappings.emplace_back(ui->btnDraw2LineFront,
        [this]() { startDrawingTool(PaintingOverlay::DrawingTool::TwoLines, "front"); },
        "drawing", "front", PaintingOverlay::DrawingTool::TwoLines);

    // 清空按钮
    m_buttonMappings.emplace_back(ui->btnClearDrawings,
        [this]() { clearDrawings("all"); },
        "clear", "all");
    m_buttonMappings.emplace_back(ui->btnClearDrawingsVertical,
        [this]() { clearDrawings("vertical"); },
        "clear", "vertical");
    m_buttonMappings.emplace_back(ui->btnClearDrawingsLeft,
        [this]() { clearDrawings("left"); },
        "clear", "left");
    m_buttonMappings.emplace_back(ui->btnClearDrawingsFront,
        [this]() { clearDrawings("front"); },
        "clear", "front");

    // 撤销绘图按钮
    m_buttonMappings.emplace_back(ui->btnCan1StepDraw,
        [this]() { undoOperation("all", false); },
        "undo", "all");
    m_buttonMappings.emplace_back(ui->btnCan1StepDrawVertical,
        [this]() { undoOperation("vertical", false); },
        "undo", "vertical");
    m_buttonMappings.emplace_back(ui->btnCan1StepDrawLeft,
        [this]() { undoOperation("left", false); },
        "undo", "left");
    m_buttonMappings.emplace_back(ui->btnCan1StepDrawFront,
        [this]() { undoOperation("front", false); },
        "undo", "front");

    // 撤销检测按钮
    m_buttonMappings.emplace_back(ui->btnCan1StepDet,
        [this]() { undoOperation("all", true); },
        "undo", "all");
    m_buttonMappings.emplace_back(ui->btnCan1StepDetVertical,
        [this]() { undoOperation("vertical", true); },
        "undo", "vertical");
    m_buttonMappings.emplace_back(ui->btnCan1StepDetLeft,
        [this]() { undoOperation("left", true); },
        "undo", "left");
    m_buttonMappings.emplace_back(ui->btnCan1StepDetFront,
        [this]() { undoOperation("front", true); },
        "undo", "front");

    // 保存图像按钮
    m_buttonMappings.emplace_back(ui->btnSaveImage,
        [this]() { saveImage("all"); },
        "save", "all");
    m_buttonMappings.emplace_back(ui->btnSaveImageVertical,
        [this]() { saveImage("vertical"); },
        "save", "vertical");
    m_buttonMappings.emplace_back(ui->btnSaveImageLeft,
        [this]() { saveImage("left"); },
        "save", "left");
    m_buttonMappings.emplace_back(ui->btnSaveImageFront,
        [this]() { saveImage("front"); },
        "save", "front");

    // 网格取消按钮
    m_buttonMappings.emplace_back(ui->btnCancelGrids,
        [this]() { onCancelGridsClicked(); },
        "grid", "all");
    m_buttonMappings.emplace_back(ui->btnCancelGridsVertical,
        [this]() { onCancelGridsVerticalClicked(); },
        "grid", "vertical");
    m_buttonMappings.emplace_back(ui->btnCancelGridsLeft,
        [this]() { onCancelGridsLeftClicked(); },
        "grid", "left");
    m_buttonMappings.emplace_back(ui->btnCancelGridsFront,
        [this]() { onCancelGridsFrontClicked(); },
        "grid", "front");

    // 自动检测按钮
    m_buttonMappings.emplace_back(ui->btnLineDet,
        [this]() { onAutoLineDetectionClicked(); },
        "detection", "all");
    m_buttonMappings.emplace_back(ui->btnCircleDet,
        [this]() { onAutoCircleDetectionClicked(); },
        "detection", "all");
    m_buttonMappings.emplace_back(ui->btnLineDetVertical,
        [this]() { onAutoLineDetectionVerticalClicked(); },
        "detection", "vertical");
    m_buttonMappings.emplace_back(ui->btnCircleDetVertical,
        [this]() { onAutoCircleDetectionVerticalClicked(); },
        "detection", "vertical");
    m_buttonMappings.emplace_back(ui->btnLineDetLeft,
        [this]() { onAutoLineDetectionLeftClicked(); },
        "detection", "left");
    m_buttonMappings.emplace_back(ui->btnCircleDetLeft,
        [this]() { onAutoCircleDetectionLeftClicked(); },
        "detection", "left");
    m_buttonMappings.emplace_back(ui->btnLineDetFront,
        [this]() { onAutoLineDetectionFrontClicked(); },
        "detection", "front");
    m_buttonMappings.emplace_back(ui->btnCircleDetFront,
        [this]() { onAutoCircleDetectionFrontClicked(); },
        "detection", "front");
}

void MutiCamApp::connectButtonSignals()
{
    // 使用映射表连接所有按钮信号
    for (const auto& mapping : m_buttonMappings) {
        if (mapping.button && mapping.handler) {
            connect(mapping.button, &QPushButton::clicked, mapping.handler);
        }
    }

    qDebug() << "已连接" << m_buttonMappings.size() << "个按钮信号";
}

// ==================== 相机状态监控实现 ====================

void MutiCamApp::initializeCameraStatusMonitoring()
{
    // 创建状态更新定时器
    m_statusUpdateTimer = new QTimer(this);
    connect(m_statusUpdateTimer, &QTimer::timeout,
            this, &MutiCamApp::updateCameraStatusDisplay);

    // 每1秒更新一次状态显示（配合新的帧率计算算法）
    m_statusUpdateTimer->start(1000);

    // 初始化帧率数据
    m_frameRateData["vertical"] = FrameRateData();
    m_frameRateData["left"] = FrameRateData();
    m_frameRateData["front"] = FrameRateData();

    // 初始化UI显示
    updateCameraOverview();
    updateAlertDisplay();

    // 添加系统启动消息
    addAlertMessage("相机状态监控系统已启动", "info");

    qDebug() << "相机状态监控系统已初始化";
}

void MutiCamApp::updateCameraStatusDisplay()
{
    // 更新总览信息
    updateCameraOverview();

    // 更新各个相机的状态
    updateSingleCameraStatus("vertical");
    updateSingleCameraStatus("left");
    updateSingleCameraStatus("front");
}

void MutiCamApp::updateCameraOverview()
{
    int totalCameras = 3;
    int onlineCameras = 0;
    int offlineCameras = 0;

    if (m_cameraManager) {
        // 检查各相机状态
        QStringList cameraIds = {"vertical", "left", "front"};
        for (const QString& cameraId : cameraIds) {
            QString stats = m_cameraManager->getCameraStats(cameraId.toStdString());
            if (stats.contains("State: Streaming") || stats.contains("State: Connected")) {
                onlineCameras++;
            } else {
                offlineCameras++;
            }
        }
    } else {
        // 相机管理器未初始化，所有相机都离线
        offlineCameras = totalCameras;
    }

    // 更新UI显示
    ui->labelTotalCameras->setText(QString("总相机数: %1").arg(totalCameras));
    ui->labelOnlineCameras->setText(QString("在线: %1").arg(onlineCameras));
    ui->labelOfflineCameras->setText(QString("离线: %1").arg(offlineCameras));
}

void MutiCamApp::updateSingleCameraStatus(const QString& cameraId)
{
    // 获取对应的UI控件
    QLabel* statusLabel = nullptr;
    QLabel* fpsLabel = nullptr;
    QLabel* resolutionLabel = nullptr;
    QLabel* exposureLabel = nullptr;
    QLabel* gainLabel = nullptr;

    if (cameraId == "vertical") {
        statusLabel = ui->labelVerticalStatus;
        fpsLabel = ui->labelVerticalFPS;
        resolutionLabel = ui->labelVerticalResolution;
        exposureLabel = ui->labelVerticalExposure;
        gainLabel = ui->labelVerticalGain;
    } else if (cameraId == "left") {
        statusLabel = ui->labelLeftStatus;
        fpsLabel = ui->labelLeftFPS;
        resolutionLabel = ui->labelLeftResolution;
        exposureLabel = ui->labelLeftExposure;
        gainLabel = ui->labelLeftGain;
    } else if (cameraId == "front") {
        statusLabel = ui->labelFrontStatus;
        fpsLabel = ui->labelFrontFPS;
        resolutionLabel = ui->labelFrontResolution;
        exposureLabel = ui->labelFrontExposure;
        gainLabel = ui->labelFrontGain;
    }

    if (!statusLabel || !fpsLabel || !resolutionLabel || !exposureLabel || !gainLabel) {
        return;
    }

    // 默认离线状态
    QString statusText = "离线";
    QString statusColor = "color: red;";
    QString fpsText = "-- fps";
    QString resolutionText = "--";
    QString exposureText = "-- μs";
    QString gainText = "--dB";

    if (m_cameraManager) {
        QString stats = m_cameraManager->getCameraStats(cameraId.toStdString());

        if (stats.contains("State: Streaming")) {
            statusText = "采集中";
            statusColor = "color: green;";

            // 解析参数信息
            QStringList lines = stats.split('\n');
            for (const QString& line : lines) {
                if (line.contains("Resolution:")) {
                    QString resolution = line.split(":").last().trimmed();
                    resolutionText = resolution;
                } else if (line.contains("Exposure:")) {
                    QString exposure = line.split(":").last().trimmed();
                    // 直接显示微秒值
                    exposureText = exposure;
                } else if (line.contains("Gain:")) {
                    QString gain = line.split(":").last().trimmed();
                    gainText = gain + "dB";
                }
            }

            // 获取帧率
            QMutexLocker locker(&m_frameRateMutex);
            if (m_frameRateData.contains(cameraId)) {
                const auto& frameData = m_frameRateData[cameraId];
                if (!frameData.hasFirstFrame) {
                    fpsText = "等待帧数据...";
                } else if (frameData.currentFPS == 0.0) {
                    fpsText = "计算中...";
                } else {
                    fpsText = QString::number(frameData.currentFPS, 'f', 1) + " fps";
                }
            }

        } else if (stats.contains("State: Connected")) {
            statusText = "已连接";
            statusColor = "color: orange;";
        }
    }

    // 更新UI显示
    statusLabel->setText(statusText);
    statusLabel->setStyleSheet(statusColor);
    fpsLabel->setText(fpsText);
    resolutionLabel->setText(resolutionText);
    exposureLabel->setText(exposureText);
    gainLabel->setText(gainText);

    // 设置离线状态的灰色显示
    if (statusText == "离线") {
        fpsLabel->setStyleSheet("color: gray;");
        resolutionLabel->setStyleSheet("color: gray;");
        exposureLabel->setStyleSheet("color: gray;");
        gainLabel->setStyleSheet("color: gray;");
    } else {
        fpsLabel->setStyleSheet("");
        resolutionLabel->setStyleSheet("");
        exposureLabel->setStyleSheet("");
        gainLabel->setStyleSheet("");
    }
}

void MutiCamApp::updateFrameRate(const QString& cameraId)
{
    QMutexLocker locker(&m_frameRateMutex);

    if (!m_frameRateData.contains(cameraId)) {
        m_frameRateData[cameraId] = FrameRateData();
    }

    auto& data = m_frameRateData[cameraId];
    auto currentTime = std::chrono::steady_clock::now();

    // 记录当前帧时间戳
    data.frameTimes.push_back(currentTime);

    // 记录第一帧
    if (!data.hasFirstFrame) {
        data.lastCalculateTime = currentTime;
        data.hasFirstFrame = true;
        data.currentFPS = 0.0;  // 第一帧时帧率为0

        // 立即触发一次状态更新，显示相机已开始接收帧
        QMetaObject::invokeMethod(this, [this, cameraId]() {
            updateSingleCameraStatus(cameraId);
        }, Qt::QueuedConnection);

        return;
    }

    // 移除超过时间窗口的旧帧时间戳
    auto windowStart = currentTime - std::chrono::seconds(FrameRateData::WINDOW_SECONDS);
    data.frameTimes.erase(
        std::remove_if(data.frameTimes.begin(), data.frameTimes.end(),
                      [windowStart](const auto& frameTime) {
                          return frameTime < windowStart;
                      }),
        data.frameTimes.end()
    );

    // 检查是否需要计算帧率（每1秒计算一次）
    auto timeSinceLastCalculate = std::chrono::duration_cast<std::chrono::milliseconds>(
        currentTime - data.lastCalculateTime).count();

    if (timeSinceLastCalculate >= 1000) {  // 1秒 = 1000毫秒
        // 计算时间窗口内的帧率
        if (data.frameTimes.size() >= 2) {
            auto windowDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
                data.frameTimes.back() - data.frameTimes.front()).count();

            if (windowDuration > 0) {
                // 帧率 = (帧数-1) * 1000 / 时间间隔(ms)
                // 减1是因为N个时间点之间有N-1个间隔
                data.currentFPS = ((data.frameTimes.size() - 1) * 1000.0) / windowDuration;
            }
        } else if (data.frameTimes.size() == 1) {
            // 只有一帧时，帧率为0
            data.currentFPS = 0.0;
        }

        // 更新计算时间
        data.lastCalculateTime = currentTime;
    }
}

void MutiCamApp::addAlertMessage(const QString& message, const QString& level)
{
    {
        QMutexLocker locker(&m_alertMutex);

        // 添加新的报警消息
        m_alertMessages.append(AlertMessage(message, level));

        // 限制报警消息数量，保留最新的100条
        while (m_alertMessages.size() > 100) {
            m_alertMessages.removeFirst();
        }
    } // 释放锁

    // 在主线程中更新显示（在锁外调用，避免死锁）
    QMetaObject::invokeMethod(this, "updateAlertDisplay", Qt::QueuedConnection);
}

void MutiCamApp::updateAlertDisplay()
{
    QString htmlContent;

    // 复制消息列表，减少锁持有时间
    QList<AlertMessage> messagesCopy;
    {
        QMutexLocker locker(&m_alertMutex);
        messagesCopy = m_alertMessages;
    } // 释放锁

    // 从最新的消息开始显示（倒序）
    for (int i = messagesCopy.size() - 1; i >= 0; --i) {
        const auto& alert = messagesCopy[i];
        QString color;

        if (alert.level == "error") {
            color = "#ff0000";  // 红色
        } else if (alert.level == "warning") {
            color = "#ff8000";  // 橙色
        } else {
            color = "#008000";  // 绿色
        }

        QString timeStr = alert.timestamp.toString("yyyy-MM-dd hh:mm:ss");
        htmlContent += QString("<p style=\"margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                              "<span style=\"color:%1;\">[%2] %3 - %4</span></p>")
                              .arg(color)
                              .arg(alert.level.toUpper())
                              .arg(alert.message)
                              .arg(timeStr);
    }

    // 设置HTML内容
    QString fullHtml = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
                              "<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" />"
                              "<style type=\"text/css\">p, li { white-space: pre-wrap; }</style></head>"
                              "<body style=\"font-family:'Microsoft YaHei UI'; font-size:9pt; font-weight:400; font-style:normal;\">"
                              "%1</body></html>").arg(htmlContent);

    ui->textEditAlerts->setHtml(fullHtml);

    // 滚动到底部显示最新消息
    QTextCursor cursor = ui->textEditAlerts->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->textEditAlerts->setTextCursor(cursor);
}

void MutiCamApp::onClearAlertsClicked()
{
    {
        QMutexLocker locker(&m_alertMutex);

        // 清空报警消息列表
        m_alertMessages.clear();
    } // 释放锁

    // 清空UI显示
    ui->textEditAlerts->clear();

    // 添加清空操作的提示信息（在锁外调用，避免死锁）
    addAlertMessage("报警信息已清空", "info");

    qDebug() << "报警信息已清空";
}

void MutiCamApp::onRefreshStatusClicked()
{
    // 立即更新一次状态显示
    updateCameraStatusDisplay();

    // 添加刷新操作的提示信息
    addAlertMessage("状态信息已刷新", "info");

    qDebug() << "相机状态已手动刷新";
}

// 载物台控制槽函数实现
void MutiCamApp::onMoveXLeftClicked()
{
    double stepSize = getCurrentStepSize();
    qDebug() << "X轴负方向移动，步长：" << stepSize << "mm";

    // 更新当前位置
    updateCurrentPosition(-stepSize, 0, 0);

    // 记录轨迹
    if (m_trajectoryRecorder && m_trajectoryRecorder->isRecording()) {
        m_trajectoryRecorder->recordMovement("x-", stepSize, m_currentX, m_currentY, m_currentZ);
    }

    // TODO: 实现实际的载物台移动逻辑
    // stageController->moveRelative(-stepSize, 0, 0);
}

void MutiCamApp::onMoveXRightClicked()
{
    double stepSize = getCurrentStepSize();
    qDebug() << "X轴正方向移动，步长：" << stepSize << "mm";

    // 更新当前位置
    updateCurrentPosition(stepSize, 0, 0);

    // 记录轨迹
    if (m_trajectoryRecorder && m_trajectoryRecorder->isRecording()) {
        m_trajectoryRecorder->recordMovement("x+", stepSize, m_currentX, m_currentY, m_currentZ);
    }

    // TODO: 实现实际的载物台移动逻辑
    // stageController->moveRelative(stepSize, 0, 0);
}

void MutiCamApp::onMoveYUpClicked()
{
    double stepSize = getCurrentStepSize();
    qDebug() << "Y轴正方向移动，步长：" << stepSize << "mm";

    // 更新当前位置
    updateCurrentPosition(0, stepSize, 0);

    // 记录轨迹
    if (m_trajectoryRecorder && m_trajectoryRecorder->isRecording()) {
        m_trajectoryRecorder->recordMovement("y+", stepSize, m_currentX, m_currentY, m_currentZ);
    }

    // TODO: 实现实际的载物台移动逻辑
    // stageController->moveRelative(0, stepSize, 0);
}

void MutiCamApp::onMoveYDownClicked()
{
    double stepSize = getCurrentStepSize();
    qDebug() << "Y轴负方向移动，步长：" << stepSize << "mm";

    // 更新当前位置
    updateCurrentPosition(0, -stepSize, 0);

    // 记录轨迹
    if (m_trajectoryRecorder && m_trajectoryRecorder->isRecording()) {
        m_trajectoryRecorder->recordMovement("y-", stepSize, m_currentX, m_currentY, m_currentZ);
    }

    // TODO: 实现实际的载物台移动逻辑
    // stageController->moveRelative(0, -stepSize, 0);
}

void MutiCamApp::onMoveZUpClicked()
{
    double stepSize = getCurrentStepSize();
    qDebug() << "Z轴正方向移动，步长：" << stepSize << "mm";

    // 更新当前位置
    updateCurrentPosition(0, 0, stepSize);

    // 记录轨迹
    if (m_trajectoryRecorder && m_trajectoryRecorder->isRecording()) {
        m_trajectoryRecorder->recordMovement("z+", stepSize, m_currentX, m_currentY, m_currentZ);
    }

    // TODO: 实现实际的载物台移动逻辑
    // stageController->moveRelative(0, 0, stepSize);
}

void MutiCamApp::onMoveZDownClicked()
{
    double stepSize = getCurrentStepSize();
    qDebug() << "Z轴负方向移动，步长：" << stepSize << "mm";

    // 更新当前位置
    updateCurrentPosition(0, 0, -stepSize);

    // 记录轨迹
    if (m_trajectoryRecorder && m_trajectoryRecorder->isRecording()) {
        m_trajectoryRecorder->recordMovement("z-", stepSize, m_currentX, m_currentY, m_currentZ);
    }

    // TODO: 实现实际的载物台移动逻辑
    // stageController->moveRelative(0, 0, -stepSize);
}

void MutiCamApp::onStageHomeClicked()
{
    qDebug() << "载物台回到原点";

    // 重置当前位置
    m_currentX = m_currentY = m_currentZ = 0.0;
    updateCurrentPosition(0, 0, 0);  // 更新UI显示

    // 记录轨迹
    if (m_trajectoryRecorder && m_trajectoryRecorder->isRecording()) {
        m_trajectoryRecorder->recordPoint(m_currentX, m_currentY, m_currentZ, "home");
    }

    // TODO: 实现实际的载物台回原点逻辑
    // stageController->moveToHome();
}

void MutiCamApp::onStageStopClicked()
{
    qDebug() << "载物台紧急停止";

    // 记录轨迹
    if (m_trajectoryRecorder && m_trajectoryRecorder->isRecording()) {
        m_trajectoryRecorder->recordPoint(m_currentX, m_currentY, m_currentZ, "emergency_stop");
    }

    // TODO: 实现实际的载物台紧急停止逻辑
    // stageController->emergencyStop();
}

double MutiCamApp::getCurrentStepSize() const
{
    if (ui->radioStep01->isChecked()) {
        return 0.1;
    } else if (ui->radioStep1->isChecked()) {
        return 1.0;
    } else if (ui->radioStep10->isChecked()) {
        return 10.0;
    }
    return 0.1; // 默认步长
}

void MutiCamApp::initializeTrajectoryRecorder()
{
    m_trajectoryRecorder = std::make_unique<TrajectoryRecorder>(this);

    // 连接轨迹记录器信号
    connect(m_trajectoryRecorder.get(), &TrajectoryRecorder::pointRecorded,
            this, &MutiCamApp::onTrajectoryPointRecorded);
    connect(m_trajectoryRecorder.get(), &TrajectoryRecorder::statisticsUpdated,
            this, &MutiCamApp::onTrajectoryStatisticsUpdated);
    connect(m_trajectoryRecorder.get(), &TrajectoryRecorder::recordingStarted,
            this, [this]() {
                ui->btnTrajectoryStart->setEnabled(false);
                ui->btnTrajectoryStop->setEnabled(true);
                ui->labelTrajectoryStatus->setText("记录中");
                ui->labelTrajectoryStatus->setStyleSheet("color: green; font-weight: bold;");
            });
    connect(m_trajectoryRecorder.get(), &TrajectoryRecorder::recordingStopped,
            this, [this]() {
                ui->btnTrajectoryStart->setEnabled(true);
                ui->btnTrajectoryStop->setEnabled(false);
                ui->labelTrajectoryStatus->setText("已停止");
                ui->labelTrajectoryStatus->setStyleSheet("color: orange; font-weight: bold;");
            });
    connect(m_trajectoryRecorder.get(), &TrajectoryRecorder::trajectoryCleared,
            this, [this]() {
                ui->labelTrajectoryPoints->setText("0");
                ui->labelTrajectoryDistance->setText("0.0 mm");
                ui->labelTrajectoryStatus->setText("未记录");
                ui->labelTrajectoryStatus->setStyleSheet("color: gray; font-weight: bold;");
            });

    qDebug() << "轨迹记录器初始化完成";
}

void MutiCamApp::updateCurrentPosition(double deltaX, double deltaY, double deltaZ)
{
    m_currentX += deltaX;
    m_currentY += deltaY;
    m_currentZ += deltaZ;

    // 更新UI显示的当前位置
    ui->labelXPositionValue->setText(QString("%1 mm").arg(m_currentX, 0, 'f', 3));
    ui->labelYPositionValue->setText(QString("%1 mm").arg(m_currentY, 0, 'f', 3));
    ui->labelZPositionValue->setText(QString("%1 mm").arg(m_currentZ, 0, 'f', 3));
}

// 轨迹记录槽函数实现
void MutiCamApp::onTrajectoryStartClicked()
{
    if (m_trajectoryRecorder) {
        m_trajectoryRecorder->startRecording();
        qDebug() << "开始记录载物台轨迹";
    }
}

void MutiCamApp::onTrajectoryStopClicked()
{
    if (m_trajectoryRecorder) {
        m_trajectoryRecorder->stopRecording();
        qDebug() << "停止记录载物台轨迹";
    }
}

void MutiCamApp::onTrajectoryClearClicked()
{
    if (m_trajectoryRecorder) {
        m_trajectoryRecorder->clearTrajectory();
        qDebug() << "清空载物台轨迹";
    }
}

void MutiCamApp::onTrajectoryExportClicked()
{
    if (!m_trajectoryRecorder) {
        return;
    }

    QString fileName = QString("trajectory_%1.csv")
                      .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    QString filePath = QDir::currentPath() + "/Logs/" + fileName;

    // 确保目录存在
    QDir().mkpath(QDir::currentPath() + "/Logs");

    if (m_trajectoryRecorder->exportToCsv(filePath)) {
        qDebug() << "轨迹导出成功:" << filePath;
        // 可以添加成功提示
    } else {
        qWarning() << "轨迹导出失败:" << filePath;
        // 可以添加错误提示
    }
}

void MutiCamApp::onTrajectoryPointRecorded(const TrajectoryPoint& point)
{
    Q_UNUSED(point)
    // 可以在这里添加实时轨迹点处理逻辑
    updateTrajectoryDisplay();
}

void MutiCamApp::onTrajectoryStatisticsUpdated(const TrajectoryStatistics& stats)
{
    // 更新UI显示的统计信息
    ui->labelTrajectoryPoints->setText(QString::number(stats.totalPoints));
    ui->labelTrajectoryDistance->setText(QString("%1 mm").arg(stats.totalDistance, 0, 'f', 2));
}

void MutiCamApp::updateTrajectoryDisplay()
{
    // 这里可以添加轨迹可视化更新逻辑
    // 例如在某个视图中绘制轨迹路径
}

// 视图控制槽函数实现
void MutiCamApp::onResetZoomVertical()
{
    if (m_verticalZoomPanWidget2) {
        m_verticalZoomPanWidget2->resetView();
        qDebug() << "垂直视图缩放已重置";
    }
}

void MutiCamApp::onResetZoomLeft()
{
    if (m_leftZoomPanWidget2) {
        m_leftZoomPanWidget2->resetView();
        qDebug() << "左视图缩放已重置";
    }
}

void MutiCamApp::onResetZoomFront()
{
    if (m_frontZoomPanWidget2) {
        m_frontZoomPanWidget2->resetView();
        qDebug() << "前视图缩放已重置";
    }
}

void MutiCamApp::onViewTransformChanged(double zoomFactor, const QPointF& panOffset)
{
    // 获取发送信号的ZoomPanWidget
    ZoomPanWidget* sender = qobject_cast<ZoomPanWidget*>(QObject::sender());
    if (!sender) {
        return;
    }

    // 更新对应的缩放信息标签
    QString zoomText = QString("缩放: %1%").arg(static_cast<int>(zoomFactor * 100));

    if (sender == m_verticalZoomPanWidget2) {
        ui->labelZoomInfoVertical->setText(zoomText);
    } else if (sender == m_leftZoomPanWidget2) {
        ui->labelZoomInfoLeft->setText(zoomText);
    } else if (sender == m_frontZoomPanWidget2) {
        ui->labelZoomInfoFront->setText(zoomText);
    }

    // 可以在这里添加其他视图变换相关的处理逻辑
    Q_UNUSED(panOffset)
}

// ==================== 物理按钮控制相关方法 ====================

void MutiCamApp::initializeSerialController()
{
    qDebug() << "初始化串口控制器...";

    // 创建串口控制器
    m_serialController = new SerialController(this);

    // 连接信号和槽
    connect(m_serialController, &SerialController::buttonEventReceived,
            this, &MutiCamApp::handleButtonEvent);

    connect(m_serialController, &SerialController::statusChanged,
            this, [this](SerialController::SerialStatus status) {
                QString statusText;
                QString styleSheet;

                switch (status) {
                    case SerialController::SerialStatus::Disconnected:
                        statusText = "未连接";
                        styleSheet = "color: red;";
                        break;
                    case SerialController::SerialStatus::Connected:
                        statusText = "已连接";
                        styleSheet = "color: orange;";
                        break;
                    case SerialController::SerialStatus::Listening:
                        statusText = "监听中";
                        styleSheet = "color: green;";
                        break;
                    case SerialController::SerialStatus::Error:
                        statusText = "错误";
                        styleSheet = "color: red;";
                        break;
                }

                ui->labelSerialStatus->setText(statusText);
                ui->labelSerialStatus->setStyleSheet(styleSheet);
            });

    connect(m_serialController, &SerialController::errorOccurred,
            this, [this](const QString& error) {
                qDebug() << "串口错误:" << error;
                ui->labelSerialStatus->setText("错误: " + error);
                ui->labelSerialStatus->setStyleSheet("color: red;");
            });

    // 连接测试按钮
    connect(ui->btnTestCaptureKey, &QPushButton::clicked, this, [this]() { testCaptureButton(); });

    // 连接串口控制按钮
    connect(ui->btnConnectSerial, &QPushButton::clicked, this, &MutiCamApp::toggleSerialConnection);

    // 尝试自动连接默认串口
    QString defaultPort = "COM5";  // 可以从设置中读取
#ifndef _WIN32
    defaultPort = "/dev/ttyUSB0";
#endif

    // 设置默认串口到选择框
    ui->comboBoxSerialPort->setCurrentText(defaultPort);

    if (m_serialController->openPort(defaultPort)) {
        m_serialController->startListening();
        ui->btnConnectSerial->setText("断开");
        qDebug() << "串口控制器初始化成功，端口:" << defaultPort;
    } else {
        qDebug() << "串口控制器初始化失败:" << m_serialController->getLastError();
        // 失败时保持"连接"按钮状态，用户可以手动重试
    }
}

void MutiCamApp::updateButtonStatus(const QString& buttonName, bool isPressed)
{
    // 只处理拍照按键状态
    if (buttonName == "CaptureKey") {
        if (isPressed) {
            ui->labelCaptureKeyStatus->setText("按下");
            ui->labelCaptureKeyStatus->setStyleSheet("color: green; font-weight: bold;");
        } else {
            ui->labelCaptureKeyStatus->setText("未按下");
            ui->labelCaptureKeyStatus->setStyleSheet("color: gray;");
        }
    }
}

void MutiCamApp::handleButtonEvent(SerialController::ButtonEvent event)
{
    qDebug() << "处理按钮事件:" << static_cast<int>(event);

    switch (event) {
        case SerialController::ButtonEvent::Button1Pressed:
            updateButtonStatus("CaptureKey", true);
            executeCaptureAction();
            break;

        case SerialController::ButtonEvent::Button1Released:
            updateButtonStatus("CaptureKey", false);
            break;

        default:
            qDebug() << "未知按钮事件:" << static_cast<int>(event);
            break;
    }
}

void MutiCamApp::executeCaptureAction()
{
    // 获取拍照按键的功能配置
    QString actionType = ui->comboBoxCaptureKeyFunction->currentText();

    qDebug() << "执行拍照按键操作:" << actionType;

    // 检查相机是否正在运行
    if (!m_isMeasuring) {
        QMessageBox::warning(this, "拍照失败", "请先启动相机采集后再进行拍照操作！");
        return;
    }

    // 根据配置的功能类型执行相应操作
    if (actionType.startsWith("拍照-")) {
        executeCaptureByType(actionType);
    } else if (actionType == "启动相机采集") {
        if (!m_isMeasuring) {
            onStartMeasureClicked();
        }
    } else if (actionType == "开始检测") {
        qDebug() << "开始检测功能待实现";
    } else if (actionType == "载物台回原点") {
        qDebug() << "载物台回原点功能待实现";
    } else {
        // 默认执行当前选项卡拍照
        executeCaptureByType("拍照-当前选项卡");
    }

    // 记录日志
    if (m_logManager) {
        m_logManager->log(QString("物理拍照按键触发: %1").arg(actionType), LogLevel::INFO);
    }
}

void MutiCamApp::executeCaptureByType(const QString& actionType)
{
    qDebug() << "执行拍照动作:" << actionType;

    // 检查相机是否正在运行
    if (!m_isMeasuring) {
        QMessageBox::warning(this, "拍照失败", "请先启动相机采集后再进行拍照操作！");
        return;
    }

    // 解析拍照类型并执行相应操作
    if (actionType == "拍照-全视图") {
        saveImage("all");  // 使用saveImage而不是saveImages
        if (m_logManager) {
            m_logManager->log("物理按钮触发: 全视图拍照", LogLevel::INFO);
        }
    } else if (actionType == "拍照-垂直视图") {
        saveImage("vertical");
        if (m_logManager) {
            m_logManager->log("物理按钮触发: 垂直视图拍照", LogLevel::INFO);
        }
    } else if (actionType == "拍照-左视图") {
        saveImage("left");
        if (m_logManager) {
            m_logManager->log("物理按钮触发: 左视图拍照", LogLevel::INFO);
        }
    } else if (actionType == "拍照-正视图") {
        saveImage("front");
        if (m_logManager) {
            m_logManager->log("物理按钮触发: 正视图拍照", LogLevel::INFO);
        }
    } else if (actionType == "拍照-当前选项卡") {
        // 根据当前选项卡保存对应视图
        int currentTab = ui->tabWidget->currentIndex();
        QString viewType;

        switch (currentTab) {
            case 0:
                saveImage("all");
                viewType = "全视图";
                break;
            case 1:
                saveImage("vertical");
                viewType = "垂直视图";
                break;
            case 2:
                saveImage("left");
                viewType = "左视图";
                break;
            case 3:
                saveImage("front");
                viewType = "正视图";
                break;
            default:
                saveImage("all");
                viewType = "全视图";
                break;
        }

        if (m_logManager) {
            m_logManager->log(QString("物理按钮触发: 当前选项卡拍照 (%1)").arg(viewType), LogLevel::INFO);
        }
    } else {
        qDebug() << "未知的拍照动作类型:" << actionType;
    }

    // 显示拍照成功提示（可选）
    statusBar()->showMessage(QString("拍照完成: %1").arg(actionType), 2000);
}

void MutiCamApp::testCaptureButton()
{
    qDebug() << "测试拍照按键";

    // 模拟拍照按键按下
    handleButtonEvent(SerialController::ButtonEvent::Button1Pressed);

    // 延迟模拟释放
    QTimer::singleShot(500, this, [this]() {
        handleButtonEvent(SerialController::ButtonEvent::Button1Released);
    });
}

void MutiCamApp::toggleSerialConnection()
{
    if (m_serialController->getStatus() == SerialController::SerialStatus::Disconnected) {
        // 尝试连接串口
        QString portName = ui->comboBoxSerialPort->currentText();
        qDebug() << "尝试连接串口:" << portName;

        if (m_serialController->openPort(portName)) {
            m_serialController->startListening();
            ui->btnConnectSerial->setText("断开");
            qDebug() << "串口连接成功:" << portName;
        } else {
            QMessageBox::warning(this, "串口连接失败",
                QString("无法连接到串口 %1\n错误信息: %2")
                .arg(portName)
                .arg(m_serialController->getLastError()));
        }
    } else {
        // 断开串口连接
        m_serialController->closePort();
        ui->btnConnectSerial->setText("连接");
        qDebug() << "串口已断开";
    }
}

// ==================== 参数预设相关方法 ====================

void MutiCamApp::initializeCapturePresets()
{
    qDebug() << "初始化拍照参数预设...";

    // 连接预设按钮信号
    connect(ui->btnSavePreset, &QPushButton::clicked, this, &MutiCamApp::saveCapturePreset);
    connect(ui->btnLoadPreset, &QPushButton::clicked, this, &MutiCamApp::loadCapturePreset);
    connect(ui->btnResetPreset, &QPushButton::clicked, this, &MutiCamApp::resetCapturePreset);

    // 连接参数变化信号，实现实时保存
    connect(ui->spinBoxCaptureInterval, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MutiCamApp::applyCapturePreset);
    connect(ui->checkBoxAutoSave, &QCheckBox::toggled,
            this, &MutiCamApp::applyCapturePreset);
    connect(ui->comboBoxCaptureFormat, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MutiCamApp::applyCapturePreset);
    connect(ui->comboBoxImageQuality, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MutiCamApp::applyCapturePreset);

    // 加载默认预设
    loadCapturePreset();

    qDebug() << "拍照参数预设初始化完成";
}

void MutiCamApp::saveCapturePreset()
{
    qDebug() << "保存拍照参数预设";

    if (m_settingsManager) {
        // 获取当前UI设置
        auto settings = m_settingsManager->getCurrentSettings();

        // 更新拍照预设参数
        settings.captureInterval = ui->spinBoxCaptureInterval->value();
        settings.autoSaveEnabled = ui->checkBoxAutoSave->isChecked();
        settings.captureFormat = ui->comboBoxCaptureFormat->currentText();
        settings.imageQuality = ui->comboBoxImageQuality->currentText();

        // 保存到设置管理器
        m_settingsManager->setCurrentSettings(settings);
        m_settingsManager->saveSettingsFromUI(this);

        // 记录日志
        if (m_logManager) {
            m_logManager->log("拍照参数预设已保存", LogLevel::INFO);
        }

        statusBar()->showMessage("拍照参数预设已保存", 2000);
    }
}

void MutiCamApp::loadCapturePreset()
{
    qDebug() << "加载拍照参数预设";

    if (m_settingsManager) {
        const auto& settings = m_settingsManager->getCurrentSettings();

        // 应用预设参数到UI
        ui->spinBoxCaptureInterval->setValue(settings.captureInterval);
        ui->checkBoxAutoSave->setChecked(settings.autoSaveEnabled);

        // 设置图像格式
        int formatIndex = ui->comboBoxCaptureFormat->findText(settings.captureFormat);
        if (formatIndex >= 0) {
            ui->comboBoxCaptureFormat->setCurrentIndex(formatIndex);
        }

        // 设置图像质量
        int qualityIndex = ui->comboBoxImageQuality->findText(settings.imageQuality);
        if (qualityIndex >= 0) {
            ui->comboBoxImageQuality->setCurrentIndex(qualityIndex);
        }

        // 记录日志
        if (m_logManager) {
            m_logManager->log("拍照参数预设已加载", LogLevel::INFO);
        }

        qDebug() << "拍照参数预设加载完成";
    }
}

void MutiCamApp::resetCapturePreset()
{
    qDebug() << "重置拍照参数预设";

    // 重置为默认值
    ui->spinBoxCaptureInterval->setValue(3);
    ui->checkBoxAutoSave->setChecked(true);
    ui->comboBoxCaptureFormat->setCurrentIndex(0); // PNG
    ui->comboBoxImageQuality->setCurrentIndex(0);  // 无损最高质量

    // 保存重置后的设置
    saveCapturePreset();

    // 记录日志
    if (m_logManager) {
        m_logManager->log("拍照参数预设已重置为默认值", LogLevel::INFO);
    }

    statusBar()->showMessage("拍照参数预设已重置", 2000);
}

void MutiCamApp::applyCapturePreset()
{
    // 实时应用参数变化（可以在这里添加实时生效的逻辑）
    qDebug() << "应用拍照参数预设:"
             << "间隔=" << ui->spinBoxCaptureInterval->value() << "秒"
             << "自动保存=" << ui->checkBoxAutoSave->isChecked()
             << "格式=" << ui->comboBoxCaptureFormat->currentText()
             << "质量=" << ui->comboBoxImageQuality->currentText();

    // 延迟保存设置（避免频繁保存）
    if (m_settingsManager) {
        m_settingsManager->saveSettingsDelayed(this);
    }
}