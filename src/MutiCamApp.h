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
    // 直线绘制相关数据结构
    struct LineObject {
        QVector<QPointF> points;             ///< 直线的点集合（至少2个点）
        bool isCompleted;                    ///< 是否完成绘制
        QColor color;                        ///< 直线颜色
        int thickness;                       ///< 线条粗细
        bool isDashed;                       ///< 是否为虚线
        
        LineObject() : isCompleted(false), color(Qt::green), thickness(2), isDashed(false) {}
    };
    
    // 圆形绘制相关数据结构
    struct CircleObject {
        QVector<QPointF> points;             ///< 圆的点集合（需要3个点确定圆）
        bool isCompleted;                    ///< 是否完成绘制
        QColor color;                        ///< 圆的颜色
        int thickness;                       ///< 线条粗细
        QPointF center;                      ///< 圆心坐标
        double radius;                       ///< 半径
        
        CircleObject() : isCompleted(false), color(QColor(0, 255, 0)), thickness(3), radius(0.0) {}
    };
    
    // 精细圆绘制相关数据结构
    struct FineCircleObject {
        QVector<QPointF> points;             ///< 精细圆的点集合（需要5个点拟合圆）
        bool isCompleted;                    ///< 是否完成绘制
        QColor color;                        ///< 圆的颜色
        int thickness;                       ///< 线条粗细
        QPointF center;                      ///< 圆心坐标
        double radius;                       ///< 半径
        
        FineCircleObject() : isCompleted(false), color(Qt::magenta), thickness(3), radius(0.0) {}
    };
    
    // 平行线绘制相关数据结构
    struct ParallelObject {
        QVector<QPointF> points;             ///< 平行线的点集合（需要3个点：第一条线2点+第二条线1点）
        bool isCompleted;                    ///< 是否完成绘制
        QColor color;                        ///< 线条颜色
        int thickness;                       ///< 线条粗细
        double distance;                     ///< 两线之间的距离
        double angle;                        ///< 直线角度（相对于水平线）
        
        ParallelObject() : isCompleted(false), color(Qt::green), thickness(2), distance(0.0), angle(0.0) {}
    };
    
    // 线与线绘制相关数据结构
    struct TwoLinesObject {
        QVector<QPointF> points;             ///< 两条线的点集合（需要4个点：第一条线2点+第二条线2点）
        bool isCompleted;                    ///< 是否完成绘制
        QColor color;                        ///< 线条颜色
        int thickness;                       ///< 线条粗细
        double angle;                        ///< 两线夹角（度数）
        QPointF intersection;                ///< 交点坐标
        bool isPreview;                      ///< 标识是否为预览状态
        
        TwoLinesObject() : isCompleted(false), color(Qt::green), thickness(2), angle(0.0), isPreview(false) {}
    };

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
     * @brief 处理VideoDisplayWidget绘制数据变化信号
     * @param viewName 视图名称
     */
    void onDrawingDataChanged(const QString& viewName);
    
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
    
    // 绘制功能相关成员变量
    bool m_isDrawingMode;                    ///< 是否处于绘制模式
    QString m_activeView;                    ///< 当前激活的视图名称
    QString m_currentDrawingType;            ///< 当前绘制类型（"point" 或 "line"）
    
    QMap<QString, QVector<QPointF>> m_viewData;  ///< 存储各视图的点数据
    
    QMap<QString, QVector<LineObject>> m_lineData;  ///< 存储各视图的直线数据
    QMap<QString, LineObject> m_currentLines;       ///< 当前正在绘制的直线
    
    QMap<QString, QVector<CircleObject>> m_circleData;  ///< 存储各视图的圆形数据
    QMap<QString, CircleObject> m_currentCircles;       ///< 当前正在绘制的圆形
    
    QMap<QString, QVector<FineCircleObject>> m_fineCircleData;  ///< 存储各视图的精细圆数据
    QMap<QString, FineCircleObject> m_currentFineCircles;       ///< 当前正在绘制的精细圆
    
    QMap<QString, QVector<ParallelObject>> m_parallelData;      ///< 存储各视图的平行线数据
    QMap<QString, ParallelObject> m_currentParallels;           ///< 当前正在绘制的平行线
    QMap<QString, ParallelObject> m_tempParallelPreview;        ///< 临时平行线预览数据（用于QLabel视图）
    
    QMap<QString, QVector<TwoLinesObject>> m_twoLinesData;      ///< 存储各视图的线与线数据
    QMap<QString, TwoLinesObject> m_currentTwoLines;            ///< 当前正在绘制的线与线
    
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
    
    /**
     * @brief 设置绘制模式
     * @param drawingType 绘制类型（"point"、"line" 或 "circle"）
     * @param viewName 视图名称
     */
    void setDrawingMode(const QString& drawingType, const QString& viewName);
    
    /**
     * @brief 退出绘制模式
     */
    void exitDrawingMode();
    
    // 绘图方法已迁移到 VideoDisplayWidget
    // void drawPointsOnImage(cv::Mat& image, const QString& viewName);
    // void drawLinesOnImage(cv::Mat& image, const QString& viewName);
    // void drawCirclesOnImage(cv::Mat& image, const QString& viewName);
    // void drawFineCirclesOnImage(cv::Mat& image, const QString& viewName);
    // void drawParallelLinesOnImage(cv::Mat& image, const QString& viewName);
    // void drawTwoLinesOnImage(cv::Mat& image, const QString& viewName);
    
    // 单个绘图方法已迁移到 VideoDisplayWidget
    // void drawSingleLine(cv::Mat& image, const LineObject& line, bool isCurrentDrawing = false);
    // QPair<QPointF, QPointF> calculateExtendedLine(const QPointF& p1, const QPointF& p2, const QSize& imageSize);
    // void drawDashedLine(cv::Mat& image, const cv::Point& start, const cv::Point& end, const cv::Scalar& color, int thickness);
    
    // 绘制处理方法已迁移到 VideoDisplayWidget
    // double calculateLineAngle(const QPointF& p1, const QPointF& p2);
    // void handleLineDrawingClick(const QString& viewName, const QPointF& imagePos);
    // void handleCircleDrawingClick(const QString& viewName, const QPointF& imagePos);
    // void handleFineCircleDrawingClick(const QString& viewName, const QPointF& imagePos);
    // void handleParallelDrawingClick(const QString& viewName, const QPointF& imagePos);
    // void handleTwoLinesDrawingClick(const QString& viewName, const QPointF& imagePos);
    
    // 单个绘图方法已迁移到 VideoDisplayWidget
    // void drawSingleCircle(cv::Mat& image, const CircleObject& circle, bool isCurrentDrawing = false);
    // void drawSingleFineCircle(cv::Mat& image, const FineCircleObject& fineCircle, bool isCurrentDrawing = false);
    // void drawSingleParallel(cv::Mat& image, const ParallelObject& parallel, bool isCurrentDrawing = false);
    // void drawSingleTwoLines(cv::Mat& image, const TwoLinesObject& twoLines, bool isCurrentDrawing = false);
    
    // 字体计算方法已迁移到 VideoDisplayWidget
    // double calculateFontScale(int height);
    
    // 几何计算方法已迁移到 VideoDisplayWidget
    // bool calculateLineIntersection(const QPointF& p1, const QPointF& p2, const QPointF& p3, const QPointF& p4, QPointF& intersection);
    // void extendLineToImageBounds(const cv::Mat& image, const cv::Point& point, const cv::Point2f& direction, cv::Point& start, cv::Point& end);
    
    // 虚线绘制方法已迁移到 VideoDisplayWidget
    // void drawDashedLine(cv::Mat& image, const cv::Point& start, const cv::Point& end, const cv::Scalar& color, int thickness, int dashLength, int gapLength);
    
    /**
     * @brief 线与线按钮点击事件处理
     */
    void onDrawTwoLinesClicked();
    
    // 延长线计算方法已迁移到 VideoDisplayWidget
    // void calculateExtendedLine(const cv::Point& start, const cv::Point& end, const cv::Size& imageSize, cv::Point& extendedStart, cv::Point& extendedEnd);
    
    // 字体大小计算方法已迁移到 VideoDisplayWidget
    // double calculateFontScale(const cv::Mat& image);
    
    // 三点计算圆心方法已迁移到 VideoDisplayWidget
    // bool calculateCircleFromThreePoints(const QPointF& p1, const QPointF& p2, const QPointF& p3, QPointF& center, double& radius);
    
    // 五点拟合圆心方法已迁移到 VideoDisplayWidget
    // bool calculateCircleFromFivePoints(const QVector<QPointF>& points, QPointF& center, double& radius);
    
    /**
     * @brief 安装鼠标事件过滤器
     */
    void installMouseEventFilters();
    
    /**
     * @brief 事件过滤器
     */
    bool eventFilter(QObject* obj, QEvent* event) override;
    
    /**
     * @brief 处理标签鼠标点击
     * @param label 被点击的标签
     * @param pos 点击位置
     */
    void handleLabelClick(QLabel* label, const QPoint& pos);
    
    /**
     * @brief 处理VideoDisplayWidget鼠标点击
     * @param widget 被点击的VideoDisplayWidget
     * @param pos 点击位置
     */
    void handleVideoWidgetClick(VideoDisplayWidget* widget, const QPoint& pos);
    
    /**
     * @brief 处理鼠标移动事件
     * @param label 鼠标所在的标签
     * @param pos 鼠标位置
     */
    void handleMouseMove(QLabel* label, const QPoint& pos);
    
    /**
     * @brief 处理VideoDisplayWidget鼠标移动事件
     * @param widget 鼠标所在的VideoDisplayWidget
     * @param pos 鼠标位置
     */
    void handleVideoWidgetMouseMove(VideoDisplayWidget* widget, const QPoint& pos);
    
    // 绘制渲染方法已迁移到 VideoDisplayWidget
    // cv::Mat renderDrawingsOnFrame(const cv::Mat& frame, const QString& viewName);
    
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
     
     // 带背景文本绘制方法已迁移到 VideoDisplayWidget
     // void drawTextWithBackground(cv::Mat& image, const std::string& text, const cv::Point& position, double fontScale, const cv::Scalar& textColor, const cv::Scalar& bgColor, int thickness = 1, int padding = 3);
     
     // 文本背景尺寸计算方法已迁移到 VideoDisplayWidget
     // cv::Size calculateTextBackgroundSize(const std::string& text, double fontScale, int thickness, int padding);
     
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
      * @brief 更新指定视图的几何图形数据（事件驱动）
      * @param viewName 视图名称
      */
     void updateDrawingDataForView(const QString& viewName);
     
     /**
      * @brief 更新单个控件的绘制数据
      * @param widget VideoDisplayWidget指针
      * @param viewName 视图名称
      */
     void updateWidgetDrawingData(VideoDisplayWidget* widget, const QString& viewName);
     
     /**
      * @brief 获取指定视图的VideoDisplayWidget
      * @param viewName 视图名称
      * @return VideoDisplayWidget指针
      */
     VideoDisplayWidget* getVideoDisplayWidget(const QString& viewName);
     
     /**
       * @brief 使用硬件加速显示图像
       * @param viewName 视图名称
       * @param frame 图像帧
       */
      // {{ AURA-X: Delete - 绘图功能已迁移到VideoDisplayWidget. Approval: 寸止(ID:migration_cleanup). }}
    // displayImageWithHardwareAcceleration方法已迁移到VideoDisplayWidget
      
      // 类型转换函数
      /**
       * @brief 将MutiCamApp::LineObject转换为VideoDisplayWidget的LineObject
       * @param lines MutiCamApp的LineObject列表
       * @return VideoDisplayWidget的LineObject列表
       */
      QVector<::LineObject> convertToWidgetLineObjects(const QVector<LineObject>& lines);
      
      /**
       * @brief 将MutiCamApp::CircleObject转换为VideoDisplayWidget的CircleObject
       * @param circles MutiCamApp的CircleObject列表
       * @return VideoDisplayWidget的CircleObject列表
       */
      QVector<::CircleObject> convertToWidgetCircleObjects(const QVector<CircleObject>& circles);
      
      /**
       * @brief 将MutiCamApp::FineCircleObject转换为VideoDisplayWidget的FineCircleObject
       * @param fineCircles MutiCamApp的FineCircleObject列表
       * @return VideoDisplayWidget的FineCircleObject列表
       */
      QVector<::FineCircleObject> convertToWidgetFineCircleObjects(const QVector<FineCircleObject>& fineCircles);
      
      /**
       * @brief 将MutiCamApp::ParallelObject转换为VideoDisplayWidget的ParallelObject
       * @param parallels MutiCamApp的ParallelObject列表
       * @return VideoDisplayWidget的ParallelObject列表
       */
      QVector<::ParallelObject> convertToWidgetParallelObjects(const QVector<ParallelObject>& parallels);
      
      /**
       * @brief 将MutiCamApp::TwoLinesObject转换为VideoDisplayWidget的TwoLinesObject
       * @param twoLines MutiCamApp的TwoLinesObject列表
       * @return VideoDisplayWidget的TwoLinesObject列表
       */
      QVector<::TwoLinesObject> convertToWidgetTwoLinesObjects(const QVector<TwoLinesObject>& twoLines);

};