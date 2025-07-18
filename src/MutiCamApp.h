#pragma once
#include "ui_MutiCamApp.h"
#include "camera/camera_manager.h"
#include "VideoDisplayWidget.h"
#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QMouseEvent>
#include <QPoint>
#include <QPointF>
#include <memory>
#include <vector>
#include <chrono>
#include <QCache>
#include <cmath>
#include <opencv2/opencv.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class MutiCamApp : public QMainWindow
{
    Q_OBJECT

public:
    // 绘制数据结构已迁移到VideoDisplayWidget.h中

    MutiCamApp(QWidget *parent = nullptr);
    ~MutiCamApp();

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
     * @brief 画点按钮点击事件处理
     */
    void onDrawPointClicked();
    void onDrawPointVerticalClicked();
    void onDrawPointLeftClicked();
    void onDrawPointFrontClicked();
    
    /**
     * @brief 画直线按钮点击事件处理
     */
    void onDrawLineClicked();
    void onDrawLineVerticalClicked();
    void onDrawLineLeftClicked();
    void onDrawLineFrontClicked();
    
    /**
     * @brief 画圆按钮点击事件处理
     */
    void onDrawSimpleCircleClicked();
    void onDrawSimpleCircleVerticalClicked();
    void onDrawSimpleCircleLeftClicked();
    void onDrawSimpleCircleFrontClicked();
    
    /**
     * @brief 画精细圆按钮点击事件处理
     */
    void onDrawFineCircleClicked();
    void onDrawFineCircleVerticalClicked();
    void onDrawFineCircleLeftClicked();
    void onDrawFineCircleFrontClicked();
    
    /**
     * @brief 画平行线按钮点击事件处理
     */
    void onDrawParallelClicked();
    void onDrawParallelVerticalClicked();
    void onDrawParallelLeftClicked();
    void onDrawParallelFrontClicked();
    
    /**
     * @brief 清空绘图按钮点击事件处理
     */
    void onClearDrawingsClicked();
    void onClearDrawingsVerticalClicked();
    void onClearDrawingsLeftClicked();
    void onClearDrawingsFrontClicked();
    
    // {{ AURA-X: Add - 绘图同步槽函数. Approval: 寸止(ID:drawing_sync). }}
    void onDrawingSync(const QString& viewName);
    
    /**
     * @brief 选项卡切换事件处理
     * @param index 选项卡索引
     */
    void onTabChanged(int index);

private:
    Ui_MutiCamApp* ui;
    
    // 相机管理器
    std::unique_ptr<MutiCam::Camera::CameraManager> m_cameraManager;
    
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
    std::chrono::steady_clock::time_point m_lastUpdateTime; ///< 上次更新时间
    static constexpr double UPDATE_INTERVAL = 1.0 / 60.0;   ///< 更新间隔（60fps限制）
    static constexpr int CACHE_TIMEOUT_MS = 500;            ///< 缓存超时时间（毫秒）
    static constexpr int MAX_CACHED_FRAMES = 10;             ///< 最大缓存帧数
    
    // 渲染缓冲区
    QMap<QString, cv::Mat> m_renderBuffers;              ///< 各视图的渲染缓冲区
    
    // 硬件加速显示控件
    VideoDisplayWidget* m_verticalDisplayWidget;         ///< 垂直视图显示控件
    VideoDisplayWidget* m_leftDisplayWidget;            ///< 左视图显示控件
    VideoDisplayWidget* m_frontDisplayWidget;           ///< 前视图显示控件
    
    // 选项卡VideoDisplayWidget
    VideoDisplayWidget* m_verticalDisplayWidget2;
    VideoDisplayWidget* m_leftDisplayWidget2;
    VideoDisplayWidget* m_frontDisplayWidget2;
    
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
    
    /**
     * @brief 线与线按钮点击事件处理
     */
    void onDrawTwoLinesClicked();
    
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
     * @brief 处理VideoDisplayWidget鼠标点击
     * @param widget 被点击的VideoDisplayWidget
     * @param pos 点击位置
     */
    void handleVideoWidgetClick(VideoDisplayWidget* widget, const QPoint& pos);
    
    // {{ AURA-X: Delete - 残留的鼠标移动处理方法. Approval: 寸止(ID:cleanup). }}
    // 鼠标移动处理已移除，现在直接使用VideoDisplayWidget
    
    /**
     * @brief 处理VideoDisplayWidget鼠标移动事件
     * @param widget 鼠标所在的VideoDisplayWidget
     * @param pos 鼠标位置
     */
    void handleVideoWidgetMouseMove(VideoDisplayWidget* widget, const QPoint& pos);
    

    
    /**
     * @brief 获取视图名称
     * @param label 标签指针
     * @return 视图名称
     */
    QString getViewName(QLabel* label);
    
    /**
     * @brief 获取视图名称
     * @param widget VideoDisplayWidget指针
     * @return 视图名称
     */
    QString getViewName(VideoDisplayWidget* widget);
    
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
      * @param widget VideoDisplayWidget
      * @param windowPos 窗口坐标
      * @return 图像坐标
      */
     QPointF windowToImageCoordinates(VideoDisplayWidget* widget, const QPoint& windowPos);
     
     // 性能优化相关方法
     /**
      * @brief 检查是否需要更新（更新频率控制）
      * @return 是否需要更新
      */
     bool shouldUpdate();
     
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
      * @brief 初始化硬件加速显示控件
      */
     void initializeVideoDisplayWidgets();
     
     /**
      * @brief 更新VideoDisplayWidget的绘制数据
      * @param viewName 视图名称
      * @param frame 图像帧
      */
     void updateVideoDisplayWidget(const QString& viewName, const cv::Mat& frame);
     

     
     /**
      * @brief 获取指定视图的VideoDisplayWidget
      * @param viewName 视图名称
      * @return VideoDisplayWidget指针
      */
     VideoDisplayWidget* getVideoDisplayWidget(const QString& viewName);
     
     /**
      * @brief 获取当前活动的VideoDisplayWidget（基于当前选项卡）
      * @return 当前活动的VideoDisplayWidget指针
      */
     VideoDisplayWidget* getActiveVideoWidget();
     
     // 硬件加速显示方法已迁移到VideoDisplayWidget
      


};