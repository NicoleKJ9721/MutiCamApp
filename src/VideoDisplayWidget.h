#ifndef VIDEODISPLAYWIDGET_H
#define VIDEODISPLAYWIDGET_H

#include <QLabel>
#include <QVector>
#include <QPointF>
#include <QColor>
#include <QCache>
#include <chrono>
#include <QMouseEvent>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QStack>

// 前向声明
class QPaintEvent;
class QPainter;
class QPixmap;
class MutiCamApp;

// 绘制数据结构体声明
struct LineObject {
    QVector<QPointF> points;
    bool isCompleted;
    QColor color;
    int thickness;
    bool isDashed;
    
    LineObject() : isCompleted(false), color(Qt::green), thickness(2), isDashed(false) {}
};

struct LineSegmentObject {
    QVector<QPointF> points;  // 只包含两个点：起点和终点
    bool isCompleted;
    QColor color;
    int thickness;
    bool isDashed;
    
    LineSegmentObject() : isCompleted(false), color(Qt::green), thickness(2), isDashed(false) {}
};

struct CircleObject {
    QVector<QPointF> points;
    bool isCompleted;
    QColor color;
    int thickness;
    QPointF center;
    double radius;
    
    CircleObject() : isCompleted(false), color(Qt::green), thickness(3), radius(0.0) {}
};

struct FineCircleObject {
    QVector<QPointF> points;
    bool isCompleted;
    QColor color;
    int thickness;
    QPointF center;
    double radius;
    
    FineCircleObject() : isCompleted(false), color(Qt::green), thickness(3), radius(0.0) {}
};

struct ParallelObject {
    QVector<QPointF> points;
    bool isCompleted;
    QColor color;
    int thickness;
    double distance;
    double angle;
    bool isPreview;  // 标识是否为预览状态
    
    ParallelObject() : isCompleted(false), color(Qt::green), thickness(2), distance(0.0), angle(0.0), isPreview(false) {}
};

struct TwoLinesObject {
    QVector<QPointF> points;
    bool isCompleted;
    QColor color;
    int thickness;
    double angle;
    QPointF intersection;
    bool isPreview;  // 标识是否为预览状态
    
    TwoLinesObject() : isCompleted(false), color(Qt::green), thickness(2), angle(0.0), isPreview(false) {}
};

/**
 * @brief 自定义视频显示控件，支持硬件加速绘制
 * 
 * 这个控件将视频帧和用户绘制的几何图形分离为两个独立的层：
 * - 背景层：视频帧（QPixmap）
 * - 前景层：用户绘制的几何图形（QPainter硬件加速绘制）
 * 
 * 优势：
 * - 避免在cv::Mat上进行CPU密集型绘制
 * - 利用QPainter的硬件加速能力
 * - 减少每帧的绘制开销
 * - 支持高质量的抗锯齿和虚线绘制
 */
// {{ AURA-X: Optimize - 绘制上下文结构体，预创建常用对象避免重复创建. Approval: 寸止(ID:performance_optimization). }}
// 绘制上下文结构体，包含预创建的画笔、字体等对象
struct DrawingContext {
    double scale;           ///< 缩放因子
    double fontSize;        ///< 字体大小
    QFont font;            ///< 预创建的字体
    
    // 预创建的画笔
    QPen greenPen;
    QPen blackPen;
    QPen whitePen;
    QPen redPen;
    QPen bluePen;
    QPen yellowPen;
    QPen grayPen;
    QPen redDashedPen;  ///< 红色虚线画笔
    
    // 预创建的画刷
    QBrush greenBrush;
    QBrush blackBrush;
    QBrush whiteBrush;
    QBrush redBrush;
    QBrush blueBrush;
    QBrush noBrush;
};

class VideoDisplayWidget : public QLabel
{
    Q_OBJECT

signals:
    /**
     * @brief 绘制数据发生变化时发出的信号
     * @param viewName 视图名称
     */
    void drawingDataChanged(const QString& viewName);
    
    /**
     * @brief 测量完成时发出的信号
     * @param viewName 视图名称
     * @param result 测量结果字符串
     */
    void measurementCompleted(const QString& viewName, const QString& result);

public:
    // 绘图工具枚举
    enum class DrawingTool {
        None,
        Point,
        Line,
        Circle,
        FineCircle,
        Parallel,
        TwoLines
    };
    
    // 动作类型枚举（用于历史记录）
    enum class ActionType { Point, Line, LineSegment, Circle, FineCircle, Parallel, TwoLines };

    
    explicit VideoDisplayWidget(QWidget *parent = nullptr);
    
    // 设置背景视频帧
    void setVideoFrame(const QPixmap& pixmap);
    
    // 设置视图名称（用于标识不同的视图）
    void setViewName(const QString& viewName);
    QString getViewName() const { return m_viewName; }
    
    // {{ AURA-X: Delete - 绘制数据设置接口已移至private区域. Approval: 寸止(ID:encapsulation). }}
    // 绘制数据设置接口已移至private区域，通过setDrawingState统一管理
    
    // 设置当前正在绘制的直线（用于实时预览）
    void setCurrentLineData(const LineObject& currentLine);
    
    // 清除当前正在绘制的直线
    void clearCurrentLineData();
    
    // 设置当前正在绘制的圆形（用于实时预览）
    void setCurrentCircleData(const CircleObject& currentCircle);
    
    // 清除当前正在绘制的圆形
    void clearCurrentCircleData();
    
    // 设置当前正在绘制的精细圆（用于实时预览）
    void setCurrentFineCircleData(const FineCircleObject& currentFineCircle);
    
    // 清除当前正在绘制的精细圆
    void clearCurrentFineCircleData();
    
    // 设置当前正在绘制的平行线（用于实时预览）
    void setCurrentParallelData(const ParallelObject& currentParallel);
    
    // 清除当前正在绘制的平行线
    void clearCurrentParallelData();
    
    // 设置当前正在绘制的两线（用于实时预览）
    void setCurrentTwoLinesData(const TwoLinesObject& currentTwoLines);
    
    // 清除当前正在绘制的两线
    void clearCurrentTwoLinesData();
    
    // 清除所有绘制数据
    void clearAllDrawings();
    
    // 撤销上一个绘制的图形
    void undoLastDrawing();
    
    // {{ AURA-X: Add - 通用绘图提交逻辑. Approval: 寸止(ID:code_refactor). }}
    void commitDrawingAction(ActionType type, const QString& result);
    
    // 更新绘制（触发重绘）
    void updateDrawings();
    
    // 坐标转换
    QPointF imageToWidget(const QPointF& imagePos) const;
    QPointF widgetToImage(const QPointF& widgetPos) const;
    
    // 获取图像尺寸
    QSize getImageSize() const { return m_videoFrame.size(); }
    
    // 绘图模式控制
    void startDrawing(DrawingTool tool);
    void stopDrawing();
    bool isDrawing() const { return m_isDrawingMode; }
    DrawingTool getCurrentDrawingTool() const { return m_currentDrawingTool; }
    
    // 选择功能
    void enableSelection(bool enable);
    bool isSelectionEnabled() const;
    void clearSelection();
    QString getSelectedObjectInfo() const;
    
    // {{ AURA-X: Add - 右键菜单和删除功能. Approval: 寸止(ID:context_menu_delete). }}
    // 删除选中的对象
    void deleteSelectedObjects();
    
    // {{ AURA-X: Add - 绘图状态同步方法. Approval: 寸止(ID:drawing_sync). }}
    // 获取当前绘图状态
    struct DrawingState {
        QVector<QPointF> points;
        QVector<LineObject> lines;
        QVector<LineSegmentObject> lineSegments;
        QVector<CircleObject> circles;
        QVector<FineCircleObject> fineCircles;
        QVector<ParallelObject> parallels;
        QVector<TwoLinesObject> twoLines;
    };
    
    DrawingState getDrawingState() const;
    void setDrawingState(const DrawingState& state);
    
public slots:
    // 选择状态改变信号
    void onSelectionChanged();
    
signals:
    // 选择状态改变信号
    void selectionChanged(const QString& info);
    
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    // {{ AURA-X: Add - 右键菜单事件处理. Approval: 寸止(ID:context_menu_delete). }}
    void contextMenuEvent(QContextMenuEvent *event) override;
    
private:
    // {{ AURA-X: Optimize - 使用DrawingContext简化绘制函数接口. Approval: 寸止(ID:performance_optimization). }}
    // 绘制各种几何图形的私有方法
    void drawPoints(QPainter& painter, const DrawingContext& ctx);
    void drawLines(QPainter& painter, const DrawingContext& ctx);
    void drawLineSegments(QPainter& painter, const DrawingContext& ctx);
    void drawCircles(QPainter& painter, const DrawingContext& ctx);
    void drawFineCircles(QPainter& painter, const DrawingContext& ctx);
    void drawParallelLines(QPainter& painter, const DrawingContext& ctx);
    void drawTwoLines(QPainter& painter, const DrawingContext& ctx);
    
    // 绘制单个图形的私有方法
    void drawSingleLine(QPainter& painter, const LineObject& line, bool isCurrentDrawing, const DrawingContext& ctx);
    void drawSingleLineSegment(QPainter& painter, const LineSegmentObject& lineSegment, const DrawingContext& ctx);
    void drawSingleCircle(QPainter& painter, const CircleObject& circle, const DrawingContext& ctx);
    void drawSingleFineCircle(QPainter& painter, const FineCircleObject& fineCircle, const DrawingContext& ctx);
    void drawSingleTwoLines(QPainter& painter, const TwoLinesObject& twoLines, const DrawingContext& ctx);
    void drawSingleParallel(QPainter& painter, const ParallelObject& parallel, const DrawingContext& ctx);
    
    // 辅助方法
    QPen createPen(const QColor& color, int thickness, double scale, bool dashed = false);
    QFont createFont(int size, double scale);
    void drawTextWithBackground(QPainter& painter,
                               const QPointF& anchorPoint,
                               const QString& text,
                               const QFont& font,
                               const QColor& textColor,
                               const QColor& bgColor,
                               double padding,
                               double borderWidth,
                               const QPointF& offset);
    
    // 【终极API设计】计算文本背景矩形的辅助函数
    QRectF calculateTextWithBackgroundRect(const QPointF& anchorPoint, const QString& text, 
                                          const QFont& font, double padding, const QPointF& offset);
    
    // 【终极API设计】在预先计算好的矩形中绘制文本
    void drawTextInRect(QPainter& painter, const QRectF& rect, const QString& text,
                       const QFont& font, const QColor& textColor, const QColor& bgColor,
                       double borderWidth);
    
    // 计算缩放比例
    double getScaleFactor() const;
    QPointF getImageOffset() const;
    
    // {{ AURA-X: Add - DrawingContext缓存管理方法. Approval: 寸止(ID:context_cache). }}
    // DrawingContext缓存管理
    void updateDrawingContext();
    bool needsDrawingContextUpdate() const;
    
    // 计算函数
    double calculateFontSize() const;
    double calculateLineAngle(const QPointF& start, const QPointF& end) const;
    void calculateExtendedLine(const QPointF& start, const QPointF& end, QPointF& extStart, QPointF& extEnd) const;
    
    // 鼠标事件处理的私有方法
    void handlePointDrawingClick(const QPointF& imagePos);
    void handleLineDrawingClick(const QPointF& imagePos);
    void handleCircleDrawingClick(const QPointF& imagePos);
    void handleFineCircleDrawingClick(const QPointF& imagePos);
    void handleParallelDrawingClick(const QPointF& imagePos);
    void handleTwoLinesDrawingClick(const QPointF& imagePos);
    void handleSelectionClick(const QPointF& imagePos, bool ctrlPressed);
    
    // 选择功能的私有方法
    void selectObject(const QString& type, int index, bool isMiddleLine = false);
    void drawSelectionHighlight(QPainter& painter, const DrawingContext& ctx);
    
    // 命中测试方法
    bool hitTestPoint(const QPointF& testPos, int index) const;
    bool hitTestLine(const QPointF& testPos, int index) const;
    bool hitTestLineSegment(const QPointF& testPos, int index) const;
    bool hitTestCircle(const QPointF& testPos, int index) const;
    bool hitTestFineCircle(const QPointF& testPos, int index) const;
    QString hitTestParallel(const QPointF& testPos, int index) const; // 返回0=无命中, 1=第一条线, 2=第二条线, 3=中线
    bool hitTestTwoLines(const QPointF& testPos, int index) const;
    
    // 几何计算辅助方法
    bool calculateCircleFromThreePoints(const QPointF& p1, const QPointF& p2, const QPointF& p3, 
                                       QPointF& center, double& radius) const;
    bool calculateCircleFromFivePoints(const QVector<QPointF>& points, 
                                      QPointF& center, double& radius) const;
    bool calculateLineIntersection(const QPointF& p1, const QPointF& p2, const QPointF& p3, const QPointF& p4, QPointF& intersection) const;
    QPair<QPointF, QPointF> calculateExtendedLine(const QPointF& p1, const QPointF& p2, const QSize& imageSize) const;
    double calculateDistancePointToLine(const QPointF& point, const QPointF& lineStart, const QPointF& lineEnd) const;
    
    // 从选中的点创建线段
    void createLineFromSelectedPoints();
    
    // {{ AURA-X: Add - 绘制数据设置接口移至private区域. Approval: 寸止(ID:encapsulation). }}
    // 绘制数据设置接口（仅供内部使用）
    void setPointsData(const QVector<QPointF>& points);
    void setLinesData(const QVector<LineObject>& lines);
    void setLineSegmentsData(const QVector<LineSegmentObject>& lineSegments);
    void setCirclesData(const QVector<CircleObject>& circles);
    void setFineCirclesData(const QVector<FineCircleObject>& fineCircles);
    void setParallelLinesData(const QVector<ParallelObject>& parallels);
    void setTwoLinesData(const QVector<TwoLinesObject>& twoLines);
    
private:
    QString m_viewName;                              ///< 视图名称
    QPixmap m_videoFrame;                           ///< 背景视频帧
    
    // 绘图状态
    bool m_isDrawingMode;                           ///< 是否处于绘制模式
    DrawingTool m_currentDrawingTool;               ///< 当前绘制工具
    
    // 绘制数据
    QVector<QPointF> m_points;                      ///< 点数据
    QVector<LineObject> m_lines;                    ///< 直线数据
    QVector<LineSegmentObject> m_lineSegments;      ///< 线段数据
    QVector<CircleObject> m_circles;                ///< 圆形数据
    QVector<FineCircleObject> m_fineCircles;        ///< 精细圆数据
    QVector<ParallelObject> m_parallels;            ///< 平行线数据
    QVector<TwoLinesObject> m_twoLines;             ///< 双线数据
    
    // 当前正在绘制的数据
    LineObject m_currentLine;                       ///< 当前正在绘制的直线
    bool m_hasCurrentLine;                          ///< 是否有当前正在绘制的直线
    CircleObject m_currentCircle;                   ///< 当前正在绘制的圆形
    bool m_hasCurrentCircle;                        ///< 是否有当前正在绘制的圆形
    FineCircleObject m_currentFineCircle;           ///< 当前正在绘制的精细圆
    bool m_hasCurrentFineCircle;                    ///< 是否有当前正在绘制的精细圆
    ParallelObject m_currentParallel;               ///< 当前正在绘制的平行线
    bool m_hasCurrentParallel;                      ///< 是否有当前正在绘制的平行线
    TwoLinesObject m_currentTwoLines;               ///< 当前正在绘制的两线
    bool m_hasCurrentTwoLines;                      ///< 是否有当前正在绘制的两线
    
    // 鼠标预览位置
    QPointF m_currentMousePos;                      ///< 当前鼠标位置（用于预览）
    bool m_hasValidMousePos;                        ///< 是否有有效的鼠标位置
    
    // 缓存的绘制属性
    mutable double m_cachedScaleFactor;             ///< 缓存的缩放因子
    mutable QPointF m_cachedImageOffset;            ///< 缓存的图像偏移
    mutable QSize m_cachedWidgetSize;               ///< 缓存的控件尺寸
    mutable QSize m_cachedImageSize;                ///< 缓存的图像尺寸
    
    // 选择功能相关成员变量
    bool m_selectionEnabled;                        ///< 是否启用选择功能
    QSet<int> m_selectedPoints;                     ///< 选中的点索引
    QSet<int> m_selectedLines;                      ///< 选中的线索引
    QSet<int> m_selectedLineSegments;               ///< 选中的线段索引
    QSet<int> m_selectedCircles;                    ///< 选中的圆索引
    QSet<int> m_selectedFineCircles;                ///< 选中的精细圆索引
    QSet<int> m_selectedParallels;                  ///< 选中的平行线索引
    QSet<int> m_selectedTwoLines;                   ///< 选中的两线夹角索引
    QSet<int> m_selectedParallelMiddleLines;        ///< 选中的平行线中线索引
    
    // {{ AURA-X: Add - 缓存的DrawingContext避免重复创建. Approval: 寸止(ID:context_cache). }}
    // 缓存的绘制上下文
    mutable DrawingContext m_cachedDrawingContext;  ///< 缓存的绘制上下文
    mutable bool m_drawingContextValid;             ///< 绘制上下文是否有效
    mutable QSize m_lastContextWidgetSize;          ///< 上次更新上下文时的控件尺寸
    mutable QSize m_lastContextImageSize;           ///< 上次更新上下文时的图像尺寸
    
    // {{ AURA-X: Add - 历史记录栈支持撤销功能. Approval: 寸止(ID:undo_feature). }}
    // 历史记录栈，用于实现撤销功能
    struct DrawingAction {
        ActionType type;
        int index; // 记录被添加对象在对应列表中的索引
    };
    QStack<DrawingAction> m_drawingHistory;         ///< 绘制历史记录栈
};

#endif // VIDEODISPLAYWIDGET_H