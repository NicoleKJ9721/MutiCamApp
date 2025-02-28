#ifndef DRAWINGMANAGER_H
#define DRAWINGMANAGER_H

#include <QObject>
#include <QPoint>
#include <QMap>
#include <QLabel>
#include <QCursor>
#include <QMenu>
#include <QAction>
#include <opencv2/opencv.hpp>

// 绘图对象类型枚举
enum class DrawingType {
    NONE,
    POINT,
    LINE_SEGMENT,
    LINE,
    CIRCLE,
    PARALLEL,
    CIRCLE_LINE,
    TWO_LINES,
    LINE_DETECT,
    CIRCLE_DETECT,
    POINT_TO_LINE
};

// 绘图对象类
class DrawingObject {
public:
    DrawingObject(DrawingType type = DrawingType::NONE);
    
    DrawingType type;
    QVector<QPoint> points;
    QMap<QString, QVariant> properties;
    bool selected;
    bool visible;
    int id;
    
    static int nextId;
};

// 图层管理器类
class LayerManager : public QObject {
    Q_OBJECT
    
public:
    explicit LayerManager(QObject *parent = nullptr);
    
    // 开始绘制
    void startDrawing(DrawingType type, const QMap<QString, QVariant>& properties);
    
    // 更新当前绘制对象
    void updateCurrentObject(const QPoint& point);
    
    // 提交绘制
    void commitDrawing();
    
    // 渲染帧
    cv::Mat renderFrame(const cv::Mat& frame);
    
    // 渲染单个对象
    void renderObject(cv::Mat& frame, DrawingObject* obj);
    
    // 清空绘制
    void clearDrawings();
    
    // 清空选择
    void clearSelection();
    
    // 查找点击位置的对象
    DrawingObject* findObjectAtPoint(const QPoint& point, int tolerance = 10);
    
    // 绘制点
    void drawPoint(cv::Mat& frame, DrawingObject* obj);
    
    // 当前绘制对象
    DrawingObject* currentObject;
    
    // 所有绘制对象
    QVector<DrawingObject*> drawingObjects;
};

// 测量管理器类
class MeasurementManager : public QObject {
    Q_OBJECT
    
public:
    explicit MeasurementManager(QObject *parent = nullptr);
    ~MeasurementManager();
    
    // 启动各种测量模式
    void startPointMeasurement();
    void startLineMeasurement();
    void startCircleMeasurement();
    void startLineSegmentMeasurement();
    void startParallelMeasurement();
    void startCircleLineMeasurement();
    void startTwoLinesMeasurement();
    void startLineDetection();
    void startCircleDetection();
    
    // 清空测量
    void clearMeasurements();
    
    // 处理鼠标事件
    cv::Mat handleMousePress(const QPoint& eventPos, const cv::Mat& currentFrame);
    cv::Mat handleMouseMove(const QPoint& eventPos, const cv::Mat& currentFrame);
    cv::Mat handleMouseRelease(const QPoint& eventPos, const cv::Mat& currentFrame);
    cv::Mat handleMouseRightClick(const QPoint& eventPos, const cv::Mat& currentFrame);
    
    // 图层管理器
    LayerManager* layerManager;
    
    // 绘制模式
    DrawingType drawMode;
    
    // 是否正在绘制
    bool drawing;
    
    // 绘制历史
    QVector<DrawingObject*> drawingHistory;
};

// 绘图管理器类
class DrawingManager : public QObject {
    Q_OBJECT
    
public:
    explicit DrawingManager(QObject *parent = nullptr);
    ~DrawingManager();
    
    // 初始化
    void initialize();
    
    // 启动各种测量模式
    void startPointMeasurement();
    void startLineMeasurement();
    void startCircleMeasurement();
    void startLineSegmentMeasurement();
    void startParallelMeasurement();
    void startCircleLineMeasurement();
    void startTwoLinesMeasurement();
    void startLineDetection();
    void startCircleDetection();
    
    // 进入选择模式
    void enterSelectionMode();
    
    // 清空绘制
    void clearDrawings(QLabel* label = nullptr);
    
    // 处理鼠标事件
    void handleMousePress(QMouseEvent* event, QLabel* label);
    void handleMouseMove(QMouseEvent* event, QLabel* label);
    void handleMouseRelease(QMouseEvent* event, QLabel* label);
    
    // 创建右键菜单
    void createContextMenu(QLabel* label, const QPoint& pos);
    
    // 删除选中对象
    void deleteSelectedObject(QLabel* label);
    
    // 转换鼠标坐标到图像坐标
    QPoint convertMouseToImageCoords(const QPoint& pos, QLabel* label, const cv::Mat& currentFrame);
    
    // 获取测量管理器
    MeasurementManager* getMeasurementManager(QLabel* label);
    
    // 同步绘制
    void syncDrawings(QLabel* sourceLabel, QLabel* targetLabel);
    
    // 添加测量管理器
    void addMeasurementManager(QLabel* label, MeasurementManager* manager);
    
    // 添加视图对
    void addViewPair(QLabel* sourceLabel, QLabel* targetLabel);
    
    // 添加新方法声明
    void updateDisplay(QLabel* label, const cv::Mat& newFrame);
    
private:
    // 测量管理器映射
    QMap<QLabel*, MeasurementManager*> measurementManagers;
    
    // 视图对映射
    QMap<QLabel*, QLabel*> viewPairs;
    
    // 当前活动视图
    QLabel* activeView;
    
    // 当前活动测量管理器
    MeasurementManager* activeMeasurement;
    
    // 是否处于选择模式
    bool selectionMode;
};

#endif // DRAWINGMANAGER_H 