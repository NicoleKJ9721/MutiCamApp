#ifndef PAINTINGOVERLAY_H
#define PAINTINGOVERLAY_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QVector>
#include <QStack>
#include <QSet>
#include <QPointF>
#include <QRectF>
#include <QFont>
#include <QPen>
#include <QBrush>
#include <QPainterPath>
#include <QSvgRenderer>
#include <QMenu>
#include <QAction>
#include <QTime>
#include <QCursor>
#include <QDialog>
#include <QListWidget>
#include <QCheckBox>
#include <QDateTime>
#include <QHash>
#include <memory>
#include <QPolygonF>

// 解决Windows SDK和OpenCV的符号冲突
#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#endif

#include <opencv2/opencv.hpp>
#include "../image_processing/edge_detector.h"
#include "../config/TemplateMatchingConfig.h"

// 前向声明
class ZoomPanWidget;
#include "../image_processing/shape_detector.h"

// 前向声明
class MutiCamApp;

// 前向声明 Halcon 类型，避免在头文件引入重型依赖
namespace HalconCpp { class HTuple; }

// 模板匹配相关数据结构
struct TemplateInfo {
    QString name;                    // 模板名称
    QString imagePath;               // 图像文件路径
    QString metadataPath;            // 元数据文件路径
    cv::Mat templateImage;           // 模板图像
    QRectF originalROI;              // 原始ROI区域
    double originalAngle;            // 原始角度
    QDateTime createdTime;           // 创建时间
    bool isSelected;                 // 是否被选中用于匹配
    QString halconModelPath;         // Halcon形状模板文件路径（.shm）
    QVector<QPolygonF> modelContours; // Halcon形状模型的轮廓（模板坐标系下）

    TemplateInfo() : originalAngle(0.0), isSelected(false) {}
};

struct TemplateMatchResult {
    QString templateName;            // 匹配的模板名称
    QPointF position;                // 匹配位置（中心点）
    QRectF boundingRect;             // 匹配边界框
    double confidence;               // 置信度 (0.0-1.0)
    double angle;                    // 匹配角度
    double scale;                    // 匹配缩放比例
    QDateTime timestamp;             // 匹配时间戳

    TemplateMatchResult() : confidence(0.0), angle(0.0), scale(1.0) {}
};

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
        ROI_CircleDetect,  // ROI圆形检测
        ROI_CREATION       // ROI创建模式
    };

    // 绘图对象结构体
    enum DerivedPointSource {
        DerivedNone = 0,
        DerivedCircle = 1,
        DerivedFineCircle = 2
    };

    struct PointObject {
        QPointF position;
        QString label;
        bool isVisible = true;
        bool isDerived = false;
        int derivedSource = DerivedNone;
        int derivedIndex = -1;
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

    // ROI对象结构体
    struct ROIObject {
        QRectF rect;                 // ROI矩形区域
        qreal angle = 0.0;           // 旋转角度（度）
        QColor borderColor = Qt::red; // 边框颜色
        int thickness = 2;           // 边框粗细
        bool isDashed = true;        // 虚线边框
        bool isActive = false;       // 是否处于编辑状态
        bool showHandles = true;     // 是否显示控制点
        QString templateName;        // 模板名称
        bool isVisible = true;

        // 控制点枚举
        enum HandleType {
            NoHandle = -1,
            TopLeft, TopRight, BottomLeft, BottomRight,        // 角点
            TopCenter, BottomCenter, LeftCenter, RightCenter,  // 边中点
            RotationHandle,                                    // 旋转手柄
            MoveHandle                                         // 移动手柄(中心)
        };
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

    struct LineSegmentAngleObject {
        QVector<QPointF> points;        // 两条线段的4个点：[line1_start, line1_end, line2_start, line2_end]
        bool isCompleted = false;
        QColor color = Qt::red;         // 红色显示，与Python版本一致
        int thickness = 2;
        QPointF intersection;           // 交点位置
        double angle = 0.0;             // 角度值（度）
        QString label;                  // 角度标签
        bool isVisible = true;
        bool hasIntersection = false;   // 是否有有效交点
    };

    // ROI检测对象结构体
    struct ROIDetectionObject {
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
            AddLineSegmentAngle,
            AddROI,
            DeletePoint,
            DeleteLine,
            DeleteLineSegment,
            DeleteCircle,
            DeleteFineCircle,
            DeleteParallel,
            DeleteTwoLines,
            DeleteLineSegmentAngle,
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

    // 像素标定相关接口
    void setPixelScale(double scale, const QString& unit = "μm");
    double getPixelScale() const;
    QString getUnit() const;
    bool isCalibrated() const;
    void startCalibration();           // 启动单点标定
    void startMultiPointCalibration(); // 启动多点标定
    void startCircleCalibration();     // 启动圆标定（基于自动检测圆）
    void resetCalibration();

    // 变倍比作弊功能
    void setZoomCheatEnabled(bool enabled);

    // 单次点选（用于载物台辅助标定等场景）
    void startPointPick(const QString& purpose = QString());
    void cancelPointPick();
    bool isPointPicking() const;

    DrawingState getDrawingState() const;
    void setDrawingState(const DrawingState& state);

    // 渲染所有绘制内容到指定的QPainter（用于保存可视化图像）
    void renderToImage(QPainter& painter, const QSize& imageSize);

    // 选择功能
    void enableSelection(bool enabled);
    bool isSelectionEnabled() const;
    void clearSelection();
    QString getSelectedObjectInfo() const;

    // ROI功能
    void startROICreation();              // 开始ROI创建
    void finishROICreation();             // 完成ROI创建
    void cancelROICreation();             // 取消ROI创建
    bool hasActiveROI() const;            // 是否有活动的ROI
    QRectF getCurrentROI() const;         // 获取当前ROI矩形
    qreal getCurrentROIAngle() const;     // 获取当前ROI角度
    QString getCurrentROITemplateName() const; // 获取当前ROI模板名称
    void setCurrentROITemplateName(const QString& name); // 设置当前ROI模板名称

    // 网格功能
    void setGridSpacing(int spacing);
    void setGridColor(const QColor& color);
    void setGridStyle(Qt::PenStyle style);
    void setGridWidth(int width);
    int getGridSpacing() const;
    QColor getGridColor() const;
    Qt::PenStyle getGridStyle() const;
    int getGridWidth() const;
    void setLineCircleThickness(int thickness);
    int getLineCircleThickness() const;
    void setOverlayTextSize(int size);
    int getOverlayTextSize() const;

    // 参数设置接口
    void setEdgeDetectionParams(const EdgeDetector::EdgeDetectionParams& params);
    void setLineDetectionParams(const ShapeDetector::LineDetectionParams& params);
    void setCircleDetectionParams(const ShapeDetector::CircleDetectionParams& params);

    // 模板创建相关公共方法
    /**
     * @brief 从当前图像中提取ROI区域
     * @param sourceImage 源图像
     * @return 提取的ROI图像，如果失败返回空Mat
     */
    cv::Mat extractROIImage(const cv::Mat& sourceImage) const;

    /**
     * @brief 验证和预处理ROI图像
     * @param roiImage 提取的ROI图像
     * @return 预处理后的图像，如果验证失败返回空Mat
     */
    cv::Mat validateAndPreprocessROI(const cv::Mat& roiImage) const;

    /**
     * @brief 保存模板数据
     * @param templateImage 模板图像
     * @param templateName 模板名称
     * @param templateDir 模板保存目录
     * @return 是否保存成功
     */
    bool saveTemplateData(const cv::Mat& templateImage, const QString& templateName, const QString& templateDir = QString()) const;

    /**
     * @brief 从当前ROI创建模板（整合函数）
     * @param sourceImage 源图像
     * @param templateName 模板名称
     * @param templateDir 模板保存目录
     * @return 是否创建成功
     */
    bool createTemplateFromROI(const cv::Mat& sourceImage, const QString& templateName, const QString& templateDir = QString()) const;

    // 模板加载和管理相关方法
    /**
     * @brief 从指定目录加载所有模板
     * @param templateDir 模板目录路径，为空则使用默认目录
     * @return 加载的模板列表
     */
    QVector<TemplateInfo> loadTemplatesFromDirectory(const QString& templateDir = QString()) const;

    /**
     * @brief 加载单个模板文件
     * @param imagePath 模板图像文件路径
     * @param metadataPath 元数据文件路径
     * @return 模板信息，加载失败返回空的TemplateInfo
     */
    TemplateInfo loadSingleTemplate(const QString& imagePath, const QString& metadataPath) const;

    /**
     * @brief 启动模板匹配
     * @param selectedTemplates 选中的模板列表
     * @param matchingParams 匹配参数（可选，为空则使用配置文件参数）
     * @return 是否启动成功
     */
    bool startTemplateMatching(const QVector<TemplateInfo>& selectedTemplates,
                              const TemplateMatchingConfig::TemplateMatchingParams* matchingParams = nullptr);

    /**
     * @brief 停止模板匹配
     */
    void stopTemplateMatching();

    /**
     * @brief 在指定图像中执行模板匹配
     * @param sourceImage 源图像
     * @return 匹配结果列表
     */
    QVector<TemplateMatchResult> performMatching(const cv::Mat& sourceImage);

    // 匹配结果可视化相关方法
    /**
     * @brief 绘制模板匹配结果
     * @param painter QPainter对象
     * @param ctx 绘制上下文
     */
    void drawMatchResults(QPainter& painter, const DrawingContext& ctx) const;

    /**
     * @brief 绘制单个匹配结果
     * @param painter QPainter对象
     * @param match 匹配结果
     * @param ctx 绘制上下文
     */
    void drawSingleMatchResult(QPainter& painter, const TemplateMatchResult& match, const DrawingContext& ctx) const;

    /**
     * @brief 更新当前匹配结果并触发重绘
     * @param matches 新的匹配结果列表
     */
    void updateMatchResults(const QVector<TemplateMatchResult>& matches);

    /**
     * @brief 处理新的相机帧并执行模板匹配
     * @param frame 新的相机帧
     */
    void processFrameForMatching(const cv::Mat& frame);

    /**
     * @brief 清理Halcon模型缓存的公共方法
     * @param logContext 日志上下文，用于标识调用位置
     */
    void clearHalconModelCache(const QString& logContext = QString());

signals:
    void drawingCompleted(const QString& viewName); // 绘图完成信号
    void selectionChanged(const QString& info);   // 选择变化信号
    void measurementCompleted(const QString& viewName, const QString& result); // 测量完成信号
    void drawingDataChanged(const QString& viewName); // 绘图数据变化信号
    void overlayActivated(PaintingOverlay* overlay); // overlay被激活信号
    void viewDoubleClicked(const QString& viewName); // 视图双击信号
    void pointPicked(const QString& viewName, const QPointF& imagePos); // 单次点选完成信号（图像坐标）

    // ROI相关信号
    void roiCreated(const QString& viewName, const QRectF& rect, qreal angle); // ROI创建完成
    void roiChanged(const QString& viewName, const QRectF& rect, qreal angle); // ROI变化
    void roiFinished(const QString& viewName); // ROI编辑完成
    void roiCancelled(const QString& viewName); // ROI创建取消

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    // 自动检测相关方法
    void performAutoDetection(const ROIDetectionObject& roi);
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
    QVector<LineSegmentAngleObject> m_lineSegmentAngles;
    QVector<ROIDetectionObject> m_rois;          // ROI检测列表
    QVector<ROIObject> m_roiCreations;           // ROI创建列表

    // SVG图标渲染器
    mutable QSvgRenderer* m_rotationIconRenderer;

    // ROI信息显示缓存（性能优化）
    mutable QString m_cachedROIInfoText;
    mutable QRectF m_cachedROIRect;
    mutable double m_cachedROIAngle;
    
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
    ROIObject m_currentROI;              // 用于ROI创建功能
    bool m_hasCurrentROI;
    ROIDetectionObject m_currentROIDetection;  // 用于ROI检测功能
    bool m_hasCurrentROIDetection;
    QVector<QPointF> m_currentPoints; // 当前绘制过程中的临时点

    // ROI创建模式相关
    bool m_roiCreationMode;           // 是否处于ROI创建模式
    ROIObject::HandleType m_activeHandle; // 当前活动的控制点
    ROIObject::HandleType m_hoverHandle;  // 当前悬停的控制点（用于性能优化）
    QPointF m_lastMousePos;           // 上次鼠标位置
    bool m_isDragging;                // 是否正在拖拽
    bool m_isHoveringConfirmButton;   // 是否悬浮在确认按钮上
    bool m_isHoveringCancelButton;    // 是否悬浮在取消按钮上
    bool m_isHoveringRotationHandle;  // 是否悬浮在旋转按钮上
    bool m_panDragPending;            // 是否等待触发平移拖拽
    QPoint m_panDragStartPos;         // 平移拖拽起点（控件坐标）

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
    QSet<int> m_selectedLineSegmentAngles;
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

    // 像素标定相关
    double m_pixelScale;        // 像素比例 (μm/pixel)
    QString m_unit;            // 单位
    bool m_isCalibrated;       // 是否已标定
    bool m_isCalibrationMode;  // 是否处于标定模式
    bool m_isMultiPointCalibrationMode; // 是否处于多点标定模式
    bool m_isCircleCalibrationMode;     // 是否处于圆标定模式

    // 变倍比作弊标志
    bool m_enableZoomCheat;

    // 获取作弊倍率系数
    double getCheatRatio(double pixelLength) const;

    // 获取测量比例因子（包含作弊逻辑）
    double getMeasurementScaleFactor(double pixelLength) const;

    // 单次点选模式（不影响绘制/选择模式，只拦截一次左键点击）
    bool m_isPointPickMode = false;
    QString m_pointPickPurpose;
    QCursor m_pointPickPrevCursor;
    bool m_hasPointPickPrevCursor = false;

    // 多点标定数据结构
    struct CalibrationPoint {
        int lineSegmentIndex;   // 对应的线段索引
        double pixelLength;     // 像素长度
        double realLength;      // 实际长度
        bool isValid;           // 是否有效

        CalibrationPoint() : lineSegmentIndex(-1), pixelLength(0.0), realLength(0.0), isValid(false) {}
    };
    QVector<CalibrationPoint> m_calibrationPoints; // 标定点集合
    QString m_multiPointCalibrationUnit; // 多点标定的单位

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
    int m_lineCircleThickness; // 直线/圆线宽（屏幕像素）
    int m_overlayTextSize;     // 文字显示大小（屏幕像素）

    // 网格缓存相关
    mutable bool m_gridCacheValid;      // 网格缓存是否有效
    mutable QSize m_lastGridImageSize;  // 上次绘制网格时的图像尺寸
    mutable int m_lastGridSpacing;      // 上次绘制网格时的间距

    // 模板匹配相关成员
    QVector<TemplateInfo> m_loadedTemplates;         // 已加载的模板列表
    QVector<TemplateMatchResult> m_currentMatches;   // 当前匹配结果
    bool m_isMatchingEnabled;                        // 是否启用匹配
    int m_matchingFrameSkip;                         // 匹配帧跳过计数
    static const int MATCHING_FRAME_INTERVAL = 3;   // 每3帧执行一次匹配

    // Halcon 模型缓存：key 采用 halconModelPath，否则回退到 imagePath/name
    QHash<QString, std::shared_ptr<HalconCpp::HTuple>> m_halconModelCache;

    // 私有绘图方法
    void drawGrid(QPainter& painter, const DrawingContext& ctx) const;
    void drawPoints(QPainter& painter, const DrawingContext& ctx) const;
    void drawLines(QPainter& painter, const DrawingContext& ctx) const;
    void drawLineSegments(QPainter& painter, const DrawingContext& ctx) const;
    void drawCircles(QPainter& painter, const DrawingContext& ctx) const;
    void drawFineCircles(QPainter& painter, const DrawingContext& ctx) const;
    void drawParallels(QPainter& painter, const DrawingContext& ctx) const;
    void drawTwoLines(QPainter& painter, const DrawingContext& ctx) const;
    void drawLineSegmentAngles(QPainter& painter, const DrawingContext& ctx) const;
    void drawROIs(QPainter& painter, const DrawingContext& ctx) const;
    
    // 单个对象绘制方法
    void drawSinglePoint(QPainter& painter, const QPointF& point, int index, const DrawingContext& ctx) const;
    void drawSingleLine(QPainter& painter, const LineObject& line, bool isPreview, const DrawingContext& ctx) const;
    void drawSingleLineSegment(QPainter& painter, const LineSegmentObject& lineSegment, const DrawingContext& ctx) const;
    void drawSingleCircle(QPainter& painter, const CircleObject& circle, const DrawingContext& ctx) const;
    void drawSingleFineCircle(QPainter& painter, const FineCircleObject& fineCircle, const DrawingContext& ctx) const;
    void drawSingleParallel(QPainter& painter, const ParallelObject& parallel, const DrawingContext& ctx) const;
    void drawSingleTwoLines(QPainter& painter, const TwoLinesObject& twoLines, const DrawingContext& ctx) const;
    void drawSingleLineSegmentAngle(QPainter& painter, const LineSegmentAngleObject& angleObj, int index, const DrawingContext& ctx) const;
    void drawSingleROI(QPainter& painter, const ROIDetectionObject& roi, const DrawingContext& ctx) const;
    void drawSingleROICreation(QPainter& painter, const ROIObject& roi, const DrawingContext& ctx) const;
    void drawROIHandles(QPainter& painter, const ROIObject& roi, const DrawingContext& ctx) const;
    ROIObject::HandleType getROIHandleAt(const QPointF& pos) const;
    void handleROIDrag(ROIObject::HandleType handle, const QPointF& delta);
    void handleROIRotation(const QPointF& delta);
    void updateROICursor(ROIObject::HandleType handle);
    void drawROIButtons(QPainter& painter, const DrawingContext& ctx) const;
    void drawROIInfo(QPainter& painter, const DrawingContext& ctx) const;
    bool isPointInROIButton(const QPointF& pos, bool& isConfirm) const;
    
    // ROI旋转拖拽辅助函数
    QPointF getAnchorPointInLocalCoords(ROIObject::HandleType handle, const QRectF& rect) const;
    QPointF rotatePoint(const QPointF& point, const QPointF& center, qreal angleDegrees) const;
    QVector<QPointF> getRotatedRectCorners(const QRectF& rect, qreal angleDegrees) const;




    
    // 预览绘制方法
    void drawCurrentPreview(QPainter& painter, const DrawingContext& ctx) const;
    void drawSelectionHighlights(QPainter& painter) const;

    // 按历史顺序绘制方法
    void drawObjectsByHistory(QPainter& painter, const DrawingContext& ctx) const;
    void drawSingleObjectByAction(QPainter& painter, const DrawingAction& action, const DrawingContext& ctx) const;

    // 鼠标事件处理方法
    void handlePointDrawingClick(const QPointF& pos);
    void handleLineDrawingClick(const QPointF& pos);
    void handleLineSegmentDrawingClick(const QPointF& pos);
    void handleCircleDrawingClick(const QPointF& pos);
    void handleFineCircleDrawingClick(const QPointF& pos);
    void handleParallelDrawingClick(const QPointF& pos);
    void handleTwoLinesDrawingClick(const QPointF& pos);
    void handleROIDrawingClick(const QPointF& pos);
    void handleROICreationClick(const QPointF& pos);
    void handleSelectionClick(const QPointF& pos, bool ctrlPressed);

    // 命中测试方法
    int hitTestPoint(const QPointF& pos, double tolerance = 5.0) const;
    int hitTestCircleCenter(const QPointF& testPos, QPointF& centerOut, double tolerance, bool fineCircle) const;
    int findDerivedPoint(int sourceType, int sourceIndex) const;
    void removeDerivedPointsForSourceIndices(const QList<int>& removedIndices, int sourceType);
    int hitTestLine(const QPointF& pos, double tolerance = 5.0) const;
    int hitTestLineSegment(const QPointF& pos, double tolerance = 5.0) const;
    int hitTestCircle(const QPointF& pos, double tolerance = 5.0) const;
    int hitTestFineCircle(const QPointF& pos, double tolerance = 5.0) const;
    int hitTestParallel(const QPointF& pos, double tolerance = 5.0) const;
    int hitTestTwoLines(const QPointF& pos, double tolerance = 5.0) const;
    int hitTestLineSegmentAngle(const QPointF& pos, double tolerance = 5.0) const;
    int hitTestROI(const QPointF& pos, double tolerance = 5.0) const;

    // ROI管理方法
    void removeLastROI();
    
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
    QRectF constrainROIToBounds(const QRectF& roi) const;
    bool isROIWithinBounds(const QRectF& roi) const;
    
    // 绘图辅助方法
    QPen createPen(const QColor& color, int width, double scale, bool dashed = false) const;
    QFont createFont(int baseSize, double scale) const;
    double calculateFontSize() const;
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
    QString analyzeLineSegmentCircleRelation(const QPointF& lineStart, const QPointF& lineEnd, const QPointF& circleCenter, double radius) const;
    double calculateLineSegmentAngle(const QPointF& line1Start, const QPointF& line1End, const QPointF& line2Start, const QPointF& line2End) const;
    QPointF calculatePerpendicularFoot(const QPointF& point, const QPointF& lineStart, const QPointF& lineEnd) const;
    bool isPointOnLineSegment(const QPointF& point, const QPointF& lineStart, const QPointF& lineEnd, double tolerance) const;

    // 平行线中线辅助函数
    bool getParallelMiddleLinePoints(int parallelIndex, QPointF& lineStart, QPointF& lineEnd) const;
    // Bisector line helper (TwoLinesObject)
    bool getBisectorLinePoints(int twoLinesIndex, QPointF& lineStart, QPointF& lineEnd) const;

    // 像素标定辅助函数
    void updateAllMeasurementLabels();
    void performCalibrationWithLineSegment(int lineSegmentIndex);
    void performMultiPointCalibrationWithLineSegment(int lineSegmentIndex);
    void performCircleCalibration(double pixelRadius);
    void showMultiPointCalibrationDialog();
    double calculateMultiPointPixelScale() const;

    // 标定转换辅助函数
    QString formatDistance(double pixelDistance) const;
    QString formatSignedDistance(double pixelDelta) const;
    QString formatCoordinate(const QPointF& pixelCoord) const;
    QString formatRadius(double pixelRadius) const;

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
    void setROIsData(const QVector<ROIDetectionObject>& rois);

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
    void setCurrentROIData(const ROIDetectionObject& currentROI);
    void clearCurrentROIData();

};

#endif // PAINTINGOVERLAY_H
