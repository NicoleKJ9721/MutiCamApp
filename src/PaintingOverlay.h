#ifndef PAINTINGOVERLAY_H
#define PAINTINGOVERLAY_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QVector>
#include <QStack>
#include <QSet>
#include <QPointF>
#include <QRectF>
#include <QFont>
#include <QPen>
#include <QBrush>
#include <QMenu>
#include <QAction>
#include <QTime>
#include <opencv2/opencv.hpp>
#include "image_processing/edge_detector.h"

// 前向声明
class ZoomPanWidget;
#include "image_processing/shape_detector.h"

// 前向声明
class MutiCamApp;

class PaintingOverlay : public QWidget
{
    Q_OBJECT

public:
    // 绘图工具枚举
    enum class DrawingTool {
        None,
        Point,
        Line,
        LineSegment,
        Circle,
        FineCircle,
        Parallel,
        TwoLines,
        ROI_LineDetect,    // ROI直线检测
        ROI_CircleDetect   // ROI圆形检测
    };

    // 绘图对象结构体
    struct PointObject {
        QPointF position;
        QString label;
        bool isVisible = true;
    };

    struct LineObject {
        QVector<QPointF> points;
        bool isCompleted = false;
        QColor color = Qt::green;
        int thickness = 2;
        bool isDashed = false;
        QPointF start;
        QPointF end;
        QString label;
        bool isVisible = true;
        bool showLength = false;
        double length = 0.0;
    };

    struct CircleObject {
        QVector<QPointF> points; // 用于构造圆的点
        bool isCompleted = false;
        QColor color = Qt::green;
        int thickness = 2;
        bool isDashed = false;
        QPointF center;
        double radius;
        QString label;
        bool isVisible = true;
    };

    struct LineSegmentObject {
        QVector<QPointF> points;
        bool isCompleted = false;
        QColor color = Qt::green;
        double thickness = 2.0;
        bool isDashed = false;
        QString label;
        bool isVisible = true;
        bool showLength = false;
        double length = 0.0;
    };

    struct FineCircleObject {
        QVector<QPointF> points;     // 存储5个拟合点
        bool isCompleted = false;    // 标记是否完成绘制
        QColor color = Qt::green;    // 圆形颜色（绿色）
        int thickness = 2;           // 线条粗细
        QPointF center;              // 拟合得到的圆心
        double radius = 0.0;         // 拟合得到的半径
        QString label;
        bool isVisible = true;
    };

    // 绘图上下文结构体（用于性能优化）
    struct DrawingContext {
        double scale = 1.0;
        double fontSize = 12.0;
        QFont font;
        
        // 预创建的画笔
        QPen greenPen;
        QPen blackPen;
        QPen whitePen;
        QPen redPen;
        QPen bluePen;
        QPen yellowPen;
        QPen cyanPen;
        QPen magentaPen;
        QPen grayPen;
        QPen redDashedPen;
        QPen greenDashedPen;
        QPen blueDashedPen;
        QPen blackDashedPen;
        QPen cyanDashedPen;
        QPen magentaDashedPen;
        QPen yellowDashedPen;

        // 预创建的画刷
        QBrush greenBrush;
        QBrush blackBrush;
        QBrush whiteBrush;
        QBrush redBrush;
        QBrush blueBrush;
        QBrush cyanBrush;
        QBrush magentaBrush;
        QBrush yellowBrush;
    };

    struct ParallelObject {
        QVector<QPointF> points;
        bool isCompleted = false;
        QColor color = Qt::green;
        int thickness = 2;
        bool isDashed = false;
        QVector<LineObject> lines;
        QPointF midStart;
        QPointF midEnd;
        QString label;
        bool isVisible = true;
        double distance = 0.0;
        double angle = 0.0;
        bool isPreview = false;
    };

    struct TwoLinesObject {
        QVector<QPointF> points;
        bool isCompleted = false;
        QColor color = Qt::green;
        int thickness = 2;
        bool isDashed = false;
        QVector<LineObject> lines;
        QPointF intersection;
        double angle = 0.0;
        QString label;
        bool isVisible = true;
        bool hasIntersection = false;
    };

    // ROI检测对象结构体
    struct ROIObject {
        QVector<QPointF> points;        // ROI矩形的两个对角点
        bool isCompleted = false;       // 是否完成绘制
        QColor color = Qt::yellow;      // ROI矩形颜色
        int thickness = 2;              // 线条粗细
        bool isDashed = true;           // 虚线显示
        QString label;                  // 标签
        bool isVisible = true;          // 是否可见
        DrawingTool detectionType;      // 检测类型（直线或圆形）
        bool isDetecting = false;       // 是否正在检测中

        // 获取ROI矩形
        QRectF getRect() const {
            if (points.size() >= 2) {
                QPointF topLeft = points[0];
                QPointF bottomRight = points[1];

                // 确保topLeft在左上角，bottomRight在右下角
                double left = qMin(topLeft.x(), bottomRight.x());
                double top = qMin(topLeft.y(), bottomRight.y());
                double width = qAbs(bottomRight.x() - topLeft.x());
                double height = qAbs(bottomRight.y() - topLeft.y());

                return QRectF(left, top, width, height);
            }
            return QRectF();
        }
    };

    // 绘图动作结构体（用于历史记录）
    struct DrawingAction {
        enum Type {
            AddPoint,
            AddLine,
            AddLineSegment,
            AddCircle,
            AddFineCircle,
            AddParallel,
            AddTwoLines,
            AddROI,
            DeletePoint,
            DeleteLine,
            DeleteLineSegment,
            DeleteCircle,
            DeleteFineCircle,
            DeleteParallel,
            DeleteTwoLines,
            DeleteROI
        };

        enum Source {
            ManualDrawing,    // 手动绘制
            AutoDetection     // 自动检测
        };

        Type type;
        Source source;        // 操作来源：手动绘制或自动检测
        int index;           // 在对应容器中的索引
        QVariant data;       // 存储对象数据
    };

    // 绘图状态结构体
    struct DrawingState {
        QVector<PointObject> points;
        QVector<LineObject> lines;
        QVector<LineSegmentObject> lineSegments;
        QVector<CircleObject> circles;
        QVector<FineCircleObject> fineCircles;
        QVector<ParallelObject> parallels;
        QVector<TwoLinesObject> twoLines;
        QVector<ROIObject> rois;
        QStack<DrawingAction> history;
    };

explicit PaintingOverlay(QWidget *parent = nullptr);
    ~PaintingOverlay();

    // 公共接口，由 MutiCamApp 调用
    void startDrawing(DrawingTool tool);
    void stopDrawing();
    void clearAllDrawings();          // 清空手动绘制的图形
    void clearManualDrawings();       // 清空手动绘制的图形（内部实现）
    void undoLastDrawing();           // 撤销最后一次手动绘制
    void undoLastDetection();         // 撤销最后一次自动检测
    void deleteSelectedObjects();
    void createLineFromSelectedPoints();
    void setTransforms(const QPointF& offset, double scale, const QSize& imageSize); // 用于同步坐标系
    
    // 设置视图名称（用于measurementCompleted信号）
    void setViewName(const QString& viewName);
    QString getViewName() const;

    DrawingState getDrawingState() const;
    void setDrawingState(const DrawingState& state);

    // 渲染所有绘制内容到指定的QPainter（用于保存可视化图像）
    void renderToImage(QPainter& painter, const QSize& imageSize);

    // 选择功能
    void enableSelection(bool enabled);
    bool isSelectionEnabled() const;
    void clearSelection();
    QString getSelectedObjectInfo() const;

    // 网格功能
    void setGridSpacing(int spacing);
    void setGridColor(const QColor& color);
    void setGridStyle(Qt::PenStyle style);
    void setGridWidth(int width);
    int getGridSpacing() const;
    QColor getGridColor() const;
    Qt::PenStyle getGridStyle() const;
    int getGridWidth() const;

    // 参数设置接口
    void setEdgeDetectionParams(const EdgeDetector::EdgeDetectionParams& params);
    void setLineDetectionParams(const ShapeDetector::LineDetectionParams& params);
    void setCircleDetectionParams(const ShapeDetector::CircleDetectionParams& params);

signals:
    void drawingCompleted(const QString& viewName); // 绘图完成信号
    void selectionChanged(const QString& info);   // 选择变化信号
    void measurementCompleted(const QString& viewName, const QString& result); // 测量完成信号
    void drawingDataChanged(const QString& viewName); // 绘图数据变化信号
    void overlayActivated(PaintingOverlay* overlay); // overlay被激活信号
    void viewDoubleClicked(const QString& viewName); // 视图双击信号

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    // 自动检测相关方法
    void performAutoDetection(const ROIObject& roi);
    cv::Mat getCurrentFrameFromParent() const;
    void performLineDetection(const cv::Mat& frame, const cv::Rect& roi);
    void performCircleDetection(const cv::Mat& frame, const cv::Rect& roi);

    // 绘图模式相关
    bool m_isDrawingMode;
    DrawingTool m_currentDrawingTool;
    
    // 绘图数据
    QVector<PointObject> m_points;
    QVector<LineObject> m_lines;
    QVector<LineSegmentObject> m_lineSegments;
    QVector<CircleObject> m_circles;
    QVector<FineCircleObject> m_fineCircles;
    QVector<ParallelObject> m_parallels;
    QVector<TwoLinesObject> m_twoLines;
    QVector<ROIObject> m_rois;
    
    // 当前正在绘制的数据
    LineObject m_currentLine;
    bool m_hasCurrentLine;
    LineSegmentObject m_currentLineSegment;
    bool m_hasCurrentLineSegment;
    CircleObject m_currentCircle;
    bool m_hasCurrentCircle;
    FineCircleObject m_currentFineCircle;
    bool m_hasCurrentFineCircle;
    ParallelObject m_currentParallel;
    bool m_hasCurrentParallel;
    TwoLinesObject m_currentTwoLines;
    bool m_hasCurrentTwoLines;
    ROIObject m_currentROI;
    bool m_hasCurrentROI;
    QVector<QPointF> m_currentPoints; // 当前绘制过程中的临时点

    // 鼠标预览位置
    QPointF m_currentMousePos;
    bool m_hasValidMousePos;

    // 历史记录
    QStack<DrawingAction> m_drawingHistory;

    // 选择状态
    bool m_selectionEnabled;
    QSet<int> m_selectedPoints;
    QSet<int> m_selectedLines;
    QSet<int> m_selectedLineSegments;
    QSet<int> m_selectedCircles;
    QSet<int> m_selectedFineCircles;
    QSet<int> m_selectedParallels;
    QSet<int> m_selectedTwoLines;
    QSet<int> m_selectedParallelMiddleLines;
    QSet<int> m_selectedBisectorLines; // 选中的角平分线
    QSet<int> m_selectedROIs;
    
    // 绘图上下文缓存
    mutable DrawingContext m_cachedDrawingContext;
    mutable bool m_drawingContextValid;
    mutable QSize m_lastContextWidgetSize;
    mutable QSize m_lastContextImageSize;
    
    // 坐标变换参数
    QPointF m_imageOffset;
    double m_scaleFactor;
    QSize m_imageSize;
    
    // 视图名称
    QString m_viewName;

    // 图像处理相关
    EdgeDetector* m_edgeDetector;
    ShapeDetector* m_shapeDetector;

    // 性能优化相关
    cv::Mat m_lastProcessedFrame;       // 上次处理的帧
    QString m_lastFrameHash;            // 上次帧的哈希值
    QTime m_lastDetectionTime;          // 上次检测时间

    // 网格相关成员变量
    int m_gridSpacing;          // 网格间距，0表示不显示网格
    QColor m_gridColor;         // 网格颜色
    Qt::PenStyle m_gridStyle;   // 网格线样式
    int m_gridWidth;            // 网格线宽度

    // 网格缓存相关
    mutable bool m_gridCacheValid;      // 网格缓存是否有效
    mutable QSize m_lastGridImageSize;  // 上次绘制网格时的图像尺寸
    mutable int m_lastGridSpacing;      // 上次绘制网格时的间距

    // 私有绘图方法
    void drawGrid(QPainter& painter, const DrawingContext& ctx) const;
    void drawPoints(QPainter& painter, const DrawingContext& ctx) const;
    void drawLines(QPainter& painter, const DrawingContext& ctx) const;
    void drawLineSegments(QPainter& painter, const DrawingContext& ctx) const;
    void drawCircles(QPainter& painter, const DrawingContext& ctx) const;
    void drawFineCircles(QPainter& painter, const DrawingContext& ctx) const;
    void drawParallels(QPainter& painter, const DrawingContext& ctx) const;
    void drawTwoLines(QPainter& painter, const DrawingContext& ctx) const;
    void drawROIs(QPainter& painter, const DrawingContext& ctx) const;
    
    // 单个对象绘制方法
    void drawSinglePoint(QPainter& painter, const QPointF& point, int index, const DrawingContext& ctx) const;
    void drawSingleLine(QPainter& painter, const LineObject& line, bool isPreview, const DrawingContext& ctx) const;
    void drawSingleLineSegment(QPainter& painter, const LineSegmentObject& lineSegment, const DrawingContext& ctx) const;
    void drawSingleCircle(QPainter& painter, const CircleObject& circle, const DrawingContext& ctx) const;
    void drawSingleFineCircle(QPainter& painter, const FineCircleObject& fineCircle, const DrawingContext& ctx) const;
    void drawSingleParallel(QPainter& painter, const ParallelObject& parallel, const DrawingContext& ctx) const;
    void drawSingleTwoLines(QPainter& painter, const TwoLinesObject& twoLines, const DrawingContext& ctx) const;
    void drawSingleROI(QPainter& painter, const ROIObject& roi, const DrawingContext& ctx) const;
    
    // 预览绘制方法
    void drawCurrentPreview(QPainter& painter, const DrawingContext& ctx) const;
    void drawSelectionHighlights(QPainter& painter) const;
    
    // 鼠标事件处理方法
    void handlePointDrawingClick(const QPointF& pos);
    void handleLineDrawingClick(const QPointF& pos);
    void handleLineSegmentDrawingClick(const QPointF& pos);
    void handleCircleDrawingClick(const QPointF& pos);
    void handleFineCircleDrawingClick(const QPointF& pos);
    void handleParallelDrawingClick(const QPointF& pos);
    void handleTwoLinesDrawingClick(const QPointF& pos);
    void handleROIDrawingClick(const QPointF& pos);
    void handleSelectionClick(const QPointF& pos, bool ctrlPressed);

    // 命中测试方法
    int hitTestPoint(const QPointF& pos, double tolerance = 5.0) const;
    int hitTestLine(const QPointF& pos, double tolerance = 5.0) const;
    int hitTestLineSegment(const QPointF& pos, double tolerance = 5.0) const;
    int hitTestCircle(const QPointF& pos, double tolerance = 5.0) const;
    int hitTestFineCircle(const QPointF& pos, double tolerance = 5.0) const;
    int hitTestParallel(const QPointF& pos, double tolerance = 5.0) const;
    int hitTestTwoLines(const QPointF& pos, double tolerance = 5.0) const;
    int hitTestROI(const QPointF& pos, double tolerance = 5.0) const;
    
    // 几何计算方法
    bool calculateCircleFromThreePoints(const QVector<QPointF>& points, QPointF& center, double& radius) const;
    bool calculateCircleFromFivePoints(const QVector<QPointF>& points, QPointF& center, double& radius) const;
    bool calculateLineIntersection(const QPointF& p1, const QPointF& p2, const QPointF& p3, const QPointF& p4, QPointF& intersection) const;
    double calculateDistancePointToLine(const QPointF& point, const QPointF& lineStart, const QPointF& lineEnd) const;
    double calculateLineAngle(const QPointF& start, const QPointF& end) const;
    void calculateExtendedLine(const QPointF& start, const QPointF& end, QPointF& extStart, QPointF& extEnd) const;
    
    // 坐标转换方法
    QPointF widgetToImage(const QPointF& widgetPos) const;
    QPointF imageToWidget(const QPointF& imagePos) const;

    // 边界检查方法
    bool isPointInImageBounds(const QPointF& imagePos) const;
    
    // 绘图辅助方法
    QPen createPen(const QColor& color, int width, double scale, bool dashed = false) const;
    QFont createFont(int baseSize, double scale) const;
    double calculateFontSize() const;
    void drawTextWithBackground(QPainter& painter, const QPointF& pos, const QString& text, const QColor& textColor, const QColor& bgColor) const;
    void drawTextWithBackground(QPainter& painter, const QPointF& anchorPoint, const QString& text, const QFont& font, const QColor& textColor, const QColor& bgColor, double padding, double borderWidth, const QPointF& offset) const;
    QRectF calculateTextWithBackgroundRect(const QPointF& anchorPoint, const QString& text, const QFont& font, double padding, const QPointF& offset) const;
    void drawTextInRect(QPainter& painter, const QRectF& rect, const QString& text, const QFont& font, const QColor& textColor, const QColor& bgColor, double borderWidth) const;
    // 已删除QPointF版本的drawTextInRect重载，统一使用calculateTextWithBackgroundRect + drawTextInRect(QRectF)组合
    
    // 绘图上下文管理
    void updateDrawingContext() const;
    bool needsDrawingContextUpdate() const;

    // 复合测量功能
    void performComplexMeasurement(const QString& measurementType);
    double calculatePointToLineDistance(const QPointF& point, const QPointF& lineStart, const QPointF& lineEnd) const;
    double calculatePointToCircleDistance(const QPointF& point, const QPointF& circleCenter, double radius, bool toCircumference = true) const;
    QString analyzeLineCircleRelation(const QPointF& lineStart, const QPointF& lineEnd, const QPointF& circleCenter, double radius) const;
    double calculateLineSegmentAngle(const QPointF& line1Start, const QPointF& line1End, const QPointF& line2Start, const QPointF& line2End) const;
    QPointF calculatePerpendicularFoot(const QPointF& point, const QPointF& lineStart, const QPointF& lineEnd) const;
    bool isPointOnLineSegment(const QPointF& point, const QPointF& lineStart, const QPointF& lineEnd, double tolerance) const;
    
    // 绘图动作管理
    void commitDrawingAction(const DrawingAction& action);
    void undoAction(const DrawingAction& action);  // 执行具体的撤销操作
    void onSelectionChanged();

    // 辅助方法：按索引删除容器中的元素
    template<typename T>
    void removeItemsByIndices(QVector<T>& container, const QSet<int>& indicesToRemove);
    
    // 数据设置方法（用于与MutiCamApp同步）
    void setPointsData(const QVector<QPointF>& points);
    void setLinesData(const QVector<LineObject>& lines);
    void setLineSegmentsData(const QVector<LineSegmentObject>& lineSegments);
    void setCirclesData(const QVector<CircleObject>& circles);
    void setFineCirclesData(const QVector<FineCircleObject>& fineCircles);
    void setParallelLinesData(const QVector<ParallelObject>& parallels);
    void setTwoLinesData(const QVector<TwoLinesObject>& twoLines);
    void setROIsData(const QVector<ROIObject>& rois);

    // 当前绘制数据管理
    void setCurrentLineData(const LineObject& currentLine);
    void clearCurrentLineData();
    void setCurrentLineSegmentData(const LineSegmentObject& currentLineSegment);
    void clearCurrentLineSegmentData();
    void setCurrentCircleData(const CircleObject& currentCircle);
    void clearCurrentCircleData();
    void setCurrentFineCircleData(const FineCircleObject& currentFineCircle);
    void clearCurrentFineCircleData();
    void setCurrentParallelData(const ParallelObject& currentParallel);
    void clearCurrentParallelData();
    void setCurrentTwoLinesData(const TwoLinesObject& currentTwoLines);
    void clearCurrentTwoLinesData();
    void setCurrentROIData(const ROIObject& currentROI);
    void clearCurrentROIData();

};

#endif // PAINTINGOVERLAY_H