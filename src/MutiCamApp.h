#pragma once
#include "ui_MutiCamApp.h"
#include "camera/camera_manager.h"
#include "VideoDisplayWidget.h"
#include "ZoomPanWidget.h"
#include "PaintingOverlay.h"
#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QPoint>
#include <QPointF>
#include <QProgressDialog>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <QMutex>
#include <QDateTime>
#include <QMap>
#include <memory>
#include <vector>
#include <chrono>
#include <algorithm>
#include <QCache>
#include <cmath>
#include <opencv2/opencv.hpp>
#include "image_processing/edge_detector.h"
#include "image_processing/shape_detector.h"
#include "SettingsManager.h"
#include "CalibrationDialog.h"
#include "CheckerboardCalibrationDialog.h"
#include "LogManager.h"
#include "TrajectoryRecorder.h"
#include "SerialController.h"
#include <functional>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class MutiCamApp : public QMainWindow
{
    Q_OBJECT

public:
    // 按钮映射结构，用于统一处理按钮连接
    struct ButtonMapping {
        QPushButton* button;
        std::function<void()> handler;
        QString category;           // 按钮类别：drawing, clear, undo, detection, save, grid
        QString viewName;          // 视图名称：all, vertical, left, front
        PaintingOverlay::DrawingTool tool; // 绘图工具类型

        ButtonMapping() : button(nullptr), tool(PaintingOverlay::DrawingTool::None) {}
        ButtonMapping(QPushButton* btn, std::function<void()> h, const QString& cat,
                     const QString& view, PaintingOverlay::DrawingTool t = PaintingOverlay::DrawingTool::None)
            : button(btn), handler(h), category(cat), viewName(view), tool(t) {}
    };

public:
    // 绘制数据结构已迁移到VideoDisplayWidget.h中

    MutiCamApp(QWidget *parent = nullptr);
    ~MutiCamApp();

    /**
     * @brief 获取指定视图的当前帧
     * @param viewName 视图名称
     * @return 当前帧的cv::Mat，如果没有则返回空Mat
     */
    cv::Mat getCurrentFrame(const QString& viewName) const;

    /**
     * @brief 启动自动检测
     * @param viewName 视图名称，空字符串表示当前活动视图
     * @param detectionType 检测类型（直线或圆形）
     */
    void startAutoDetection(const QString& viewName, PaintingOverlay::DrawingTool detectionType);

    /**
     * @brief 获取当前活动视图名称
     * @return 当前活动视图名称
     */
    QString getCurrentActiveViewName() const;

private slots:
    /**
     * @brief 开始测量按钮点击事件
     */
    void onStartMeasureClicked();
    
    /**
     * @brief 停止测量按钮点击事件
     */
    void onStopMeasureClicked();
    
    /**
     * @brief 处理相机帧就绪信号
     * @param cameraId 相机ID
     * @param frame 图像帧
     */
    void onCameraFrameReady(const QString& cameraId, const cv::Mat& frame);
    
    /**
     * @brief 处理测量结果信号
     * @param viewName 视图名称
     * @param result 测量结果
     */
    void onMeasurementResult(const QString& viewName, const QString& result);
    
    /**
     * @brief 处理VideoDisplayWidget选择状态变化信号
     * @param info 选择信息
     */
    void onSelectionChanged(const QString& info);
    
    /**
     * @brief 处理相机状态变化信号
     * @param cameraId 相机ID
     * @param state 相机状态
     */
    void onCameraStateChanged(const QString& cameraId, MutiCam::Camera::CameraState state);
    
    /**
     * @brief 处理相机错误信号
     * @param cameraId 相机ID
     * @param error 错误信息
     */
    void onCameraError(const QString& cameraId, const QString& error);

    /**
     * @brief 清空报警信息
     */
    void onClearAlertsClicked();

    /**
     * @brief 刷新相机状态
     */
    void onRefreshStatusClicked();

    /**
     * @brief 定时更新相机状态显示
     */
    void updateCameraStatusDisplay();

    /**
     * @brief 更新报警显示（槽函数）
     */
    void updateAlertDisplay();
    
    // 绘图、清空、撤销相关槽函数已被通用方法和按钮映射系统替代

    /**
     * @brief 保存图像按钮点击事件处理
     */
    void onSaveImageClicked();
    void onSaveImageVerticalClicked();
    void onSaveImageLeftClicked();
    void onSaveImageFrontClicked();
    void onAsyncSaveFinished();  // 异步保存完成槽函数

    /**
     * @brief 像素标定相关槽函数
     */
    void startPixelCalibration();  // 启动像素标定（弹窗选择方式）
    void startPixelCalibrationForView(const QString& viewName); // 启动指定视图的像素标定（弹窗选择方式）
    void startSinglePointCalibration(PaintingOverlay* overlay); // 启动单点标定
    void startMultiPointCalibration(PaintingOverlay* overlay);  // 启动多点标定
    void startCheckerboardCalibration(PaintingOverlay* overlay); // 启动棋盘格标定
    void loadCalibrationSettings(); // 加载标定设置
    void saveCalibrationSettings(); // 保存标定设置
    void syncCalibrationParameters(const QString& viewName); // 同步标定参数

    /**
     * @brief 自动检测按钮点击事件处理
     */
    void onAutoLineDetectionClicked();      // 主界面直线查找
    void onAutoCircleDetectionClicked();    // 主界面圆查找
    void onAutoLineDetectionVerticalClicked();  // 垂直视图直线查找
    void onAutoCircleDetectionVerticalClicked(); // 垂直视图圆查找
    void onAutoLineDetectionLeftClicked();      // 左侧视图直线查找
    void onAutoCircleDetectionLeftClicked();    // 左侧视图圆查找
    void onAutoLineDetectionFrontClicked();     // 对向视图直线查找
    void onAutoCircleDetectionFrontClicked();   // 对向视图圆查找

    /**
     * @brief 网格相关槽函数
     */
    void onGridDensityChanged();           // 主界面网格密度变化
    void onGridDensityVerticalChanged();   // 垂直视图网格密度变化
    void onGridDensityLeftChanged();       // 左侧视图网格密度变化
    void onGridDensityFrontChanged();      // 对向视图网格密度变化
    void onCancelGridsClicked();           // 主界面取消网格
    void onCancelGridsVerticalClicked();   // 垂直视图取消网格
    void onCancelGridsLeftClicked();       // 左侧视图取消网格
    void onCancelGridsFrontClicked();      // 对向视图取消网格

    /**
     * @brief 载物台控制按钮点击事件处理
     */
    void onMoveXLeftClicked();             // X轴负方向移动
    void onMoveXRightClicked();            // X轴正方向移动
    void onMoveYUpClicked();               // Y轴正方向移动
    void onMoveYDownClicked();             // Y轴负方向移动
    void onMoveZUpClicked();               // Z轴正方向移动
    void onMoveZDownClicked();             // Z轴负方向移动
    void onStageHomeClicked();             // 回到原点
    void onStageStopClicked();             // 紧急停止

    /**
     * @brief 轨迹记录控制槽函数
     */
    void onTrajectoryStartClicked();       // 开始记录轨迹
    void onTrajectoryStopClicked();        // 停止记录轨迹
    void onTrajectoryClearClicked();       // 清空轨迹
    void onTrajectoryExportClicked();      // 导出轨迹
    void onTrajectoryPointRecorded(const TrajectoryPoint& point);  // 轨迹点记录
    void onTrajectoryStatisticsUpdated(const TrajectoryStatistics& stats);  // 统计信息更新

    // {{ AURA-X: Add - 绘图同步槽函数. Approval: 寸止(ID:drawing_sync). }}
    void onDrawingSync(const QString& viewName);

    /**
     * @brief 记录最后活动的PaintingOverlay
     */
    void onOverlayActivated(PaintingOverlay* overlay);
    
    /**
     * @brief 选项卡切换事件处理
     * @param index 选项卡索引
     */
    void onTabChanged(int index);

    /**
     * @brief 视图双击事件处理
     * @param viewName 视图名称
     */
    void onViewDoubleClicked(const QString& viewName);

    /**
     * @brief 设置加载完成槽函数
     * @param success 是否成功
     */
    void onSettingsLoaded(bool success);

    /**
     * @brief 参数输入框文本改变槽函数（实时保存）
     */
    void onSettingsTextChanged();

    /**
     * @brief UI尺寸参数改变槽函数（调整窗口大小）
     */
    void onUISizeChanged();

    /**
     * @brief 相机序列号参数改变槽函数（重新初始化相机系统）
     */
    void onCameraSerialChanged();

    /**
     * @brief 模板匹配相关槽函数
     */
    void onCreateTemplateVerticalClicked();    // 垂直视图创建模板
    void onStartMatchingVerticalClicked();     // 垂直视图开始匹配

    // 视图控制槽函数
    /**
     * @brief 重置垂直视图缩放和平移
     */
    void onResetZoomVertical();

    /**
     * @brief 重置左视图缩放和平移
     */
    void onResetZoomLeft();

    /**
     * @brief 重置前视图缩放和平移
     */
    void onResetZoomFront();

    /**
     * @brief 视图变换改变事件处理
     * @param zoomFactor 缩放因子
     * @param panOffset 平移偏移
     */
    void onViewTransformChanged(double zoomFactor, const QPointF& panOffset);

    // 通用按钮处理方法
    /**
     * @brief 通用绘图工具启动方法
     * @param tool 绘图工具类型
     * @param viewName 目标视图名称（"all", "vertical", "left", "front"）
     */
    void startDrawingTool(PaintingOverlay::DrawingTool tool, const QString& viewName);

    /**
     * @brief 通用清空绘图方法
     * @param viewName 目标视图名称
     */
    void clearDrawings(const QString& viewName);

    /**
     * @brief 通用撤销方法
     * @param viewName 目标视图名称
     * @param isDetection 是否为撤销检测（true）还是撤销绘图（false）
     */
    void undoOperation(const QString& viewName, bool isDetection);

    /**
     * @brief 通用保存图像方法
     * @param viewName 目标视图名称
     */
    void saveImage(const QString& viewName);

protected:
    /**
     * @brief 窗口大小改变事件处理
     * @param event 大小改变事件
     */
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui_MutiCamApp* ui;

    // 按钮映射管理
    std::vector<ButtonMapping> m_buttonMappings;
    void initializeButtonMappings();
    void connectButtonSignals();

    // 相机管理器
    std::unique_ptr<MutiCam::Camera::CameraManager> m_cameraManager;

    // 载物台控制
    double getCurrentStepSize() const;               ///< 获取当前选择的步长
    std::unique_ptr<TrajectoryRecorder> m_trajectoryRecorder;  ///< 轨迹记录器
    double m_currentX, m_currentY, m_currentZ;       ///< 当前载物台位置
    void updateCurrentPosition(double deltaX, double deltaY, double deltaZ);  ///< 更新当前位置
    void updateTrajectoryDisplay();                  ///< 更新轨迹显示

    // 相机状态监控
    QTimer* m_statusUpdateTimer;                     ///< 状态更新定时器

    // 帧率计算
    struct FrameRateData {
        std::chrono::steady_clock::time_point lastCalculateTime;  // 上次计算时间
        std::vector<std::chrono::steady_clock::time_point> frameTimes;  // 帧时间戳队列
        double currentFPS;                                        // 当前帧率
        bool hasFirstFrame;                                       // 是否已收到第一帧
        static constexpr int WINDOW_SECONDS = 5;                  // 时间窗口：5秒

        FrameRateData() : currentFPS(0.0), hasFirstFrame(false) {
            lastCalculateTime = std::chrono::steady_clock::now();
            frameTimes.reserve(300);  // 预分配空间，假设最大60fps*5秒
        }
    };

    QMutex m_frameRateMutex;                         ///< 帧率数据保护锁
    QMap<QString, FrameRateData> m_frameRateData;    ///< 各相机帧率数据

    // 报警系统
    struct AlertMessage {
        QString message;
        QString level;  // "info", "warning", "error"
        QDateTime timestamp;

        AlertMessage() = default;  // 默认构造函数

        AlertMessage(const QString& msg, const QString& lvl)
            : message(msg), level(lvl), timestamp(QDateTime::currentDateTime()) {}
    };

    QList<AlertMessage> m_alertMessages;             ///< 报警消息列表
    QMutex m_alertMutex;                            ///< 报警消息保护锁
    
    /**
     * @brief 同步指定视图的视频层和绘画层的坐标变换
     * @param viewName 视图名称 ("vertical", "left", "front", etc.)
     */
    void syncOverlayTransforms(const QString& viewName);

    // 相机状态监控私有方法
    /**
     * @brief 更新相机状态总览
     */
    void updateCameraOverview();

    /**
     * @brief 更新单个相机状态显示
     * @param cameraId 相机ID
     */
    void updateSingleCameraStatus(const QString& cameraId);

    /**
     * @brief 计算并更新帧率
     * @param cameraId 相机ID
     */
    void updateFrameRate(const QString& cameraId);

    /**
     * @brief 添加报警消息
     * @param message 消息内容
     * @param level 消息级别 ("info", "warning", "error")
     */
    void addAlertMessage(const QString& message, const QString& level);

    /**
     * @brief 初始化相机状态监控
     */
    void initializeCameraStatusMonitoring();
    
    // 测量状态
    bool m_isMeasuring;
    
    // 绘制功能已完全迁移到VideoDisplayWidget，不再需要绘制状态管理
    
    // 当前帧存储
    cv::Mat m_currentFrameVertical;          ///< 垂直视图当前帧
    cv::Mat m_currentFrameLeft;              ///< 左视图当前帧
    cv::Mat m_currentFrameFront;             ///< 前视图当前帧
    
    // 性能优化：缓存机制
    struct CachedFrame {
        cv::Mat frame;
        std::chrono::steady_clock::time_point timestamp;
        QString viewName;
        size_t pointsHash;  // 点数据的哈希值，用于检测变化
    };
    
    QCache<QString, CachedFrame> m_frameCache;           ///< 帧缓存
    static constexpr int CACHE_TIMEOUT_MS = 500;            ///< 缓存超时时间（毫秒）
    static constexpr int MAX_CACHED_FRAMES = 10;             ///< 最大缓存帧数
    
    // 渲染缓冲区
    QMap<QString, cv::Mat> m_renderBuffers;              ///< 各视图的渲染缓冲区

    // 图像转换优化缓冲区
    mutable cv::Mat m_rgbConvertBuffer;                  ///< RGB转换缓冲区，避免重复分配
    
    // 缩放平移显示控件
    ZoomPanWidget* m_verticalZoomPanWidget;         ///< 垂直视图缩放平移控件
    ZoomPanWidget* m_leftZoomPanWidget;            ///< 左视图缩放平移控件
    ZoomPanWidget* m_frontZoomPanWidget;           ///< 前视图缩放平移控件

    // 选项卡ZoomPanWidget
    ZoomPanWidget* m_verticalZoomPanWidget2;
    ZoomPanWidget* m_leftZoomPanWidget2;
    ZoomPanWidget* m_frontZoomPanWidget2;
    
    // RotatableROIOverlay用于模板匹配
    RotatableROIOverlay* m_verticalROIOverlay;
    
    // PaintingOverlay 成员变量
    PaintingOverlay* m_verticalPaintingOverlay;
    PaintingOverlay* m_leftPaintingOverlay;
    PaintingOverlay* m_frontPaintingOverlay;
    
    // 选项卡PaintingOverlay
    PaintingOverlay* m_verticalPaintingOverlay2;
    PaintingOverlay* m_leftPaintingOverlay2;
    PaintingOverlay* m_frontPaintingOverlay2;

    // 记录最后活动的PaintingOverlay，用于撤销功能
    PaintingOverlay* m_lastActivePaintingOverlay;
    
    /**
     * @brief 初始化相机系统
     */
    void initializeCameraSystem();
    
    /**
     * @brief 连接信号和槽
     */
    void connectSignalsAndSlots();
    
    /**
     * @brief 高DPI显示帧到标签（内部辅助函数）
     * @param label 显示标签
     * @param frame 图像帧
     */
    void displayFrameWithHighDPI(QLabel* label, const cv::Mat& frame);
    
    // 图像显示方法已迁移到 VideoDisplayWidget
    // void displayImageOnLabel(QLabel* label, const cv::Mat& frame, const QString& viewName = QString());
    
    /**
     * @brief 将OpenCV Mat转换为QPixmap
     * @param mat OpenCV图像
     * @return QPixmap图像
     */
    QPixmap matToQPixmap(const cv::Mat& mat, bool setDevicePixelRatio = true);
    
    // 绘制模式管理方法已移除，现在直接通过按钮槽函数调用VideoDisplayWidget的startDrawing方法
    
    // 所有绘图处理方法已迁移到 VideoDisplayWidget
    
    // 线与线按钮点击事件处理已被通用方法替代
    
    // 几何计算方法已迁移到 VideoDisplayWidget
    
    /**
     * @brief 安装鼠标事件过滤器
     */
    void installMouseEventFilters();
    
    /**
     * @brief 事件过滤器
     */
    bool eventFilter(QObject* obj, QEvent* event) override;

    // {{ AURA-X: Delete - 残留的标签点击处理方法. Approval: 寸止(ID:cleanup). }}
    // 标签点击处理已移除，现在直接使用VideoDisplayWidget
    
    /**
     * @brief 处理ZoomPanWidget鼠标点击
     * @param widget 被点击的ZoomPanWidget
     * @param pos 点击位置
     */
    void handleZoomPanWidgetClick(ZoomPanWidget* widget, const QPoint& pos);

    // {{ AURA-X: Delete - 残留的鼠标移动处理方法. Approval: 寸止(ID:cleanup). }}
    // 鼠标移动处理已移除，现在直接使用ZoomPanWidget

    /**
     * @brief 处理ZoomPanWidget鼠标移动事件
     * @param widget 鼠标所在的ZoomPanWidget
     * @param pos 鼠标位置
     */
    void handleZoomPanWidgetMouseMove(ZoomPanWidget* widget, const QPoint& pos);
    

    
    /**
     * @brief 获取视图名称
     * @param label 标签指针
     * @return 视图名称
     */
    QString getViewName(QLabel* label);
    
    /**
     * @brief 获取视图名称
     * @param widget ZoomPanWidget指针
     * @return 视图名称
     */
    QString getViewName(ZoomPanWidget* widget);
    
    /**
     * @brief 更新视图显示
     * @param viewName 视图名称
     */
    void updateViewDisplay(const QString& viewName);
    
    /**
      * @brief 将窗口坐标转换为图像坐标
      * @param label 标签
      * @param windowPos 窗口坐标
      * @return 图像坐标
      */
     QPointF windowToImageCoordinates(QLabel* label, const QPoint& windowPos);
     
     /**
      * @brief 将窗口坐标转换为图像坐标
      * @param widget ZoomPanWidget
      * @param windowPos 窗口坐标
      * @return 图像坐标
      */
     QPointF windowToImageCoordinates(ZoomPanWidget* widget, const QPoint& windowPos);
     
     // 性能优化相关方法
     
     /**
      * @brief 获取缓存的帧
      * @param viewName 视图名称
      * @param originalFrame 原始帧
      * @return 缓存的渲染帧，如果没有缓存则返回空Mat
      */
     cv::Mat getCachedFrame(const QString& viewName, const cv::Mat& originalFrame);
     
     /**
      * @brief 缓存渲染后的帧
      * @param viewName 视图名称
      * @param renderedFrame 渲染后的帧
      */
     void cacheFrame(const QString& viewName, const cv::Mat& renderedFrame);
     
     /**
      * @brief 计算点数据的哈希值
      * @param viewName 视图名称
      * @return 哈希值
      */
     size_t calculatePointsHash(const QString& viewName);
     
     /**
      * @brief 使缓存失效
      * @param viewName 视图名称，为空则清空所有缓存
      */
     void invalidateCache(const QString& viewName = QString());
     
     /**
      * @brief 检查指定视图是否有绘制数据
      * @param viewName 视图名称
      * @return 是否有绘制数据
      */
     bool hasDrawingData(const QString& viewName);
     
     // 文本绘制方法已迁移到 VideoDisplayWidget
     
     /**
      * @brief 初始化缩放平移显示控件
      */
     void initializeZoomPanWidgets();

     /**
      * @brief 初始化设置管理器
      */
     void initializeSettingsManager();

     /**
      * @brief 初始化日志管理器
      */
     void initializeLogManager();

     /**
      * @brief 初始化轨迹记录器
      */
     void initializeTrajectoryRecorder();

     /**
      * @brief 连接参数设置的实时保存信号
      */
     void connectSettingsSignals();

     /**
      * @brief 连接相机管理器信号
      */
     void connectCameraManagerSignals();

     /**
      * @brief 根据设置参数应用UI尺寸
      */
     void applyUISizeFromSettings();

     /**
      * @brief 重新初始化相机系统（当序列号参数改变时）
      */
     void reinitializeCameraSystem();
     
     /**
      * @brief 更新ZoomPanWidget的绘制数据
      * @param viewName 视图名称
      * @param frame 图像帧
      */
     void updateZoomPanWidget(const QString& viewName, const cv::Mat& frame);

private:
     /**
      * @brief 从UI获取边缘检测参数
      * @return EdgeDetector参数结构体
      */
     EdgeDetector::EdgeDetectionParams getEdgeDetectionParams() const;

     /**
      * @brief 从UI获取直线检测参数
      * @return ShapeDetector直线检测参数结构体
      */
     ShapeDetector::LineDetectionParams getLineDetectionParams() const;

     /**
      * @brief 从UI获取圆形检测参数
      * @return ShapeDetector圆形检测参数结构体
      */
     ShapeDetector::CircleDetectionParams getCircleDetectionParams() const;

     /**
      * @brief 配置检测参数
      * @param overlay PaintingOverlay指针
      * @param detectionType 检测类型
      */
     void configureDetectionParameters(PaintingOverlay* overlay, PaintingOverlay::DrawingTool detectionType);

     /**
      * @brief 初始化检测参数默认值
      */
     void initializeDetectionParameters();
     

     
     /**
      * @brief 获取指定视图的ZoomPanWidget
      * @param viewName 视图名称
      * @return ZoomPanWidget指针
      */
     ZoomPanWidget* getZoomPanWidget(const QString& viewName);

     /**
      * @brief 获取当前活动的ZoomPanWidget（基于当前选项卡）
      * @return 当前活动的ZoomPanWidget指针
      */
     ZoomPanWidget* getActiveZoomPanWidget();
     
     /**
      * @brief 获取指定视图的PaintingOverlay
      * @param viewName 视图名称
      * @return PaintingOverlay指针
      */
     PaintingOverlay* getPaintingOverlay(const QString& viewName);
     
     /**
      * @brief 获取当前活动的PaintingOverlay（基于当前选项卡）
      * @return 当前活动的PaintingOverlay指针
      */
     PaintingOverlay* getActivePaintingOverlay();

     /**
      * @brief 保存图像相关方法
      */
     void saveImages(const QString& viewType);
     cv::Mat renderVisualizedImage(const cv::Mat& originalFrame, PaintingOverlay* overlay);
     QString createSaveDirectory(const QString& viewName);

     // 异步保存相关方法
     void saveImagesAsync(const QStringList& viewTypes);
     void saveImagesSynchronous(const QStringList& viewTypes);

     // 异步保存相关成员变量
     QProgressDialog* m_saveProgressDialog;
     QFutureWatcher<void>* m_saveWatcher;

     // 设置管理器
     SettingsManager* m_settingsManager;

     // 日志管理器
     LogManager* m_logManager;

     // 串口控制器
     SerialController* m_serialController;

     // UI尺寸双向绑定相关
     bool m_isUpdatingUISize;           ///< 正在更新UI尺寸标志，避免循环触发

     // 硬件加速显示方法已迁移到VideoDisplayWidget

     // 物理按钮控制相关方法
     void initializeSerialController();
     void updateButtonStatus(const QString& buttonName, bool isPressed);
     void handleButtonEvent(SerialController::ButtonEvent event);
    void executeCaptureAction();
    void executeCaptureByType(const QString& actionType);
    void testCaptureButton();
    void toggleSerialConnection();

    // 参数预设相关方法
    void initializeCapturePresets();
    void saveCapturePreset();
    void loadCapturePreset();
    void resetCapturePreset();
    void applyCapturePreset();

    // 模板匹配相关私有方法
    void onROIDrawingCompleted(const QString& viewName);
    void onROIEditingFinished();
    void saveTemplate(const cv::Mat& templateImage, const QString& templateName, const QString& viewName);
    void performTemplateMatching(const QString& templateFileName, const QString& viewName);

};