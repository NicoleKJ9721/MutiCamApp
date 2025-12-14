#include "PaintingOverlay.h"
#include "../core/MutiCamApp.h"
#include <QApplication>
#include <QDebug>
#include <QtMath>
#include <QMessageBox>
#include <QInputDialog>
#include <QTime>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <exception>
#include <stdexcept>
#include <QCoreApplication>
#include <QStringList>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QFileInfo>
#include <algorithm>
#include <cmath>
#include <opencv2/opencv.hpp>
#ifndef __APPLE__
#include "HalconCpp.h"
#include "HDevThread.h"
#else
#include <HALCONCpp/HalconCpp.h>
#include <HALCONCpp/HDevThread.h>
#endif
using namespace HalconCpp;
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {
QString resolveRuntimePath(const QString& relativePath)
{
    const QString appDir = QCoreApplication::applicationDirPath();
    QStringList candidates = {
        QDir(appDir).filePath(relativePath),
        QDir(appDir).filePath("../" + relativePath),
        QDir(appDir).filePath("../../" + relativePath)
    };
    for (const QString& candidate : candidates) {
        if (QFile::exists(candidate)) {
            return candidate;
        }
    }
    return candidates.front();
}

// 从 Halcon 形状模型中提取轮廓，并转换为 Qt 多边形列表（模板坐标系下，x=col,y=row）
static QVector<QPolygonF> extractContoursFromHalconModel(const HalconCpp::HTuple& modelId)
{
    QVector<QPolygonF> contours;

    try {
        HalconCpp::HObject hoContours;
        HalconCpp::GetShapeModelContours(&hoContours, modelId, 1);

        HalconCpp::HTuple hvNum;
        HalconCpp::CountObj(hoContours, &hvNum);
        const Hlong numContours = hvNum.TupleLength() > 0 ? hvNum[0].I() : 0;

        for (Hlong i = 1; i <= numContours; ++i) {
            HalconCpp::HObject hoSingle;
            HalconCpp::SelectObj(hoContours, &hoSingle, i);

            HalconCpp::HTuple hvRow, hvCol;
            HalconCpp::GetContourXld(hoSingle, &hvRow, &hvCol);

            const Hlong length = hvRow.TupleLength();
            if (length <= 0) {
                continue;
            }

            QPolygonF poly;
            poly.reserve(static_cast<int>(length));

            for (Hlong j = 0; j < length; ++j) {
                const double row = static_cast<double>(hvRow[j]);
                const double col = static_cast<double>(hvCol[j]);
                poly.append(QPointF(col, row)); // Qt: x=col, y=row
            }

            if (!poly.isEmpty()) {
                contours.append(poly);
            }
        }
    } catch (const HalconCpp::HException& e) {
        qWarning() << "从Halcon模型提取轮廓失败:" << e.ErrorMessage().TextA();
    } catch (const std::exception& e) {
        qWarning() << "从Halcon模型提取轮廓时发生异常:" << e.what();
    }

    return contours;
}
}

PaintingOverlay::PaintingOverlay(QWidget *parent)
    : QWidget(parent)
    , m_isDrawingMode(false)
    , m_currentDrawingTool(DrawingTool::None)
    , m_rotationIconRenderer(nullptr)
    , m_cachedROIAngle(-999.0) // 初始化为不可能的值，强制第一次更新
    , m_hasCurrentLine(false)
    , m_hasCurrentLineSegment(false)
    , m_hasCurrentCircle(false)
    , m_hasCurrentFineCircle(false)
    , m_hasCurrentParallel(false)
    , m_hasCurrentTwoLines(false)
    , m_hasCurrentROI(false)
    , m_hasCurrentROIDetection(false)
    , m_roiCreationMode(false)
    , m_activeHandle(ROIObject::NoHandle)
    , m_hoverHandle(ROIObject::NoHandle)
    , m_isDragging(false)
    , m_isHoveringConfirmButton(false)
    , m_isHoveringCancelButton(false)
    , m_isHoveringRotationHandle(false)
    , m_hasValidMousePos(false)
    , m_selectionEnabled(true)
    , m_drawingContextValid(false)
    , m_scaleFactor(1.0)
    , m_imageSize(QSize())
    , m_pixelScale(1.0)           // 默认像素比例
    , m_unit("μm")               // 默认单位微米
    , m_isCalibrated(false)      // 默认未标定
    , m_isCalibrationMode(false) // 默认非标定模式
    , m_isMultiPointCalibrationMode(false) // 默认非多点标定模式
    , m_isCircleCalibrationMode(false)    // 默认非圆标定模式
    , m_edgeDetector(nullptr)
    , m_shapeDetector(nullptr)
    , m_gridSpacing(0)              // 默认不显示网格
    , m_gridColor(Qt::red)          // 红色网格
    , m_gridStyle(Qt::DashLine)     // 虚线样式
    , m_gridWidth(1)                // 线宽1像素
    , m_gridCacheValid(false)       // 网格缓存初始无效
    , m_lastGridImageSize(QSize())  // 初始图像尺寸
    , m_lastGridSpacing(0)          // 初始网格间距
    , m_isMatchingEnabled(false)     // 默认禁用匹配
    , m_matchingFrameSkip(0)         // 帧跳过计数初始为0
{
    // 关键：设置透明背景，并让鼠标事件穿透到下层（如果需要）
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setMouseTracking(true);

    // 设置焦点策略，使PaintingOverlay能够接收焦点
    setFocusPolicy(Qt::ClickFocus);

    // 初始化图像处理对象
    m_edgeDetector = new EdgeDetector();
    m_shapeDetector = new ShapeDetector();
    
    // 加载配置文件
    TemplateMatchingConfig::instance()->loadConfig();
}

PaintingOverlay::~PaintingOverlay()
{
    // 清理图像处理对象
    if (m_edgeDetector) {
        delete m_edgeDetector;
        m_edgeDetector = nullptr;
    }

    if (m_shapeDetector) {
        delete m_shapeDetector;
        m_shapeDetector = nullptr;
    }

    if (m_rotationIconRenderer) {
        delete m_rotationIconRenderer;
        m_rotationIconRenderer = nullptr;
    }

    // 清理缓存的图像数据
    if (!m_lastProcessedFrame.empty()) {
        m_lastProcessedFrame.release();
    }

    // 清理 Halcon 模型缓存
    clearHalconModelCache("析构");

    qDebug() << "PaintingOverlay析构完成";
}

void PaintingOverlay::startDrawing(DrawingTool tool)
{
    qDebug() << "PaintingOverlay::startDrawing called with tool:" << static_cast<int>(tool);
    m_currentDrawingTool = tool;
    m_isDrawingMode = true;
    m_selectionEnabled = false;

    // 清除选择状态（从选择模式切换到绘画模式时）
    clearSelection();

    // 清除当前绘制状态
    m_hasCurrentLine = false;
    m_hasCurrentCircle = false;
    m_hasCurrentParallel = false;
    m_hasCurrentTwoLines = false;
    m_hasCurrentROI = false;
    m_currentPoints.clear();

    setCursor(Qt::CrossCursor);
    update();
}

void PaintingOverlay::stopDrawing()
{
    m_isDrawingMode = false;
    m_currentDrawingTool = DrawingTool::None;

    // 清除当前绘制状态
    m_hasCurrentLine = false;
    m_hasCurrentCircle = false;
    m_hasCurrentParallel = false;
    m_hasCurrentTwoLines = false;
    // 注意：不清除m_hasCurrentROI，因为它用于ROI创建模式
    m_hasCurrentROIDetection = false; // 只清除ROI检测相关的状态
    m_currentPoints.clear();

    // 停止绘制模式时重新启用选择模式并清除选择状态
    m_selectionEnabled = true;
    clearSelection();

    setCursor(Qt::ArrowCursor);
    update();
}

void PaintingOverlay::clearAllDrawings()
{
    // 只清空手动绘制的图形，保留自动检测的结果
    clearManualDrawings();
}

void PaintingOverlay::clearManualDrawings()
{
    // 收集需要删除的手动绘制图形的索引
    QSet<int> pointsToRemove, linesToRemove, lineSegmentsToRemove;
    QSet<int> circlesToRemove, fineCirclesToRemove, parallelsToRemove;
    QSet<int> twoLinesToRemove, lineSegmentAnglesToRemove, roisToRemove;

    // 遍历历史记录，找出所有手动绘制的图形
    for (const DrawingAction& action : m_drawingHistory) {
        if (action.source == DrawingAction::ManualDrawing) {
            switch (action.type) {
                case DrawingAction::AddPoint:
                    if (action.index < m_points.size()) {
                        pointsToRemove.insert(action.index);
                    }
                    break;
                case DrawingAction::AddLine:
                    if (action.index < m_lines.size()) {
                        linesToRemove.insert(action.index);
                    }
                    break;
                case DrawingAction::AddLineSegment:
                    if (action.index < m_lineSegments.size()) {
                        lineSegmentsToRemove.insert(action.index);
                    }
                    break;
                case DrawingAction::AddCircle:
                    if (action.index < m_circles.size()) {
                        circlesToRemove.insert(action.index);
                    }
                    break;
                case DrawingAction::AddFineCircle:
                    if (action.index < m_fineCircles.size()) {
                        fineCirclesToRemove.insert(action.index);
                    }
                    break;
                case DrawingAction::AddParallel:
                    if (action.index < m_parallels.size()) {
                        parallelsToRemove.insert(action.index);
                    }
                    break;
                case DrawingAction::AddTwoLines:
                    if (action.index < m_twoLines.size()) {
                        twoLinesToRemove.insert(action.index);
                    }
                    break;
                case DrawingAction::AddLineSegmentAngle:
                    if (action.index < m_lineSegmentAngles.size()) {
                        lineSegmentAnglesToRemove.insert(action.index);
                    }
                    break;
                case DrawingAction::AddROI:
                    if (action.index < m_rois.size()) {
                        roisToRemove.insert(action.index);
                    }
                    break;
                default:
                    break;
            }
        }
    }

    // 从后往前删除图形（避免索引变化问题）
    removeItemsByIndices(m_points, pointsToRemove);
    removeItemsByIndices(m_lines, linesToRemove);
    removeItemsByIndices(m_lineSegments, lineSegmentsToRemove);
    removeItemsByIndices(m_circles, circlesToRemove);
    removeItemsByIndices(m_fineCircles, fineCirclesToRemove);
    removeItemsByIndices(m_parallels, parallelsToRemove);
    removeItemsByIndices(m_twoLines, twoLinesToRemove);
    removeItemsByIndices(m_lineSegmentAngles, lineSegmentAnglesToRemove);
    removeItemsByIndices(m_rois, roisToRemove);

    // 清理历史记录中的手动绘制操作
    QStack<DrawingAction> newHistory;
    for (const DrawingAction& action : m_drawingHistory) {
        if (action.source != DrawingAction::ManualDrawing) {
            newHistory.push(action);
        }
    }
    m_drawingHistory = newHistory;

    // 清除选择状态
    clearSelection();

    // 发送同步信号
    emit drawingDataChanged(m_viewName);

    update();
}

void PaintingOverlay::undoLastDrawing()
{
    // 从历史记录中找到最后一次手动绘制操作
    for (int i = m_drawingHistory.size() - 1; i >= 0; --i) {
        if (m_drawingHistory[i].source == DrawingAction::ManualDrawing) {
            DrawingAction action = m_drawingHistory[i];
            m_drawingHistory.removeAt(i);

            // 执行撤销操作
            undoAction(action);
            return;
        }
    }
}

void PaintingOverlay::undoLastDetection()
{
    // 从历史记录中找到最后一次自动检测操作
    for (int i = m_drawingHistory.size() - 1; i >= 0; --i) {
        if (m_drawingHistory[i].source == DrawingAction::AutoDetection) {
            DrawingAction action = m_drawingHistory[i];
            m_drawingHistory.removeAt(i);

            // 执行撤销操作
            undoAction(action);
            return;
        }
    }
}

void PaintingOverlay::setTransforms(const QPointF& offset, double scale, const QSize& imageSize)
{
    m_imageOffset = offset;
    m_scaleFactor = scale;

    // 如果图像尺寸发生变化，使网格缓存失效
    if (m_imageSize != imageSize) {
        m_gridCacheValid = false;
    }

    m_imageSize = imageSize;
    update();
}

void PaintingOverlay::setViewName(const QString& viewName)
{
    m_viewName = viewName;
}

QString PaintingOverlay::getViewName() const
{
    return m_viewName;
}

PaintingOverlay::DrawingState PaintingOverlay::getDrawingState() const
{
    DrawingState state;
    state.points = m_points;
    state.lines = m_lines;
    state.lineSegments = m_lineSegments;
    state.circles = m_circles;
    state.fineCircles = m_fineCircles;
    state.parallels = m_parallels;
    state.twoLines = m_twoLines;
    state.history = m_drawingHistory;
    return state;
}

void PaintingOverlay::setDrawingState(const DrawingState& state)
{
    m_points = state.points;
    m_lines = state.lines;
    m_lineSegments = state.lineSegments;
    m_circles = state.circles;
    m_fineCircles = state.fineCircles;
    m_parallels = state.parallels;
    m_twoLines = state.twoLines;
    m_drawingHistory = state.history;
    clearSelection();
    update();
}



void PaintingOverlay::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    
    // 启用抗锯齿和高质量渲染
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    
    // 2. 绘制前景几何图形（硬件加速优化）
    if (!m_imageSize.isEmpty()) {
        // 缓存变换参数避免重复函数调用，使用QTransform提升性能
        // 在paintEvent开头计算一次变换参数，避免重复函数调用开销
        const double scale = m_scaleFactor;
        const QPointF offset = m_imageOffset;
        
        painter.save();
        
        // 使用QTransform统一管理坐标变换，性能更优
        QTransform transform;
        transform.translate(offset.x(), offset.y());
        transform.scale(scale, scale);
        painter.setTransform(transform);
        
        // 在变换后的坐标系中设置裁剪区域，使用图像坐标
        painter.setClipRect(QRect(0, 0, m_imageSize.width(), m_imageSize.height()));
        
        // 检查是否需要更新DrawingContext缓存
        if (needsDrawingContextUpdate()) {
            updateDrawingContext();
        }
        
        // 使用缓存的DrawingContext，避免重复创建对象
        const DrawingContext& ctx = m_cachedDrawingContext;
        
        // 现在坐标系原点就是图像左上角，所有绘制函数可以直接使用图像坐标
        // 传递绘制上下文，避免重复创建对象

        // 0. 首先绘制网格（作为最底层背景）
        drawGrid(painter, ctx);

        // 1. 先绘制选中状态高亮（作为底层外描边）
        drawSelectionHighlights(painter);

        // 2. 按历史顺序绘制所有已完成的图形（在高亮之上）
        drawObjectsByHistory(painter, ctx);

        // 3. 【关键】绘制当前正在预览的图形（在最上层）
        if (m_isDrawingMode) {
            drawCurrentPreview(painter, ctx);
        }

        // 4. 绘制模板匹配结果（在所有图形之上）
        drawMatchResults(painter, ctx);

        painter.restore();
    }
}

void PaintingOverlay::mousePressEvent(QMouseEvent *event)
{


    // 检查是否应该将事件传递给ZoomPanWidget（空格键平移模式）
    ZoomPanWidget* zoomPanWidget = qobject_cast<ZoomPanWidget*>(parentWidget());
    if (zoomPanWidget && zoomPanWidget->isSpacePressed()) {

        // 空格键按下时，将鼠标事件传递给ZoomPanWidget
        QApplication::sendEvent(zoomPanWidget, event);
        return;
    }

    // 设置焦点到当前overlay，以便getActivePaintingOverlay能正确识别
    setFocus();

    // 发送overlay激活信号，用于记录最后活动的overlay
    emit overlayActivated(this);

    // 处理右键点击：如果在绘制模式且没有选中任何图元，退出绘制模式
    if (event->button() == Qt::RightButton) {
        // 特殊处理多点标定模式
        if (m_isMultiPointCalibrationMode) {
            // 在多点标定模式下右键显示选项
            showMultiPointCalibrationDialog();
            return;
        }

        // ROI创建模式的特殊处理：右键不退出模式
        if (m_roiCreationMode) {
            return;
        }

        if (m_isDrawingMode) {
            // 检查是否有选中的对象
            bool hasSelection = !m_selectedPoints.isEmpty() || !m_selectedLines.isEmpty() ||
                               !m_selectedCircles.isEmpty() || !m_selectedFineCircles.isEmpty() ||
                               !m_selectedParallels.isEmpty() || !m_selectedTwoLines.isEmpty() ||
                               !m_selectedLineSegments.isEmpty() || !m_selectedLineSegmentAngles.isEmpty() ||
                               !m_selectedParallelMiddleLines.isEmpty() || !m_selectedROIs.isEmpty();

            if (!hasSelection) {
                stopDrawing();
                update();
                return;
            }
        }
        QWidget::mousePressEvent(event);
        return;
    }

    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    QPointF imagePos = widgetToImage(event->pos());

    // 检查坐标是否在图像范围内
    if (!isPointInImageBounds(imagePos)) {
        // 坐标在图像外，不处理绘制事件
        QWidget::mousePressEvent(event);
        return;
    }

    // 单次点选模式：优先拦截一次左键点击（用于载物台辅助标定等）
    if (m_isPointPickMode) {
        m_isPointPickMode = false;
        m_pointPickPurpose.clear();
        if (m_hasPointPickPrevCursor) {
            setCursor(m_pointPickPrevCursor);
            m_hasPointPickPrevCursor = false;
        } else {
            unsetCursor();
        }
        emit pointPicked(m_viewName, imagePos);
        return;
    }

    if (m_selectionEnabled && !m_roiCreationMode) {
        // 处理选择逻辑
        bool ctrlPressed = (event->modifiers() & Qt::ControlModifier) != 0;
        handleSelectionClick(imagePos, ctrlPressed);
        return;
    } else if (m_selectionEnabled && m_roiCreationMode) {
        // 跳过选择处理
    }
    
    // ROI创建模式需要特殊处理
    if (!m_isDrawingMode && !m_roiCreationMode) {
        return;
    }

    // ROI创建模式的特殊处理
    if (m_roiCreationMode) {
        handleROICreationClick(imagePos);
        return;
    }

    // 移除频繁的鼠标点击日志
    switch (m_currentDrawingTool) {
        case DrawingTool::Point:
            handlePointDrawingClick(imagePos);
            break;
        case DrawingTool::Line:
            handleLineDrawingClick(imagePos);
            break;
        case DrawingTool::Circle:
            handleCircleDrawingClick(imagePos);
            break;
        case DrawingTool::FineCircle:
            handleFineCircleDrawingClick(imagePos);
            break;
        case DrawingTool::Parallel:
            handleParallelDrawingClick(imagePos);
            break;
        case DrawingTool::TwoLines:
            handleTwoLinesDrawingClick(imagePos);
            break;
        case DrawingTool::LineSegment:
            handleLineSegmentDrawingClick(imagePos);
            break;
        case DrawingTool::ROI_LineDetect:
        case DrawingTool::ROI_CircleDetect:
            handleROIDrawingClick(imagePos);
            break;
        case DrawingTool::ROI_CREATION:
            handleROICreationClick(imagePos);
            break;
        default:
            break;
    }
}

void PaintingOverlay::startPointPick(const QString& purpose)
{
    m_isPointPickMode = true;
    m_pointPickPurpose = purpose;

    m_pointPickPrevCursor = cursor();
    m_hasPointPickPrevCursor = true;
    setCursor(Qt::CrossCursor);
}

void PaintingOverlay::cancelPointPick()
{
    if (!m_isPointPickMode) {
        return;
    }

    m_isPointPickMode = false;
    m_pointPickPurpose.clear();
    if (m_hasPointPickPrevCursor) {
        setCursor(m_pointPickPrevCursor);
        m_hasPointPickPrevCursor = false;
    } else {
        unsetCursor();
    }
}

bool PaintingOverlay::isPointPicking() const
{
    return m_isPointPickMode;
}

void PaintingOverlay::mouseMoveEvent(QMouseEvent *event)
{
    // 检查是否应该将事件传递给ZoomPanWidget（空格键平移模式）
    ZoomPanWidget* zoomPanWidget = qobject_cast<ZoomPanWidget*>(parentWidget());
    if (zoomPanWidget && zoomPanWidget->isPanning()) {
        // 平移模式时，将鼠标事件传递给ZoomPanWidget
        QApplication::sendEvent(zoomPanWidget, event);
        return;
    }

    // 事件节流：限制更新频率以提升性能
    static QTime lastUpdateTime = QTime::currentTime();
    QTime currentTime = QTime::currentTime();

    if (lastUpdateTime.msecsTo(currentTime) < 16) { // 约60fps
        return;
    }
    lastUpdateTime = currentTime;
    
    QPointF imagePos = widgetToImage(event->pos());

    // 检查坐标是否在图像范围内
    if (!isPointInImageBounds(imagePos)) {
        // 坐标在图像外，清除鼠标位置信息
        m_hasValidMousePos = false;
        return;
    }

    m_currentMousePos = imagePos;
    m_hasValidMousePos = true;

    // 处理直线预览逻辑
    if (m_hasCurrentLine && m_currentLine.points.size() >= 1) {
        // 直线绘制：更新第二个点进行实时预览
        if (m_currentLine.points.size() == 1) {
            m_currentLine.points.append(imagePos);
        } else if (m_currentLine.points.size() >= 2) {
            m_currentLine.points[1] = imagePos;
        }
    }

    // 处理线段预览逻辑
    if (m_currentDrawingTool == DrawingTool::LineSegment && m_currentPoints.size() == 1) {
        // 线段绘制：只有在有一个点时才进行预览
        // 不修改m_currentPoints，而是使用鼠标位置进行预览
        m_currentMousePos = imagePos;
        m_hasValidMousePos = true;
    }

    // 处理两线预览逻辑
    if (m_hasCurrentTwoLines) {
        if (m_currentTwoLines.points.size() == 1) {
            // 第一条线的第二个点预览，使用鼠标位置
            m_currentMousePos = imagePos;
            m_hasValidMousePos = true;
        } else if (m_currentTwoLines.points.size() == 2) {
            // 第三个点预览，使用鼠标位置
            m_currentMousePos = imagePos;
            m_hasValidMousePos = true;
        } else if (m_currentTwoLines.points.size() == 3) {
            // 第二条线的第二个点预览，使用鼠标位置
            m_currentMousePos = imagePos;
            m_hasValidMousePos = true;
        }
    }

    // 处理圆形预览逻辑
    if (m_hasCurrentCircle) {
        if (m_currentCircle.points.size() == 1) {
            // 第二个点预览：添加第二个点
            m_currentCircle.points.append(imagePos);
        } else if (m_currentCircle.points.size() == 2) {
            // 第二个点预览：更新第二个点位置
            m_currentCircle.points[1] = imagePos;
        } else if (m_currentCircle.points.size() == 3) {
            // 第三个点预览：更新第三个点位置
            m_currentCircle.points[2] = imagePos;
        }
    }

    // 处理精细圆预览逻辑
    if (m_hasCurrentFineCircle && m_currentFineCircle.points.size() >= 1) {
        // 精细圆：鼠标位置作为半径参考点
        m_currentMousePos = imagePos;
        m_hasValidMousePos = true;
    }

    // 处理平行线预览逻辑
    if (m_hasCurrentParallel && m_currentParallel.points.size() == 2) {
        // 平行线绘制：只有在有2个点后才进行第三个点的预览
        // 添加第三个点进行预览
        m_currentParallel.points.append(imagePos);
        m_currentParallel.isPreview = true;
    } else if (m_hasCurrentParallel && m_currentParallel.points.size() >= 3) {
        // 更新第三个点的预览位置
        m_currentParallel.points[2] = imagePos;
    }

    // 处理ROI预览逻辑
    if (m_hasCurrentROIDetection && !m_currentROIDetection.isCompleted) {
        if (m_currentROIDetection.points.size() == 1) {
            // ROI绘制：添加第二个点进行实时预览
            m_currentROIDetection.points.append(imagePos);
        } else if (m_currentROIDetection.points.size() >= 2) {
            // ROI绘制：更新第二个点进行实时预览
            m_currentROIDetection.points[1] = imagePos;
        }
    }

    // 处理ROI创建的拖拽逻辑
    if (m_roiCreationMode && m_hasCurrentROI) {
        if (m_isDragging && m_activeHandle != PaintingOverlay::ROIObject::NoHandle) {
            QPointF delta = imagePos - m_lastMousePos;
            QRectF oldROIRect = m_currentROI.rect;
            handleROIDrag(m_activeHandle, delta);
            m_lastMousePos = imagePos;

            // 只重绘ROI相关区域，而不是整个widget
            QRectF updateRect = oldROIRect.united(m_currentROI.rect);
            updateRect = updateRect.adjusted(-50, -50, 50, 50); // 扩展一些边距
            update(updateRect.toRect());
        } else {
            // 检查ROI按钮悬浮状态
            bool isConfirm;
            bool currentlyHoveringButton = isPointInROIButton(imagePos, isConfirm);
            bool currentlyHoveringConfirm = currentlyHoveringButton && isConfirm;
            bool currentlyHoveringCancel = currentlyHoveringButton && !isConfirm;
            
            // 检查旋转按钮悬浮状态
            PaintingOverlay::ROIObject::HandleType hoverHandle = getROIHandleAt(imagePos);
            bool currentlyHoveringRotation = (hoverHandle == PaintingOverlay::ROIObject::RotationHandle);
            
            // 更新悬浮状态
            if (currentlyHoveringConfirm != m_isHoveringConfirmButton ||
                currentlyHoveringCancel != m_isHoveringCancelButton ||
                currentlyHoveringRotation != m_isHoveringRotationHandle) {
                
                m_isHoveringConfirmButton = currentlyHoveringConfirm;
                m_isHoveringCancelButton = currentlyHoveringCancel;
                m_isHoveringRotationHandle = currentlyHoveringRotation;
                
                if (currentlyHoveringButton || m_isHoveringRotationHandle) {
                    setCursor(Qt::PointingHandCursor);
                } else {
                    m_hoverHandle = hoverHandle;
                    updateROICursor(hoverHandle);
                }
                update(); // 重绘以更新按钮颜色
            } else if (!currentlyHoveringButton && !m_isHoveringRotationHandle) {
                if (hoverHandle != m_hoverHandle) {
                    m_hoverHandle = hoverHandle;
                    updateROICursor(hoverHandle);
                }
            }
        }
    }

    // 只在绘图模式或有当前绘制对象时更新
    if (m_isDrawingMode || m_hasCurrentLine || m_hasCurrentCircle ||
        m_hasCurrentFineCircle || m_hasCurrentParallel || m_hasCurrentTwoLines ||
        m_hasCurrentROIDetection || m_roiCreationMode) {
        update(); // 更新预览
    }
}

void PaintingOverlay::mouseReleaseEvent(QMouseEvent *event)
{
    // 检查是否应该将事件传递给ZoomPanWidget（空格键平移模式）
    ZoomPanWidget* zoomPanWidget = qobject_cast<ZoomPanWidget*>(parentWidget());
    if (zoomPanWidget && zoomPanWidget->isPanning()) {
        // 平移模式时，将鼠标事件传递给ZoomPanWidget
        QApplication::sendEvent(zoomPanWidget, event);
        return;
    }

    // 只处理左键释放
    if (event->button() != Qt::LeftButton) {
        QWidget::mouseReleaseEvent(event);
        return;
    }

    // 处理ROI创建的拖拽结束
    if (m_roiCreationMode && m_hasCurrentROI && m_isDragging) {
        m_isDragging = false;
        m_activeHandle = PaintingOverlay::ROIObject::NoHandle;

        // 发送ROI变化信号
        emit roiChanged(m_viewName, m_currentROI.rect, m_currentROI.angle);

        qDebug() << "ROI拖拽结束，当前ROI:" << m_currentROI.rect << "角度:" << m_currentROI.angle;
        update();
        return;
    }

    QWidget::mouseReleaseEvent(event);
}

void PaintingOverlay::mouseDoubleClickEvent(QMouseEvent *event)
{
    // 只处理左键双击
    if (event->button() != Qt::LeftButton) {
        QWidget::mouseDoubleClickEvent(event);
        return;
    }

    // 只在主界面视图上启用双击跳转（避免选项卡视图的循环跳转）
    if (m_viewName == "Vertical" || m_viewName == "Left" || m_viewName == "Front") {
        emit viewDoubleClicked(m_viewName);
    }

    QWidget::mouseDoubleClickEvent(event);
}

void PaintingOverlay::contextMenuEvent(QContextMenuEvent *event)
{
    // 只在选择模式下显示右键菜单
    if (!m_selectionEnabled) {
        QWidget::contextMenuEvent(event);
        return;
    }
    
    // 检查是否有选中的对象
    bool hasSelection = !m_selectedPoints.isEmpty() || !m_selectedLines.isEmpty() ||
                       !m_selectedCircles.isEmpty() || !m_selectedFineCircles.isEmpty() ||
                       !m_selectedParallels.isEmpty() || !m_selectedTwoLines.isEmpty() ||
                       !m_selectedLineSegments.isEmpty() || !m_selectedLineSegmentAngles.isEmpty() ||
                       !m_selectedParallelMiddleLines.isEmpty() || !m_selectedBisectorLines.isEmpty() ||
                       !m_selectedROIs.isEmpty();
    
    if (!hasSelection) {
        QWidget::contextMenuEvent(event);
        return;
    }
    
    // 创建右键菜单
    QMenu contextMenu(this);
    
    // 添加删除选项
    QAction *deleteAction = contextMenu.addAction("删除");
    deleteAction->setIcon(QIcon(resolveRuntimePath("icon/delete.svg")));
    
    // 检查是否选中了恰好两个点，如果是则添加"点与点"选项
    QAction *pointToPointAction = nullptr;
    if (m_selectedPoints.size() == 2 && m_selectedLines.isEmpty() &&
        m_selectedCircles.isEmpty() && m_selectedFineCircles.isEmpty() &&
        m_selectedParallels.isEmpty() && m_selectedTwoLines.isEmpty()) {
        pointToPointAction = contextMenu.addAction("点与点");
    }

    // 复合测量选项
    QAction *pointToLineAction = nullptr;
    QAction *pointToCircleAction = nullptr;
    QAction *lineToCircleAction = nullptr;
    QAction *lineAngleAction = nullptr;
    QAction *pointToMidlineAction = nullptr;
    QAction *pointToBisectorAction = nullptr;

    // 点与线距离测量（支持直线或平行线中线）
    if (m_selectedPoints.size() == 1 &&
        ((m_selectedLines.size() == 1 && m_selectedParallelMiddleLines.isEmpty()) ||
         (m_selectedParallelMiddleLines.size() == 1 && m_selectedLines.isEmpty())) &&
        m_selectedCircles.isEmpty() && m_selectedFineCircles.isEmpty()) {
        pointToLineAction = contextMenu.addAction("点与线距离");
    }

    // 点与圆距离测量
    if (m_selectedPoints.size() == 1 && m_selectedCircles.size() == 1 &&
        m_selectedLines.isEmpty() && m_selectedFineCircles.isEmpty()) {
        pointToCircleAction = contextMenu.addAction("点与圆距离");
    }

    // 点与精细圆距离测量
    QAction *pointToFineCircleAction = nullptr;
    if (m_selectedPoints.size() == 1 && m_selectedFineCircles.size() == 1 &&
        m_selectedLines.isEmpty() && m_selectedCircles.isEmpty()) {
        pointToFineCircleAction = contextMenu.addAction("点与精细圆距离");
    }

    // 线与圆关系分析（支持直线或平行线中线）
    if (((m_selectedLines.size() == 1 && m_selectedParallelMiddleLines.isEmpty()) ||
         (m_selectedParallelMiddleLines.size() == 1 && m_selectedLines.isEmpty())) &&
        m_selectedCircles.size() == 1 && m_selectedPoints.isEmpty() &&
        m_selectedFineCircles.isEmpty()) {
        lineToCircleAction = contextMenu.addAction("线与圆关系");
    }

    // 线与精细圆关系分析（支持直线或平行线中线）
    QAction *lineToFineCircleAction = nullptr;
    if (((m_selectedLines.size() == 1 && m_selectedParallelMiddleLines.isEmpty()) ||
         (m_selectedParallelMiddleLines.size() == 1 && m_selectedLines.isEmpty())) &&
        m_selectedFineCircles.size() == 1 && m_selectedPoints.isEmpty() &&
        m_selectedCircles.isEmpty()) {
        lineToFineCircleAction = contextMenu.addAction("线与精细圆关系");
    }

    // 两线段夹角测量
    if (m_selectedLineSegments.size() == 2 && m_selectedPoints.isEmpty() &&
        m_selectedLines.isEmpty() && m_selectedCircles.isEmpty()) {
        lineAngleAction = contextMenu.addAction("线段夹角");
    }

    // 点与平行线中线距离测量（已统一到"点与线距离"中）
    // if (m_selectedPoints.size() == 1 && m_selectedParallelMiddleLines.size() == 1 &&
    //     m_selectedLines.isEmpty() && m_selectedCircles.isEmpty()) {
    //     pointToMidlineAction = contextMenu.addAction("点与平行线中线距离");
    // }

    // 点与角平分线距离测量（TwoLinesObject的角平分线）
    if (m_selectedPoints.size() == 1 && m_selectedBisectorLines.size() == 1 &&
        m_selectedLines.isEmpty() && m_selectedCircles.isEmpty()) {
        pointToBisectorAction = contextMenu.addAction("点与角平分线距离");
    }
    
    // 点与角平分线距离测量（LineSegmentObject的角平分线）
    QAction *pointToBisectorLineSegmentAction = nullptr;
    QAction *pointToLineSegmentAction = nullptr;
    if (m_selectedPoints.size() == 1 && m_selectedLineSegments.size() == 1 &&
        m_selectedLines.isEmpty() && m_selectedCircles.isEmpty()) {
        // 检查选中的LineSegmentObject是否为角平分线
        int lineSegmentIndex = *m_selectedLineSegments.begin();
        if (lineSegmentIndex >= 0 && lineSegmentIndex < m_lineSegments.size()) {
            const LineSegmentObject& lineSegment = m_lineSegments[lineSegmentIndex];
            if (lineSegment.label.startsWith("BISECTOR:")) {
                pointToBisectorLineSegmentAction = contextMenu.addAction("点与角平分线距离");
            } else {
                // 普通线段的点与线段距离测量
                pointToLineSegmentAction = contextMenu.addAction("点与线段距离");
            }
        }
    }

    // 线段与圆关系分析
    QAction *lineSegmentToCircleAction = nullptr;
    if (m_selectedLineSegments.size() == 1 && m_selectedCircles.size() == 1 &&
        m_selectedPoints.isEmpty() && m_selectedLines.isEmpty() &&
        m_selectedFineCircles.isEmpty()) {
        lineSegmentToCircleAction = contextMenu.addAction("线段与圆关系");
    }

    // 线段与精细圆关系（仅支持一条线段与一个精细圆的组合）
    QAction *lineSegmentToFineCircleAction = nullptr;
    if (m_selectedLineSegments.size() == 1 && m_selectedFineCircles.size() == 1 &&
        m_selectedPoints.isEmpty() && m_selectedLines.isEmpty() &&
        m_selectedCircles.isEmpty()) {
        lineSegmentToFineCircleAction = contextMenu.addAction("线段与精细圆关系");
    }

    // 圆心与圆心距离（圆与圆测量）
    QAction *circleToCircleAction = nullptr;
    if (m_selectedCircles.size() == 2 &&
        m_selectedPoints.isEmpty() && m_selectedLines.isEmpty() &&
        m_selectedFineCircles.isEmpty() && m_selectedLineSegments.isEmpty()) {
        circleToCircleAction = contextMenu.addAction("圆与圆距离");
    }

    // 两条直线夹角测量（支持直线、平行线中线的各种组合）
    QAction *twoLinesAngleAction = nullptr;
    int totalLineCount = m_selectedLines.size() + m_selectedParallelMiddleLines.size();
    if (totalLineCount == 2 && m_selectedPoints.isEmpty() &&
        m_selectedCircles.isEmpty() && m_selectedFineCircles.isEmpty() &&
        m_selectedParallels.isEmpty() && m_selectedTwoLines.isEmpty() &&
        m_selectedLineSegments.isEmpty()) {
        twoLinesAngleAction = contextMenu.addAction("两线夹角");
    }

    // 显示菜单并处理选择
    QAction *selectedAction = contextMenu.exec(event->globalPos());

    if (selectedAction == deleteAction) {
        deleteSelectedObjects();
    } else if (selectedAction == pointToPointAction) {
        createLineFromSelectedPoints();
    } else if (selectedAction == pointToLineAction) {
        performComplexMeasurement("点到线距离");
    } else if (selectedAction == pointToCircleAction) {
        performComplexMeasurement("点与圆距离");
    } else if (selectedAction == pointToFineCircleAction) {
        performComplexMeasurement("点与精细圆距离");
    } else if (selectedAction == lineToCircleAction) {
        performComplexMeasurement("直线与圆关系");
    } else if (selectedAction == lineToFineCircleAction) {
        performComplexMeasurement("直线与精细圆关系");
    } else if (selectedAction == circleToCircleAction) {
        performComplexMeasurement("圆与圆距离");
    } else if (selectedAction == lineAngleAction) {
        performComplexMeasurement("线段夹角");
    } else if (selectedAction == twoLinesAngleAction) {
        performComplexMeasurement("直线夹角");
    } else if (selectedAction == pointToBisectorAction) {
        performComplexMeasurement("点到平分线距离");
    } else if (selectedAction == pointToBisectorLineSegmentAction) {
        performComplexMeasurement("点到平分线距离LineSegment");
    } else if (selectedAction == pointToLineSegmentAction) {
        performComplexMeasurement("点到线段距离");
    } else if (selectedAction == lineSegmentToCircleAction) {
        performComplexMeasurement("线段与圆关系");
    } else if (selectedAction == lineSegmentToFineCircleAction) {
        performComplexMeasurement("线段与精细圆关系");
    }
}

void PaintingOverlay::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        if (m_roiCreationMode) {
            cancelROICreation();
        }
        if (m_isDrawingMode || m_hasCurrentLine || m_hasCurrentCircle ||
            m_hasCurrentParallel || m_hasCurrentTwoLines || m_hasCurrentROI ||
            m_hasCurrentROIDetection) {
            stopDrawing();
        }
        event->accept();
        return;
    }

    QWidget::keyPressEvent(event);
}




void PaintingOverlay::handlePointDrawingClick(const QPointF& pos)
{
    PointObject point;
    point.position = pos;
    point.label = QString("P%1").arg(m_points.size() + 1);
    
    m_points.append(point);
    
    DrawingAction action;
    action.type = DrawingAction::AddPoint;
    action.source = DrawingAction::ManualDrawing;  // 手动绘制
    action.index = m_points.size() - 1;
    commitDrawingAction(action);
}

void PaintingOverlay::handleLineDrawingClick(const QPointF& pos)
{
    if (!m_hasCurrentLine) {
        // 开始新线段 - 完全重新初始化对象
        m_currentLine = LineObject(); // 重置为默认值
        m_currentLine.points.append(pos);
        m_currentLine.start = pos;
        m_hasCurrentLine = true;
    } else {
        // 完成线段 - 确认第二个点的位置
        if (m_currentLine.points.size() >= 2) {
            m_currentLine.points[1] = pos; // 确认第二个点位置
        } else {
            m_currentLine.points.append(pos); // 如果只有一个点，添加第二个点
        }
        m_currentLine.end = pos;
        m_currentLine.label = QString("L%1").arg(m_lines.size() + 1);
        
        // 计算长度
        QPointF diff = m_currentLine.end - m_currentLine.start;
        m_currentLine.length = qSqrt(diff.x() * diff.x() + diff.y() * diff.y());
        
        m_lines.append(m_currentLine);
        
        DrawingAction action;
        action.type = DrawingAction::AddLine;
        action.source = DrawingAction::ManualDrawing;  // 手动绘制
        action.index = m_lines.size() - 1;
        commitDrawingAction(action);
        
        m_hasCurrentLine = false;
    }
}

void PaintingOverlay::handleCircleDrawingClick(const QPointF& imagePos)
{
    if (!m_hasCurrentCircle) {
        // 开始新的圆形
        m_currentCircle = CircleObject();
        m_currentCircle.points.append(imagePos);
        m_hasCurrentCircle = true;
    } else if (m_currentCircle.points.size() == 2) {
        // 确认第二个点，添加第三个点用于预览
        m_currentCircle.points[1] = imagePos; // 确认第二个点的位置
        m_currentCircle.points.append(imagePos); // 添加第三个点（将在鼠标移动时更新）
    } else if (m_currentCircle.points.size() == 3) {
        // 确认第三个点，完成圆形
        m_currentCircle.points[2] = imagePos; // 确认第三个点的位置

        if (m_currentCircle.points.size() == 3) {
            // 计算圆心和半径
            if (calculateCircleFromThreePoints(m_currentCircle.points, m_currentCircle.center, m_currentCircle.radius)) {
                m_currentCircle.isCompleted = true;

                // 计算圆的面积和周长并使用通用提交逻辑
                double area = M_PI * m_currentCircle.radius * m_currentCircle.radius;
                double circumference = 2 * M_PI * m_currentCircle.radius;
                QString result = QString("圆形: 半径 %1, 面积 %2, 周长 %3")
                                .arg(m_currentCircle.radius, 0, 'f', 1).arg(area, 0, 'f', 1).arg(circumference, 0, 'f', 1);

                m_circles.append(m_currentCircle);

                DrawingAction action;
                action.type = DrawingAction::AddCircle;
                action.source = DrawingAction::ManualDrawing;  // 手动绘制
                action.index = m_circles.size() - 1;
                commitDrawingAction(action);

                emit measurementCompleted(m_viewName, result);
                clearCurrentCircleData();
            } else {
                // 三点共线，重新开始
                clearCurrentCircleData();
            }
        }
    }
    update();
}

void PaintingOverlay::handleFineCircleDrawingClick(const QPointF& pos)
{
    if (!m_hasCurrentFineCircle) {
        // 开始新的精细圆
        m_currentFineCircle = FineCircleObject();
        m_currentFineCircle.points.append(pos);
        m_hasCurrentFineCircle = true;
    } else if (m_currentFineCircle.points.size() < 5) {
        // 添加点直到有5个点
        m_currentFineCircle.points.append(pos);
        
        if (m_currentFineCircle.points.size() == 5) {
            // 使用五点拟合圆
            if (calculateCircleFromFivePoints(m_currentFineCircle.points, 
                                            m_currentFineCircle.center, 
                                            m_currentFineCircle.radius)) {
                m_currentFineCircle.isCompleted = true;
                
                // 计算精细圆的面积和周长
                double area = M_PI * m_currentFineCircle.radius * m_currentFineCircle.radius;
                double circumference = 2 * M_PI * m_currentFineCircle.radius;
                QString result = QString("精细圆: 半径 %1, 面积 %2, 周长 %3")
                                .arg(m_currentFineCircle.radius, 0, 'f', 1).arg(area, 0, 'f', 1).arg(circumference, 0, 'f', 1);
                
                DrawingAction action;
                action.type = DrawingAction::AddFineCircle;
                action.source = DrawingAction::ManualDrawing;  // 手动绘制
                action.index = m_fineCircles.size();
                action.data = result;
                commitDrawingAction(action);
            } else {
                // 拟合失败，重新开始
                clearCurrentFineCircleData();
            }
        }
    }
    update();
}

void PaintingOverlay::handleParallelDrawingClick(const QPointF& imagePos)
{
    if (!m_hasCurrentParallel) {
        // 开始新的平行线
        m_currentParallel = ParallelObject();
        m_currentParallel.points.append(imagePos);
        m_hasCurrentParallel = true;
    } else {
        // 如果当前处于预览状态，先清除预览点
        if (m_currentParallel.isPreview && m_currentParallel.points.size() > 2) {
            m_currentParallel.points.resize(2);
            m_currentParallel.isPreview = false;
        }

        if (m_currentParallel.points.size() < 3) {
            // 添加点直到有3个点
            m_currentParallel.points.append(imagePos);
            // 清除鼠标预览状态
            if (m_currentParallel.points.size() == 2) {
                m_hasValidMousePos = false;
            }
        }
        if (m_currentParallel.points.size() == 3) {
            // 计算平行线的距离和角度
            QPointF p1 = m_currentParallel.points[0];
            QPointF p2 = m_currentParallel.points[1];
            QPointF p3 = m_currentParallel.points[2];

            // 计算第一条线的角度
            m_currentParallel.angle = calculateLineAngle(p1, p2);

            // 计算点到直线的距离
            double A = p2.y() - p1.y();
            double B = p1.x() - p2.x();
            double C = p2.x() * p1.y() - p1.x() * p2.y();
            double denominator = sqrt(A * A + B * B);

            if (denominator > 0) {
                m_currentParallel.distance = abs(A * p3.x() + B * p3.y() + C) / denominator;

                // 计算第四个点和中线
                QPointF direction = p2 - p1;
                QPointF p4 = p3 + direction;

                // 计算中线的两个点
                m_currentParallel.midStart = (p1 + p3) / 2.0;
                m_currentParallel.midEnd = (p2 + p4) / 2.0;

                m_currentParallel.isCompleted = true;

                // 使用通用提交逻辑
                QString result = QString("平行线: 距离 %1, 角度 %2°")
                                .arg(formatDistance(m_currentParallel.distance)).arg(m_currentParallel.angle, 0, 'f', 1);

                m_parallels.append(m_currentParallel);

                DrawingAction action;
                action.type = DrawingAction::AddParallel;
                action.source = DrawingAction::ManualDrawing;  // 手动绘制
                action.index = m_parallels.size() - 1;
                commitDrawingAction(action);

                emit measurementCompleted(m_viewName, result);
                clearCurrentParallelData();
            } else {
                // 计算失败，重新开始
                clearCurrentParallelData();
            }
        }
    }
    update();
}

void PaintingOverlay::handleTwoLinesDrawingClick(const QPointF& imagePos)
{
    if (!m_hasCurrentTwoLines) {
        // 开始新的两线
        m_currentTwoLines = TwoLinesObject();
        m_currentTwoLines.points.append(imagePos);
        m_hasCurrentTwoLines = true;
    } else {
        if (m_currentTwoLines.points.size() < 4) {
            // 添加点直到有4个点
            m_currentTwoLines.points.append(imagePos);
            // 修复点击逻辑，确保每次点击都正确添加点并清除预览状态
            if (m_currentTwoLines.points.size() == 2) {
                // 第一条线完成后，清除预览状态
                m_hasValidMousePos = false;
            } else if (m_currentTwoLines.points.size() == 4) {
                // 第二条线完成后，清除预览状态
                m_hasValidMousePos = false;
            }
        }
        if (m_currentTwoLines.points.size() == 4) {
            // 计算两线的交点和夹角
            QPointF p1 = m_currentTwoLines.points[0];
            QPointF p2 = m_currentTwoLines.points[1];
            QPointF p3 = m_currentTwoLines.points[2];
            QPointF p4 = m_currentTwoLines.points[3];

            if (calculateLineIntersection(p1, p2, p3, p4, m_currentTwoLines.intersection)) {
                // 计算两线夹角
                double angle1 = calculateLineAngle(p1, p2);
                double angle2 = calculateLineAngle(p3, p4);
                double angleDiff = abs(angle1 - angle2);

                // 确保角度在0-180度之间，保留钝角
                if (angleDiff > 180) {
                    angleDiff = 360 - angleDiff;
                }

                m_currentTwoLines.angle = angleDiff;
                m_currentTwoLines.isCompleted = true;

                // 使用通用提交逻辑
                QString result = QString("两线夹角: %1°\n交点坐标: %2")
                                .arg(m_currentTwoLines.angle, 0, 'f', 1).arg(formatCoordinate(m_currentTwoLines.intersection));

                m_twoLines.append(m_currentTwoLines);

                DrawingAction action;
                action.type = DrawingAction::AddTwoLines;
                action.source = DrawingAction::ManualDrawing;  // 手动绘制
                action.index = m_twoLines.size() - 1;
                commitDrawingAction(action);

                emit measurementCompleted(m_viewName, result);
                clearCurrentTwoLinesData();
            } else {
                // 两线平行，重新开始
                clearCurrentTwoLinesData();
            }
        }
    }
    update();
}

void PaintingOverlay::drawPoints(QPainter& painter, const DrawingContext& ctx) const
{
    if (m_points.isEmpty()) return;
    

    // 预计算常量（匹配Python版本）
    const double innerRadius = 2.0 / m_scaleFactor;
    const double textpadding = qMax(4.0, ctx.fontSize * 0.5);  // 动态padding，字体大小的一半
    
    for (int i = 0; i < m_points.size(); ++i) {
        const QPointF& point = m_points[i].position;  // 直接使用图像坐标
        // 绘制点（绿色实心圆）- 使用预创建的画刷
        painter.setPen(Qt::NoPen);
        painter.setBrush(ctx.greenBrush);
        painter.drawEllipse(point, innerRadius, innerRadius);
        
        // 绘制坐标文本（应用标定转换）
        QString coordText = formatCoordinate(point);
        
        // 计算将文本框定位到点右上角所需的偏移量
        QFontMetrics fm(ctx.font);
        QRect textBoundingRect = fm.boundingRect(coordText);
        double bgHeight = textBoundingRect.height() + 2 * textpadding;
        
        QPointF offset(innerRadius, -innerRadius - bgHeight);
        
        // 调用带有offset参数的完美辅助函数
        drawTextWithBackground(painter, point, coordText, ctx.font, QColor(0, 255, 0), Qt::black,
                             textpadding, 1.0, offset);
    }
}

void PaintingOverlay::drawSinglePoint(QPainter& painter, const QPointF& point, int index, const DrawingContext& ctx) const
{
    // 预计算常量（匹配Python版本）
    const double innerRadius = 2.0 / m_scaleFactor;
    const double textpadding = qMax(4.0, ctx.fontSize * 0.5);  // 动态padding，字体大小的一半

    // 绘制点（绿色实心圆）- 使用预创建的画刷
    painter.setPen(Qt::NoPen);
    painter.setBrush(ctx.greenBrush);
    painter.drawEllipse(point, innerRadius, innerRadius);

    // 绘制坐标文本（应用标定转换）
    QString coordText = formatCoordinate(point);

    // 计算将文本框定位到点右上角所需的偏移量
    QFontMetrics fm(ctx.font);
    QRect textBoundingRect = fm.boundingRect(coordText);
    double bgHeight = textBoundingRect.height() + 2 * textpadding;

    QPointF offset(innerRadius, -innerRadius - bgHeight);

    // 调用带有offset参数的完美辅助函数
    drawTextWithBackground(painter, point, coordText, ctx.font, QColor(0, 255, 0), Qt::black,
                         textpadding, 1.0, offset);
}

void PaintingOverlay::drawLines(QPainter& painter, const DrawingContext& ctx) const
{
    // 绘制已完成的直线
    for (const auto& line : m_lines) {
        drawSingleLine(painter, line, false, ctx);
    }
}

void PaintingOverlay::drawSingleLine(QPainter& painter, const LineObject& line, bool isCurrentDrawing, const DrawingContext& ctx) const
{
    if (line.points.size() < 2) {
        return;
    }
    
    const QPointF& start = line.points[0];
    const QPointF& end = line.points[1];
    
    // Calculate direction vector for infinite line extension
    QPointF direction = end - start;
    qreal length = sqrt(direction.x() * direction.x() + direction.y() * direction.y());
    
    if (length < 1.0) {
        return; // 两点过于接近，无法确定方向
    }
    
    // 使用与平行线和两线一致的延伸方法
    QPointF extendedStart, extendedEnd;
    calculateExtendedLine(start, end, extendedStart, extendedEnd);
    
    // 使用图元自身的厚度作为屏幕像素宽度
    int desiredThickness = qMax(1, line.thickness);
    QPen linePen = createPen(line.color, desiredThickness, ctx.scale);
    linePen.setCapStyle(Qt::RoundCap);

    // 根据绘制状态决定线条样式
    if (line.isDashed || isCurrentDrawing) {
        // 虚线：原本就是虚线 或 当前正在绘制（预览状态）
        linePen.setStyle(Qt::DashLine);
        painter.setPen(linePen);
        painter.drawLine(extendedStart, extendedEnd);
    } else {
        // 实线：已完成的直线
        painter.setPen(linePen);
        painter.drawLine(extendedStart, extendedEnd);
    }
    
    // Draw user-clicked original endpoints (match Python style)
    // 绘制起始点标记（固定屏幕像素的小圆点）
    double pointRadius = 2.0 / m_scaleFactor;
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(line.color));
    painter.drawEllipse(start, pointRadius, pointRadius);
    
    // Calculate and display angle information
    double angle = calculateLineAngle(start, end);
    
    // 格式化角度文本（包含度数符号）
    QString angleText = QString::asprintf("%.1f°", angle);
    
    // 动态计算文本布局参数（与缩放无关，只与基础字号相关）
    double textOffset = qMax(8.0, ctx.fontSize * 0.4);
    double textPadding = qMax(4.0, ctx.fontSize * 0.5);  // 动态padding，字体大小的一半
    int bgBorderWidth = 1;
    
    // 计算角度文本框定位到起始点右上方所需的精确偏移量
    QFontMetrics fm(ctx.font);
    QRect textBoundingRect = fm.boundingRect(angleText);
    double bgHeight = textBoundingRect.height() + 2 * textPadding;
    QPointF angleTextOffset(textOffset, -textOffset - bgHeight);
    
    // 使用完整版drawTextWithBackground函数
    drawTextWithBackground(painter, start, angleText, ctx.font, line.color, Qt::black, textPadding, bgBorderWidth, angleTextOffset);
}

void PaintingOverlay::drawCircles(QPainter& painter, const DrawingContext& ctx) const
{
    // 绘制已完成的圆形
    for (const auto& circle : m_circles) {
        drawSingleCircle(painter, circle, ctx);
    }
}

void PaintingOverlay::drawSingleCircle(QPainter& painter, const CircleObject& circle, const DrawingContext& ctx) const
{
    // 使用预创建的字体和画笔
    painter.setFont(ctx.font);
    
    // 计算动态尺寸参数
    double fontSize = ctx.fontSize;
    
    // 统一计算所有动态尺寸参数
    double pointInnerRadius = qMax(3.0, 5.0 * ctx.scale);
    double pointOuterRadius = qMax(4.0, 8.0 * ctx.scale);
    int pointPenWidth = qMax(1, static_cast<int>(2.0 * ctx.scale));
    double textOffset = qMax(10.0, ctx.fontSize * 0.5);
    double textPadding = qMax(4.0, ctx.fontSize * 0.5);  // 动态padding，字体大小的一半
    int bgBorderWidth = 1;
    
    // 只在绘制过程中显示辅助点，圆形完成后不显示
    if (!circle.isCompleted) {
        // 绘制圆形的点
        for (int i = 0; i < circle.points.size(); ++i) {
            const QPointF& imagePos = circle.points[i];
            
            // 绘制点 - 实心圆 + 空心圆环 - 使用绿色
            painter.setPen(Qt::NoPen);
            painter.setBrush(ctx.greenBrush);
            painter.drawEllipse(imagePos, pointInnerRadius, pointInnerRadius);
            
            // 绘制空心圆环
            painter.setPen(ctx.greenPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(imagePos, pointOuterRadius, pointOuterRadius);
            
            // 显示序号
            QString pointText = QString::number(i + 1);
            QPointF textPos = imagePos + QPointF(textOffset, textOffset);
            
            // 绘制文本（使用绿色）
            painter.setPen(ctx.greenPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawText(textPos, pointText);
        }
    }
    
    // 绘制正在绘制的连接线 - 使用绿色
    if (!circle.isCompleted && circle.points.size() > 1) {
        painter.setPen(ctx.greenPen);
        for (int i = 0; i < circle.points.size() - 1; ++i) {
            const QPointF& start = circle.points[i];
            const QPointF& end = circle.points[i + 1];
            painter.drawLine(start, end);
        }
    }
    
    // 绘制圆形：已完成的圆形（直接使用center和radius）或有3个点的预览圆形
    QPointF centerImage;
    double radiusImage;
    bool shouldDrawCircle = false;

    if (circle.isCompleted && circle.radius > 0) {
        // 已完成的圆形，使用存储的圆心和半径
        centerImage = circle.center;
        radiusImage = circle.radius;
        shouldDrawCircle = true;
    } else if (!circle.isCompleted && circle.points.size() >= 3) {
        // 预览状态，实时计算圆心和半径
        if (calculateCircleFromThreePoints(circle.points, centerImage, radiusImage)) {
            shouldDrawCircle = true;
        }
    }

    if (shouldDrawCircle) {

        // 绘制圆形（已完成使用圆形颜色，预览时使用绿色虚线）
        if (circle.isCompleted) {
            QPen circlePen = createPen(circle.color, circle.thickness, ctx.scale);
            painter.setPen(circlePen);
        } else {
            painter.setPen(ctx.greenDashedPen); // 预览时使用虚线
        }
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(centerImage, radiusImage, radiusImage);
        
        // 圆心标记半径固定为2个屏幕像素
        const double centerMarkRadius = 2.0 / m_scaleFactor;

        // 绘制圆心 - 红色实心圆
        painter.setPen(Qt::NoPen);
        painter.setBrush(ctx.redBrush);
        painter.drawEllipse(centerImage, centerMarkRadius, centerMarkRadius);

        // 只在圆形完成后显示文字信息
        if (circle.isCompleted) {
            // 显示圆心坐标和半径 - 使用与点相同的布局方式
            QString centerText = formatCoordinate(circle.center);
            QString radiusText = formatRadius(circle.radius);

            // 计算将文本框定位到圆心右上角所需的偏移量（与点的布局一致）
            QFontMetrics fm(ctx.font);
            QRect centerTextBoundingRect = fm.boundingRect(centerText);
            double centerBgHeight = centerTextBoundingRect.height() + 2 * textPadding;

            // 第一个文本框偏移量（与点相同：右上角）
            QPointF centerOffset(centerMarkRadius, -centerMarkRadius - centerBgHeight);

            // 1. 绘制第一个文本框（坐标文本）- 使用圆形颜色
            drawTextWithBackground(painter, centerImage, centerText, ctx.font, circle.color, Qt::black, textPadding, bgBorderWidth, centerOffset);

            // 2. 计算第二个文本框的位置（紧挨着第一个文本框下方）
            QRect radiusTextBoundingRect = fm.boundingRect(radiusText);
            double radiusBgHeight = radiusTextBoundingRect.height() + 2 * textPadding;
            QPointF radiusOffset(centerMarkRadius, -centerMarkRadius - centerBgHeight + centerBgHeight);

            // 绘制第二个文本框（半径文本）- 使用圆形颜色
            drawTextWithBackground(painter, centerImage, radiusText, ctx.font, circle.color, Qt::black, textPadding, bgBorderWidth, radiusOffset);
        }
    }
}

void PaintingOverlay::drawFineCircles(QPainter& painter, const DrawingContext& ctx) const
{
    // 绘制已完成的精细圆
    for (const auto& fineCircle : m_fineCircles) {
        drawSingleFineCircle(painter, fineCircle, ctx);
    }
}

void PaintingOverlay::drawSingleFineCircle(QPainter& painter, const FineCircleObject& fineCircle, const DrawingContext& ctx) const
{
    // 使用预创建的字体和画笔
    painter.setFont(ctx.font);
    
    // 计算动态尺寸参数
    double pointInnerRadius = qMax(3.0, 5.0 * ctx.scale);
    double pointOuterRadius = qMax(5.0, 8.0 * ctx.scale);
    double textOffset = qMax(8.0, ctx.fontSize * 0.4);
    double textPadding = qMax(4.0, ctx.fontSize * 0.5);  // 动态padding，字体大小的一半
    int bgBorderWidth = 1;
    
    // 只在绘制过程中显示辅助点，精细圆完成后不显示
    if (!fineCircle.isCompleted) {
        // 绘制精细圆的点
        for (int i = 0; i < fineCircle.points.size(); ++i) {
            const QPointF& imagePos = fineCircle.points[i];
            
            // 绘制点 - 实心圆 + 空心圆环
            painter.setPen(Qt::NoPen);
            painter.setBrush(ctx.redBrush);
            painter.drawEllipse(imagePos, pointInnerRadius, pointInnerRadius);
            
            // 绘制空心圆环
            painter.setPen(ctx.redPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(imagePos, pointOuterRadius, pointOuterRadius);
            
            // 显示序号
            QString pointText = QString::number(i + 1);
            QPointF textPos = imagePos + QPointF(textOffset, textOffset);
            painter.setBrush(Qt::NoBrush);
            painter.drawText(textPos, pointText);
        }
    }
    
    // 只在正在绘制时显示点之间的连接线（灰色虚线）
    if (!fineCircle.isCompleted && fineCircle.points.size() >= 2) {
        painter.setPen(ctx.grayPen);
        for (int i = 0; i < fineCircle.points.size() - 1; ++i) {
            const QPointF& start = fineCircle.points[i];
            const QPointF& end = fineCircle.points[i + 1];
            painter.drawLine(start, end);
        }
    }
    
    // 如果精细圆已完成，绘制圆形和显示信息
    if (fineCircle.isCompleted && fineCircle.radius > 0) {
        const QPointF& centerImage = fineCircle.center;
        double radiusImage = fineCircle.radius;
        
        // 绘制精细圆（使用绿色）
        painter.setPen(ctx.greenPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(centerImage, radiusImage, radiusImage);

        // 绘制圆心标记（使用红色，与简单圆保持一致），半径固定为2个屏幕像素
        const double centerMarkRadius = 2.0 / m_scaleFactor;
        painter.setPen(Qt::NoPen);
        painter.setBrush(ctx.redBrush);
        painter.drawEllipse(centerImage, centerMarkRadius, centerMarkRadius);
        
        // 显示圆心坐标和半径信息
        QString centerText = formatCoordinate(fineCircle.center);
        QString radiusText = formatRadius(fineCircle.radius);
        
        // 1. 计算第一个文本框的矩形
        QRectF centerTextRect = calculateTextWithBackgroundRect(centerImage, centerText, ctx.font, textPadding, QPointF(textOffset, -textOffset));
        // 2. 绘制第一个文本框
        drawTextInRect(painter, centerTextRect, centerText, ctx.font, fineCircle.color, Qt::black, bgBorderWidth);
        
        // 3. 计算第二个文本框的位置（紧挨着第一个文本框下方）
        QPointF radiusAnchorPoint = QPointF(centerTextRect.left(), centerTextRect.bottom());
        QRectF radiusTextRect = calculateTextWithBackgroundRect(radiusAnchorPoint, radiusText, ctx.font, textPadding, QPointF(0, 0));
        // 4. 绘制第二个文本框
        drawTextInRect(painter, radiusTextRect, radiusText, ctx.font, fineCircle.color, Qt::black, bgBorderWidth);
    }
}

void PaintingOverlay::drawParallels(QPainter& painter, const DrawingContext& ctx) const
{
    // 绘制已完成的平行线
    for (const auto& parallel : m_parallels) {
        drawSingleParallel(painter, parallel, ctx);
    }
}

void PaintingOverlay::drawSingleParallel(QPainter& painter, const ParallelObject& parallel, const DrawingContext& ctx) const
{
    // 使用预创建的字体和画笔
    painter.setFont(ctx.font);
    
    // 计算动态尺寸参数
    double fontSize = ctx.fontSize;
    double pointInnerRadius = qMax(3.0, 5.0 * ctx.scale);
    double pointOuterRadius = qMax(5.0, 8.0 * ctx.scale);
    int pointPenWidth = qMax(1, static_cast<int>(2.0 * ctx.scale));
    double textOffset = qMax(10.0, ctx.fontSize * 0.5);
    double textPadding = qMax(4.0, ctx.fontSize * 0.5);  // 动态padding，字体大小的一半
    int bgBorderWidth = 1;
    double desiredThickness = parallel.thickness * 2.0 * ctx.scale;
    int thickLine = qMax(2, static_cast<int>(desiredThickness));
    
    // 只在绘制过程中显示辅助点，平行线完成后不显示
    if (!parallel.isCompleted) {
        // 绘制平行线的点
        for (int i = 0; i < parallel.points.size(); ++i) {
            const QPointF& imagePos = parallel.points[i];
            
            // 判断是否为预览点（第三个点且处于预览状态）
            bool isPreviewPoint = (i == 2 && parallel.isPreview);

            if (isPreviewPoint) {
                // 预览点使用虚线样式的圆环，不显示序号
                // 使用绿色画笔（虚线版本）
                painter.setPen(ctx.greenPen);
                painter.setBrush(Qt::NoBrush);
                painter.drawEllipse(imagePos, pointOuterRadius, pointOuterRadius);
            } else {
                // 正常点：绘制实心圆 + 空心圆环 + 序号
                painter.setPen(Qt::NoPen);
                // 使用绿色画刷
                painter.setBrush(ctx.greenBrush);
                painter.drawEllipse(imagePos, pointInnerRadius, pointInnerRadius);
                
                // 绘制空心圆环
                // 根据颜色选择合适的画笔
                if (parallel.color == Qt::red) {
                    painter.setPen(ctx.redPen);
                } else if (parallel.color == Qt::green) {
                    painter.setPen(ctx.greenPen);
                } else if (parallel.color == Qt::blue) {
                    painter.setPen(ctx.bluePen);
                } else {
                    painter.setPen(ctx.blackPen);
                }
                painter.setBrush(Qt::NoBrush);
                painter.drawEllipse(imagePos, pointOuterRadius, pointOuterRadius);
                
                // 绘制点的序号（不带背景）
                QString pointText = QString::number(i + 1);
                QPointF textPos = imagePos + QPointF(textOffset, textOffset);
                
                // 绘制文本（使用相同颜色的画笔）
                painter.setBrush(Qt::NoBrush);
                painter.drawText(textPos, pointText);
            }
        }
        
        // 当有2个点时，显示第三个点的预览
        if (parallel.points.size() == 2 && m_hasValidMousePos) {
            const QPointF& previewPos = m_currentMousePos;
            
            // 绘制预览点 - 使用虚线样式的圆环，不显示序号
            // 根据颜色选择合适的画笔（虚线版本）
            // 使用绿色画笔
            painter.setPen(ctx.greenPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(previewPos, pointOuterRadius, pointOuterRadius);
        }
    }
    
    // 绘制第一条线（延伸到图像边界）
    if (parallel.points.size() >= 1) {
        QPointF p1 = parallel.points[0];
        QPointF p2;
        bool hasSecondPoint = false;
        
        if (parallel.points.size() >= 2) {
            p2 = parallel.points[1];
            hasSecondPoint = true;
        } else if (m_hasValidMousePos) {
            // 使用鼠标位置作为第二个点进行预览
            p2 = m_currentMousePos;
            hasSecondPoint = true;
        }
        
        if (hasSecondPoint) {
            QPointF extStart1, extEnd1;
            calculateExtendedLine(p1, p2, extStart1, extEnd1);
            
            // 根据点数决定线条样式
            if (parallel.points.size() < 2) {
                // 预览状态：虚线
                // 使用绿色虚线画笔
                painter.setPen(ctx.greenDashedPen);
            } else {
                // 正常状态：实线
                // 使用绿色画笔
                painter.setPen(ctx.greenPen);
            }
            painter.drawLine(extStart1, extEnd1);
        }
    }
    
    // 当只有一个点时，绘制从第一个点到鼠标位置的预览线
    if (parallel.points.size() == 1 && m_hasValidMousePos) {
        QPointF start = parallel.points[0];
        QPointF end = m_currentMousePos;
        
        // 计算延伸线
        QPointF extStart, extEnd;
        calculateExtendedLine(start, end, extStart, extEnd);
        
        // 使用虚线绘制预览
        // 使用绿色虚线画笔
        painter.setPen(ctx.greenDashedPen);
        painter.drawLine(extStart, extEnd);
    }

    // 如果有第三个点，绘制第二条平行线
    if (parallel.points.size() >= 3) {
        // 计算平行线的方向向量
        QPointF direction = parallel.points[1] - parallel.points[0];
        QPointF parallelStart = parallel.points[2];
        QPointF parallelEnd = parallelStart + direction;

        // 延伸第二条线到图像边界
        QPointF extStart2, extEnd2;
        calculateExtendedLine(parallelStart, parallelEnd, extStart2, extEnd2);

        // 绘制第二条平行线（根据完成状态决定样式）
        if (parallel.isCompleted) {
            // 完成状态：实线
            painter.setPen(ctx.greenPen);
        } else {
            // 预览状态：虚线
            painter.setPen(ctx.greenDashedPen);
        }
        painter.drawLine(extStart2, extEnd2);
    }

    // 如果平行线已完成，绘制中线和距离角度信息
    if (parallel.isCompleted && parallel.points.size() >= 3) {
        // 绘制中线（红色虚线）
        QPointF extMidStart, extMidEnd;
        calculateExtendedLine(parallel.midStart, parallel.midEnd, extMidStart, extMidEnd);
        
        painter.setPen(ctx.redDashedPen); // 红色虚线
        painter.drawLine(extMidStart, extMidEnd);
        
        // 显示距离和角度信息
        QString distanceText = formatDistance(parallel.distance);
        QString angleText = QString::asprintf("%.1f°", parallel.angle);
        // 【标注位置修正】标注的锚点应该是中线的中点
        QPointF textAnchorPoint = (extMidStart + extMidEnd) / 2.0;
        
        // 1. 计算将距离文本框定位到中线中点右上方所需的精确偏移量
        QFontMetrics fm(ctx.font);
        QRect textBoundingRect = fm.boundingRect(distanceText);
        double bgHeight = textBoundingRect.height() + 2 * textPadding;
        QPointF distanceTextOffset(textOffset, -textOffset - bgHeight);
        
        // 2. 使用完整版drawTextWithBackground函数
        drawTextWithBackground(painter, textAnchorPoint, distanceText, ctx.font, parallel.color, Qt::black, textPadding, bgBorderWidth, distanceTextOffset);
        
        // 3. 计算角度文本框的位置（紧挨着距离文本框下方）
        QRect distanceTextRect = fm.boundingRect(distanceText);
        double distanceTextHeight = distanceTextRect.height() + 2 * textPadding;
        QPointF angleAnchorPoint = textAnchorPoint + QPointF(0, distanceTextHeight) + distanceTextOffset;
        
        // 4. 角度文本框相对于新锚点的偏移量为0，因为它要紧挨着
        drawTextWithBackground(painter, angleAnchorPoint, angleText, ctx.font, parallel.color, Qt::black, textPadding, bgBorderWidth, QPointF(0, 0));
    }
}

void PaintingOverlay::drawTwoLines(QPainter& painter, const DrawingContext& ctx) const
{
    // 绘制已完成的两线
    for (const auto& twoLine : m_twoLines) {
        drawSingleTwoLines(painter, twoLine, ctx);
    }
}

void PaintingOverlay::drawLineSegmentAngles(QPainter& painter, const DrawingContext& ctx) const
{
    // 绘制已完成的线段夹角
    for (int i = 0; i < m_lineSegmentAngles.size(); ++i) {
        const auto& angleObj = m_lineSegmentAngles[i];
        if (angleObj.isVisible) {
            drawSingleLineSegmentAngle(painter, angleObj, i, ctx);
        }
    }
}

// 两线
void PaintingOverlay::drawSingleTwoLines(QPainter& painter, const TwoLinesObject& twoLines, const DrawingContext& ctx) const
{
    // 使用预创建的字体和画笔
    painter.setFont(ctx.font);
    
    // 计算动态尺寸参数
    double fontSize = ctx.fontSize;
    double pointInnerRadius = qMax(3.0, 5.0 * ctx.scale);
    double pointOuterRadius = qMax(5.0, 8.0 * ctx.scale);
    double textOffset = qMax(10.0, 15.0 * ctx.scale);
    double textPadding = qMax(4.0, ctx.fontSize * 0.5);  // 动态padding，字体大小的一半
    int bgBorderWidth = 1;
    double desiredThickness = twoLines.thickness * 2.0 * ctx.scale;
    int thickLine = qMax(2, static_cast<int>(desiredThickness));
    double intersectionRadius = 2.0 / m_scaleFactor;
    int intersectionPenWidth = qMax(2, static_cast<int>(3.0 * ctx.scale));
    double bisectorLength = qMax(30.0, 50.0 * ctx.scale);
    int bisectorPenWidth = qMax(1, static_cast<int>(2.0 * ctx.scale));
    
    // 只在绘制过程中显示辅助点，两线完成后不显示
    if (!twoLines.isCompleted) {
        // 绘制两线的点
        for (int i = 0; i < twoLines.points.size(); ++i) {
            const QPointF& imagePos = twoLines.points[i];
            
            // 绘制点 - 实心圆 + 空心圆环 - 直接使用期望的屏幕像素值
            painter.setPen(Qt::NoPen);
            // 使用绿色画刷
            painter.setBrush(ctx.greenBrush);
            painter.drawEllipse(imagePos, pointInnerRadius, pointInnerRadius);
            
            // 绘制空心圆环
            // 使用绿色画笔
            painter.setPen(ctx.greenPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(imagePos, pointOuterRadius, pointOuterRadius);
            
            // 绘制点的序号（与drawSingleParallel保持一致，不带背景）
            QString pointText = QString::number(i + 1);
            QPointF textPos = imagePos + QPointF(textOffset, textOffset);
            
            // 绘制文本（使用相同颜色的画笔）
            painter.setBrush(Qt::NoBrush);
            painter.drawText(textPos, pointText);
        }
        
        // 移除第三个点的预览点显示
        // 用户不希望看到第三个点的预览点
    }
    
    // 绘制第一条线（延伸到图像边界）
    if (twoLines.points.size() >= 1) {
        QPointF p1 = twoLines.points[0];
        QPointF p2;
        bool hasSecondPoint = false;
        
        if (twoLines.points.size() >= 2) {
            p2 = twoLines.points[1];
            hasSecondPoint = true;
        } else if (m_hasValidMousePos) {
            // 使用鼠标位置作为第二个点进行预览
            p2 = m_currentMousePos;
            hasSecondPoint = true;
        }
        
        if (hasSecondPoint) {
            QPointF extStart1, extEnd1;
            calculateExtendedLine(p1, p2, extStart1, extEnd1);

            // 根据第一条线的完成状态决定样式
            if (twoLines.points.size() >= 2) {
                // 第一条线已确定：实线
                painter.setPen(ctx.greenPen);
            } else {
                // 第一条线预览状态：虚线
                painter.setPen(ctx.greenDashedPen);
            }
            painter.drawLine(extStart1, extEnd1);
        }
    }
    
    // 绘制第二条线：预览状态下3个点即可，完成状态需要4个点
    if (twoLines.points.size() >= 3) {
        QPointF start2 = twoLines.points[2];
        QPointF end2;
        bool hasSecondLineEnd = false;
        
        if (twoLines.points.size() >= 4) {
            // 完成状态：使用第4个点作为终点
            end2 = twoLines.points[3];
            hasSecondLineEnd = true;
        } else if (m_hasValidMousePos) {
            // 预览状态：使用鼠标位置作为第4个点
            end2 = m_currentMousePos;
            hasSecondLineEnd = true;
        }
        
        if (hasSecondLineEnd) {
            // 计算第二条线延伸到图像边界
            QPointF extStart2, extEnd2;
            calculateExtendedLine(start2, end2, extStart2, extEnd2);

            // 根据第二条线的完成状态决定样式
            if (twoLines.points.size() >= 4) {
                // 第二条线已确定：实线
                painter.setPen(ctx.greenPen);
            } else {
                // 第二条线预览状态：虚线
                painter.setPen(ctx.greenDashedPen);
            }
            painter.drawLine(extStart2, extEnd2);
        }
    }
    
    // 如果两线已完成，绘制交点、角平分线和角度信息
    if (twoLines.isCompleted && twoLines.points.size() >= 4) {
        const QPointF& intersection = twoLines.intersection;
        
        // 绘制交点 - 红色实心点，直接使用期望的屏幕像素值
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(Qt::red));
        painter.drawEllipse(intersection, intersectionRadius, intersectionRadius);
        
        // 计算角平分线方向
        QPointF dir1 = twoLines.points[1] - twoLines.points[0];
        QPointF dir2 = twoLines.points[3] - twoLines.points[2];
        
        // 归一化方向向量
        double len1 = sqrt(dir1.x() * dir1.x() + dir1.y() * dir1.y());
        double len2 = sqrt(dir2.x() * dir2.x() + dir2.y() * dir2.y());
        if (len1 > 0) dir1 /= len1;
        if (len2 > 0) dir2 /= len2;
        
        // 计算角平分线方向
        QPointF bisectorDir = dir1 + dir2;
        double bisectorLen = sqrt(bisectorDir.x() * bisectorDir.x() + bisectorDir.y() * bisectorDir.y());
        if (bisectorLen > 0) bisectorDir /= bisectorLen;
        
        // 绘制角平分线（红色虚线），延伸到图像边界
        QPointF bisectorStart = twoLines.intersection - bisectorDir * 5000.0;
        QPointF bisectorEnd = twoLines.intersection + bisectorDir * 5000.0;
        
        painter.setPen(ctx.redDashedPen); // 红色虚线
        painter.drawLine(bisectorStart, bisectorEnd);
        
        // 显示角度和坐标信息 - 使用drawTextWithBackground辅助函数
        QString angleText = QString::asprintf("%.1f°", twoLines.angle);
        QString coordText = formatCoordinate(twoLines.intersection);
        
        // 动态计算文本布局参数（与缩放无关，只与基础字号相关）
        double textOffset = qMax(8.0, ctx.fontSize * 0.4);
        double textPadding = qMax(4.0, ctx.fontSize * 0.5);  // 动态padding，字体大小的一半
        int bgBorderWidth = 1;
        
        // 检查交点是否在视图范围内，如果不在则使用两线中间区域作为文字锚点
        QSize widgetSize = size();
        QPointF textAnchorPoint = intersection;
        
        if (!widgetSize.isEmpty()) {
            bool intersectionInView = (intersection.x() >= 0 && intersection.x() <= widgetSize.width() && 
                                     intersection.y() >= 0 && intersection.y() <= widgetSize.height());
            
            if (!intersectionInView) {
                // 计算两条线在视图内的中点作为文字锚点
                QPointF line1Center = (twoLines.points[0] + twoLines.points[1]) / 2.0;
                QPointF line2Center = (twoLines.points[2] + twoLines.points[3]) / 2.0;
                textAnchorPoint = (line1Center + line2Center) / 2.0;
            }
        }
        
        // 使用新的优雅API进行相对布局
        // 1. 计算角度文本框的矩形（定位到交点右上方）
        QPointF angleTextOffset(textOffset, -textOffset);
        QRectF angleTextRect = calculateTextWithBackgroundRect(textAnchorPoint, angleText, ctx.font, textPadding, angleTextOffset);
        
        // 4. 绘制角度文本框
        drawTextInRect(painter, angleTextRect, angleText, ctx.font, twoLines.color, Qt::black, bgBorderWidth);
        
        // 3. 计算坐标文本框的矩形（紧挨着角度文本框下方）
        QPointF coordTextAnchor = QPointF(angleTextRect.left(), angleTextRect.bottom());
        QRectF coordTextRect = calculateTextWithBackgroundRect(coordTextAnchor, coordText, ctx.font, textPadding, QPointF(0, 0));
        
        // 4. 绘制坐标文本框
        drawTextInRect(painter, coordTextRect, coordText, ctx.font, twoLines.color, Qt::black, bgBorderWidth);
    }
}

void PaintingOverlay::drawSingleLineSegmentAngle(QPainter& painter, const LineSegmentAngleObject& angleObj, int index, const DrawingContext& ctx) const
{
    if (!angleObj.isCompleted || angleObj.points.size() < 4) {
        return;
    }

    // 使用预创建的字体和画笔
    painter.setFont(ctx.font);

    // 计算动态尺寸参数
    double intersectionRadius = 2.0 / m_scaleFactor;  // 交点圆点半径（固定为2个屏幕像素）
    double textOffset = qMax(8.0, ctx.fontSize * 0.4);
    double textPadding = qMax(4.0, ctx.fontSize * 0.5);
    int bgBorderWidth = 1;

    // 绘制两条线段（用于显示角度关系）
    QPointF line1Start = angleObj.points[0];
    QPointF line1End = angleObj.points[1];
    QPointF line2Start = angleObj.points[2];
    QPointF line2End = angleObj.points[3];

    // 设置线段样式（厚度按图元自身的 thickness 解释为屏幕像素）
    QPen linePen = createPen(angleObj.color, angleObj.thickness, ctx.scale);
    painter.setPen(linePen);
    painter.drawLine(line1Start, line1End);
    painter.drawLine(line2Start, line2End);

    // 如果有交点，绘制交点（红色实心圆点）
    if (angleObj.hasIntersection) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(Qt::red));
        painter.drawEllipse(angleObj.intersection, intersectionRadius, intersectionRadius);

        // 绘制角度文本框
        QString angleText = angleObj.label;  // 已经包含度数符号

        // 计算文本位置（在交点右上方）
        QPointF textAnchorPoint = angleObj.intersection;
        QPointF angleTextOffset(textOffset, -textOffset);
        QRectF angleTextRect = calculateTextWithBackgroundRect(textAnchorPoint, angleText, ctx.font, textPadding, angleTextOffset);

        // 绘制角度文本框
        drawTextInRect(painter, angleTextRect, angleText, ctx.font, angleObj.color, Qt::black, bgBorderWidth);
    } else {
        // 如果没有交点（平行线），在两线段中间显示角度
        QPointF line1Center = (line1Start + line1End) / 2.0;
        QPointF line2Center = (line2Start + line2End) / 2.0;
        QPointF textAnchorPoint = (line1Center + line2Center) / 2.0;

        QString angleText = angleObj.label;
        QPointF angleTextOffset(0, -textOffset);
        QRectF angleTextRect = calculateTextWithBackgroundRect(textAnchorPoint, angleText, ctx.font, textPadding, angleTextOffset);

        // 绘制角度文本框
        drawTextInRect(painter, angleTextRect, angleText, ctx.font, angleObj.color, Qt::black, bgBorderWidth);
    }
}

void PaintingOverlay::drawCurrentPreview(QPainter& painter, const DrawingContext& ctx) const
{
    // 绘制当前正在绘制的图形预览
    switch (m_currentDrawingTool) {
        case DrawingTool::Line:
            if (m_hasCurrentLine && !m_currentLine.points.isEmpty()) {
                drawSingleLine(painter, m_currentLine, true, ctx);
            }
            break;

        case DrawingTool::Circle:
            if (m_hasCurrentCircle && !m_currentCircle.points.isEmpty()) {
                drawSingleCircle(painter, m_currentCircle, ctx);
            }
            break;

        case DrawingTool::FineCircle:
            if (m_hasCurrentFineCircle && !m_currentFineCircle.points.isEmpty()) {
                drawSingleFineCircle(painter, m_currentFineCircle, ctx);
            }
            break;

        case DrawingTool::Parallel:
            if (m_hasCurrentParallel && !m_currentParallel.points.isEmpty()) {
                drawSingleParallel(painter, m_currentParallel, ctx);
            }
            break;

        case DrawingTool::TwoLines:
            if (m_hasCurrentTwoLines && !m_currentTwoLines.points.isEmpty()) {
                drawSingleTwoLines(painter, m_currentTwoLines, ctx);
            }
            break;

        case DrawingTool::LineSegment:
            if (!m_currentPoints.isEmpty()) {
                // 绘制线段预览
                if (m_currentPoints.size() == 1) {
                    // 绘制起点
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(QBrush(Qt::red));
                    double pointRadius = 2.0 / m_scaleFactor;
                    painter.drawEllipse(m_currentPoints[0], pointRadius, pointRadius);

                    // 如果有有效的鼠标位置，绘制预览线
                    if (m_hasValidMousePos) {
                        QPen previewPen = createPen(Qt::red, 2, ctx.scale, false);
                        painter.setPen(previewPen);
                        painter.drawLine(m_currentPoints[0], m_currentMousePos);

                        // 绘制鼠标位置的端点
                        painter.setPen(Qt::NoPen);
                        painter.setBrush(QBrush(Qt::red));
                        painter.drawEllipse(m_currentMousePos, pointRadius, pointRadius);
                    }
                }
            }
            break;

        case DrawingTool::ROI_LineDetect:
        case DrawingTool::ROI_CircleDetect:
            if (m_hasCurrentROIDetection && !m_currentROIDetection.points.isEmpty()) {
                drawSingleROI(painter, m_currentROIDetection, ctx);
            }
            break;

        case DrawingTool::ROI_CREATION:
            if (m_hasCurrentROI) {
                drawSingleROICreation(painter, m_currentROI, ctx);
            }
            break;

        default:
            break;
    }
}

void PaintingOverlay::drawSelectionHighlights(QPainter& painter) const
{
    if (!m_selectionEnabled) {
        return;
    }

    // 获取绘图上下文
    const DrawingContext& ctx = m_cachedDrawingContext;

    // 创建高亮画笔（蓝色，适中粗细）- 使用正确的缩放补偿
    QPen highlightPen(Qt::blue);
    highlightPen.setWidthF(qMax(1.0, 4.0 / ctx.scale)); // 确保在屏幕上看起来是4像素宽
    highlightPen.setCapStyle(Qt::RoundCap);
    highlightPen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(highlightPen);
    painter.setBrush(Qt::NoBrush);

    // 高亮选中的点
    for (int index : m_selectedPoints) {
        if (index >= 0 && index < m_points.size()) {
            const PointObject& point = m_points[index];
            if (!point.isVisible) continue;

            // 绘制高亮圆环
            double radius = 25.0 * (std::max)(1.0, (std::min)(ctx.scale, 4.0));
            painter.drawEllipse(point.position, radius, radius);
        }
    }

    // 高亮选中的线 - 使用更粗的画笔绘制外描边
    QPen thickHighlightPen(Qt::blue);
    thickHighlightPen.setWidthF(qMax(1.0, 8.0 / ctx.scale)); // 更粗的外描边
    thickHighlightPen.setCapStyle(Qt::RoundCap);
    thickHighlightPen.setJoinStyle(Qt::RoundJoin);

    for (int index : m_selectedLines) {
        if (index >= 0 && index < m_lines.size()) {
            const LineObject& line = m_lines[index];
            if (!line.isVisible || line.points.size() < 2) continue;

            // 绘制延伸的高亮线（更粗的外描边）
            painter.setPen(thickHighlightPen);
            QPointF direction = line.points[1] - line.points[0];
            QPointF extendedStart = line.points[0] - direction * 5000.0;
            QPointF extendedEnd = line.points[1] + direction * 5000.0;
            painter.drawLine(extendedStart, extendedEnd);
        }
    }

    // 恢复原来的画笔
    painter.setPen(highlightPen);

    // 高亮选中的线段 - 使用更粗的画笔绘制外描边
    for (int index : m_selectedLineSegments) {
        if (index >= 0 && index < m_lineSegments.size()) {
            const LineSegmentObject& lineSegment = m_lineSegments[index];
            if (!lineSegment.isVisible || lineSegment.points.size() < 2) continue;

            // 绘制高亮线段（更粗的外描边）
            painter.setPen(thickHighlightPen);
            painter.drawLine(lineSegment.points[0], lineSegment.points[1]);
        }
    }

    // 恢复原来的画笔
    painter.setPen(highlightPen);

    // 高亮选中的圆
    for (int index : m_selectedCircles) {
        if (index >= 0 && index < m_circles.size()) {
            const CircleObject& circle = m_circles[index];
            if (!circle.isVisible) continue;

            // 兼容两种圆的表示方式：
            // 1) 手动画圆：通过三个点计算圆心和半径
            // 2) 自动检测圆：直接使用 center + radius
            QPointF center;
            double radius = 0.0;
            bool hasCircle = false;

            if (circle.isCompleted && circle.radius > 0) {
                center = circle.center;
                radius = circle.radius;
                hasCircle = true;
            } else if (circle.points.size() >= 3 &&
                       calculateCircleFromThreePoints(circle.points, center, radius)) {
                hasCircle = true;
            }

            if (hasCircle) {
                // 绘制稍大的圆环作为外描边
                double highlightRadius = radius + qMax(2.0, 3.0 / ctx.scale);
                painter.drawEllipse(center, highlightRadius, highlightRadius);
            }
        }
    }

    // 高亮选中的精细圆
    for (int index : m_selectedFineCircles) {
        if (index >= 0 && index < m_fineCircles.size()) {
            const FineCircleObject& fineCircle = m_fineCircles[index];
            if (!fineCircle.isVisible || !fineCircle.isCompleted) continue;

            // 绘制稍大的圆环作为外描边
            double highlightRadius = fineCircle.radius + qMax(2.0, 3.0 / ctx.scale);
            painter.drawEllipse(fineCircle.center, highlightRadius, highlightRadius);
        }
    }

    // 高亮选中的平行线 - 使用更粗的画笔绘制外描边
    for (int index : m_selectedParallels) {
        if (index >= 0 && index < m_parallels.size()) {
            const ParallelObject& parallel = m_parallels[index];
            if (!parallel.isVisible || !parallel.isCompleted) continue;

            painter.setPen(thickHighlightPen);

            // 高亮第一条线
            if (parallel.points.size() >= 2) {
                QPointF extStart1, extEnd1;
                calculateExtendedLine(parallel.points[0], parallel.points[1], extStart1, extEnd1);
                painter.drawLine(extStart1, extEnd1);
            }

            // 高亮第二条线
            if (parallel.points.size() >= 3) {
                QPointF direction = parallel.points[1] - parallel.points[0];
                QPointF parallelStart = parallel.points[2];
                QPointF parallelEnd = parallelStart + direction;
                QPointF extStart2, extEnd2;
                calculateExtendedLine(parallelStart, parallelEnd, extStart2, extEnd2);
                painter.drawLine(extStart2, extEnd2);
            }
        }
    }

    // 恢复原来的画笔
    painter.setPen(highlightPen);

    // 高亮选中的两线 - 使用更粗的画笔绘制外描边
    for (int index : m_selectedTwoLines) {
        if (index >= 0 && index < m_twoLines.size()) {
            const TwoLinesObject& twoLines = m_twoLines[index];
            if (!twoLines.isVisible || !twoLines.isCompleted) continue;

            painter.setPen(thickHighlightPen);

            // 高亮第一条线
            if (twoLines.points.size() >= 2) {
                QPointF extStart1, extEnd1;
                calculateExtendedLine(twoLines.points[0], twoLines.points[1], extStart1, extEnd1);
                painter.drawLine(extStart1, extEnd1);
            }

            // 高亮第二条线
            if (twoLines.points.size() >= 4) {
                QPointF extStart2, extEnd2;
                calculateExtendedLine(twoLines.points[2], twoLines.points[3], extStart2, extEnd2);
                painter.drawLine(extStart2, extEnd2);
            }
        }
    }

    // 恢复原来的画笔
    painter.setPen(highlightPen);

    // 高亮选中的平行线中线 - 使用更粗的画笔绘制外描边
    for (int index : m_selectedParallelMiddleLines) {
        if (index >= 0 && index < m_parallels.size()) {
            const ParallelObject& parallel = m_parallels[index];
            if (!parallel.isVisible || !parallel.isCompleted) continue;

            // 绘制高亮中线（延伸到图像边界，更粗的外描边）
            painter.setPen(thickHighlightPen);
            QPointF extMidStart, extMidEnd;
            calculateExtendedLine(parallel.midStart, parallel.midEnd, extMidStart, extMidEnd);
            painter.drawLine(extMidStart, extMidEnd);
        }
    }

    // 高亮选中的角平分线 - 使用更粗的画笔绘制外描边
    for (int index : m_selectedBisectorLines) {
        if (index >= 0 && index < m_twoLines.size()) {
            const TwoLinesObject& twoLines = m_twoLines[index];
            if (!twoLines.isVisible || !twoLines.isCompleted) continue;

            // 计算角平分线方向（与绘制时相同的逻辑）
            QPointF dir1 = twoLines.points[1] - twoLines.points[0];
            QPointF dir2 = twoLines.points[3] - twoLines.points[2];

            // 归一化方向向量
            double len1 = sqrt(dir1.x() * dir1.x() + dir1.y() * dir1.y());
            double len2 = sqrt(dir2.x() * dir2.x() + dir2.y() * dir2.y());
            if (len1 > 0) dir1 /= len1;
            if (len2 > 0) dir2 /= len2;

            // 计算角平分线方向
            QPointF bisectorDir = dir1 + dir2;
            double bisectorLen = sqrt(bisectorDir.x() * bisectorDir.x() + bisectorDir.y() * bisectorDir.y());
            if (bisectorLen > 0) {
                bisectorDir /= bisectorLen;

                // 绘制高亮角平分线（延伸到图像边界，更粗的外描边）
                painter.setPen(thickHighlightPen);
                QPointF bisectorStart = twoLines.intersection - bisectorDir * 5000.0;
                QPointF bisectorEnd = twoLines.intersection + bisectorDir * 5000.0;
                painter.drawLine(bisectorStart, bisectorEnd);
            }
        }
    }

    // 高亮选中的线段夹角 - 使用更粗的画笔绘制外描边
    for (int index : m_selectedLineSegmentAngles) {
        if (index >= 0 && index < m_lineSegmentAngles.size()) {
            const LineSegmentAngleObject& angleObj = m_lineSegmentAngles[index];
            if (!angleObj.isVisible || !angleObj.isCompleted || angleObj.points.size() < 4) continue;

            // 绘制高亮的两条线段
            painter.setPen(thickHighlightPen);
            painter.drawLine(angleObj.points[0], angleObj.points[1]);  // 第一条线段
            painter.drawLine(angleObj.points[2], angleObj.points[3]);  // 第二条线段

            // 如果有交点，高亮交点
            if (angleObj.hasIntersection) {
                painter.setPen(Qt::NoPen);
                painter.setBrush(QBrush(Qt::yellow));  // 黄色高亮交点
                double highlightRadius = 2.0 / m_scaleFactor;
                painter.drawEllipse(angleObj.intersection, highlightRadius, highlightRadius);
            }
        }
    }
}

QPointF PaintingOverlay::widgetToImage(const QPointF& widgetPos) const
{
    // 逆向坐标变换
    QPointF imagePos = (widgetPos - m_imageOffset) / m_scaleFactor;
    return imagePos;
}

QPointF PaintingOverlay::imageToWidget(const QPointF& imagePos) const
{
    // 正向坐标变换
    QPointF widgetPos = imagePos * m_scaleFactor + m_imageOffset;
    return widgetPos;
}

bool PaintingOverlay::isPointInImageBounds(const QPointF& imagePos) const
{
    // 检查坐标是否在图像范围内
    if (m_imageSize.isEmpty()) {
        return false;
    }

    return (imagePos.x() >= 0 && imagePos.x() < m_imageSize.width() &&
            imagePos.y() >= 0 && imagePos.y() < m_imageSize.height());
}

QRectF PaintingOverlay::constrainROIToBounds(const QRectF& roi) const
{
    // 如果没有图像尺寸信息，返回原始ROI
    if (m_imageSize.isEmpty()) {
        return roi;
    }

    // 获取图像边界
    const double imageWidth = m_imageSize.width();
    const double imageHeight = m_imageSize.height();
    
    QRectF constrainedROI = roi;
    
    // 确保ROI不超出图像左上角
    if (constrainedROI.x() < 0) {
        constrainedROI.moveLeft(0);
    }
    if (constrainedROI.y() < 0) {
        constrainedROI.moveTop(0);
    }
    
    // 确保ROI不超出图像右下角
    if (constrainedROI.right() > imageWidth) {
        constrainedROI.moveRight(imageWidth);
    }
    if (constrainedROI.bottom() > imageHeight) {
        constrainedROI.moveBottom(imageHeight);
    }
    
    // 如果ROI太大，缩小到图像尺寸
    if (constrainedROI.width() > imageWidth) {
        constrainedROI.setWidth(imageWidth);
    }
    if (constrainedROI.height() > imageHeight) {
        constrainedROI.setHeight(imageHeight);
    }
    
    return constrainedROI;
}

bool PaintingOverlay::isROIWithinBounds(const QRectF& roi) const
{
    // 如果没有图像尺寸信息，认为无效
    if (m_imageSize.isEmpty()) {
        return false;
    }
    
    // 检查ROI是否完全在图像范围内
    return (roi.x() >= 0 && roi.y() >= 0 &&
            roi.right() <= m_imageSize.width() &&
            roi.bottom() <= m_imageSize.height() &&
            roi.width() > 0 && roi.height() > 0);
}

void PaintingOverlay::drawObjectsByHistory(QPainter& painter, const DrawingContext& ctx) const
{
    // 按历史记录顺序绘制所有图形，确保后创建的图形在上层
    for (const DrawingAction& action : m_drawingHistory) {
        // 只绘制添加操作，跳过删除操作
        if (action.type >= DrawingAction::AddPoint && action.type <= DrawingAction::AddROI) {
            drawSingleObjectByAction(painter, action, ctx);
        }
    }
}

void PaintingOverlay::drawSingleObjectByAction(QPainter& painter, const DrawingAction& action, const DrawingContext& ctx) const
{
    // 根据动作类型和索引绘制对应的单个图形对象
    switch (action.type) {
        case DrawingAction::AddPoint:
            if (action.index >= 0 && action.index < m_points.size()) {
                drawSinglePoint(painter, m_points[action.index].position, action.index, ctx);
            }
            break;
        case DrawingAction::AddLine:
            if (action.index >= 0 && action.index < m_lines.size()) {
                drawSingleLine(painter, m_lines[action.index], false, ctx);
            }
            break;
        case DrawingAction::AddLineSegment:
            if (action.index >= 0 && action.index < m_lineSegments.size()) {
                drawSingleLineSegment(painter, m_lineSegments[action.index], ctx);
            }
            break;
        case DrawingAction::AddCircle:
            if (action.index >= 0 && action.index < m_circles.size()) {
                drawSingleCircle(painter, m_circles[action.index], ctx);
            }
            break;
        case DrawingAction::AddFineCircle:
            if (action.index >= 0 && action.index < m_fineCircles.size()) {
                drawSingleFineCircle(painter, m_fineCircles[action.index], ctx);
            }
            break;
        case DrawingAction::AddParallel:
            if (action.index >= 0 && action.index < m_parallels.size()) {
                drawSingleParallel(painter, m_parallels[action.index], ctx);
            }
            break;
        case DrawingAction::AddTwoLines:
            if (action.index >= 0 && action.index < m_twoLines.size()) {
                drawSingleTwoLines(painter, m_twoLines[action.index], ctx);
            }
            break;
        case DrawingAction::AddLineSegmentAngle:
            if (action.index >= 0 && action.index < m_lineSegmentAngles.size()) {
                drawSingleLineSegmentAngle(painter, m_lineSegmentAngles[action.index], action.index, ctx);
            }
            break;
        case DrawingAction::AddROI:
            if (action.index >= 0 && action.index < m_rois.size()) {
                drawSingleROI(painter, m_rois[action.index], ctx);
            }
            break;
        default:
            // 跳过删除操作和未知操作
            break;
    }
}

QPen PaintingOverlay::createPen(const QColor& color, int width, double scale, bool dashed) const
{
    Q_UNUSED(scale);

    QPen pen(color);
    // 使用 cosmetic 模式，使线宽始终以屏幕像素为单位，不随缩放变化
    pen.setWidth(qMax(1, width));
    pen.setCosmetic(true);
    if (dashed) {
        pen.setStyle(Qt::DashLine);
    }
    return pen;
}

QFont PaintingOverlay::createFont(int targetScreenSize, double scale) const
{
    QFont font;
    // 基于图像分辨率的目标字号，同时对当前缩放做 1/scale 补偿，
    // 这样在视图放大/缩小时，屏幕上的文字高度基本保持不变。
    double effectiveScale = (scale > 0.0) ? scale : 1.0;
    double pointSize = qMax(6.0, static_cast<double>(targetScreenSize) / effectiveScale);
    font.setPointSizeF(pointSize);
    font.setBold(true);
    return font;
}

double PaintingOverlay::calculateFontSize() const
{
    // 按当前显示区域（控件高度）来适配字体大小，而不是按原始图像分辨率。
    int widgetHeight = height();
    if (widgetHeight <= 0) {
        return 8.0; // 合理的默认字号（更小）
    }

    // 以可见区域高度的约 1.9% 作为基础字号
    // （在上一版基础上放大约 1.5 倍）：
    // 400 像素高的视图下，大约是 7~8 像素。
    double baseFontSize = (widgetHeight / 80.0) * 1.5;

    // 限制字体大小范围，避免过小或过大
    double fontSize = qBound(6.0, baseFontSize, 20.0);
    return fontSize;
}

void PaintingOverlay::drawTextWithBackground(QPainter& painter,
                                            const QPointF& anchorPoint,
                                            const QString& text,
                                            const QFont& font,
                                            const QColor& textColor,
                                            const QColor& bgColor,
                                            double padding,
                                            double borderWidth,
                                            const QPointF& offset) const
{
    QFontMetrics fm(font);
    QRect textBoundingRect = fm.boundingRect(text);

    // 创建一个以(0,0)为左上角的、包含内边距的总内容框
    QRectF contentRectWithPadding = QRectF(0, 0, textBoundingRect.width(), textBoundingRect.height())
                                     .adjusted(-padding, -padding, padding, padding);

    // 将内容框移动到最终位置（仍在当前世界坐标系下）
    contentRectWithPadding.moveTopLeft(anchorPoint + offset);

    // 绘制背景和边框
    painter.setPen(createPen(textColor, borderWidth, m_scaleFactor)); // 使用文本颜色作为边框颜色
    painter.setBrush(QBrush(bgColor));
    painter.drawRect(contentRectWithPadding);

    // 在内容框内居中绘制文本
    painter.setPen(createPen(textColor, 1, m_scaleFactor));
    painter.setFont(font);
    painter.drawText(contentRectWithPadding, Qt::AlignCenter, text);
}

// 简化版本已删除，统一使用完整版本的drawTextWithBackground函数

QRectF PaintingOverlay::calculateTextWithBackgroundRect(const QPointF& anchorPoint, const QString& text, 
                                                       const QFont& font, double padding, const QPointF& offset) const
{
    QFontMetrics fm(font);
    QRect textBoundingRect = fm.boundingRect(text);
    QRectF contentRectWithPadding = QRectF(0, 0, textBoundingRect.width(), textBoundingRect.height()).adjusted(-padding, -padding, padding, padding);
    contentRectWithPadding.moveTopLeft(anchorPoint + offset);
    return contentRectWithPadding;
}

void PaintingOverlay::drawTextInRect(QPainter& painter, const QRectF& rect, const QString& text,
                                    const QFont& font, const QColor& textColor, const QColor& bgColor,
                                    double borderWidth) const
{
    // 绘制背景和边框
    painter.setPen(createPen(textColor, borderWidth, m_scaleFactor)); // 使用文本颜色作为边框颜色
    painter.setBrush(QBrush(bgColor));
    painter.drawRect(rect);

    // 在矩形内居中绘制文本
    painter.setPen(createPen(textColor, 1, m_scaleFactor));
    painter.setFont(font);
    painter.drawText(rect, Qt::AlignCenter, text);
}

// 已删除QPointF版本的drawTextInRect重载，统一使用calculateTextWithBackgroundRect + drawTextInRect(QRectF)组合

void PaintingOverlay::undoAction(const DrawingAction& action)
{
    switch (action.type) {
        case DrawingAction::AddPoint:
            if (!m_points.isEmpty()) {
                m_points.removeLast(); // 移除最后添加的点
            }
            break;
        case DrawingAction::AddLine:
            if (!m_lines.isEmpty()) {
                m_lines.removeLast(); // 移除最后添加的线
            }
            break;
        case DrawingAction::AddLineSegment:
            if (!m_lineSegments.isEmpty()) {
                m_lineSegments.removeLast(); // 移除最后添加的线段
            }
            break;
        case DrawingAction::AddCircle:
            if (!m_circles.isEmpty()) {
                m_circles.removeLast(); // 移除最后添加的圆
            }
            break;
        case DrawingAction::AddFineCircle:
            if (!m_fineCircles.isEmpty()) {
                m_fineCircles.removeLast(); // 移除最后添加的精细圆
            }
            break;
        case DrawingAction::AddParallel:
            if (!m_parallels.isEmpty()) {
                m_parallels.removeLast(); // 移除最后添加的平行线
            }
            break;
        case DrawingAction::AddTwoLines:
            if (!m_twoLines.isEmpty()) {
                m_twoLines.removeLast(); // 移除最后添加的两线
            }
            break;
        case DrawingAction::AddLineSegmentAngle:
            if (!m_lineSegmentAngles.isEmpty()) {
                m_lineSegmentAngles.removeLast(); // 移除最后添加的线段夹角
            }
            break;
        case DrawingAction::AddROI:
            if (!m_rois.isEmpty()) {
                m_rois.removeLast(); // 移除最后添加的ROI
            }
            break;
        default:
            break;
    }

    // 清除选择状态
    clearSelection();

    // 发送同步信号
    emit drawingDataChanged(m_viewName);

    update();
}

void PaintingOverlay::commitDrawingAction(const DrawingAction& action)
{
    m_drawingHistory.push(action);

    // 发射信号通知外部绘图完成
    QString result;
    QString viewName = m_viewName.isEmpty() ? "Unknown" : m_viewName;
    
    switch (action.type) {
        case DrawingAction::AddPoint:
            result = QString("添加点: %1").arg(m_points[action.index].label);
            break;
        case DrawingAction::AddLine:
            result = QString("添加线段: %1").arg(m_lines[action.index].label);
            // 线段测量完成，发射测量完成信号
            emit measurementCompleted(viewName, result);
            break;
        case DrawingAction::AddCircle:
            result = QString("添加圆: %1").arg(m_circles[action.index].label);
            // 圆测量完成，发射测量完成信号
            emit measurementCompleted(viewName, result);
            break;
        case DrawingAction::AddFineCircle:
            if (m_hasCurrentFineCircle && m_currentFineCircle.isCompleted) {
                m_fineCircles.append(m_currentFineCircle);
                clearCurrentFineCircleData();
                emit measurementCompleted(viewName, action.data.toString());
            }
            break;
        case DrawingAction::AddParallel:
            result = QString("添加平行线: %1").arg(m_parallels[action.index].label);
            // 平行线测量完成，发射测量完成信号
            emit measurementCompleted(viewName, result);
            break;
        case DrawingAction::AddTwoLines:
            result = QString("添加双线: %1").arg(m_twoLines[action.index].label);
            // 双线测量完成，发射测量完成信号
            emit measurementCompleted(viewName, result);
            break;
        case DrawingAction::AddLineSegment:
            if (m_hasCurrentLineSegment && m_currentLineSegment.isCompleted) {
                m_lineSegments.append(m_currentLineSegment);
                clearCurrentLineSegmentData();
                emit measurementCompleted(viewName, action.data.toString());
            }
            break;
        default:
            result = "绘图操作完成";
            break;
    }
    
    emit drawingCompleted(m_viewName);
}

// 几何计算方法实现
bool PaintingOverlay::calculateCircleFromThreePoints(const QVector<QPointF>& points, QPointF& center, double& radius) const
{
    if (points.size() < 3) {
        return false;
    }
    
    double x1 = points[0].x(), y1 = points[0].y();
    double x2 = points[1].x(), y2 = points[1].y();
    double x3 = points[2].x(), y3 = points[2].y();
    
    double a = x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2);
    
    if (qAbs(a) < 1e-10) {
        return false; // 三点共线
    }
    
    double bx = -(y1 * (x2 * x2 - x3 * x3 + y2 * y2 - y3 * y3) + y2 * (x3 * x3 - x1 * x1 + y3 * y3 - y1 * y1) + y3 * (x1 * x1 - x2 * x2 + y1 * y1 - y2 * y2));
    double by = x1 * (x2 * x2 - x3 * x3 + y2 * y2 - y3 * y3) + x2 * (x3 * x3 - x1 * x1 + y3 * y3 - y1 * y1) + x3 * (x1 * x1 - x2 * x2 + y1 * y1 - y2 * y2);
    
    double cx = bx / (2 * a);
    double cy = by / (2 * a);
    
    center = QPointF(cx, cy);
    radius = qSqrt((cx - x1) * (cx - x1) + (cy - y1) * (cy - y1));
    
    return true;
}

bool PaintingOverlay::calculateCircleFromFivePoints(const QVector<QPointF>& points, QPointF& center, double& radius) const
{
    if (points.size() != 5) {
        return false;
    }
    
    // 使用最小二乘法拟合圆
    double sumX = 0, sumY = 0, sumX2 = 0, sumY2 = 0, sumXY = 0;
    double sumX3 = 0, sumY3 = 0, sumX2Y = 0, sumXY2 = 0;
    
    for (const QPointF& p : points) {
        double x = p.x(), y = p.y();
        sumX += x;
        sumY += y;
        sumX2 += x * x;
        sumY2 += y * y;
        sumXY += x * y;
        sumX3 += x * x * x;
        sumY3 += y * y * y;
        sumX2Y += x * x * y;
        sumXY2 += x * y * y;
    }
    
    int n = points.size();
    
    // 构建矩阵方程
    double m11 = sumX2, m12 = sumXY, m13 = sumX;
    double m21 = sumXY, m22 = sumY2, m23 = sumY;
    double m31 = sumX, m32 = sumY, m33 = n;
    
    double v1 = -(sumX3 + sumXY2);
    double v2 = -(sumX2Y + sumY3);
    double v3 = -(sumX2 + sumY2);
    
    // 使用克拉默法则求解
    double det = m11 * (m22 * m33 - m23 * m32) - m12 * (m21 * m33 - m23 * m31) + m13 * (m21 * m32 - m22 * m31);
    
    if (qAbs(det) < 1e-10) {
        return false;
    }
    
    double A = (v1 * (m22 * m33 - m23 * m32) - m12 * (v2 * m33 - m23 * v3) + m13 * (v2 * m32 - m22 * v3)) / det;
    double B = (m11 * (v2 * m33 - m23 * v3) - v1 * (m21 * m33 - m23 * m31) + m13 * (m21 * v3 - v2 * m31)) / det;
    
    center = QPointF(-A / 2, -B / 2);
    
    // 计算半径
    QPointF p0 = points[0];
    radius = qSqrt((center.x() - p0.x()) * (center.x() - p0.x()) + (center.y() - p0.y()) * (center.y() - p0.y()));
    
    return true;
}

bool PaintingOverlay::calculateLineIntersection(const QPointF& p1, const QPointF& p2, const QPointF& p3, const QPointF& p4, QPointF& intersection) const
{
    double x1 = p1.x(), y1 = p1.y();
    double x2 = p2.x(), y2 = p2.y();
    double x3 = p3.x(), y3 = p3.y();
    double x4 = p4.x(), y4 = p4.y();
    
    double denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    
    if (qAbs(denom) < 1e-10) {
        return false; // 两线平行
    }
    
    double t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denom;
    
    intersection = QPointF(x1 + t * (x2 - x1), y1 + t * (y2 - y1));
    
    return true;
}

double PaintingOverlay::calculateDistancePointToLine(const QPointF& point, const QPointF& lineStart, const QPointF& lineEnd) const
{
    double A = lineEnd.y() - lineStart.y();
    double B = lineStart.x() - lineEnd.x();
    double C = lineEnd.x() * lineStart.y() - lineStart.x() * lineEnd.y();
    
    double distance = qAbs(A * point.x() + B * point.y() + C) / qSqrt(A * A + B * B);
    
    return distance;
}

void PaintingOverlay::calculateExtendedLine(const QPointF& start, const QPointF& end, QPointF& extStart, QPointF& extEnd) const
{
    // 获取图像尺寸 - 使用实际图像尺寸作为边界
    if (m_imageSize.isEmpty()) {
        extStart = start;
        extEnd = end;
        return;
    }
    
    double imageWidth = m_imageSize.width();
    double imageHeight = m_imageSize.height();
    
    // 计算直线方向向量
    double dx = end.x() - start.x();
    double dy = end.y() - start.y();
    
    // 避免除零错误
    if (qAbs(dx) < 1e-6 && qAbs(dy) < 1e-6) {
        extStart = start;
        extEnd = end;
        return;
    }
    
    // 计算延伸参数
    double t1 = -1000.0; // 向后延伸
    double t2 = 1000.0;  // 向前延伸
    
    // 与图像边界的交点计算
    if (qAbs(dx) > 1e-6) {
        // 与左边界 (x=0) 的交点
        double t_left = (0 - start.x()) / dx;
        // 与右边界 (x=imageWidth) 的交点
        double t_right = (imageWidth - start.x()) / dx;
        
        t1 = qMax(t1, qMin(t_left, t_right));
        t2 = qMin(t2, qMax(t_left, t_right));
    }
    
    if (qAbs(dy) > 1e-6) {
        // 与上边界 (y=0) 的交点
        double t_top = (0 - start.y()) / dy;
        // 与下边界 (y=imageHeight) 的交点
        double t_bottom = (imageHeight - start.y()) / dy;
        
        t1 = qMax(t1, qMin(t_top, t_bottom));
        t2 = qMin(t2, qMax(t_top, t_bottom));
    }
    
    // 计算延伸后的端点
    extStart = QPointF(start.x() + t1 * dx, start.y() + t1 * dy);
    extEnd = QPointF(start.x() + t2 * dx, start.y() + t2 * dy);
    
    // 确保端点在图像范围内
    extStart.setX(qBound(0.0, extStart.x(), imageWidth));
    extStart.setY(qBound(0.0, extStart.y(), imageHeight));
    extEnd.setX(qBound(0.0, extEnd.x(), imageWidth));
    extEnd.setY(qBound(0.0, extEnd.y(), imageHeight));
}

double PaintingOverlay::calculateLineAngle(const QPointF& start, const QPointF& end) const
{
    QPointF diff = end - start;
    double angle = qAtan2(diff.y(), diff.x()) * 180.0 / M_PI;
    if (angle < 0) {
        angle += 180.0;
    }
    return angle;
}

// 选择功能实现
void PaintingOverlay::enableSelection(bool enabled)
{
    m_selectionEnabled = enabled;
    if (enabled) {
        stopDrawing();
        setCursor(Qt::ArrowCursor);
    }
    update();
}

bool PaintingOverlay::isSelectionEnabled() const
{
    return m_selectionEnabled;
}

void PaintingOverlay::clearSelection()
{
    m_selectedPoints.clear();
    m_selectedLines.clear();
    m_selectedLineSegments.clear();
    m_selectedCircles.clear();
    m_selectedFineCircles.clear();
    m_selectedParallels.clear();
    m_selectedTwoLines.clear();
    m_selectedLineSegmentAngles.clear();
    m_selectedParallelMiddleLines.clear();
    m_selectedBisectorLines.clear(); // 清除角平分线选择
    m_selectedROIs.clear(); // 清除ROI选择
    update();
}

QString PaintingOverlay::getSelectedObjectInfo() const
{
    QStringList info;
    
    if (!m_selectedPoints.isEmpty()) {
        info << QString("选中点: %1个").arg(m_selectedPoints.size());
    }
    if (!m_selectedLines.isEmpty()) {
        info << QString("选中线段: %1个").arg(m_selectedLines.size());
    }
    if (!m_selectedCircles.isEmpty()) {
        info << QString("选中圆: %1个").arg(m_selectedCircles.size());
    }
    if (!m_selectedFineCircles.isEmpty()) {
        info << QString("选中精细圆: %1个").arg(m_selectedFineCircles.size());
    }
    if (!m_selectedParallels.isEmpty()) {
        info << QString("选中平行线: %1个").arg(m_selectedParallels.size());
    }
    if (!m_selectedTwoLines.isEmpty()) {
        info << QString("选中双线: %1个").arg(m_selectedTwoLines.size());
    }
    if (!m_selectedLineSegmentAngles.isEmpty()) {
        info << QString("选中线段夹角: %1个").arg(m_selectedLineSegmentAngles.size());
    }
    if (!m_selectedROIs.isEmpty()) {
        info << QString("选中ROI: %1个").arg(m_selectedROIs.size());
    }

    return info.join(", ");
}

void PaintingOverlay::deleteSelectedObjects()
{
    // 删除选中的点（从后往前删除以避免索引问题）
    QList<int> pointIndices = m_selectedPoints.values();
    std::sort(pointIndices.rbegin(), pointIndices.rend());
    for (int index : pointIndices) {
        if (index >= 0 && index < m_points.size()) {
            m_points.removeAt(index);
        }
    }

    // 删除选中的线
    QList<int> lineIndices = m_selectedLines.values();
    std::sort(lineIndices.rbegin(), lineIndices.rend());
    for (int index : lineIndices) {
        if (index >= 0 && index < m_lines.size()) {
            m_lines.removeAt(index);
        }
    }

    // 删除选中的线段
    QList<int> lineSegmentIndices = m_selectedLineSegments.values();
    std::sort(lineSegmentIndices.rbegin(), lineSegmentIndices.rend());
    for (int index : lineSegmentIndices) {
        if (index >= 0 && index < m_lineSegments.size()) {
            m_lineSegments.removeAt(index);
        }
    }

    // 删除选中的圆
    QList<int> circleIndices = m_selectedCircles.values();
    std::sort(circleIndices.rbegin(), circleIndices.rend());
    for (int index : circleIndices) {
        if (index >= 0 && index < m_circles.size()) {
            m_circles.removeAt(index);
        }
    }

    // 删除选中的精细圆
    QList<int> fineCircleIndices = m_selectedFineCircles.values();
    std::sort(fineCircleIndices.rbegin(), fineCircleIndices.rend());
    for (int index : fineCircleIndices) {
        if (index >= 0 && index < m_fineCircles.size()) {
            m_fineCircles.removeAt(index);
        }
    }

    // 删除选中的平行线
    QList<int> parallelIndices = m_selectedParallels.values();
    std::sort(parallelIndices.rbegin(), parallelIndices.rend());
    for (int index : parallelIndices) {
        if (index >= 0 && index < m_parallels.size()) {
            m_parallels.removeAt(index);
        }
    }

    // 删除选中的两线
    QList<int> twoLinesIndices = m_selectedTwoLines.values();
    std::sort(twoLinesIndices.rbegin(), twoLinesIndices.rend());
    for (int index : twoLinesIndices) {
        if (index >= 0 && index < m_twoLines.size()) {
            m_twoLines.removeAt(index);
        }
    }

    // 删除选中的线段夹角
    QList<int> lineSegmentAnglesIndices = m_selectedLineSegmentAngles.values();
    std::sort(lineSegmentAnglesIndices.rbegin(), lineSegmentAnglesIndices.rend());
    for (int index : lineSegmentAnglesIndices) {
        if (index >= 0 && index < m_lineSegmentAngles.size()) {
            m_lineSegmentAngles.removeAt(index);
        }
    }

    // 删除选中的ROI
    QList<int> roiIndices = m_selectedROIs.values();
    std::sort(roiIndices.rbegin(), roiIndices.rend());
    for (int index : roiIndices) {
        if (index >= 0 && index < m_rois.size()) {
            m_rois.removeAt(index);
        }
    }

    // 注意：平行线中线不能单独删除，它们是平行线的一部分
    // 如果选中了中线，我们清除中线选择但不删除平行线本身
    m_selectedParallelMiddleLines.clear();

    // 清除选择状态
    clearSelection();

    // 发出数据变化信号
    emit drawingDataChanged(m_viewName);

    update();
}

void PaintingOverlay::createLineFromSelectedPoints()
{
    if (m_selectedPoints.size() != 2) {
        return;
    }

    QList<int> pointIndices = m_selectedPoints.values();
    if (pointIndices.size() != 2) {
        return;
    }

    int index1 = pointIndices[0];
    int index2 = pointIndices[1];

    if (index1 < 0 || index1 >= m_points.size() ||
        index2 < 0 || index2 >= m_points.size()) {
        return;
    }

    // 创建新的线段
    LineSegmentObject lineSegment;
    lineSegment.points.append(m_points[index1].position);
    lineSegment.points.append(m_points[index2].position);
    lineSegment.isCompleted = true;

    // 计算长度和角度
    QPointF p1 = lineSegment.points[0];
    QPointF p2 = lineSegment.points[1];
    lineSegment.length = sqrt(pow(p2.x() - p1.x(), 2) + pow(p2.y() - p1.y(), 2));
    lineSegment.showLength = true;

    // 计算角度（相对于水平轴的角度）
    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();
    double angleRadians = atan2(dy, dx);
    double angleDegrees = angleRadians * 180.0 / M_PI;

    // 确保角度在0-360度范围内
    if (angleDegrees < 0) {
        angleDegrees += 360.0;
    }

    // 更新标签包含长度和角度信息
    lineSegment.label = QString("长度: %1, 角度: %2°").arg(formatDistance(lineSegment.length)).arg(angleDegrees, 0, 'f', 1);

    m_lineSegments.append(lineSegment);

    // 记录历史
    DrawingAction action;
    action.type = DrawingAction::AddLineSegment;
    action.source = DrawingAction::ManualDrawing;  // 手动绘制
    action.index = m_lineSegments.size() - 1;
    commitDrawingAction(action);

    // 清除选择状态
    clearSelection();

    // 发出信号
    QString result = QString("线段: 长度 %1, 角度 %2°").arg(formatDistance(lineSegment.length)).arg(angleDegrees, 0, 'f', 1);
    emit measurementCompleted(m_viewName, result);
    emit drawingDataChanged(m_viewName);

    update();
}

// 线段绘制相关方法实现
void PaintingOverlay::handleLineSegmentDrawingClick(const QPointF& pos)
{
    m_currentPoints.append(pos);
    
    if (m_currentPoints.size() == 2) {
        // 创建新的线段对象
        LineSegmentObject newLineSegment;
        newLineSegment.points = m_currentPoints;
        newLineSegment.color = Qt::red;
        newLineSegment.thickness = 2.0;
        newLineSegment.isDashed = false;
        newLineSegment.isCompleted = true;
        newLineSegment.showLength = true;

        // 计算线段长度
        QPointF start = m_currentPoints[0];
        QPointF end = m_currentPoints[1];
        double pixelLength = sqrt(pow(end.x() - start.x(), 2) + pow(end.y() - start.y(), 2));
        newLineSegment.length = pixelLength;

        // 根据标定状态设置标签
        newLineSegment.label = QString("长度: %1").arg(formatDistance(pixelLength));
        
        // 添加到线段列表
        m_lineSegments.append(newLineSegment);
        
        // 提交绘图动作
        DrawingAction action;
        action.type = DrawingAction::AddLineSegment;
        action.source = DrawingAction::ManualDrawing;
        action.index = m_lineSegments.size() - 1;
        commitDrawingAction(action);
        
        // 清除当前点
        m_currentPoints.clear();

        // 如果处于标定模式，启动标定流程
        if (m_isCalibrationMode) {
            performCalibrationWithLineSegment(m_lineSegments.size() - 1);
            m_isCalibrationMode = false;
            // 单点标定完成后停止绘制
            stopDrawing();
        } else if (m_isMultiPointCalibrationMode) {
            performMultiPointCalibrationWithLineSegment(m_lineSegments.size() - 1);
            // 多点标定模式下不自动停止绘制，由用户在对话框中选择
        } else {
            // 普通线段绘制完成后停止绘制
            stopDrawing();
        }
    }
    
    update();
}

void PaintingOverlay::drawLineSegments(QPainter& painter, const DrawingContext& ctx) const
{
    // 绘制所有线段
    for (const auto& lineSegment : m_lineSegments) {
        drawSingleLineSegment(painter, lineSegment, ctx);
    }
    
    // 绘制当前正在绘制的线段（实时预览）
    if (m_hasCurrentLineSegment && !m_currentLineSegment.points.isEmpty()) {
        drawSingleLineSegment(painter, m_currentLineSegment, ctx);
    }
}

void PaintingOverlay::drawSingleLineSegment(QPainter& painter, const LineSegmentObject& lineSegment, const DrawingContext& ctx) const
{
    if (!lineSegment.isCompleted) {
        return;
    }
    
    // 绘制线段
    if (lineSegment.points.size() < 2) return;
    const QPointF& start = lineSegment.points[0];
    const QPointF& end = lineSegment.points[1];
    
    // 创建线段画笔（厚度按图元自身的 thickness 解释为屏幕像素）
    int desiredThickness = qMax(1, static_cast<int>(lineSegment.thickness));
    QPen linePen = createPen(lineSegment.color, desiredThickness, ctx.scale);
    linePen.setCapStyle(Qt::RoundCap);
    
    if (lineSegment.isDashed) {
        linePen.setStyle(Qt::DashLine);
    }
    
    // 绘制线段（直接连接两点，不延伸）
    painter.setPen(linePen);
    painter.drawLine(start, end);
    
    // 绘制端点标记（固定屏幕像素的小圆点）
    double pointRadius = 2.0 / m_scaleFactor;
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(lineSegment.color));
    painter.drawEllipse(start, pointRadius, pointRadius);
    painter.drawEllipse(end, pointRadius, pointRadius);
    
    // 检测是否为角平分线（特殊处理）
    if (lineSegment.label.startsWith("BISECTOR:")) {
        // 解析角平分线的标签格式: "BISECTOR:角度°:(x,y)"
        QString labelContent = lineSegment.label.mid(9); // 移除"BISECTOR:"前缀
        QStringList parts = labelContent.split(":");
        if (parts.size() >= 2) {
            QString angleText = parts[0]; // 角度部分，例如"45.5°"
            QString coordText = parts[1]; // 坐标部分，例如"(100.2,200.3)"
            
            // 计算线段中点作为文本位置
            QPointF midPoint = (start + end) / 2.0;
            
            // 动态计算文本布局参数（与缩放无关）
            double textOffset = qMax(8.0, ctx.fontSize * 0.4);
            double textPadding = qMax(4.0, ctx.fontSize * 0.5);
            int bgBorderWidth = 1;
            
            // 使用与TwoLinesObject相同的布局方式
            // 1. 计算角度文本框的矩形（定位到中点右上方）
            QPointF angleTextOffset(textOffset, -textOffset);
            QRectF angleTextRect = calculateTextWithBackgroundRect(midPoint, angleText, ctx.font, textPadding, angleTextOffset);
            
            // 绘制角度文本框
            drawTextInRect(painter, angleTextRect, angleText, ctx.font, lineSegment.color, Qt::black, bgBorderWidth);
            
            // 2. 计算坐标文本框的矩形（紧挨着角度文本框下方）
            QPointF coordTextAnchor = QPointF(angleTextRect.left(), angleTextRect.bottom());
            QRectF coordTextRect = calculateTextWithBackgroundRect(coordTextAnchor, coordText, ctx.font, textPadding, QPointF(0, 0));
            
            // 绘制坐标文本框
            drawTextInRect(painter, coordTextRect, coordText, ctx.font, lineSegment.color, Qt::black, bgBorderWidth);
        }
        return; // 角平分线处理完成，直接返回
    }
    
    // 准备分层显示的文本信息
    QString lengthText;
    QString angleText;

    if (!lineSegment.label.isEmpty()) {
        // 解析标签中的长度和角度信息
        // 标签格式: "长度: xxx, 角度: xxx°"
        QStringList parts = lineSegment.label.split(", ");
        if (parts.size() >= 2) {
            // 提取长度部分
            QString lengthPart = parts[0];
            if (lengthPart.startsWith("长度: ")) {
                lengthText = lengthPart;
            }

            // 提取角度部分
            QString anglePart = parts[1];
            if (anglePart.startsWith("角度: ")) {
                angleText = anglePart;
            }
        }
    }

    // 如果解析失败，检查是否需要默认格式
    if (lengthText.isEmpty() && angleText.isEmpty()) {
        // 对于虚线（测量线段），只显示标签内容，不自动添加角度
        if (lineSegment.isDashed) {
            lengthText = lineSegment.label; // 直接使用标签内容
            angleText = ""; // 虚线不显示角度
        } else {
            // 对于普通线段，使用默认的长度和角度格式
            double length = sqrt(pow(end.x() - start.x(), 2) + pow(end.y() - start.y(), 2));
            double angle = atan2(end.y() - start.y(), end.x() - start.x()) * 180.0 / M_PI;
            if (angle < 0) angle += 360.0;

            lengthText = QString("长度: %1").arg(formatDistance(length));
            angleText = QString("角度: %1°").arg(angle, 0, 'f', 1);
        }
    } else if (lengthText.isEmpty()) {
        // 只缺少长度信息，使用标签内容
        lengthText = lineSegment.label;
    }

    // 计算线段中点作为文本位置
    QPointF midPoint = (start + end) / 2.0;

    // 动态计算文本布局参数
    double textOffset = qMax(8.0, ctx.fontSize * 0.4);
    double textPadding = qMax(4.0, ctx.fontSize * 0.5);  // 动态padding，字体大小的一半
    int bgBorderWidth = 1;

    // 计算第一个文本框（长度）定位到中点右上方所需的精确偏移量
    QFontMetrics fm(ctx.font);
    QRect lengthTextBoundingRect = fm.boundingRect(lengthText);
    double lengthBgHeight = lengthTextBoundingRect.height() + 2 * textPadding;
    QPointF lengthTextOffset(textOffset, -textOffset - lengthBgHeight);

    // 1. 绘制第一个文本框（长度文本）
    drawTextWithBackground(painter, midPoint, lengthText, ctx.font, lineSegment.color, Qt::black, textPadding, bgBorderWidth, lengthTextOffset);

    // 2. 只有当角度文本不为空时才绘制第二个文本框
    if (!angleText.isEmpty()) {
        // 计算第二个文本框的位置（紧挨着第一个文本框下方）
        QRect angleTextBoundingRect = fm.boundingRect(angleText);
        double angleBgHeight = angleTextBoundingRect.height() + 2 * textPadding;
        QPointF angleTextOffset(textOffset, -textOffset - lengthBgHeight + lengthBgHeight);

        // 绘制第二个文本框（角度文本）
        drawTextWithBackground(painter, midPoint, angleText, ctx.font, lineSegment.color, Qt::black, textPadding, bgBorderWidth, angleTextOffset);
    }
}

// 数据设置方法实现
void PaintingOverlay::setLineSegmentsData(const QVector<LineSegmentObject>& lineSegments)
{
    m_lineSegments = lineSegments;
    update();
}

void PaintingOverlay::setCurrentLineSegmentData(const LineSegmentObject& currentLineSegment)
{
    m_currentLineSegment = currentLineSegment;
    m_hasCurrentLineSegment = true;
    update();
}

void PaintingOverlay::clearCurrentLineSegmentData()
{
    m_currentLineSegment = LineSegmentObject();
    m_hasCurrentLineSegment = false;
}

void PaintingOverlay::clearCurrentFineCircleData()
{
    m_currentFineCircle = FineCircleObject();
    m_hasCurrentFineCircle = false;
}

void PaintingOverlay::clearCurrentParallelData()
{
    m_currentParallel = ParallelObject();
    m_hasCurrentParallel = false;
    m_hasValidMousePos = false;
}

void PaintingOverlay::clearCurrentTwoLinesData()
{
    m_currentTwoLines = TwoLinesObject();
    m_hasCurrentTwoLines = false;
    m_hasValidMousePos = false;
}

void PaintingOverlay::clearCurrentCircleData()
{
    m_currentCircle = CircleObject();
    m_hasCurrentCircle = false;
}

// 绘图上下文管理方法实现
void PaintingOverlay::updateDrawingContext() const
{
    // 计算当前绘制参数
    double scale = m_scaleFactor;
    double fontSize = calculateFontSize();
    QFont font = createFont(static_cast<int>(fontSize), scale);
    
    // 更新DrawingContext
    m_cachedDrawingContext.scale = scale;
    m_cachedDrawingContext.fontSize = fontSize;
    m_cachedDrawingContext.font = font;
    
    // 创建所有画笔
    m_cachedDrawingContext.greenPen = createPen(Qt::green, 2, scale);
    m_cachedDrawingContext.blackPen = createPen(Qt::black, 2, scale);
    m_cachedDrawingContext.whitePen = createPen(Qt::white, 2, scale);
    m_cachedDrawingContext.redPen = createPen(Qt::red, 2, scale);
    m_cachedDrawingContext.bluePen = createPen(Qt::blue, 2, scale);
    m_cachedDrawingContext.yellowPen = createPen(Qt::yellow, 2, scale);
    m_cachedDrawingContext.cyanPen = createPen(Qt::cyan, 2, scale);
    m_cachedDrawingContext.magentaPen = createPen(Qt::magenta, 2, scale);
    m_cachedDrawingContext.grayPen = createPen(Qt::gray, 1, scale);
    m_cachedDrawingContext.redDashedPen = createPen(Qt::red, 2, scale, true); // 红色虚线画笔
    m_cachedDrawingContext.greenDashedPen = createPen(Qt::green, 2, scale, true); // 绿色虚线画笔
    m_cachedDrawingContext.blueDashedPen = createPen(Qt::blue, 2, scale, true); // 蓝色虚线画笔
    m_cachedDrawingContext.blackDashedPen = createPen(Qt::black, 2, scale, true); // 黑色虚线画笔
    m_cachedDrawingContext.cyanDashedPen = createPen(Qt::cyan, 2, scale, true); // 青色虚线画笔
    m_cachedDrawingContext.magentaDashedPen = createPen(Qt::magenta, 2, scale, true); // 紫色虚线画笔
    m_cachedDrawingContext.yellowDashedPen = createPen(Qt::yellow, 2, scale, true); // 黄色虚线画笔
    
    // 创建所有画刷
    m_cachedDrawingContext.greenBrush = QBrush(Qt::green);
    m_cachedDrawingContext.blackBrush = QBrush(Qt::black);
    m_cachedDrawingContext.whiteBrush = QBrush(Qt::white);
    m_cachedDrawingContext.redBrush = QBrush(Qt::red);
    m_cachedDrawingContext.blueBrush = QBrush(Qt::blue);
    m_cachedDrawingContext.cyanBrush = QBrush(Qt::cyan);
    m_cachedDrawingContext.magentaBrush = QBrush(Qt::magenta);
    m_cachedDrawingContext.yellowBrush = QBrush(Qt::yellow);
    
    // 更新缓存状态
    m_drawingContextValid = true;
    m_lastContextWidgetSize = size();
    // 注意：这里没有 m_videoFrame，所以暂时不设置 m_lastContextImageSize
}

bool PaintingOverlay::needsDrawingContextUpdate() const
{
    // 检查是否需要更新DrawingContext
    if (!m_drawingContextValid) {
        return true;
    }
    
    // 检查控件尺寸是否变化
    if (m_lastContextWidgetSize != size()) {
        return true;
    }

    // 缩放因子变化时也需要更新，以便字体大小按 1/scale 补偿
    if (!qFuzzyCompare(m_cachedDrawingContext.scale, m_scaleFactor)) {
        return true;
    }
    
    return false;
}

void PaintingOverlay::handleSelectionClick(const QPointF& pos, bool ctrlPressed)
{
    // 实现选择逻辑
    bool foundSelection = false;



    // 检查点击是否命中任何对象
    int pointIndex = hitTestPoint(pos, 30.0); // 增加点的命中容差

    if (pointIndex >= 0) {

        if (ctrlPressed) {
            if (m_selectedPoints.contains(pointIndex)) {
                m_selectedPoints.remove(pointIndex);
            } else {
                m_selectedPoints.insert(pointIndex);
            }
        } else {
            clearSelection();
            m_selectedPoints.insert(pointIndex);
        }
        foundSelection = true;
    }

    int lineIndex = hitTestLine(pos, 20.0); // 增加线的命中容差
    if (lineIndex >= 0 && !foundSelection) {
        if (ctrlPressed) {
            if (m_selectedLines.contains(lineIndex)) {
                m_selectedLines.remove(lineIndex);
            } else {
                m_selectedLines.insert(lineIndex);
            }
        } else {
            clearSelection();
            m_selectedLines.insert(lineIndex);
        }
        foundSelection = true;
    }
    
    int circleIndex = hitTestCircle(pos, 20.0); // 增加圆的命中容差
    if (circleIndex >= 0 && !foundSelection) {
        if (ctrlPressed) {
            if (m_selectedCircles.contains(circleIndex)) {
                m_selectedCircles.remove(circleIndex);
            } else {
                m_selectedCircles.insert(circleIndex);
            }
        } else {
            clearSelection();
            m_selectedCircles.insert(circleIndex);
        }
        foundSelection = true;
    }

    // 测试线段
    int lineSegmentIndex = hitTestLineSegment(pos, 20.0); // 增加线段的命中容差
    if (lineSegmentIndex >= 0 && !foundSelection) {
        if (ctrlPressed) {
            if (m_selectedLineSegments.contains(lineSegmentIndex)) {
                m_selectedLineSegments.remove(lineSegmentIndex);
            } else {
                m_selectedLineSegments.insert(lineSegmentIndex);
            }
        } else {
            clearSelection();
            m_selectedLineSegments.insert(lineSegmentIndex);
        }
        foundSelection = true;
    }

    // 测试精细圆
    int fineCircleIndex = hitTestFineCircle(pos, 20.0); // 增加精细圆的命中容差
    if (fineCircleIndex >= 0 && !foundSelection) {
        if (ctrlPressed) {
            if (m_selectedFineCircles.contains(fineCircleIndex)) {
                m_selectedFineCircles.remove(fineCircleIndex);
            } else {
                m_selectedFineCircles.insert(fineCircleIndex);
            }
        } else {
            clearSelection();
            m_selectedFineCircles.insert(fineCircleIndex);
        }
        foundSelection = true;
    }

    // 测试平行线
    int parallelIndex = hitTestParallel(pos, 20.0); // 增加平行线的命中容差
    if (parallelIndex != -1 && !foundSelection) {
        if (parallelIndex < -999) {
            // 这是中线命中，提取实际索引
            int actualIndex = -(parallelIndex + 1000);
            if (ctrlPressed) {
                if (m_selectedParallelMiddleLines.contains(actualIndex)) {
                    m_selectedParallelMiddleLines.remove(actualIndex);
                } else {
                    m_selectedParallelMiddleLines.insert(actualIndex);
                }
            } else {
                clearSelection();
                m_selectedParallelMiddleLines.insert(actualIndex);
            }
        } else {
            // 这是普通平行线命中
            if (ctrlPressed) {
                if (m_selectedParallels.contains(parallelIndex)) {
                    m_selectedParallels.remove(parallelIndex);
                } else {
                    m_selectedParallels.insert(parallelIndex);
                }
            } else {
                clearSelection();
                m_selectedParallels.insert(parallelIndex);
            }
        }
        foundSelection = true;
    }

    // 测试两线
    int twoLinesIndex = hitTestTwoLines(pos, 20.0); // 增加两线的命中容差
    if (twoLinesIndex != -1 && !foundSelection) {
        if (twoLinesIndex < -1999) {
            // 这是角平分线命中，提取实际索引
            int actualIndex = -(twoLinesIndex + 2000);
            if (ctrlPressed) {
                if (m_selectedBisectorLines.contains(actualIndex)) {
                    m_selectedBisectorLines.remove(actualIndex);
                } else {
                    m_selectedBisectorLines.insert(actualIndex);
                }
            } else {
                clearSelection();
                m_selectedBisectorLines.insert(actualIndex);
            }
        } else {
            // 这是普通两线命中
            if (ctrlPressed) {
                if (m_selectedTwoLines.contains(twoLinesIndex)) {
                    m_selectedTwoLines.remove(twoLinesIndex);
                } else {
                    m_selectedTwoLines.insert(twoLinesIndex);
                }
            } else {
                clearSelection();
                m_selectedTwoLines.insert(twoLinesIndex);
            }
        }
        foundSelection = true;
    }

    // 测试线段夹角
    int lineSegmentAngleIndex = hitTestLineSegmentAngle(pos, 10.0);
    if (lineSegmentAngleIndex >= 0 && !foundSelection) {
        if (ctrlPressed) {
            if (m_selectedLineSegmentAngles.contains(lineSegmentAngleIndex)) {
                m_selectedLineSegmentAngles.remove(lineSegmentAngleIndex);
            } else {
                m_selectedLineSegmentAngles.insert(lineSegmentAngleIndex);
            }
        } else {
            clearSelection();
            m_selectedLineSegmentAngles.insert(lineSegmentAngleIndex);
        }
        foundSelection = true;
    }

    // 测试ROI
    int roiIndex = hitTestROI(pos, 20.0); // 增加ROI的命中容差
    if (roiIndex >= 0 && !foundSelection) {
        if (ctrlPressed) {
            if (m_selectedROIs.contains(roiIndex)) {
                m_selectedROIs.remove(roiIndex);
            } else {
                m_selectedROIs.insert(roiIndex);
            }
        } else {
            clearSelection();
            m_selectedROIs.insert(roiIndex);
        }
        foundSelection = true;
    }

    // 如果没有命中任何对象且不是Ctrl点击，清除选择
    if (!foundSelection && !ctrlPressed) {
        clearSelection();
    }
    
    // 触发选择变化事件
    onSelectionChanged();
    update();
}

int PaintingOverlay::hitTestPoint(const QPointF& testPos, double tolerance) const
{
    for (int i = 0; i < m_points.size(); ++i) {
        const PointObject& point = m_points[i];
        if (!point.isVisible) continue;

        double distance = sqrt(pow(testPos.x() - point.position.x(), 2) + pow(testPos.y() - point.position.y(), 2));
        if (distance <= tolerance) {
            return i;
        }
    }
    return -1;
}

int PaintingOverlay::hitTestLine(const QPointF& testPos, double tolerance) const
{
    for (int i = 0; i < m_lines.size(); ++i) {
        const LineObject& line = m_lines[i];
        if (!line.isVisible || line.points.size() < 2) continue;
        
        double distance = calculateDistancePointToLine(testPos, line.points[0], line.points[1]);
        if (distance <= tolerance) {
            return i;
        }
    }
    return -1;
}

int PaintingOverlay::hitTestCircle(const QPointF& testPos, double tolerance) const
{
    for (int i = 0; i < m_circles.size(); ++i) {
        const CircleObject& circle = m_circles[i];
        if (!circle.isVisible) continue;

        // 同时支持：
        // 1) 手动画圆（通过三点计算）
        // 2) 自动检测圆（直接使用 center + radius）
        QPointF center;
        double radius = 0.0;
        bool hasCircle = false;

        if (circle.isCompleted && circle.radius > 0) {
            center = circle.center;
            radius = circle.radius;
            hasCircle = true;
        } else if (circle.points.size() >= 3 &&
                   calculateCircleFromThreePoints(circle.points, center, radius)) {
            hasCircle = true;
        }

        if (!hasCircle) {
            continue;
        }

        double distance = sqrt(pow(testPos.x() - center.x(), 2) + pow(testPos.y() - center.y(), 2));
        // 只有在圆周附近才能选中
        if (abs(distance - radius) <= tolerance) {
            return i;
        }
    }
    return -1;
}

int PaintingOverlay::hitTestLineSegment(const QPointF& testPos, double tolerance) const
{
    for (int i = 0; i < m_lineSegments.size(); ++i) {
        const LineSegmentObject& lineSegment = m_lineSegments[i];
        if (!lineSegment.isVisible || lineSegment.points.size() < 2) continue;

        // 使用新的方法检查点是否在线段范围内（不包括延长线）
        if (isPointOnLineSegment(testPos, lineSegment.points[0], lineSegment.points[1], tolerance)) {
            return i;
        }
    }
    return -1;
}

int PaintingOverlay::hitTestFineCircle(const QPointF& testPos, double tolerance) const
{
    for (int i = 0; i < m_fineCircles.size(); ++i) {
        const FineCircleObject& fineCircle = m_fineCircles[i];
        if (!fineCircle.isVisible || !fineCircle.isCompleted) continue;

        double distance = sqrt(pow(testPos.x() - fineCircle.center.x(), 2) + pow(testPos.y() - fineCircle.center.y(), 2));
        // 只有在圆周附近才能选中
        if (abs(distance - fineCircle.radius) <= tolerance) {
            return i;
        }
    }
    return -1;
}

int PaintingOverlay::hitTestParallel(const QPointF& testPos, double tolerance) const
{
    for (int i = 0; i < m_parallels.size(); ++i) {
        const ParallelObject& parallel = m_parallels[i];
        if (!parallel.isVisible || !parallel.isCompleted) continue;

        // 优先测试中线（如果存在）
        if (parallel.isCompleted && parallel.points.size() >= 3) {
            double distMid = calculateDistancePointToLine(testPos, parallel.midStart, parallel.midEnd);
            if (distMid <= tolerance) {
                // 返回一个特殊值表示中线命中：负数表示中线，绝对值-1是索引
                return -(i + 1000); // 使用1000作为偏移来区分中线
            }
        }

        // 测试第一条线
        if (parallel.points.size() >= 2) {
            double distance1 = calculateDistancePointToLine(testPos, parallel.points[0], parallel.points[1]);
            if (distance1 <= tolerance) {
                return i;
            }
        }

        // 测试第二条线
        if (parallel.points.size() >= 3) {
            QPointF direction = parallel.points[1] - parallel.points[0];
            QPointF parallelStart = parallel.points[2];
            QPointF parallelEnd = parallelStart + direction;
            double distance2 = calculateDistancePointToLine(testPos, parallelStart, parallelEnd);
            if (distance2 <= tolerance) {
                return i;
            }
        }
    }
    return -1;
}

int PaintingOverlay::hitTestTwoLines(const QPointF& testPos, double tolerance) const
{
    for (int i = 0; i < m_twoLines.size(); ++i) {
        const TwoLinesObject& twoLines = m_twoLines[i];
        if (!twoLines.isVisible || !twoLines.isCompleted) continue;

        // 优先测试角平分线（如果存在）
        if (twoLines.isCompleted && twoLines.points.size() >= 4) {
            // 计算角平分线方向
            QPointF dir1 = twoLines.points[1] - twoLines.points[0];
            QPointF dir2 = twoLines.points[3] - twoLines.points[2];

            // 归一化方向向量
            double len1 = sqrt(dir1.x() * dir1.x() + dir1.y() * dir1.y());
            double len2 = sqrt(dir2.x() * dir2.x() + dir2.y() * dir2.y());
            if (len1 > 0) dir1 /= len1;
            if (len2 > 0) dir2 /= len2;

            // 计算角平分线方向
            QPointF bisectorDir = dir1 + dir2;
            double bisectorLen = sqrt(bisectorDir.x() * bisectorDir.x() + bisectorDir.y() * bisectorDir.y());
            if (bisectorLen > 0) {
                bisectorDir /= bisectorLen;

                // 角平分线上的两个点（用于距离计算）
                QPointF bisectorStart = twoLines.intersection - bisectorDir * 5000.0;
                QPointF bisectorEnd = twoLines.intersection + bisectorDir * 5000.0;

                // 测试角平分线
                double distBisector = calculateDistancePointToLine(testPos, bisectorStart, bisectorEnd);
                if (distBisector <= tolerance) {
                    // 返回一个特殊值表示角平分线命中：负数表示角平分线，绝对值-1是索引
                    return -(i + 2000); // 使用2000作为偏移来区分角平分线
                }
            }
        }

        // 测试第一条线
        if (twoLines.points.size() >= 2) {
            double distance1 = calculateDistancePointToLine(testPos, twoLines.points[0], twoLines.points[1]);
            if (distance1 <= tolerance) {
                return i;
            }
        }

        // 测试第二条线
        if (twoLines.points.size() >= 4) {
            double distance2 = calculateDistancePointToLine(testPos, twoLines.points[2], twoLines.points[3]);
            if (distance2 <= tolerance) {
                return i;
            }
        }
    }
    return -1;
}

int PaintingOverlay::hitTestLineSegmentAngle(const QPointF& testPos, double tolerance) const
{
    for (int i = 0; i < m_lineSegmentAngles.size(); ++i) {
        const LineSegmentAngleObject& angleObj = m_lineSegmentAngles[i];
        if (!angleObj.isVisible || !angleObj.isCompleted || angleObj.points.size() < 4) continue;

        // 测试两条线段是否被点击
        QPointF line1Start = angleObj.points[0];
        QPointF line1End = angleObj.points[1];
        QPointF line2Start = angleObj.points[2];
        QPointF line2End = angleObj.points[3];

        // 测试第一条线段
        double dist1 = calculateDistancePointToLine(testPos, line1Start, line1End);
        if (dist1 <= tolerance) {
            return i;
        }

        // 测试第二条线段
        double dist2 = calculateDistancePointToLine(testPos, line2Start, line2End);
        if (dist2 <= tolerance) {
            return i;
        }

        // 如果有交点，也测试交点附近的区域
        if (angleObj.hasIntersection) {
            double distToIntersection = sqrt(pow(testPos.x() - angleObj.intersection.x(), 2) +
                                           pow(testPos.y() - angleObj.intersection.y(), 2));
            if (distToIntersection <= tolerance * 2) {  // 交点区域容差更大
                return i;
            }
        }
    }
    return -1;
}

void PaintingOverlay::onSelectionChanged()
{
    // 更新显示
    update();

    // 可以在这里发射信号通知外部选择变化
    // emit selectionChanged(getSelectedObjectInfo());
}

// 复合测量功能实现
void PaintingOverlay::performComplexMeasurement(const QString& measurementType)
{
    if (measurementType == "点与线距离") {
        if (m_selectedPoints.size() == 1 &&
            (m_selectedLines.size() == 1 || m_selectedParallelMiddleLines.size() == 1)) {

            int pointIndex = *m_selectedPoints.begin();
            const PointObject& point = m_points[pointIndex];

            QPointF lineStart, lineEnd;
            bool hasValidLine = false;

            // 获取线的起止点（直线或平行线中线）
            if (m_selectedLines.size() == 1) {
                int lineIndex = *m_selectedLines.begin();
                if (lineIndex >= 0 && lineIndex < m_lines.size()) {
                    const LineObject& line = m_lines[lineIndex];
                    if (line.points.size() >= 2) {
                        lineStart = line.points[0];
                        lineEnd = line.points[1];
                        hasValidLine = true;
                    }
                }
            } else if (m_selectedParallelMiddleLines.size() == 1) {
                int parallelIndex = *m_selectedParallelMiddleLines.begin();
                hasValidLine = getParallelMiddleLinePoints(parallelIndex, lineStart, lineEnd);
            }

            if (hasValidLine) {
                // 计算垂足点
                QPointF footPoint = calculatePerpendicularFoot(point.position, lineStart, lineEnd);

                // 计算距离
                double distance = calculatePointToLineDistance(point.position, lineStart, lineEnd);

                // 创建垂线段
                LineSegmentObject perpendicular;
                perpendicular.points.append(point.position);
                perpendicular.points.append(footPoint);
                perpendicular.isCompleted = true;
                perpendicular.color = Qt::red;
                perpendicular.thickness = 2.0;
                perpendicular.isDashed = true;
                perpendicular.isVisible = true;
                perpendicular.length = distance;
                perpendicular.label = QString("距离: %1").arg(formatDistance(distance));

                // 添加到线段列表
                m_lineSegments.append(perpendicular);

                // 记录历史
                DrawingAction action;
                action.type = DrawingAction::AddLineSegment;
                action.source = DrawingAction::ManualDrawing;
                action.index = m_lineSegments.size() - 1;
                commitDrawingAction(action);

                // 发送测量完成信号
                QString result = QString("点到直线距离: %1").arg(formatDistance(distance));
                emit measurementCompleted(m_viewName, result);

                // 清除选择并更新显示
                clearSelection();
                emit drawingDataChanged(m_viewName);
                update();
            }
        }
    } else if (measurementType == "点与圆距离") {
        if (m_selectedPoints.size() == 1 && m_selectedCircles.size() == 1) {
            int pointIndex = *m_selectedPoints.begin();
            int circleIndex = *m_selectedCircles.begin();

            if (pointIndex >= 0 && pointIndex < m_points.size() &&
                circleIndex >= 0 && circleIndex < m_circles.size()) {

                const PointObject& point = m_points[pointIndex];
                const CircleObject& circle = m_circles[circleIndex];

                if (circle.isCompleted) {
                    // 计算点到圆周的距离
                    double distanceToCircumference = calculatePointToCircleDistance(
                        point.position, circle.center, circle.radius, true);

                    // 计算圆周上最近的点
                    QPointF direction = point.position - circle.center;
                    double distanceToCenter = std::sqrt(direction.x() * direction.x() + direction.y() * direction.y());

                    if (distanceToCenter > 1e-6) {
                        direction /= distanceToCenter; // 单位化
                        QPointF closestPointOnCircle = circle.center + direction * circle.radius;

                        // 创建到圆周的距离线段
                        LineSegmentObject toCircumference;
                        toCircumference.points.append(point.position);
                        toCircumference.points.append(closestPointOnCircle);
                        toCircumference.isCompleted = true;
                        toCircumference.color = Qt::red;
                        toCircumference.thickness = 2.0;
                        toCircumference.isDashed = true;
                        toCircumference.isVisible = true;
                        toCircumference.length = distanceToCircumference;
                        toCircumference.label = QString("距离: %1").arg(formatDistance(distanceToCircumference));

                        m_lineSegments.append(toCircumference);

                        // 记录历史
                        DrawingAction action;
                        action.type = DrawingAction::AddLineSegment;
                        action.source = DrawingAction::ManualDrawing;
                        action.index = m_lineSegments.size() - 1;
                        commitDrawingAction(action);

                        // 发送测量完成信号
                        QString result = QString("点到圆距离: %1").arg(formatDistance(distanceToCircumference));
                        emit measurementCompleted(m_viewName, result);

                        // 清除选择并更新显示
                        clearSelection();
                        emit drawingDataChanged(m_viewName);
                        update();
                    }
                }
            }
        }
    } else if (measurementType == "圆与圆距离") {
        if (m_selectedCircles.size() == 2 &&
            m_selectedPoints.isEmpty() && m_selectedLines.isEmpty() &&
            m_selectedFineCircles.isEmpty() && m_selectedLineSegments.isEmpty()) {

            auto resolveCircle = [&](const CircleObject& circle, QPointF& center, double& radius) -> bool {
                if (circle.isCompleted && circle.radius > 0) {
                    center = circle.center;
                    radius = circle.radius;
                    return true;
                }
                if (circle.points.size() >= 3) {
                    return calculateCircleFromThreePoints(circle.points, center, radius);
                }
                return false;
            };

            auto it = m_selectedCircles.begin();
            int firstIndex = *it;
            ++it;
            int secondIndex = *it;

            if (firstIndex >= 0 && firstIndex < m_circles.size() &&
                secondIndex >= 0 && secondIndex < m_circles.size()) {

                const CircleObject& c1 = m_circles[firstIndex];
                const CircleObject& c2 = m_circles[secondIndex];

                QPointF center1, center2;
                double radius1 = 0.0, radius2 = 0.0;

                if (resolveCircle(c1, center1, radius1) && resolveCircle(c2, center2, radius2)) {
                    double dx = center2.x() - center1.x();
                    double dy = center2.y() - center1.y();
                    double centerDistance = std::sqrt(dx * dx + dy * dy);

                    // 绘制连线并标注
                    LineSegmentObject segment;
                    segment.points.append(center1);
                    segment.points.append(center2);
                    segment.isCompleted = true;
                    segment.color = Qt::red;
                    segment.thickness = 2.0;
                    segment.isDashed = true;
                    segment.isVisible = true;
                    segment.length = centerDistance;
                    segment.label = QString("圆心距: %1").arg(formatDistance(centerDistance));

                    m_lineSegments.append(segment);

                    DrawingAction action;
                    action.type = DrawingAction::AddLineSegment;
                    action.source = DrawingAction::ManualDrawing;
                    action.index = m_lineSegments.size() - 1;
                    commitDrawingAction(action);

                    QString result = QString("圆与圆圆心距离: %1").arg(formatDistance(centerDistance));
                    emit measurementCompleted(m_viewName, result);

                    clearSelection();
                    emit drawingDataChanged(m_viewName);
                    update();
                }
            }
        }
    } else if (measurementType == "点与精细圆距离") {
        if (m_selectedPoints.size() == 1 && m_selectedFineCircles.size() == 1) {
            int pointIndex = *m_selectedPoints.begin();
            int circleIndex = *m_selectedFineCircles.begin();

            if (pointIndex >= 0 && pointIndex < m_points.size() &&
                circleIndex >= 0 && circleIndex < m_fineCircles.size()) {

                const PointObject& point = m_points[pointIndex];
                const FineCircleObject& circle = m_fineCircles[circleIndex];

                if (circle.isCompleted) {
                    // 计算点到圆周的距离
                    double distanceToCircumference = calculatePointToCircleDistance(
                        point.position, circle.center, circle.radius, true);

                    // 计算圆周上最近的点
                    QPointF direction = point.position - circle.center;
                    double distanceToCenter = std::sqrt(direction.x() * direction.x() + direction.y() * direction.y());

                    if (distanceToCenter > 1e-6) {
                        direction /= distanceToCenter; // 单位化
                        QPointF closestPointOnCircle = circle.center + direction * circle.radius;

                        // 创建到圆周的距离线段
                        LineSegmentObject toCircumference;
                        toCircumference.points.append(point.position);
                        toCircumference.points.append(closestPointOnCircle);
                        toCircumference.isCompleted = true;
                        toCircumference.color = Qt::red;
                        toCircumference.thickness = 2.0;
                        toCircumference.isDashed = true;
                        toCircumference.isVisible = true;
                        toCircumference.length = distanceToCircumference;
                        toCircumference.label = QString("距离: %1").arg(formatDistance(distanceToCircumference));

                        m_lineSegments.append(toCircumference);

                        // 记录历史
                        DrawingAction action;
                        action.type = DrawingAction::AddLineSegment;
                        action.source = DrawingAction::ManualDrawing;
                        action.index = m_lineSegments.size() - 1;
                        commitDrawingAction(action);

                        // 发送测量完成信号
                        QString result = QString("点到精细圆距离: %1").arg(formatDistance(distanceToCircumference));
                        emit measurementCompleted(m_viewName, result);

                        // 清除选择并更新显示
                        clearSelection();
                        emit drawingDataChanged(m_viewName);
                        update();
                    }
                }
            }
        }
    } else if (measurementType == "线与圆关系") {
        if ((m_selectedLines.size() == 1 || m_selectedParallelMiddleLines.size() == 1) &&
            m_selectedCircles.size() == 1) {

            int circleIndex = *m_selectedCircles.begin();
            const CircleObject& circle = m_circles[circleIndex];

            QPointF lineStart, lineEnd;
            bool hasValidLine = false;

            // 获取线的起止点（直线或平行线中线）
            if (m_selectedLines.size() == 1) {
                int lineIndex = *m_selectedLines.begin();
                if (lineIndex >= 0 && lineIndex < m_lines.size()) {
                    const LineObject& line = m_lines[lineIndex];
                    if (line.points.size() >= 2) {
                        lineStart = line.points[0];
                        lineEnd = line.points[1];
                        hasValidLine = true;
                    }
                }
            } else if (m_selectedParallelMiddleLines.size() == 1) {
                int parallelIndex = *m_selectedParallelMiddleLines.begin();
                hasValidLine = getParallelMiddleLinePoints(parallelIndex, lineStart, lineEnd);
            }

            if (hasValidLine && circle.isCompleted) {
                // 计算圆心到直线的垂足
                QPointF footPoint = calculatePerpendicularFoot(circle.center, lineStart, lineEnd);

                    // 计算从圆心到直线垂足的方向向量
                    QPointF direction = footPoint - circle.center;
                    double dirLength = sqrt(direction.x() * direction.x() + direction.y() * direction.y());
                    
                    if (dirLength > 0) {
                        // 归一化方向向量
                        direction /= dirLength;
                        
                        // 计算圆周上的垂足点
                        QPointF circleFootPoint = circle.center + direction * circle.radius;
                        
                        // 创建从直线垂足到圆周垂足的虚线段
                        LineSegmentObject perpendicular;
                        perpendicular.points.append(footPoint);  // 从直线垂足开始
                        perpendicular.points.append(circleFootPoint);  // 到圆周垂足结束
                        perpendicular.isCompleted = true;
                        perpendicular.color = Qt::magenta;
                        perpendicular.thickness = 2.0;
                        perpendicular.isDashed = true;
                        perpendicular.isVisible = true;

                    // 计算直线到圆周的距离
                    double distanceToCenter = calculatePointToLineDistance(circle.center, lineStart, lineEnd);
                    double distanceToCircle = abs(distanceToCenter - circle.radius);
                    perpendicular.length = distanceToCircle;
                    perpendicular.label = QString("距离: %1").arg(formatDistance(distanceToCircle));

                    m_lineSegments.append(perpendicular);

                    // 记录历史
                    DrawingAction action;
                    action.type = DrawingAction::AddLineSegment;
                    action.source = DrawingAction::ManualDrawing;
                    action.index = m_lineSegments.size() - 1;
                    commitDrawingAction(action);

                    // 分析关系并发送信号
                    QString relationResult = analyzeLineCircleRelation(lineStart, lineEnd, circle.center, circle.radius);
                    emit measurementCompleted(m_viewName, relationResult);
                }

                // 清除选择并更新显示
                clearSelection();
                emit drawingDataChanged(m_viewName);
                update();
            }
        }
    } else if (measurementType == "线与精细圆关系") {
        if ((m_selectedLines.size() == 1 || m_selectedParallelMiddleLines.size() == 1) &&
            m_selectedFineCircles.size() == 1) {

            int fineCircleIndex = *m_selectedFineCircles.begin();
            const FineCircleObject& fineCircle = m_fineCircles[fineCircleIndex];

            QPointF lineStart, lineEnd;
            bool hasValidLine = false;

            // 获取线的起止点（直线或平行线中线）
            if (m_selectedLines.size() == 1) {
                int lineIndex = *m_selectedLines.begin();
                if (lineIndex >= 0 && lineIndex < m_lines.size()) {
                    const LineObject& line = m_lines[lineIndex];
                    if (line.points.size() >= 2) {
                        lineStart = line.points[0];
                        lineEnd = line.points[1];
                        hasValidLine = true;
                    }
                }
            } else if (m_selectedParallelMiddleLines.size() == 1) {
                int parallelIndex = *m_selectedParallelMiddleLines.begin();
                hasValidLine = getParallelMiddleLinePoints(parallelIndex, lineStart, lineEnd);
            }

            if (hasValidLine && fineCircle.isCompleted) {
                // 计算圆心到直线的垂足
                QPointF footPoint = calculatePerpendicularFoot(fineCircle.center, lineStart, lineEnd);

                    // 计算从圆心到直线垂足的方向向量
                    QPointF direction = footPoint - fineCircle.center;
                    double dirLength = sqrt(direction.x() * direction.x() + direction.y() * direction.y());
                    
                    if (dirLength > 0) {
                        // 归一化方向向量
                        direction /= dirLength;
                        
                        // 计算圆周上的垂足点
                        QPointF circleFootPoint = fineCircle.center + direction * fineCircle.radius;
                        
                        // 创建从直线垂足到圆周垂足的虚线段
                        LineSegmentObject perpendicular;
                        perpendicular.points.append(footPoint);  // 从直线垂足开始
                        perpendicular.points.append(circleFootPoint);  // 到圆周垂足结束
                        perpendicular.isCompleted = true;
                        perpendicular.color = Qt::magenta;
                        perpendicular.thickness = 2.0;
                        perpendicular.isDashed = true;
                        perpendicular.isVisible = true;

                    // 计算直线到圆周的距离
                    double distanceToCenter = calculatePointToLineDistance(fineCircle.center, lineStart, lineEnd);
                    double distanceToCircle = abs(distanceToCenter - fineCircle.radius);
                    perpendicular.length = distanceToCircle;
                    perpendicular.label = QString("距离: %1").arg(formatDistance(distanceToCircle));

                    m_lineSegments.append(perpendicular);

                    // 记录历史
                    DrawingAction action;
                    action.type = DrawingAction::AddLineSegment;
                    action.source = DrawingAction::ManualDrawing;
                    action.index = m_lineSegments.size() - 1;
                    commitDrawingAction(action);

                    // 分析关系并发送信号
                    QString relationResult = analyzeLineCircleRelation(lineStart, lineEnd, fineCircle.center, fineCircle.radius);
                    emit measurementCompleted(m_viewName, relationResult);
                }

                // 清除选择并更新显示
                clearSelection();
                emit drawingDataChanged(m_viewName);
                update();
            }
        }
    } else if (measurementType == "线段夹角") {
        if (m_selectedLineSegments.size() == 2) {
            QList<int> indices = m_selectedLineSegments.values();
            int index1 = indices[0];
            int index2 = indices[1];

            if (index1 >= 0 && index1 < m_lineSegments.size() &&
                index2 >= 0 && index2 < m_lineSegments.size()) {

                const LineSegmentObject& line1 = m_lineSegments[index1];
                const LineSegmentObject& line2 = m_lineSegments[index2];

                if (line1.points.size() >= 2 && line2.points.size() >= 2) {
                    // 计算角度
                    double angle = calculateLineSegmentAngle(
                        line1.points[0], line1.points[1],
                        line2.points[0], line2.points[1]);

                    // 计算两线段的交点
                    QPointF intersection;
                    bool hasIntersection = calculateLineIntersection(
                        line1.points[0], line1.points[1],
                        line2.points[0], line2.points[1], intersection);

                    // 创建线段夹角对象
                    LineSegmentAngleObject angleObj;
                    angleObj.points.append(line1.points[0]);  // line1_start
                    angleObj.points.append(line1.points[1]);  // line1_end
                    angleObj.points.append(line2.points[0]);  // line2_start
                    angleObj.points.append(line2.points[1]);  // line2_end
                    angleObj.isCompleted = true;
                    angleObj.color = Qt::red;
                    angleObj.thickness = 2;
                    angleObj.angle = angle;
                    angleObj.intersection = intersection;
                    angleObj.hasIntersection = hasIntersection;
                    angleObj.label = QString("%1°").arg(angle, 0, 'f', 1);
                    angleObj.isVisible = true;

                    m_lineSegmentAngles.append(angleObj);
                    
                    // 确保线段夹角对象被正确保存
                    qDebug() << QString("创建线段夹角对象，索引: %1, 可见: %2, 完成: %3")
                             .arg(m_lineSegmentAngles.size() - 1)
                             .arg(angleObj.isVisible)
                             .arg(angleObj.isCompleted);

                    // 记录历史
                    DrawingAction action;
                    action.type = DrawingAction::AddLineSegmentAngle;
                    action.source = DrawingAction::ManualDrawing;
                    action.index = m_lineSegmentAngles.size() - 1;
                    commitDrawingAction(action);

                    // 发送测量完成信号
                    QString result = QString("线段夹角: %1 度").arg(angle, 0, 'f', 2);
                    emit measurementCompleted(m_viewName, result);

                    // 清除选择并更新显示
                    clearSelection();
                    emit drawingDataChanged(m_viewName);
                    update();
                }
            }
        }
    } else if (measurementType == "点与角平分线距离") {
        if (m_selectedPoints.size() == 1 && m_selectedBisectorLines.size() == 1) {
            QList<int> pointIndices = m_selectedPoints.values();
            QList<int> twoLinesIndices = m_selectedBisectorLines.values();
            int pointIndex = pointIndices[0];
            int twoLinesIndex = twoLinesIndices[0];

            if (pointIndex >= 0 && pointIndex < m_points.size() &&
                twoLinesIndex >= 0 && twoLinesIndex < m_twoLines.size()) {

                const PointObject& point = m_points[pointIndex];
                const TwoLinesObject& twoLines = m_twoLines[twoLinesIndex];

                if (twoLines.isCompleted && twoLines.points.size() >= 4) {
                    // 计算角平分线方向
                    QPointF dir1 = twoLines.points[1] - twoLines.points[0];
                    QPointF dir2 = twoLines.points[3] - twoLines.points[2];

                    // 归一化方向向量
                    double len1 = sqrt(dir1.x() * dir1.x() + dir1.y() * dir1.y());
                    double len2 = sqrt(dir2.x() * dir2.x() + dir2.y() * dir2.y());
                    if (len1 > 0) dir1 /= len1;
                    if (len2 > 0) dir2 /= len2;

                    // 计算角平分线方向
                    QPointF bisectorDir = dir1 + dir2;
                    double bisectorLen = sqrt(bisectorDir.x() * bisectorDir.x() + bisectorDir.y() * bisectorDir.y());
                    if (bisectorLen > 0) bisectorDir /= bisectorLen;

                    // 角平分线上的两个点（用于距离计算）
                    QPointF bisectorStart = twoLines.intersection - bisectorDir * 5000.0;
                    QPointF bisectorEnd = twoLines.intersection + bisectorDir * 5000.0;

                    // 计算点到角平分线的距离
                    double distance = calculatePointToLineDistance(point.position, bisectorStart, bisectorEnd);

                    // 计算垂足
                    QPointF footPoint = calculatePerpendicularFoot(point.position, bisectorStart, bisectorEnd);

                    // 创建垂线段
                    LineSegmentObject perpendicular;
                    perpendicular.points.append(point.position);
                    perpendicular.points.append(footPoint);
                    perpendicular.isCompleted = true;
                    perpendicular.color = Qt::red;
                    perpendicular.thickness = 2.0;
                    perpendicular.isDashed = true;
                    perpendicular.isVisible = true;
                    perpendicular.length = distance;
                    perpendicular.label = QString("距离: %1").arg(formatDistance(distance));

                    // 添加到线段列表
                    m_lineSegments.append(perpendicular);

                    // 记录历史
                    DrawingAction action;
                    action.type = DrawingAction::AddLineSegment;
                    action.source = DrawingAction::ManualDrawing;
                    action.index = m_lineSegments.size() - 1;
                    commitDrawingAction(action);

                    // 发送测量完成信号
                    QString result = QString("点到角平分线距离: %1").arg(formatDistance(distance));
                    emit measurementCompleted(m_viewName, result);

                    // 清除选择并更新显示
                    clearSelection();
                    emit drawingDataChanged(m_viewName);
                    update();
                }
            }
        }
    } else if (measurementType == "两线夹角") {
        int totalLineCount = m_selectedLines.size() + m_selectedParallelMiddleLines.size();
        if (totalLineCount == 2) {
            QPointF line1Start, line1End, line2Start, line2End;
            bool hasValidLines = false;

            // 获取两条线的起止点
            QList<int> lineIndices = m_selectedLines.values();
            QList<int> parallelIndices = m_selectedParallelMiddleLines.values();

            if (m_selectedLines.size() == 2) {
                // 两条直线
                const LineObject& line1 = m_lines[lineIndices[0]];
                const LineObject& line2 = m_lines[lineIndices[1]];
                if (line1.points.size() >= 2 && line2.points.size() >= 2) {
                    line1Start = line1.points[0]; line1End = line1.points[1];
                    line2Start = line2.points[0]; line2End = line2.points[1];
                    hasValidLines = true;
                }
            } else if (m_selectedLines.size() == 1 && m_selectedParallelMiddleLines.size() == 1) {
                // 一条直线 + 一条平行线中线
                const LineObject& line = m_lines[lineIndices[0]];
                if (line.points.size() >= 2 && getParallelMiddleLinePoints(parallelIndices[0], line2Start, line2End)) {
                    line1Start = line.points[0]; line1End = line.points[1];
                    hasValidLines = true;
                }
            } else if (m_selectedParallelMiddleLines.size() == 2) {
                // 两条平行线中线
                if (getParallelMiddleLinePoints(parallelIndices[0], line1Start, line1End) &&
                    getParallelMiddleLinePoints(parallelIndices[1], line2Start, line2End)) {
                    hasValidLines = true;
                }
            }

            if (hasValidLines) {
                // 计算两条直线的交点
                QPointF intersection;
                bool hasIntersection = calculateLineIntersection(line1Start, line1End, line2Start, line2End, intersection);

                if (hasIntersection) {
                    // 计算夹角
                    double angle = calculateLineSegmentAngle(line1Start, line1End, line2Start, line2End);

                    // 计算两条直线的方向向量
                    QPointF dir1 = line1End - line1Start;
                    QPointF dir2 = line2End - line2Start;
                        
                        // 归一化方向向量
                        double len1 = sqrt(dir1.x() * dir1.x() + dir1.y() * dir1.y());
                        double len2 = sqrt(dir2.x() * dir2.x() + dir2.y() * dir2.y());
                        if (len1 > 0) dir1 /= len1;
                        if (len2 > 0) dir2 /= len2;
                        
                        // 计算角平分线方向
                        QPointF bisectorDir = dir1 + dir2;
                        double bisectorLen = sqrt(bisectorDir.x() * bisectorDir.x() + bisectorDir.y() * bisectorDir.y());
                        if (bisectorLen > 0) {
                            bisectorDir /= bisectorLen;
                            
                            // 创建角平分线线段（延伸到图像边界，与TwoLinesObject一致）
                            LineSegmentObject bisector;
                            bisector.points.append(intersection - bisectorDir * 5000.0);
                            bisector.points.append(intersection + bisectorDir * 5000.0);
                            bisector.isCompleted = true;
                            bisector.color = Qt::red;  // 使用红色
                            bisector.thickness = 2.0;
                            bisector.isDashed = true;
                            bisector.isVisible = true;
                            bisector.label = QString("BISECTOR:%1°:%2").arg(angle, 0, 'f', 1).arg(formatCoordinate(intersection));
                            
                            // 添加到线段列表
                            m_lineSegments.append(bisector);
                            
                            // 记录历史
                            DrawingAction action;
                            action.type = DrawingAction::AddLineSegment;
                            action.source = DrawingAction::ManualDrawing;
                            action.index = m_lineSegments.size() - 1;
                            commitDrawingAction(action);
                        }

                    // 发送测量完成信号
                    QString result = QString("两线夹角: %1°\n交点坐标: %2")
                                    .arg(angle, 0, 'f', 1).arg(formatCoordinate(intersection));
                    emit measurementCompleted(m_viewName, result);
                } else {
                    // 两线平行
                    QString result = "两线平行，无交点";
                    emit measurementCompleted(m_viewName, result);
                }

                // 清除选择并更新显示
                clearSelection();
                emit drawingDataChanged(m_viewName);
                update();
            }
        }
    } else if (measurementType == "点与角平分线距离LineSegment") {
        if (m_selectedPoints.size() == 1 && m_selectedLineSegments.size() == 1) {
            QList<int> pointIndices = m_selectedPoints.values();
            QList<int> lineSegmentIndices = m_selectedLineSegments.values();
            int pointIndex = pointIndices[0];
            int lineSegmentIndex = lineSegmentIndices[0];

            if (pointIndex >= 0 && pointIndex < m_points.size() &&
                lineSegmentIndex >= 0 && lineSegmentIndex < m_lineSegments.size()) {

                const PointObject& point = m_points[pointIndex];
                const LineSegmentObject& lineSegment = m_lineSegments[lineSegmentIndex];

                // 确认是角平分线
                if (lineSegment.label.startsWith("BISECTOR:") && lineSegment.points.size() >= 2) {
                    // 计算点到角平分线的距离
                    double distance = calculatePointToLineDistance(point.position, lineSegment.points[0], lineSegment.points[1]);

                    // 计算垂足
                    QPointF footPoint = calculatePerpendicularFoot(point.position, lineSegment.points[0], lineSegment.points[1]);

                    // 创建垂线段
                    LineSegmentObject perpendicular;
                    perpendicular.points.append(point.position);
                    perpendicular.points.append(footPoint);
                    perpendicular.isCompleted = true;
                    perpendicular.color = Qt::red;
                    perpendicular.thickness = 2.0;
                    perpendicular.isDashed = true;
                    perpendicular.isVisible = true;
                    perpendicular.length = distance;
                    perpendicular.label = QString("距离: %1").arg(formatDistance(distance));

                    // 添加到线段列表
                    m_lineSegments.append(perpendicular);

                    // 记录历史
                    DrawingAction action;
                    action.type = DrawingAction::AddLineSegment;
                    action.source = DrawingAction::ManualDrawing;
                    action.index = m_lineSegments.size() - 1;
                    commitDrawingAction(action);

                    // 发送测量完成信号
                    QString result = QString("点到角平分线距离: %1").arg(formatDistance(distance));
                    emit measurementCompleted(m_viewName, result);

                    // 清除选择并更新显示
                    clearSelection();
                    emit drawingDataChanged(m_viewName);
                    update();
                }
            }
        }
    } else if (measurementType == "点与线段距离") {
        if (m_selectedPoints.size() == 1 && m_selectedLineSegments.size() == 1) {
            QList<int> pointIndices = m_selectedPoints.values();
            QList<int> lineSegmentIndices = m_selectedLineSegments.values();
            int pointIndex = pointIndices[0];
            int lineSegmentIndex = lineSegmentIndices[0];

            if (pointIndex >= 0 && pointIndex < m_points.size() &&
                lineSegmentIndex >= 0 && lineSegmentIndex < m_lineSegments.size()) {

                const PointObject& point = m_points[pointIndex];
                const LineSegmentObject& lineSegment = m_lineSegments[lineSegmentIndex];

                // 确保线段有两个点
                if (lineSegment.points.size() >= 2) {
                    // 计算点到线段的距离
                    double distance = calculatePointToLineDistance(point.position, lineSegment.points[0], lineSegment.points[1]);

                    // 计算垂足
                    QPointF footPoint = calculatePerpendicularFoot(point.position, lineSegment.points[0], lineSegment.points[1]);

                    // 创建垂线段
                    LineSegmentObject perpendicular;
                    perpendicular.points.append(point.position);
                    perpendicular.points.append(footPoint);
                    perpendicular.isCompleted = true;
                    perpendicular.color = Qt::red;
                    perpendicular.thickness = 2.0;
                    perpendicular.isDashed = true;
                    perpendicular.isVisible = true;
                    perpendicular.length = distance;
                    perpendicular.label = QString("距离: %1").arg(formatDistance(distance));

                    // 添加到线段列表
                    m_lineSegments.append(perpendicular);

                    // 记录历史
                    DrawingAction action;
                    action.type = DrawingAction::AddLineSegment;
                    action.source = DrawingAction::ManualDrawing;
                    action.index = m_lineSegments.size() - 1;
                    commitDrawingAction(action);

                    // 发送测量完成信号
                    QString result = QString("点到线段距离: %1").arg(formatDistance(distance));
                    emit measurementCompleted(m_viewName, result);

                    // 清除选择并更新显示
                    clearSelection();
                    emit drawingDataChanged(m_viewName);
                    update();
                }
            }
        }
    } else if (measurementType == "线段与圆关系") {
        if (m_selectedLineSegments.size() == 1 && m_selectedCircles.size() == 1) {
            int lineSegmentIndex = *m_selectedLineSegments.begin();
            int circleIndex = *m_selectedCircles.begin();

            if (lineSegmentIndex >= 0 && lineSegmentIndex < m_lineSegments.size() &&
                circleIndex >= 0 && circleIndex < m_circles.size()) {

                const LineSegmentObject& lineSegment = m_lineSegments[lineSegmentIndex];
                const CircleObject& circle = m_circles[circleIndex];

                if (lineSegment.points.size() >= 2 && circle.isCompleted) {
                    // 计算线段到圆的最短距离和对应点
                    QPointF lineStart = lineSegment.points[0];
                    QPointF lineEnd = lineSegment.points[1];

                    // 计算圆心到线段的垂足（无论是否在线段上都使用垂足，与点与线段逻辑一致）
                    QPointF footPoint = calculatePerpendicularFoot(circle.center, lineStart, lineEnd);

                    // 计算从圆心到垂足的方向向量
                    QPointF direction = footPoint - circle.center;
                    double dirLength = sqrt(direction.x() * direction.x() + direction.y() * direction.y());

                    if (dirLength > 0) {
                        // 归一化方向向量
                        direction /= dirLength;

                        // 计算圆周上的对应点
                        QPointF circlePoint = circle.center + direction * circle.radius;

                        // 创建垂线段
                        LineSegmentObject perpendicular;
                        perpendicular.points.append(footPoint);  // 从垂足开始
                        perpendicular.points.append(circlePoint);
                        perpendicular.isCompleted = true;
                        perpendicular.color = Qt::magenta;
                        perpendicular.thickness = 2.0;
                        perpendicular.isDashed = true;
                        perpendicular.isVisible = true;

                        // 计算线段到圆周的距离
                        double distanceToCircle = abs(dirLength - circle.radius);
                        perpendicular.length = distanceToCircle;
                        perpendicular.label = QString("距离: %1").arg(formatDistance(distanceToCircle));

                        m_lineSegments.append(perpendicular);

                        // 记录历史
                        DrawingAction action;
                        action.type = DrawingAction::AddLineSegment;
                        action.source = DrawingAction::ManualDrawing;
                        action.index = m_lineSegments.size() - 1;
                        commitDrawingAction(action);

                        // 分析关系并发送信号
                        QString relationResult = analyzeLineSegmentCircleRelation(
                            lineStart, lineEnd, circle.center, circle.radius);
                        emit measurementCompleted(m_viewName, relationResult);
                    }

                    // 清除选择并更新显示
                    clearSelection();
                    emit drawingDataChanged(m_viewName);
                    update();
                }
            }
        }
    } else if (measurementType == "线段与精细圆关系") {
        if (m_selectedLineSegments.size() == 1 && m_selectedFineCircles.size() == 1) {
            int lineSegmentIndex = *m_selectedLineSegments.begin();
            int fineCircleIndex = *m_selectedFineCircles.begin();

            if (lineSegmentIndex >= 0 && lineSegmentIndex < m_lineSegments.size() &&
                fineCircleIndex >= 0 && fineCircleIndex < m_fineCircles.size()) {

                const LineSegmentObject& lineSegment = m_lineSegments[lineSegmentIndex];
                const FineCircleObject& fineCircle = m_fineCircles[fineCircleIndex];

                if (lineSegment.points.size() >= 2 && fineCircle.isCompleted) {
                    // 计算线段到圆的最短距离和对应点
                    QPointF lineStart = lineSegment.points[0];
                    QPointF lineEnd = lineSegment.points[1];

                    // 计算圆心到线段的垂足（无论是否在线段上都使用垂足，与点与线段逻辑一致）
                    QPointF footPoint = calculatePerpendicularFoot(fineCircle.center, lineStart, lineEnd);

                    // 计算从圆心到垂足的方向向量
                    QPointF direction = footPoint - fineCircle.center;
                    double dirLength = sqrt(direction.x() * direction.x() + direction.y() * direction.y());

                    if (dirLength > 0) {
                        // 归一化方向向量
                        direction /= dirLength;

                        // 计算圆周上的对应点
                        QPointF circlePoint = fineCircle.center + direction * fineCircle.radius;

                        // 创建垂线段
                        LineSegmentObject perpendicular;
                        perpendicular.points.append(footPoint);  // 从垂足开始
                        perpendicular.points.append(circlePoint);
                        perpendicular.isCompleted = true;
                        perpendicular.color = Qt::magenta;
                        perpendicular.thickness = 2.0;
                        perpendicular.isDashed = true;
                        perpendicular.isVisible = true;

                        // 计算线段到圆周的距离
                        double distanceToCircle = abs(dirLength - fineCircle.radius);
                        perpendicular.length = distanceToCircle;
                        perpendicular.label = QString("距离: %1").arg(formatDistance(distanceToCircle));

                        m_lineSegments.append(perpendicular);

                        // 记录历史
                        DrawingAction action;
                        action.type = DrawingAction::AddLineSegment;
                        action.source = DrawingAction::ManualDrawing;
                        action.index = m_lineSegments.size() - 1;
                        commitDrawingAction(action);

                        // 分析关系并发送信号
                        QString relationResult = analyzeLineSegmentCircleRelation(
                            lineStart, lineEnd, fineCircle.center, fineCircle.radius);
                        emit measurementCompleted(m_viewName, relationResult);
                    }

                    // 清除选择并更新显示
                    clearSelection();
                    emit drawingDataChanged(m_viewName);
                    update();
                }
            }
        }
    }
}

double PaintingOverlay::calculatePointToLineDistance(const QPointF& point, const QPointF& lineStart, const QPointF& lineEnd) const
{
    // 使用点到直线距离公式：|ax + by + c| / sqrt(a² + b²)
    // 直线方程：(y2-y1)x - (x2-x1)y + (x2-x1)y1 - (y2-y1)x1 = 0

    double x1 = lineStart.x();
    double y1 = lineStart.y();
    double x2 = lineEnd.x();
    double y2 = lineEnd.y();
    double x0 = point.x();
    double y0 = point.y();

    // 计算直线方程系数
    double a = y2 - y1;
    double b = x1 - x2;
    double c = (x2 - x1) * y1 - (y2 - y1) * x1;

    // 计算距离
    double distance = std::abs(a * x0 + b * y0 + c) / std::sqrt(a * a + b * b);

    return distance;
}

double PaintingOverlay::calculatePointToCircleDistance(const QPointF& point, const QPointF& circleCenter, double radius, bool toCircumference) const
{
    // 计算点到圆心的距离
    double dx = point.x() - circleCenter.x();
    double dy = point.y() - circleCenter.y();
    double distanceToCenter = std::sqrt(dx * dx + dy * dy);

    if (toCircumference) {
        // 返回点到圆周的最短距离
        return std::abs(distanceToCenter - radius);
    } else {
        // 返回点到圆心的距离
        return distanceToCenter;
    }
}

QString PaintingOverlay::analyzeLineCircleRelation(const QPointF& lineStart, const QPointF& lineEnd, const QPointF& circleCenter, double radius) const
{
    // 计算直线到圆心的距离
    double distanceToCenter = calculatePointToLineDistance(circleCenter, lineStart, lineEnd);

    QString relation;
    if (distanceToCenter < radius - 1e-6) {
        relation = "相交";
    } else if (distanceToCenter > radius + 1e-6) {
        relation = "相离";
    } else {
        relation = "相切";
    }

    return QString("直线与圆%1\n圆心到直线距离: %2\n圆半径: %3")
           .arg(relation).arg(formatDistance(distanceToCenter)).arg(formatDistance(radius));
}

QString PaintingOverlay::analyzeLineSegmentCircleRelation(const QPointF& lineStart, const QPointF& lineEnd, const QPointF& circleCenter, double radius) const
{
    // 计算圆心到线段的距离（与点与线段逻辑一致，使用垂足距离）
    double distanceToSegment = calculatePointToLineDistance(circleCenter, lineStart, lineEnd);

    QString relation;
    if (distanceToSegment < radius - 1e-6) {
        relation = "相交";
    } else if (distanceToSegment > radius + 1e-6) {
        relation = "相离";
    } else {
        relation = "相切";
    }

    return QString("线段与圆%1\n圆心到线段距离: %2\n圆半径: %3")
           .arg(relation).arg(formatDistance(distanceToSegment)).arg(formatDistance(radius));
}

bool PaintingOverlay::getParallelMiddleLinePoints(int parallelIndex, QPointF& lineStart, QPointF& lineEnd) const
{
    if (parallelIndex >= 0 && parallelIndex < m_parallels.size()) {
        const ParallelObject& parallel = m_parallels[parallelIndex];
        if (parallel.isCompleted && parallel.points.size() >= 3) {
            lineStart = parallel.midStart;
            lineEnd = parallel.midEnd;
            return true;
        }
    }
    return false;
}

double PaintingOverlay::calculateLineSegmentAngle(const QPointF& line1Start, const QPointF& line1End, const QPointF& line2Start, const QPointF& line2End) const
{
    // 计算两条线段的方向向量
    QPointF vector1 = line1End - line1Start;
    QPointF vector2 = line2End - line2Start;

    // 计算向量的模长
    double length1 = std::sqrt(vector1.x() * vector1.x() + vector1.y() * vector1.y());
    double length2 = std::sqrt(vector2.x() * vector2.x() + vector2.y() * vector2.y());

    if (length1 < 1e-6 || length2 < 1e-6) {
        return 0.0; // 避免除零错误
    }

    // 计算点积
    double dotProduct = vector1.x() * vector2.x() + vector1.y() * vector2.y();

    // 计算夹角的余弦值
    double cosAngle = dotProduct / (length1 * length2);

    // 限制余弦值在[-1, 1]范围内，避免数值误差
    cosAngle = (std::max)(-1.0, (std::min)(1.0, cosAngle));

    // 计算夹角（弧度）
    double angleRadians = std::acos(cosAngle);

    // 转换为角度
    double angleDegrees = angleRadians * 180.0 / M_PI;

    // 返回锐角（0-90度）
    if (angleDegrees > 90.0) {
        angleDegrees = 180.0 - angleDegrees;
    }

    return angleDegrees;
}

QPointF PaintingOverlay::calculatePerpendicularFoot(const QPointF& point, const QPointF& lineStart, const QPointF& lineEnd) const
{
    // 计算点到直线的垂足
    // 使用向量投影公式

    QPointF lineVector = lineEnd - lineStart;
    QPointF pointVector = point - lineStart;

    // 计算投影长度
    double lineLength2 = lineVector.x() * lineVector.x() + lineVector.y() * lineVector.y();

    if (lineLength2 < 1e-6) {
        // 线段长度为0，返回线段起点
        return lineStart;
    }

    double projection = (pointVector.x() * lineVector.x() + pointVector.y() * lineVector.y()) / lineLength2;

    // 计算垂足坐标
    QPointF footPoint = lineStart + projection * lineVector;

    return footPoint;
}

bool PaintingOverlay::isPointOnLineSegment(const QPointF& point, const QPointF& lineStart, const QPointF& lineEnd, double tolerance) const
{
    // 首先检查点到直线的距离
    double distanceToLine = calculatePointToLineDistance(point, lineStart, lineEnd);
    if (distanceToLine > tolerance) {
        return false;
    }

    // 计算垂足
    QPointF footPoint = calculatePerpendicularFoot(point, lineStart, lineEnd);

    // 检查垂足是否在线段范围内
    // 使用参数方程：P = lineStart + t * (lineEnd - lineStart)
    // 如果 0 <= t <= 1，则垂足在线段上

    QPointF lineVector = lineEnd - lineStart;
    QPointF footVector = footPoint - lineStart;

    double lineLength2 = lineVector.x() * lineVector.x() + lineVector.y() * lineVector.y();

    if (lineLength2 < 1e-6) {
        // 线段长度为0，检查点是否接近线段起点
        double distanceToStart = sqrt(pow(point.x() - lineStart.x(), 2) + pow(point.y() - lineStart.y(), 2));
        return distanceToStart <= tolerance;
    }

    // 计算参数t
    double t = (footVector.x() * lineVector.x() + footVector.y() * lineVector.y()) / lineLength2;

    // 垂足在线段范围内（严格限制在线段内）
    return (t >= 0.0 && t <= 1.0);
}

// 网格功能实现
void PaintingOverlay::setGridSpacing(int spacing)
{
    if (m_gridSpacing != spacing) {
        m_gridSpacing = spacing;
        m_gridCacheValid = false; // 使网格缓存失效
        update(); // 触发重绘
    }
}

void PaintingOverlay::setGridColor(const QColor& color)
{
    if (m_gridColor != color) {
        m_gridColor = color;
        m_gridCacheValid = false; // 使网格缓存失效
        update(); // 触发重绘
    }
}

void PaintingOverlay::setGridStyle(Qt::PenStyle style)
{
    if (m_gridStyle != style) {
        m_gridStyle = style;
        m_gridCacheValid = false; // 使网格缓存失效
        update(); // 触发重绘
    }
}

void PaintingOverlay::setGridWidth(int width)
{
    if (m_gridWidth != width) {
        m_gridWidth = width;
        m_gridCacheValid = false; // 使网格缓存失效
        update(); // 触发重绘
    }
}

int PaintingOverlay::getGridSpacing() const
{
    return m_gridSpacing;
}

QColor PaintingOverlay::getGridColor() const
{
    return m_gridColor;
}

Qt::PenStyle PaintingOverlay::getGridStyle() const
{
    return m_gridStyle;
}

int PaintingOverlay::getGridWidth() const
{
    return m_gridWidth;
}

void PaintingOverlay::drawGrid(QPainter& painter, const DrawingContext& ctx) const
{
    // 如果网格间距为0或负数，则不绘制网格
    if (m_gridSpacing <= 0) {
        return;
    }

    // 检查是否需要重新计算网格（性能优化）
    bool needsRecalculation = !m_gridCacheValid ||
                             m_lastGridImageSize != m_imageSize ||
                             m_lastGridSpacing != m_gridSpacing;

    if (needsRecalculation) {
        // 更新缓存状态
        m_gridCacheValid = true;
        m_lastGridImageSize = m_imageSize;
        m_lastGridSpacing = m_gridSpacing;
    }

    // 保存当前画笔状态
    painter.save();

    // 关闭抗锯齿以获得清晰的像素级线条
    painter.setRenderHint(QPainter::Antialiasing, false);

    // 设置网格线的画笔
    QPen gridPen(m_gridColor);
    gridPen.setWidth(m_gridWidth);
    gridPen.setStyle(m_gridStyle);
    gridPen.setCosmetic(true); // 确保像素对齐，不受坐标变换影响
    painter.setPen(gridPen);

    // 获取图像尺寸（在图像坐标系中）
    int imageWidth = m_imageSize.width();
    int imageHeight = m_imageSize.height();

    // 绘制垂直线
    for (int x = 0; x <= imageWidth; x += m_gridSpacing) {
        painter.drawLine(x, 0, x, imageHeight);
    }

    // 绘制水平线
    for (int y = 0; y <= imageHeight; y += m_gridSpacing) {
        painter.drawLine(0, y, imageWidth, y);
    }

    // 恢复画笔状态
    painter.restore();
}

// ROI相关方法实现
void PaintingOverlay::drawROIs(QPainter& painter, const DrawingContext& ctx) const
{
    for (const auto& roi : m_rois) {
        if (roi.isVisible) {
            drawSingleROI(painter, roi, ctx);
        }
    }
}

void PaintingOverlay::drawSingleROI(QPainter& painter, const ROIDetectionObject& roi, const DrawingContext& ctx) const
{
    if (roi.points.size() < 2) {
        return;
    }

    // 获取ROI矩形
    QRectF rect = roi.getRect();
    if (rect.isEmpty()) {
        return;
    }

    // 创建ROI画笔
    QPen roiPen = createPen(roi.color, roi.thickness, ctx.scale, roi.isDashed);
    painter.setPen(roiPen);
    painter.setBrush(Qt::NoBrush);

    // 绘制ROI矩形
    painter.drawRect(rect);

    // 绘制标签
    if (!roi.label.isEmpty()) {
        QString displayText = roi.label;
        if (roi.isDetecting) {
            displayText += " (检测中...)";
        }

        // 使用统一的文字样式参数
        double textPadding = qMax(4.0, ctx.fontSize * 0.5);
        int bgBorderWidth = 1;
        QPointF labelPos = rect.topLeft() + QPointF(5, -5);
        drawTextWithBackground(painter, labelPos, displayText, ctx.font, roi.color, Qt::white, textPadding, bgBorderWidth, QPointF(0, 0));
    }
}

void PaintingOverlay::drawSingleROICreation(QPainter& painter, const ROIObject& roi, const DrawingContext& ctx) const
{
    if (!roi.rect.isValid() || roi.rect.isEmpty()) {
        return;
    }

    painter.save();

    // 绘制灰色遮罩（使用路径裁剪方法）
    // 获取图像的实际显示区域
    QRectF imageRect;
    if (m_imageSize.isValid() && !m_imageSize.isEmpty()) {
        // 使用图像坐标系统的完整范围
        imageRect = QRectF(0, 0, m_imageSize.width(), m_imageSize.height());
    } else {
        // 回退到widget矩形
        imageRect = rect();
    }

    // 创建遮罩路径：整个图像区域减去ROI区域
    QPainterPath maskPath;
    maskPath.addRect(imageRect);

    QPainterPath roiPath;
    if (qAbs(roi.angle) > 0.01) {
        // 如果有旋转，创建旋转的ROI路径
        QTransform transform;
        transform.translate(roi.rect.center().x(), roi.rect.center().y());
        transform.rotate(roi.angle);
        transform.translate(-roi.rect.center().x(), -roi.rect.center().y());
        roiPath.addRect(roi.rect);
        roiPath = transform.map(roiPath);
    } else {
        roiPath.addRect(roi.rect);
    }

    // 从遮罩中减去ROI区域
    maskPath = maskPath.subtracted(roiPath);

    // 绘制遮罩
    painter.fillPath(maskPath, QColor(0, 0, 0, 100)); // 半透明黑色遮罩

    // 如果有旋转角度，应用旋转变换
    if (qAbs(roi.angle) > 0.01) {
        QPointF center = roi.rect.center();
        painter.translate(center);
        painter.rotate(roi.angle);
        painter.translate(-center);
    }

    // 创建ROI画笔
    QPen roiPen = createPen(roi.borderColor, roi.thickness, ctx.scale, roi.isDashed);
    painter.setPen(roiPen);
    painter.setBrush(Qt::NoBrush);

    // 绘制ROI矩形
    painter.drawRect(roi.rect);

    // 如果是活动状态且显示控制点，绘制控制点
    if (roi.isActive && roi.showHandles) {
        drawROIHandles(painter, roi, ctx);
    }

    painter.restore();

    // 绘制模板名称标签（不受旋转影响）
    if (!roi.templateName.isEmpty()) {
        QString displayText = roi.templateName;

        // 使用统一的文字样式参数
        double textPadding = qMax(4.0, ctx.fontSize * 0.5);
        int bgBorderWidth = 1;
        QPointF labelPos = roi.rect.topLeft() + QPointF(5, -15);
        drawTextWithBackground(painter, labelPos, displayText, ctx.font, roi.borderColor, Qt::white, textPadding, bgBorderWidth, QPointF(0, 0));
    }

    // 绘制确认/取消按钮
    drawROIButtons(painter, ctx);

    // 绘制ROI信息（尺寸和角度）
    drawROIInfo(painter, ctx);
}

void PaintingOverlay::handleROIDrawingClick(const QPointF& pos)
{
    qDebug() << "handleROIDrawingClick called at position:" << pos;
    qDebug() << "m_hasCurrentROIDetection:" << m_hasCurrentROIDetection << "points size:" << (m_hasCurrentROIDetection ? m_currentROIDetection.points.size() : 0);
    if (!m_hasCurrentROIDetection) {
        // 开始绘制ROI
        m_currentROIDetection = ROIDetectionObject();
        m_currentROIDetection.points.append(pos);
        m_currentROIDetection.detectionType = m_currentDrawingTool;
        m_currentROIDetection.label = QString("ROI_%1").arg(m_rois.size() + 1);
        m_currentROIDetection.color = Qt::red;        // 使用红色更明显
        m_currentROIDetection.thickness = 2;          // 统一线宽为2像素
        m_hasCurrentROIDetection = true;

        qDebug() << "开始绘制ROI，起点：" << pos;
    } else if (m_currentROIDetection.points.size() >= 1 && !m_currentROIDetection.isCompleted) {
        // 完成ROI绘制
        qDebug() << "准备完成ROI绘制，当前points数量：" << m_currentROIDetection.points.size();
        if (m_currentROIDetection.points.size() == 1) {
            m_currentROIDetection.points.append(pos);
        } else {
            m_currentROIDetection.points[1] = pos;  // 更新第二个点
        }
        m_currentROIDetection.isCompleted = true;

        qDebug() << "完成ROI绘制，终点：" << pos;

        // 添加到ROI列表
        m_rois.append(m_currentROIDetection);

        // 记录历史
        DrawingAction action;
        action.type = DrawingAction::AddROI;
        action.source = DrawingAction::ManualDrawing;  // 手动绘制ROI
        action.index = m_rois.size() - 1;
        commitDrawingAction(action);

        // 立即执行自动检测
        performAutoDetection(m_currentROIDetection);

        // 清除当前ROI数据
        clearCurrentROIData();

        // 发出信号
        emit drawingCompleted(m_viewName);
        emit drawingDataChanged(m_viewName);
    } else {
        qDebug() << "ROI点击未处理 - hasCurrentROIDetection:" << m_hasCurrentROIDetection
                 << "points size:" << (m_hasCurrentROIDetection ? m_currentROIDetection.points.size() : 0)
                 << "isCompleted:" << (m_hasCurrentROIDetection ? m_currentROIDetection.isCompleted : false);
    }

    update();
}

int PaintingOverlay::hitTestROI(const QPointF& testPos, double tolerance) const
{
    for (int i = 0; i < m_rois.size(); ++i) {
        const ROIDetectionObject& roi = m_rois[i];
        if (!roi.isVisible || !roi.isCompleted) continue;

        QRectF rect = roi.getRect();

        // 检查是否在矩形边界附近
        if (qAbs(testPos.x() - rect.left()) <= tolerance ||
            qAbs(testPos.x() - rect.right()) <= tolerance ||
            qAbs(testPos.y() - rect.top()) <= tolerance ||
            qAbs(testPos.y() - rect.bottom()) <= tolerance) {

            // 进一步检查是否在矩形范围内
            if (rect.contains(testPos)) {
                return i;
            }
        }
    }
    return -1;
}

void PaintingOverlay::setCurrentROIData(const ROIDetectionObject& currentROI)
{
    m_currentROIDetection = currentROI;
    m_hasCurrentROIDetection = true;
}

void PaintingOverlay::clearCurrentROIData()
{
    m_currentROIDetection = ROIDetectionObject();
    m_hasCurrentROIDetection = false;
}

void PaintingOverlay::setROIsData(const QVector<ROIDetectionObject>& rois)
{
    m_rois = rois;
    update();
}

void PaintingOverlay::removeLastROI()
{
    if (!m_rois.isEmpty()) {
        // 删除最后一个ROI（刚刚添加的用于检测的ROI）
        m_rois.removeLast();

        // 不记录删除操作到历史，因为ROI框只是临时的检测工具
        // 这样撤销检测时只需要撤销一次（删除检测结果）即可

        qDebug() << "已删除检测完成的ROI框，剩余ROI数量：" << m_rois.size();
        update();
    }
}

void PaintingOverlay::performAutoDetection(const ROIDetectionObject& roi)
{
    if (!m_edgeDetector || !m_shapeDetector) {
        qDebug() << "图像处理器未初始化";
        removeLastROI();  // 删除ROI框
        return;
    }

    // 检测频率限制（避免过于频繁的检测）
    if (m_lastDetectionTime.isValid() && m_lastDetectionTime.msecsTo(QTime::currentTime()) < 500) {
        qDebug() << "检测频率过高，跳过本次检测";
        removeLastROI();  // 删除ROI框
        return;
    }
    m_lastDetectionTime = QTime::currentTime();

    qDebug() << "执行自动检测 - ROI:" << roi.getRect();
    qDebug() << "检测类型:" << (roi.detectionType == DrawingTool::ROI_LineDetect ? "直线" : "圆形");

    // 获取当前帧图像
    cv::Mat currentFrame = getCurrentFrameFromParent();
    if (currentFrame.empty()) {
        QString errorMsg = "检测失败：无法获取当前图像帧";
        qDebug() << errorMsg;
        emit measurementCompleted(m_viewName, errorMsg);

        // 显示用户友好的错误提示
        QWidget* parentWidget = this->parentWidget();
        while (parentWidget && !qobject_cast<QMainWindow*>(parentWidget)) {
            parentWidget = parentWidget->parentWidget();
        }
        if (parentWidget) {
            QMessageBox::warning(parentWidget, "自动检测错误",
                               "无法获取当前图像，请确保相机已启动并正在采集图像。");
        }

        // 检测失败时也删除ROI框
        removeLastROI();
        return;
    }

    // 转换ROI坐标为OpenCV Rect
    QRectF roiRect = roi.getRect();
    cv::Rect cvRoi(
        static_cast<int>(roiRect.x()),
        static_cast<int>(roiRect.y()),
        static_cast<int>(roiRect.width()),
        static_cast<int>(roiRect.height())
    );

    // 验证ROI区域有效性
    if (cvRoi.x < 0 || cvRoi.y < 0 ||
        cvRoi.x + cvRoi.width > currentFrame.cols ||
        cvRoi.y + cvRoi.height > currentFrame.rows ||
        cvRoi.width < 10 || cvRoi.height < 10) {

        QString errorMsg = QString("检测失败：ROI区域无效 - 位置(%1,%2) 尺寸(%3x%4) 图像尺寸(%5x%6)")
                          .arg(cvRoi.x).arg(cvRoi.y)
                          .arg(cvRoi.width).arg(cvRoi.height)
                          .arg(currentFrame.cols).arg(currentFrame.rows);
        qDebug() << errorMsg;
        emit measurementCompleted(m_viewName, "检测失败：ROI区域无效或过小");

        // 显示详细错误信息
        QWidget* parentWidget = this->parentWidget();
        while (parentWidget && !qobject_cast<QMainWindow*>(parentWidget)) {
            parentWidget = parentWidget->parentWidget();
        }
        if (parentWidget) {
            QMessageBox::warning(parentWidget, "ROI区域错误",
                               "选择的ROI区域无效，请确保：\n"
                               "1. ROI区域完全在图像范围内\n"
                               "2. ROI区域足够大（至少10x10像素）");
        }

        // 检测失败时也删除ROI框
        removeLastROI();
        return;
    }

    // 显示检测开始提示
    QString detectionTypeName = (roi.detectionType == DrawingTool::ROI_LineDetect) ? "直线" : "圆形";
    emit measurementCompleted(m_viewName, QString("开始%1检测...").arg(detectionTypeName));

    if (roi.detectionType == DrawingTool::ROI_LineDetect) {
        performLineDetection(currentFrame, cvRoi);
    } else if (roi.detectionType == DrawingTool::ROI_CircleDetect) {
        performCircleDetection(currentFrame, cvRoi);
    }
}

cv::Mat PaintingOverlay::getCurrentFrameFromParent() const
{
    qDebug() << "getCurrentFrameFromParent called for view:" << m_viewName;

    // 通过父窗口获取当前帧
    QWidget* parentWidget = this->parentWidget();
    while (parentWidget && !qobject_cast<QMainWindow*>(parentWidget)) {
        parentWidget = parentWidget->parentWidget();
    }

    if (auto* mainWindow = qobject_cast<QMainWindow*>(parentWidget)) {
        // 尝试转换为MutiCamApp
        if (auto* mutiCamApp = qobject_cast<MutiCamApp*>(mainWindow)) {
            cv::Mat frame = mutiCamApp->getCurrentFrame(m_viewName);
            qDebug() << "获取到的帧尺寸：" << frame.cols << "x" << frame.rows;
            return frame;
        } else {
            qDebug() << "无法转换为MutiCamApp";
        }
    } else {
        qDebug() << "无法找到主窗口";
    }

    return cv::Mat();
}

cv::Mat PaintingOverlay::extractROIImage(const cv::Mat& sourceImage) const
{
    // 检查是否有有效的ROI
    if (!m_hasCurrentROI || !m_currentROI.rect.isValid() || sourceImage.empty()) {
        qWarning() << "无法提取ROI图像：ROI无效或源图像为空";
        return cv::Mat();
    }

    QRectF roiRect = m_currentROI.rect;
    double roiAngle = m_currentROI.angle;

    // 确保ROI在图像范围内
    if (roiRect.x() < 0 || roiRect.y() < 0 ||
        roiRect.right() > sourceImage.cols || roiRect.bottom() > sourceImage.rows) {
        qWarning() << "ROI区域超出图像范围";
        return cv::Mat();
    }

    try {
        // 如果没有旋转，直接提取矩形区域
        if (qAbs(roiAngle) < 0.01) {
            cv::Rect cvRect(
                static_cast<int>(roiRect.x()),
                static_cast<int>(roiRect.y()),
                static_cast<int>(roiRect.width()),
                static_cast<int>(roiRect.height())
            );
            return sourceImage(cvRect).clone();
        }

        // 如果有旋转，需要进行旋转变换
        cv::Point2f center(roiRect.center().x(), roiRect.center().y());
        cv::Size2f size(roiRect.width(), roiRect.height());

        // 创建旋转矩阵
        cv::Mat rotationMatrix = cv::getRotationMatrix2D(center, roiAngle, 1.0);

        // 计算旋转后的边界框
        cv::Rect2f boundingRect = cv::RotatedRect(center, size, roiAngle).boundingRect2f();

        // 调整旋转矩阵以包含完整的旋转区域
        rotationMatrix.at<double>(0, 2) += boundingRect.width / 2.0 - center.x;
        rotationMatrix.at<double>(1, 2) += boundingRect.height / 2.0 - center.y;

        // 执行旋转变换
        cv::Mat rotatedImage;
        cv::warpAffine(sourceImage, rotatedImage, rotationMatrix, boundingRect.size());

        // 从旋转后的图像中提取中心区域
        cv::Point2f newCenter(boundingRect.width / 2.0, boundingRect.height / 2.0);
        cv::Rect extractRect(
            static_cast<int>(newCenter.x - size.width / 2.0),
            static_cast<int>(newCenter.y - size.height / 2.0),
            static_cast<int>(size.width),
            static_cast<int>(size.height)
        );

        // 确保提取区域在旋转图像范围内
        extractRect &= cv::Rect(0, 0, rotatedImage.cols, rotatedImage.rows);

        if (extractRect.width > 0 && extractRect.height > 0) {
            return rotatedImage(extractRect).clone();
        }

    } catch (const cv::Exception& e) {
        qCritical() << "OpenCV异常：" << e.what();
    } catch (const std::exception& e) {
        qCritical() << "提取ROI图像时发生异常：" << e.what();
    }

    return cv::Mat();
}

void PaintingOverlay::performLineDetection(const cv::Mat& frame, const cv::Rect& roi)
{
    qDebug() << "开始直线检测...";
    QTime startTime = QTime::currentTime();

    // 1. 边缘检测
    cv::Mat edges;
    if (!m_edgeDetector->detectEdgesInROI(frame, roi, edges)) {
        QString errorMsg = "直线检测失败：边缘检测错误";
        qDebug() << errorMsg;
        emit measurementCompleted(m_viewName, errorMsg);

        QWidget* parentWidget = this->parentWidget();
        while (parentWidget && !qobject_cast<QMainWindow*>(parentWidget)) {
            parentWidget = parentWidget->parentWidget();
        }
        if (parentWidget) {
            QMessageBox::warning(parentWidget, "边缘检测失败",
                               "无法在选定区域检测到边缘，请尝试：\n"
                               "1. 调整Canny边缘检测参数\n"
                               "2. 选择包含更明显边缘的区域\n"
                               "3. 改善图像光照条件");
        }

        // 检测失败时也删除ROI框
        removeLastROI();
        return;
    }

    // 检查边缘图像是否有足够的边缘点
    int edgePixels = cv::countNonZero(edges);
    if (edgePixels < 50) {
        QString errorMsg = QString("直线检测失败：边缘点过少(%1个)").arg(edgePixels);
        qDebug() << errorMsg;
        emit measurementCompleted(m_viewName, "直线检测失败：检测区域边缘特征不足");

        QWidget* parentWidget = this->parentWidget();
        while (parentWidget && !qobject_cast<QMainWindow*>(parentWidget)) {
            parentWidget = parentWidget->parentWidget();
        }
        if (parentWidget) {
            QMessageBox::information(parentWidget, "边缘特征不足",
                                   "选定区域的边缘特征不足以进行直线检测，建议：\n"
                                   "1. 降低Canny边缘检测阈值\n"
                                   "2. 选择包含更清晰直线的区域");
        }

        // 检测失败时也删除ROI框
        removeLastROI();
        return;
    }

    // 2. 直线检测
    QVector<ShapeDetector::DetectedLine> detectedLines;
    if (!m_shapeDetector->detectLinesInROI(edges, roi, detectedLines)) {
        QString errorMsg = "直线检测失败：霍夫变换检测错误";
        qDebug() << errorMsg;
        emit measurementCompleted(m_viewName, errorMsg);

        QWidget* parentWidget = this->parentWidget();
        while (parentWidget && !qobject_cast<QMainWindow*>(parentWidget)) {
            parentWidget = parentWidget->parentWidget();
        }
        if (parentWidget) {
            QMessageBox::warning(parentWidget, "直线检测失败",
                               "霍夫变换直线检测失败，请检查：\n"
                               "1. 直线检测参数设置\n"
                               "2. 图像处理器状态");
        }

        // 检测失败时也删除ROI框
        removeLastROI();
        return;
    }

    if (detectedLines.isEmpty()) {
        QString infoMsg = "直线检测完成：未发现符合条件的直线";
        qDebug() << infoMsg;
        emit measurementCompleted(m_viewName, infoMsg);

        QWidget* parentWidget = this->parentWidget();
        while (parentWidget && !qobject_cast<QMainWindow*>(parentWidget)) {
            parentWidget = parentWidget->parentWidget();
        }
        if (parentWidget) {
            QMessageBox::information(parentWidget, "未检测到直线",
                                   "在选定区域未检测到符合条件的直线，建议：\n"
                                   "1. 降低直线检测阈值\n"
                                   "2. 减小最小线段长度要求\n"
                                   "3. 增大最大线段间隙\n"
                                   "4. 选择包含更明显直线的区域");
        }

        // 检测失败时也删除ROI框
        removeLastROI();
        return;
    }

    qDebug() << QString("检测到 %1 条直线").arg(detectedLines.size());

    // 3. 选择最佳直线
    ShapeDetector::DetectedLine bestLine = m_shapeDetector->selectBestLine(detectedLines);

    // 4. 转换为LineObject并添加到绘图数据
    LineObject detectedLineObj;
    detectedLineObj.points.append(QPointF(bestLine.start.x(), bestLine.start.y()));
    detectedLineObj.points.append(QPointF(bestLine.end.x(), bestLine.end.y()));
    detectedLineObj.isCompleted = true;
    detectedLineObj.color = Qt::magenta; // 紫色表示自动检测结果
    detectedLineObj.thickness = 2;
    QString lengthStr = formatDistance(bestLine.length);
    QString startCoordStr = formatCoordinate(QPointF(bestLine.start.x(), bestLine.start.y()));
    QString endCoordStr = formatCoordinate(QPointF(bestLine.end.x(), bestLine.end.y()));
    detectedLineObj.label = QString("自动检测直线 (长度: %1, 角度: %2°, 起点: %3, 终点: %4, 置信度: %5)")
                           .arg(lengthStr).arg(bestLine.angle, 0, 'f', 1).arg(startCoordStr).arg(endCoordStr).arg(bestLine.confidence, 0, 'f', 1);
    detectedLineObj.showLength = true;
    detectedLineObj.length = bestLine.length;

    // 添加到线段列表
    m_lines.append(detectedLineObj);

    // 记录历史
    DrawingAction action;
    action.type = DrawingAction::AddLine;
    action.source = DrawingAction::AutoDetection;  // 自动检测
    action.index = m_lines.size() - 1;
    commitDrawingAction(action);

    // 检测成功后删除对应的ROI框（与Python版本行为一致）
    removeLastROI();

    // 发出信号
    QString result = QString("自动直线检测成功：长度 %1 像素，角度 %2° (共检测到%3条直线)")
                    .arg(bestLine.length, 0, 'f', 1).arg(bestLine.angle, 0, 'f', 1).arg(detectedLines.size());
    emit measurementCompleted(m_viewName, result);
    emit drawingDataChanged(m_viewName);

    int elapsedMs = startTime.msecsTo(QTime::currentTime());
    qDebug() << "直线检测完成：" << result;
    qDebug() << QString("检测统计 - 边缘点数: %1, 候选直线: %2, 最佳直线置信度: %.2f, 耗时: %3ms")
                .arg(edgePixels).arg(detectedLines.size()).arg(bestLine.confidence).arg(elapsedMs);
    update();
}

void PaintingOverlay::performCircleDetection(const cv::Mat& frame, const cv::Rect& roi)
{
    qDebug() << "开始圆形检测...";
    QTime startTime = QTime::currentTime();

    // 1. 预处理图像（转换为灰度图）
    cv::Mat grayFrame;
    if (!EdgeDetector::preprocessImage(frame, grayFrame)) {
        QString errorMsg = "圆形检测失败：图像预处理错误";
        qDebug() << errorMsg;
        emit measurementCompleted(m_viewName, errorMsg);

        QWidget* parentWidget = this->parentWidget();
        while (parentWidget && !qobject_cast<QMainWindow*>(parentWidget)) {
            parentWidget = parentWidget->parentWidget();
        }
        if (parentWidget) {
            QMessageBox::warning(parentWidget, "图像预处理失败",
                               "无法将图像转换为灰度图，请检查图像格式。");
        }

        // 检测失败时也删除ROI框
        removeLastROI();
        return;
    }

    // 2. 圆形检测
    QVector<ShapeDetector::DetectedCircle> detectedCircles;
    if (!m_shapeDetector->detectCirclesInROI(grayFrame, roi, detectedCircles)) {
        QString errorMsg = "圆形检测失败：霍夫圆变换检测错误";
        qDebug() << errorMsg;
        emit measurementCompleted(m_viewName, errorMsg);

        QWidget* parentWidget = this->parentWidget();
        while (parentWidget && !qobject_cast<QMainWindow*>(parentWidget)) {
            parentWidget = parentWidget->parentWidget();
        }
        if (parentWidget) {
            QMessageBox::warning(parentWidget, "圆形检测失败",
                               "霍夫圆变换检测失败，请检查：\n"
                               "1. 圆形检测参数设置\n"
                               "2. 图像处理器状态");
        }

        // 检测失败时也删除ROI框
        removeLastROI();
        return;
    }

    if (detectedCircles.isEmpty()) {
        QString infoMsg = "圆形检测完成：未发现符合条件的圆形";
        qDebug() << infoMsg;
        emit measurementCompleted(m_viewName, infoMsg);

        QWidget* parentWidget = this->parentWidget();
        while (parentWidget && !qobject_cast<QMainWindow*>(parentWidget)) {
            parentWidget = parentWidget->parentWidget();
        }
        if (parentWidget) {
            QMessageBox::information(parentWidget, "未检测到圆形",
                                   "在选定区域未检测到符合条件的圆形，建议：\n"
                                   "1. 降低圆形检测阈值(Param2)\n"
                                   "2. 调整Canny边缘检测阈值(Param1)\n"
                                   "3. 检查半径范围设置\n"
                                   "4. 选择包含更明显圆形的区域");
        }

        // 检测失败时也删除ROI框
        removeLastROI();
        return;
    }

    qDebug() << QString("检测到 %1 个圆形").arg(detectedCircles.size());

    // 3. 选择最佳圆形
    ShapeDetector::DetectedCircle bestCircle = m_shapeDetector->selectBestCircle(detectedCircles);

    // 4. 转换为CircleObject并添加到绘图数据
    CircleObject detectedCircleObj;
    detectedCircleObj.points.append(QPointF(bestCircle.center.x(), bestCircle.center.y()));
    detectedCircleObj.points.append(QPointF(bestCircle.center.x() + bestCircle.radius, bestCircle.center.y()));
    detectedCircleObj.isCompleted = true;
    detectedCircleObj.color = Qt::magenta; // 紫色表示自动检测结果
    detectedCircleObj.thickness = 2;
    detectedCircleObj.center = QPointF(bestCircle.center.x(), bestCircle.center.y());
    detectedCircleObj.radius = bestCircle.radius;

    QString radiusStr = formatRadius(bestCircle.radius);
    QString centerCoordStr = formatCoordinate(QPointF(bestCircle.center.x(), bestCircle.center.y()));
    detectedCircleObj.label = QString("自动检测圆形 (%1, 中心: %2, 置信度: %3)")
                             .arg(radiusStr).arg(centerCoordStr).arg(bestCircle.confidence, 0, 'f', 1);

    // 添加到圆形列表
    m_circles.append(detectedCircleObj);

    // 记录历史
    DrawingAction action;
    action.type = DrawingAction::AddCircle;
    action.source = DrawingAction::AutoDetection;  // 自动检测
    action.index = m_circles.size() - 1;
    commitDrawingAction(action);

    // 检测成功后删除对应的ROI框（与Python版本行为一致）
    removeLastROI();

    // 发出信号
    QString result = QString("自动圆形检测成功：半径 %1 像素，置信度 %2 (共检测到%3个圆形)")
                    .arg(bestCircle.radius).arg(bestCircle.confidence, 0, 'f', 1).arg(detectedCircles.size());
    emit measurementCompleted(m_viewName, result);
    emit drawingDataChanged(m_viewName);

    int elapsedMs = startTime.msecsTo(QTime::currentTime());
    qDebug() << "圆形检测完成：" << result;
    qDebug() << QString("检测统计 - 候选圆形: %1, 最佳圆形中心: (%2,%3), 耗时: %4ms")
                .arg(detectedCircles.size()).arg(bestCircle.center.x()).arg(bestCircle.center.y()).arg(elapsedMs);
    update();

    // 如果处于圆标定模式，使用检测到的圆进行标定
    if (m_isCircleCalibrationMode) {
        performCircleCalibration(bestCircle.radius);
    }
}

// 模板方法实现：按索引删除容器中的元素
template<typename T>
void PaintingOverlay::removeItemsByIndices(QVector<T>& container, const QSet<int>& indicesToRemove)
{
    if (indicesToRemove.isEmpty()) {
        return;
    }

    // 将索引转换为列表并排序（从大到小，避免删除时索引变化）
    QList<int> sortedIndices = indicesToRemove.values();
    std::sort(sortedIndices.begin(), sortedIndices.end(), std::greater<int>());

    // 从后往前删除
    for (int index : sortedIndices) {
        if (index >= 0 && index < container.size()) {
            container.removeAt(index);
        }
    }
}

void PaintingOverlay::setEdgeDetectionParams(const EdgeDetector::EdgeDetectionParams& params)
{
    if (m_edgeDetector) {
        m_edgeDetector->setParams(params);
        qDebug() << "边缘检测参数已更新";
    }
}

void PaintingOverlay::setLineDetectionParams(const ShapeDetector::LineDetectionParams& params)
{
    if (m_shapeDetector) {
        m_shapeDetector->setLineDetectionParams(params);
        qDebug() << "直线检测参数已更新";
    }
}

void PaintingOverlay::setCircleDetectionParams(const ShapeDetector::CircleDetectionParams& params)
{
    if (m_shapeDetector) {
        m_shapeDetector->setCircleDetectionParams(params);
        qDebug() << "圆形检测参数已更新";
    }
}

void PaintingOverlay::renderToImage(QPainter& painter, const QSize& imageSize)
{
    // 设置临时的图像尺寸和变换参数
    QSize originalImageSize = m_imageSize;
    QPointF originalOffset = m_imageOffset;
    double originalScale = m_scaleFactor;

    // 为渲染设置1:1映射
    m_imageSize = imageSize;
    m_imageOffset = QPointF(0, 0);
    m_scaleFactor = 1.0;

    try {
        // 使用与paintEvent相同的绘制逻辑
        if (!m_imageSize.isEmpty()) {
            painter.save();

            // 设置坐标变换（1:1映射，无偏移）
            QTransform transform;
            transform.translate(m_imageOffset.x(), m_imageOffset.y());
            transform.scale(m_scaleFactor, m_scaleFactor);
            painter.setTransform(transform);

            // 设置裁剪区域
            painter.setClipRect(QRect(0, 0, m_imageSize.width(), m_imageSize.height()));

            // 检查是否需要更新DrawingContext缓存
            if (needsDrawingContextUpdate()) {
                updateDrawingContext();
            }

            // 使用缓存的DrawingContext
            const DrawingContext& ctx = m_cachedDrawingContext;

            // 绘制所有图形元素（与paintEvent相同的顺序）
            // 0. 首先绘制网格（作为最底层背景）
            drawGrid(painter, ctx);

            // 1. 先绘制选中状态高亮（作为底层外描边）
            drawSelectionHighlights(painter);

            // 2. 按历史顺序绘制所有已完成的图形（在高亮之上）
            drawObjectsByHistory(painter, ctx);

            // 3. 绘制当前正在预览的图形（在最上层）
            if (m_isDrawingMode) {
                drawCurrentPreview(painter, ctx);
            }

            painter.restore();
        }
    } catch (const std::exception& e) {
        qDebug() << "renderToImage 出错：" << e.what();
    }

    // 恢复原始的变换参数
    m_imageSize = originalImageSize;
    m_imageOffset = originalOffset;
    m_scaleFactor = originalScale;
}

// 像素标定相关函数实现
void PaintingOverlay::setPixelScale(double scale, const QString& unit)
{
    if (scale > 0.0) {
        m_pixelScale = scale;
        m_unit = unit;
        m_isCalibrated = true;

        qDebug() << QString("视图 %1 像素标定完成: %2 %3/pixel")
                    .arg(m_viewName).arg(scale, 0, 'f', 6).arg(unit);

        // 更新所有现有测量结果的显示
        updateAllMeasurementLabels();
        update();
    }
}

double PaintingOverlay::getPixelScale() const
{
    return m_pixelScale;
}

QString PaintingOverlay::getUnit() const
{
    return m_unit;
}

bool PaintingOverlay::isCalibrated() const
{
    return m_isCalibrated;
}

void PaintingOverlay::startCalibration()
{
    m_isCalibrationMode = true;
    m_isMultiPointCalibrationMode = false;
    m_isCircleCalibrationMode = false;
    startDrawing(DrawingTool::LineSegment);

    QString message = QString("视图 %1 进入单点标定模式，请绘制一条已知长度的线段").arg(m_viewName);
    emit measurementCompleted(m_viewName, message);
    qDebug() << message;
}

void PaintingOverlay::startMultiPointCalibration()
{
    m_isMultiPointCalibrationMode = true;
    m_isCalibrationMode = false;
    m_isCircleCalibrationMode = false;
    m_calibrationPoints.clear();
    m_multiPointCalibrationUnit.clear(); // 清空之前的单位设置
    startDrawing(DrawingTool::LineSegment);

    QString message = QString("视图 %1 进入多点标定模式，请绘制多条已知长度的线段（建议3-5条）").arg(m_viewName);
    emit measurementCompleted(m_viewName, message);
    qDebug() << message;
}


void PaintingOverlay::startCircleCalibration()
{
    m_isCircleCalibrationMode = true;
    m_isCalibrationMode = false;
    m_isMultiPointCalibrationMode = false;

    // 使用 ROI 圆检测工具
    startDrawing(DrawingTool::ROI_CircleDetect);

    QString message = QString("视图 %1 进入圆标定模式，请框选包含已知直径圆形的ROI，松开鼠标后自动检测圆形").arg(m_viewName);
    emit measurementCompleted(m_viewName, message);
    qDebug() << message;
}

void PaintingOverlay::resetCalibration()
{
    m_pixelScale = 1.0;
    m_unit = "μm";
    m_isCalibrated = false;
    m_isCalibrationMode = false;
    m_isMultiPointCalibrationMode = false;
    m_isCircleCalibrationMode = false;
    m_calibrationPoints.clear();

    qDebug() << QString("视图 %1 标定已重置").arg(m_viewName);

    // 更新所有现有测量结果的显示
    updateAllMeasurementLabels();
    update();
}

void PaintingOverlay::updateAllMeasurementLabels()
{
    // 更新线段标签 - 保持原有标签类型
    for (auto& lineSegment : m_lineSegments) {
        if (lineSegment.isCompleted && lineSegment.points.size() >= 2) {
            double pixelLength = lineSegment.length;
            // 特殊处理角平分线标签，更新其坐标部分
            if (lineSegment.label.contains("BISECTOR:")) {
                // 解析角平分线标签格式：BISECTOR:角度°:坐标
                QStringList parts = lineSegment.label.split(":");
                if (parts.size() >= 3) {
                    QString anglePart = parts[1]; // 角度部分
                    // 角平分线是从交点向两个方向延伸5000像素创建的
                    // 所以交点就是线段的中点
                    QPointF intersection = (lineSegment.points[0] + lineSegment.points[1]) / 2.0;
                    lineSegment.label = QString("BISECTOR:%1:%2").arg(anglePart).arg(formatCoordinate(intersection));
                }
                continue;
            }
            // 根据原标签判断是距离还是长度
            if (lineSegment.label.contains("距离:")) {
                lineSegment.label = QString("距离: %1").arg(formatDistance(pixelLength));
            } else {
                lineSegment.label = QString("长度: %1").arg(formatDistance(pixelLength));
            }
        }
    }

    // 更新直线长度标签
    for (auto& line : m_lines) {
        if (line.isCompleted && line.showLength) {
            double pixelLength = line.length;
            line.label = QString("长度: %1").arg(formatDistance(pixelLength));
        }
    }

    // 更新圆形半径标签
    for (auto& circle : m_circles) {
        if (circle.isCompleted) {
            double pixelRadius = circle.radius;
            circle.label = QString("R=%1").arg(formatRadius(pixelRadius));
        }
    }

    // 更新精细圆半径标签
    for (auto& fineCircle : m_fineCircles) {
        if (fineCircle.isCompleted) {
            double pixelRadius = fineCircle.radius;
            fineCircle.label = QString("R=%1").arg(formatRadius(pixelRadius));
        }
    }

    // 更新平行线距离标签
    for (auto& parallel : m_parallels) {
        if (parallel.isCompleted) {
            double pixelDistance = parallel.distance;
            parallel.label = QString("距离: %1").arg(formatDistance(pixelDistance));
        }
    }
}

void PaintingOverlay::performCalibrationWithLineSegment(int lineSegmentIndex)
{
    if (lineSegmentIndex < 0 || lineSegmentIndex >= m_lineSegments.size()) {
        return;
    }

    const LineSegmentObject& lineSegment = m_lineSegments[lineSegmentIndex];
    double pixelLength = lineSegment.length;

    // 首先让用户选择单位
    QStringList units = {"微米", "毫米", "厘米"};
    // QStringList unitCodes = {"μm", "mm", "cm"};
    bool unitOk;
    QString selectedUnit = QInputDialog::getItem(this,
                                                QString("像素标定 - %1").arg(m_viewName),
                                                "请选择测量单位:",
                                                units,
                                                0,
                                                false,
                                                &unitOk);
    if (unitOk) {
        // 转换中文单位为代码
        int index = units.indexOf(selectedUnit);
        if (index >= 0) {
            selectedUnit = units[index];
        }
    }
    
    if (!unitOk) {
        return; // 用户取消了单位选择
    }

    // 使用QInputDialog获取实际长度
    bool ok;
    QString inputText = QString("线段像素长度: %1 像素\n请输入实际长度(%2):").arg(pixelLength, 0, 'f', 2).arg(selectedUnit);

    double realLength = QInputDialog::getDouble(this,
                                               QString("像素标定 - %1").arg(m_viewName),
                                               inputText,
                                               100.0,    // 默认值
                                               0.001,    // 最小值
                                               999999.0, // 最大值
                                               3,        // 小数位数
                                               &ok);

    if (ok && realLength > 0.0) {
        // 计算像素比例
        double scale = realLength / pixelLength;

        // 直接使用用户选择的单位
        setPixelScale(scale, selectedUnit);

        // 发送标定完成信号
        QString chineseUnit = selectedUnit;
        if (chineseUnit == "μm") chineseUnit = "微米";
        else if (chineseUnit == "mm") chineseUnit = "毫米";
        else if (chineseUnit == "cm") chineseUnit = "厘米";
        QString result = QString("像素标定完成: %1 %2/像素\n像素长度: %3 像素, 实际长度: %4 %5")
                        .arg(scale, 0, 'f', 6).arg(chineseUnit)
                        .arg(pixelLength, 0, 'f', 2)
                        .arg(realLength, 0, 'f', 2).arg(chineseUnit);
        emit measurementCompleted(m_viewName, result);

        qDebug() << QString("视图 %1 标定完成: %2").arg(m_viewName).arg(result);
    } else {
        // 标定取消，重置标定模式
        m_isCalibrationMode = false;
        QString message = QString("视图 %1 标定已取消").arg(m_viewName);
        emit measurementCompleted(m_viewName, message);
        qDebug() << message;
    }
}

void PaintingOverlay::performMultiPointCalibrationWithLineSegment(int lineSegmentIndex)
{
    if (lineSegmentIndex < 0 || lineSegmentIndex >= m_lineSegments.size()) {
        return;
    }

    const LineSegmentObject& lineSegment = m_lineSegments[lineSegmentIndex];
    double pixelLength = lineSegment.length;

    // 如果是第一个标定点，让用户选择单位
    QString selectedUnit = m_multiPointCalibrationUnit; // 使用多点标定的单位
    if (m_calibrationPoints.isEmpty()) {
        QStringList units = {"微米", "毫米", "厘米"};
        // QStringList unitCodes = {"μm", "mm", "cm"};
        bool unitOk;
        selectedUnit = QInputDialog::getItem(this,
                                            QString("多点标定 - %1").arg(m_viewName),
                                            "请选择测量单位:",
                                            units,
                                            0, // 默认选择微米
                                            false,
                                            &unitOk);
        if (unitOk) {
            // 转换中文单位为代码
            int index = units.indexOf(selectedUnit);
            if (index >= 0) {
                selectedUnit = units[index];
            }
        }
        
        if (!unitOk) {
            return; // 用户取消了单位选择
        }
        
        // 保存选择的单位
        m_multiPointCalibrationUnit = selectedUnit;
    }

    // 使用QInputDialog获取实际长度
    bool ok;
    QString inputText = QString("线段 %1 像素长度: %2 像素\n请输入实际长度(%3):")
                       .arg(m_calibrationPoints.size() + 1)
                       .arg(pixelLength, 0, 'f', 2)
                       .arg(selectedUnit);

    double realLength = QInputDialog::getDouble(this,
                                               QString("多点标定 - %1").arg(m_viewName),
                                               inputText,
                                               100.0,    // 默认值
                                               0.001,    // 最小值
                                               999999.0, // 最大值
                                               3,        // 小数位数
                                               &ok);

    if (ok && realLength > 0.0) {
        // 添加标定点
        CalibrationPoint point;
        point.lineSegmentIndex = lineSegmentIndex;
        point.pixelLength = pixelLength;
        point.realLength = realLength;
        point.isValid = true;
        m_calibrationPoints.append(point);

        QString chineseUnit = selectedUnit;
        if (chineseUnit == "μm") chineseUnit = "微米";
        else if (chineseUnit == "mm") chineseUnit = "毫米";
        else if (chineseUnit == "cm") chineseUnit = "厘米";
        QString result = QString("已添加标定点 %1: 像素长度 %2 像素, 实际长度 %3 %4")
                        .arg(m_calibrationPoints.size())
                        .arg(pixelLength, 0, 'f', 2)
                        .arg(realLength, 0, 'f', 2)
                        .arg(selectedUnit);
        emit measurementCompleted(m_viewName, result);

        // 询问是否继续添加标定点或完成标定
        showMultiPointCalibrationDialog();

        qDebug() << QString("视图 %1 添加标定点: %2").arg(m_viewName).arg(result);
    } else {
        // 取消当前标定点，但不退出多点标定模式
        QString message = QString("视图 %1 标定点输入已取消").arg(m_viewName);
        emit measurementCompleted(m_viewName, message);
        qDebug() << message;
    }
}

void PaintingOverlay::performCircleCalibration(double pixelRadius)
{
    if (pixelRadius <= 0.0) {
        qDebug() << "圆标定失败：像素半径无效";
        QString message = QString("视图 %1 圆标定失败：检测到的圆半径无效").arg(m_viewName);
        emit measurementCompleted(m_viewName, message);
        return;
    }

    double pixelDiameter = 2.0 * pixelRadius;

    // 首先让用户选择单位
    QStringList units = {"微米", "毫米", "厘米"};
    bool unitOk;
    QString selectedUnit = QInputDialog::getItem(this,
                                                QString("圆标定 - %1").arg(m_viewName),
                                                "请选择测量单位:",
                                                units,
                                                0,
                                                false,
                                                &unitOk);
    if (unitOk) {
        // 转换中文单位为代码（目前保持中文，方便显示）
        int index = units.indexOf(selectedUnit);
        if (index >= 0) {
            selectedUnit = units[index];
        }
    }

    if (!unitOk) {
        // 用户取消
        m_isCircleCalibrationMode = false;
        QString message = QString("视图 %1 圆标定已取消").arg(m_viewName);
        emit measurementCompleted(m_viewName, message);
        qDebug() << message;
        return;
    }

    // 使用QInputDialog获取实际直径
    bool ok;
    QString inputText = QString("检测到的圆像素半径: %1 像素\n像素直径: %2 像素\n请输入实际直径(%3):")
                       .arg(pixelRadius, 0, 'f', 2)
                       .arg(pixelDiameter, 0, 'f', 2)
                       .arg(selectedUnit);

    double realDiameter = QInputDialog::getDouble(this,
                                                 QString("圆标定 - %1").arg(m_viewName),
                                                 inputText,
                                                 100.0,    // 默认值
                                                 0.001,    // 最小值
                                                 999999.0, // 最大值
                                                 3,        // 小数位数
                                                 &ok);

    if (ok && realDiameter > 0.0) {
        // 计算像素比例：实际直径 / 像素直径
        double scale = realDiameter / pixelDiameter;

        // 直接使用用户选择的单位
        setPixelScale(scale, selectedUnit);

        QString chineseUnit = selectedUnit;
        if (chineseUnit == "μm") chineseUnit = "微米";
        else if (chineseUnit == "mm") chineseUnit = "毫米";
        else if (chineseUnit == "cm") chineseUnit = "厘米";

        QString result = QString("圆标定完成: %1 %2/像素\n像素半径: %3 像素, 像素直径: %4 像素, 实际直径: %5 %6")
                        .arg(scale, 0, 'f', 6).arg(chineseUnit)
                        .arg(pixelRadius, 0, 'f', 2)
                        .arg(pixelDiameter, 0, 'f', 2)
                        .arg(realDiameter, 0, 'f', 2).arg(chineseUnit);
        emit measurementCompleted(m_viewName, result);

        qDebug() << QString("视图 %1 圆标定完成: %2").arg(m_viewName).arg(result);
    } else {
        QString message = QString("视图 %1 圆标定已取消").arg(m_viewName);
        emit measurementCompleted(m_viewName, message);
        qDebug() << message;
    }

    // 无论成功或取消，都结束圆标定模式并恢复选择模式
    m_isCircleCalibrationMode = false;
    stopDrawing();
}

void PaintingOverlay::showMultiPointCalibrationDialog()
{
    // 构建对话框内容
    QString dialogText;
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(QString("多点标定 - %1").arg(m_viewName));
    msgBox.setIcon(QMessageBox::Question);

    QPushButton* finishButton = nullptr;
    QPushButton* continueButton = nullptr;
    QPushButton* cancelButton = nullptr;

    if (m_calibrationPoints.size() < 2) {
        // 标定点不足时的对话框
        dialogText = QString("当前已有 %1 个标定点，至少需要2个标定点才能完成标定。\n\n").arg(m_calibrationPoints.size());

        if (m_calibrationPoints.size() > 0) {
            for (int i = 0; i < m_calibrationPoints.size(); ++i) {
                const CalibrationPoint& point = m_calibrationPoints[i];
                double ratio = point.realLength / point.pixelLength;
                dialogText += QString("点 %1: %2 像素 → %3 微米 (比例: %4)\n")
                             .arg(i + 1)
                             .arg(point.pixelLength, 0, 'f', 2)
                             .arg(point.realLength, 0, 'f', 2)
                             .arg(ratio, 0, 'f', 6);
            }
            dialogText += "\n";
        }

        dialogText += "选择操作:";
        msgBox.setText(dialogText);

        continueButton = msgBox.addButton("继续添加", QMessageBox::AcceptRole);
        cancelButton = msgBox.addButton("取消标定", QMessageBox::RejectRole);
    } else {
        // 标定点足够时的对话框（原有逻辑）
        // 标定点足够时的对话框（原有逻辑）
        dialogText = QString("当前已有 %1 个标定点:\n").arg(m_calibrationPoints.size());
        for (int i = 0; i < m_calibrationPoints.size(); ++i) {
            const CalibrationPoint& point = m_calibrationPoints[i];
            double ratio = point.realLength / point.pixelLength;
            dialogText += QString("点 %1: %2 像素 → %3 微米 (比例: %4)\n")
                         .arg(i + 1)
                         .arg(point.pixelLength, 0, 'f', 2)
                         .arg(point.realLength, 0, 'f', 2)
                         .arg(ratio, 0, 'f', 6);
        }

        double avgScale = calculateMultiPointPixelScale();
        dialogText += QString("\n计算得到的平均像素比例: %1 微米/像素\n\n").arg(avgScale, 0, 'f', 6);
        dialogText += "选择操作:";
        msgBox.setText(dialogText);

        finishButton = msgBox.addButton("完成标定", QMessageBox::AcceptRole);
        continueButton = msgBox.addButton("继续添加", QMessageBox::RejectRole);
        cancelButton = msgBox.addButton("取消标定", QMessageBox::DestructiveRole);
    }

    msgBox.exec();

    if (msgBox.clickedButton() == finishButton && finishButton != nullptr) {
        // 完成多点标定
        double avgScale = calculateMultiPointPixelScale();
        // 使用多点标定选择的单位
        setPixelScale(avgScale, m_multiPointCalibrationUnit);
        m_isMultiPointCalibrationMode = false;

        QString result = QString("多点标定完成: %1 %2/pixel\n使用了 %3 个标定点")
                        .arg(avgScale, 0, 'f', 6)
                        .arg(m_multiPointCalibrationUnit)
                        .arg(m_calibrationPoints.size());
        emit measurementCompleted(m_viewName, result);

        qDebug() << QString("视图 %1 多点标定完成: %2").arg(m_viewName).arg(result);

        // 停止绘制模式
        stopDrawing();
    } else if (msgBox.clickedButton() == continueButton && continueButton != nullptr) {
        // 继续添加标定点，绘制模式已经保持，无需重新启动
        QString message = QString("请继续绘制线段添加标定点（当前 %1 个）").arg(m_calibrationPoints.size());
        emit measurementCompleted(m_viewName, message);
    } else {
        // 取消标定
        m_isMultiPointCalibrationMode = false;
        m_calibrationPoints.clear();

        QString message = QString("视图 %1 多点标定已取消").arg(m_viewName);
        emit measurementCompleted(m_viewName, message);
        qDebug() << message;

        // 停止绘制模式
        stopDrawing();
    }
}

double PaintingOverlay::calculateMultiPointPixelScale() const
{
    if (m_calibrationPoints.isEmpty()) {
        return 1.0;
    }

    // 计算所有有效标定点的像素比例平均值
    double totalScale = 0.0;
    int validCount = 0;

    for (const CalibrationPoint& point : m_calibrationPoints) {
        if (point.isValid && point.pixelLength > 0.0) {
            double scale = point.realLength / point.pixelLength;
            totalScale += scale;
            validCount++;
        }
    }

    if (validCount > 0) {
        return totalScale / validCount;
    }

    return 1.0;
}



QString PaintingOverlay::formatDistance(double pixelDistance) const
{
    if (m_isCalibrated) {
        double realDistance = pixelDistance * m_pixelScale;
        QString chineseUnit = m_unit;
        if (chineseUnit == "μm") chineseUnit = "微米";
        else if (chineseUnit == "mm") chineseUnit = "毫米";
        else if (chineseUnit == "cm") chineseUnit = "厘米";
        return QString("%1 %2").arg(realDistance, 0, 'f', 2).arg(chineseUnit);
    } else {
        return QString("%1 像素").arg(pixelDistance, 0, 'f', 1);
    }
}

QString PaintingOverlay::formatCoordinate(const QPointF& pixelCoord) const
{
    if (m_isCalibrated) {
        double realX = pixelCoord.x() * m_pixelScale;
        double realY = pixelCoord.y() * m_pixelScale;
        return QString("(%1,%2)").arg(realX, 0, 'f', 2).arg(realY, 0, 'f', 2);
    } else {
        return QString("(%1,%2)").arg(pixelCoord.x(), 0, 'f', 1).arg(pixelCoord.y(), 0, 'f', 1);
    }
}

QString PaintingOverlay::formatRadius(double pixelRadius) const
{
    if (m_isCalibrated) {
        double realRadius = pixelRadius * m_pixelScale;
        QString chineseUnit = m_unit;
        if (chineseUnit == "μm") chineseUnit = "微米";
        else if (chineseUnit == "mm") chineseUnit = "毫米";
        else if (chineseUnit == "cm") chineseUnit = "厘米";
        return QString("R=%1 %2").arg(realRadius, 0, 'f', 2).arg(chineseUnit);
    } else {
        return QString("R=%1 像素").arg(pixelRadius, 0, 'f', 1);
    }
}

// ROI功能实现
void PaintingOverlay::startROICreation()
{
    qDebug() << "PaintingOverlay::startROICreation() - 视图:" << m_viewName;

    // 设置ROI创建模式
    m_roiCreationMode = true;
    m_currentDrawingTool = DrawingTool::ROI_CREATION;
    m_isDrawingMode = true;

    // 计算图像区域
    QRect imageRect = QRect(m_imageOffset.toPoint(), m_imageSize);
    if (imageRect.isEmpty()) {
        // 如果没有图像信息，使用控件大小
        imageRect = rect();
    }

    // 在图像中心创建默认大小的正方形ROI
    QPoint center = imageRect.center();
    int defaultSize = qMin(imageRect.width(), imageRect.height()) / 4;
    defaultSize = qMax(defaultSize, 100); // 最小100像素
    
    // 确保默认尺寸不超出图像边界
    if (!m_imageSize.isEmpty()) {
        defaultSize = qMin(defaultSize, qMin(m_imageSize.width(), m_imageSize.height()) - 20);
    }

    // 创建ROI对象
    m_currentROI = ROIObject();
    QRectF proposedRect(center.x() - defaultSize/2,
                       center.y() - defaultSize/2,
                       defaultSize, defaultSize);
    
    // 应用边界约束确保初始位置合理
    m_currentROI.rect = constrainROIToBounds(proposedRect);
    m_currentROI.angle = 0.0;
    m_currentROI.isActive = true;
    m_currentROI.showHandles = true;
    m_currentROI.templateName = "";
    m_hasCurrentROI = true;

    // 重置交互状态
    m_activeHandle = ROIObject::NoHandle;
    m_isDragging = false;

    // 设置默认光标
    setCursor(Qt::ArrowCursor);

    update();
    qDebug() << "ROI创建完成，位置:" << m_currentROI.rect;
}

void PaintingOverlay::finishROICreation()
{
    if (!m_hasCurrentROI || !m_roiCreationMode) {
        return;
    }

    qDebug() << "PaintingOverlay::finishROICreation() - 视图:" << m_viewName;

    // 将当前ROI添加到ROI创建列表
    m_roiCreations.append(m_currentROI);

    // 清理状态
    m_hasCurrentROI = false;
    m_roiCreationMode = false;
    m_currentDrawingTool = DrawingTool::None;
    m_isDrawingMode = false;
    m_activeHandle = ROIObject::NoHandle;
    m_isDragging = false;
    m_isHoveringConfirmButton = false;
    m_isHoveringCancelButton = false;
    m_isHoveringRotationHandle = false;
    
    // 重置鼠标光标
    setCursor(Qt::ArrowCursor);

    update();

    // 只发送roiFinished信号，不再发送roiCreated信号（避免循环）
    emit roiFinished(m_viewName);
}

void PaintingOverlay::cancelROICreation()
{
    if (!m_roiCreationMode) {
        return;
    }

    qDebug() << "PaintingOverlay::cancelROICreation() - 视图:" << m_viewName;

    // 清理状态
    m_hasCurrentROI = false;
    m_roiCreationMode = false;
    m_currentDrawingTool = DrawingTool::None;
    m_isDrawingMode = false;
    m_activeHandle = ROIObject::NoHandle;
    m_isDragging = false;
    m_isHoveringConfirmButton = false;
    m_isHoveringCancelButton = false;
    m_isHoveringRotationHandle = false;
    
    // 重置鼠标光标
    setCursor(Qt::ArrowCursor);

    update();

    // 发送信号
    emit roiCancelled(m_viewName);
}

bool PaintingOverlay::hasActiveROI() const
{
    return m_hasCurrentROI && m_roiCreationMode;
}

QRectF PaintingOverlay::getCurrentROI() const
{
    if (m_hasCurrentROI) {
        return m_currentROI.rect;
    }
    return QRectF();
}

qreal PaintingOverlay::getCurrentROIAngle() const
{
    if (m_hasCurrentROI) {
        return m_currentROI.angle;
    }
    return 0.0;
}

QString PaintingOverlay::getCurrentROITemplateName() const
{
    if (m_hasCurrentROI) {
        return m_currentROI.templateName;
    }
    return QString();
}

void PaintingOverlay::setCurrentROITemplateName(const QString& name)
{
    if (m_hasCurrentROI) {
        m_currentROI.templateName = name;
        update();
    }
}

void PaintingOverlay::drawROIHandles(QPainter& painter, const ROIObject& roi, const DrawingContext& ctx) const
{
    if (!roi.showHandles || !roi.rect.isValid()) {
        return;
    }

    // 控制点大小（屏幕像素）
    double handleSize = 8.0 / ctx.scale;
    double halfSize = handleSize / 2.0;

    // 创建控制点画笔和画刷
    QPen handlePen = createPen(Qt::blue, 1, ctx.scale, false);
    QBrush handleBrush(Qt::white);
    QBrush hoverBrush(Qt::yellow);

    painter.setPen(handlePen);

    // 获取矩形的四个角点
    QRectF rect = roi.rect;
    QPointF topLeft = rect.topLeft();
    QPointF topRight = rect.topRight();
    QPointF bottomLeft = rect.bottomLeft();
    QPointF bottomRight = rect.bottomRight();

    // 获取四个边的中点
    QPointF topCenter = QPointF(rect.center().x(), rect.top());
    QPointF bottomCenter = QPointF(rect.center().x(), rect.bottom());
    QPointF leftCenter = QPointF(rect.left(), rect.center().y());
    QPointF rightCenter = QPointF(rect.right(), rect.center().y());

    // 绘制8个调整控制点
    QVector<QPointF> handlePositions = {
        topLeft, topRight, bottomLeft, bottomRight,
        topCenter, bottomCenter, leftCenter, rightCenter
    };

    for (const QPointF& pos : handlePositions) {
        painter.setBrush(handleBrush);
        painter.drawEllipse(pos, halfSize, halfSize);
    }

    // 绘制旋转手柄（在ROI上方）
    QPointF rotationHandlePos = QPointF(rect.center().x(), rect.top() - 30.0 / ctx.scale);

    // 绘制连接线
    painter.setPen(createPen(Qt::green, 1, ctx.scale, false));
    painter.drawLine(rect.center(), rotationHandlePos);

    // 绘制SVG旋转图标（延迟加载，只创建一次）
    if (!m_rotationIconRenderer) {
        const QString rotationIconPath = resolveRuntimePath("icon/rotation.svg");
        m_rotationIconRenderer = new QSvgRenderer(rotationIconPath);
        // 如果加载失败，创建一个空的渲染器避免重复尝试
        if (!m_rotationIconRenderer->isValid()) {
            delete m_rotationIconRenderer;
            m_rotationIconRenderer = new QSvgRenderer(); // 空渲染器作为标记
        }
    }

    if (m_rotationIconRenderer && m_rotationIconRenderer->isValid()) {
        // 绘制绿色圆形背景，根据悬浮状态选择颜色（悬浮时变亮）
        double bgSize = halfSize * 1.8;
        QColor rotationBgColor = m_isHoveringRotationHandle ? QColor(144, 238, 144) : Qt::green;
        painter.setBrush(QBrush(rotationBgColor));
        painter.setPen(createPen(Qt::darkGreen, 2, ctx.scale, false));
        painter.drawEllipse(rotationHandlePos, bgSize, bgSize);

        // 计算SVG图标尺寸（比背景小一点）
        double iconSize = halfSize * 2.2;
        QRectF iconRect(rotationHandlePos.x() - iconSize/2, rotationHandlePos.y() - iconSize/2,
                        iconSize, iconSize);

        // 渲染SVG图标
        m_rotationIconRenderer->render(&painter, iconRect);
    } else {
        // 回退到简单的绿色圆形
        painter.setBrush(QBrush(Qt::green));
        painter.setPen(createPen(Qt::green, 2, ctx.scale, false));
        painter.drawEllipse(rotationHandlePos, halfSize * 1.5, halfSize * 1.5);
    }
}

void PaintingOverlay::handleROICreationClick(const QPointF& pos)
{
    if (!m_hasCurrentROI || !m_roiCreationMode) {
        return;
    }

    // 首先检查是否点击了确认/取消按钮
    bool isConfirm;
    if (isPointInROIButton(pos, isConfirm)) {
        if (isConfirm) {
            emit roiCreated(m_viewName, m_currentROI.rect, m_currentROI.angle);
        } else {
            emit roiCancelled(m_viewName);
        }
        
        // 立即清理状态并重置鼠标光标
        m_hasCurrentROI = false;
        m_roiCreationMode = false;
        m_activeHandle = ROIObject::NoHandle;
        m_isDragging = false;
        m_isHoveringConfirmButton = false;
        m_isHoveringCancelButton = false;
        m_isHoveringRotationHandle = false;
        setCursor(Qt::ArrowCursor);
        update();
        return;
    }

    // 检查是否点击了控制点
    ROIObject::HandleType clickedHandle = getROIHandleAt(pos);

    if (clickedHandle != ROIObject::NoHandle) {
        // 点击了控制点，开始拖拽
        m_activeHandle = clickedHandle;
        m_isDragging = true;
        m_lastMousePos = pos;
    } else if (m_currentROI.rect.contains(pos)) {
        // 点击了ROI内部，开始移动
        m_activeHandle = ROIObject::MoveHandle;
        m_isDragging = true;
        m_lastMousePos = pos;
    }
}

PaintingOverlay::ROIObject::HandleType PaintingOverlay::getROIHandleAt(const QPointF& pos) const
{
    if (!m_hasCurrentROI || !m_currentROI.rect.isValid()) {
        return PaintingOverlay::ROIObject::NoHandle;
    }

    // 控制点检测半径（图像坐标）-使用缓存的DrawingContext中的scale，保持固定的物理大小
    double handleRadius = 5.0 / m_cachedDrawingContext.scale;

    QRectF rect = m_currentROI.rect;

    // 如果ROI有旋转，需要将鼠标位置反向旋转到ROI的本地坐标系中进行检测
    QPointF localPos = pos;
    if (qAbs(m_currentROI.angle) > 0.01) {
        QPointF center = rect.center();
        // 创建反向旋转变换
        QTransform transform;
        transform.translate(center.x(), center.y());
        transform.rotate(-m_currentROI.angle); // 反向旋转
        transform.translate(-center.x(), -center.y());
        localPos = transform.map(pos);
        // qDebug() << "旋转检测 - 原始位置:" << pos << "本地位置:" << localPos << "角度:" << m_currentROI.angle;
    }

    // 检查8个调整控制点
    QVector<QPointF> handlePositions = {
        rect.topLeft(),     // TopLeft
        rect.topRight(),    // TopRight
        rect.bottomLeft(),  // BottomLeft
        rect.bottomRight(), // BottomRight
        QPointF(rect.center().x(), rect.top()),    // TopCenter
        QPointF(rect.center().x(), rect.bottom()), // BottomCenter
        QPointF(rect.left(), rect.center().y()),   // LeftCenter
        QPointF(rect.right(), rect.center().y())   // RightCenter
    };

    for (int i = 0; i < handlePositions.size(); ++i) {
        QPointF handlePos = handlePositions[i];
        double distance = QLineF(localPos, handlePos).length();
        if (distance <= handleRadius) {
            qDebug() << "检测到控制点" << i << "距离:" << distance;
            return static_cast<PaintingOverlay::ROIObject::HandleType>(i);
        }
    }

    // 检查旋转手柄（位置计算要与drawROIHandles保持一致）
    // 使用与绘制时相同的位置计算：rect.top() - 30.0 / ctx.scale
    // 这里使用m_scaleFactor来保持一致
    QPointF rotationHandlePos = QPointF(rect.center().x(), rect.top() - 30.0 / m_cachedDrawingContext.scale);
    double rotationDistance = QLineF(localPos, rotationHandlePos).length();
    // 使用更大的检测半径，因为旋转手柄比普通控制点大
    double rotationHandleRadius = handleRadius * 2;
    // qDebug() << "旋转手柄检测 - 绘制位置:" << rotationHandlePos << "本地鼠标:" << localPos << "距离:" << rotationDistance << "半径:" << rotationHandleRadius;
    if (rotationDistance <= rotationHandleRadius) {
        qDebug() << "检测到旋转手柄点击！";
        return PaintingOverlay::ROIObject::RotationHandle;
    }

    // 检查是否在ROI内部（用于移动）
    if (rect.contains(localPos)) {
        // qDebug() << "检测到ROI内部点击";
        return PaintingOverlay::ROIObject::MoveHandle;
    }

    return PaintingOverlay::ROIObject::NoHandle;
}

/**
 * @brief 根据控制点类型，获取其在矩形局部坐标系下的锚点（对面的点或边中点）
 */
QPointF PaintingOverlay::getAnchorPointInLocalCoords(ROIObject::HandleType handle, const QRectF& rect) const
{
    switch (handle) {
        case ROIObject::TopLeft:     return rect.bottomRight();
        case ROIObject::TopRight:    return rect.bottomLeft();
        case ROIObject::BottomLeft:  return rect.topRight();
        case ROIObject::BottomRight: return rect.topLeft();
        case ROIObject::TopCenter:   return QPointF(rect.center().x(), rect.bottom());
        case ROIObject::BottomCenter:return QPointF(rect.center().x(), rect.top());
        case ROIObject::LeftCenter:  return QPointF(rect.right(), rect.center().y());
        case ROIObject::RightCenter: return QPointF(rect.left(), rect.center().y());
        default:                     return rect.center(); // 默认返回中心点
    }
}

/**
 * @brief 将一个点围绕另一个中心点旋转指定的角度
 */
QPointF PaintingOverlay::rotatePoint(const QPointF& point, const QPointF& center, qreal angleDegrees) const
{
    qreal angleRadians = qDegreesToRadians(angleDegrees);
    qreal cosAngle = std::cos(angleRadians);
    qreal sinAngle = std::sin(angleRadians);
    QPointF translatedPoint = point - center;
    qreal newX = translatedPoint.x() * cosAngle - translatedPoint.y() * sinAngle;
    qreal newY = translatedPoint.x() * sinAngle + translatedPoint.y() * cosAngle;
    return QPointF(newX, newY) + center;
}

/**
 * @brief 计算并返回一个矩形在旋转指定角度后的四个角点的全局坐标
 */
QVector<QPointF> PaintingOverlay::getRotatedRectCorners(const QRectF& rect, qreal angleDegrees) const
{
    QVector<QPointF> corners;
    const QPointF center = rect.center();
    corners << rotatePoint(rect.topLeft(),     center, angleDegrees);
    corners << rotatePoint(rect.topRight(),    center, angleDegrees);
    corners << rotatePoint(rect.bottomRight(), center, angleDegrees);
    corners << rotatePoint(rect.bottomLeft(),  center, angleDegrees);
    return corners;
}

void PaintingOverlay::handleROIDrag(ROIObject::HandleType handle, const QPointF& delta)
{
    if (!m_hasCurrentROI || !m_currentROI.rect.isValid()) {
        return;
    }

    // 定义图像边界
    const QRectF imageBounds(0, 0, m_imageSize.width(), m_imageSize.height());
    if (imageBounds.isEmpty()) return; // 没有边界信息则无法约束

    // 声明一个变量来存储最终计算出的ROI矩形
    QRectF proposedRect = m_currentROI.rect;
    qreal currentAngle = m_currentROI.angle;

    // --- Case 1: 移动整个ROI ---
    if (handle == ROIObject::MoveHandle) {
        proposedRect.translate(delta);
    }
    // --- Case 2: 旋转ROI ---
    else if (handle == ROIObject::RotationHandle) {
        handleROIRotation(delta); // 旋转逻辑是独立的，直接返回
        return;
    }
    // --- Case 3: 调整ROI尺寸 ---
    else {
        QRectF currentRect = m_currentROI.rect;
        QPointF currentCenter = currentRect.center();
        const qreal minWidth = 20.0;
        const qreal minHeight = 20.0;

        QPointF newGlobalHandlePos = m_lastMousePos + delta;
        QPointF localMousePos = rotatePoint(newGlobalHandlePos, currentCenter, -currentAngle);
        QRectF newLocalRect = currentRect;

        switch (handle) {
            case ROIObject::TopLeft:     newLocalRect.setTopLeft(localMousePos); break;
            case ROIObject::TopRight:    newLocalRect.setTopRight(localMousePos); break;
            case ROIObject::BottomLeft:  newLocalRect.setBottomLeft(localMousePos); break;
            case ROIObject::BottomRight: newLocalRect.setBottomRight(localMousePos); break;
            case ROIObject::TopCenter:   newLocalRect.setTop(localMousePos.y()); break;
            case ROIObject::BottomCenter:newLocalRect.setBottom(localMousePos.y()); break;
            case ROIObject::LeftCenter:  newLocalRect.setLeft(localMousePos.x()); break;
            case ROIObject::RightCenter: newLocalRect.setRight(localMousePos.x()); break;
            default: return;
        }

        if (newLocalRect.width() < minWidth) {
            if (handle == ROIObject::LeftCenter || handle == ROIObject::TopLeft || handle == ROIObject::BottomLeft)
                newLocalRect.setLeft(newLocalRect.right() - minWidth);
            else
                newLocalRect.setRight(newLocalRect.left() + minWidth);
        }
        if (newLocalRect.height() < minHeight) {
            if (handle == ROIObject::TopCenter || handle == ROIObject::TopLeft || handle == ROIObject::TopRight)
                newLocalRect.setTop(newLocalRect.bottom() - minHeight);
            else
                newLocalRect.setBottom(newLocalRect.top() + minHeight);
        }

        newLocalRect = newLocalRect.normalized();
        
        QPointF newLocalCenter = newLocalRect.center();
        QPointF localCenterDisplacement = newLocalCenter - currentRect.center();
        QPointF globalCenterDisplacement = rotatePoint(localCenterDisplacement, QPointF(0,0), currentAngle);
        QPointF newGlobalCenter = currentCenter + globalCenterDisplacement;

        proposedRect = QRectF(
            newGlobalCenter.x() - newLocalRect.width() / 2.0,
            newGlobalCenter.y() - newLocalRect.height() / 2.0,
            newLocalRect.width(),
            newLocalRect.height()
        );
    }

    // --- 统一的边界约束逻辑 ---
    // 获取提议的、旋转后的ROI的四个角点
    QVector<QPointF> corners = getRotatedRectCorners(proposedRect, currentAngle);

    // 计算需要将整个ROI移动多少才能回到边界内
    qreal dx_adjust = 0.0;
    qreal dy_adjust = 0.0;

    for (const QPointF& corner : corners) {
        if (corner.x() < imageBounds.left()) {
            dx_adjust = qMax(dx_adjust, imageBounds.left() - corner.x());
        }
        if (corner.x() > imageBounds.right()) {
            dx_adjust = qMin(dx_adjust, imageBounds.right() - corner.x());
        }
        if (corner.y() < imageBounds.top()) {
            dy_adjust = qMax(dy_adjust, imageBounds.top() - corner.y());
        }
        if (corner.y() > imageBounds.bottom()) {
            dy_adjust = qMin(dy_adjust, imageBounds.bottom() - corner.y());
        }
    }

    // 应用计算出的调整量
    proposedRect.translate(dx_adjust, dy_adjust);

    // 最终更新ROI
    m_currentROI.rect = proposedRect;
    emit roiChanged(m_viewName, m_currentROI.rect, m_currentROI.angle);
}

void PaintingOverlay::handleROIRotation(const QPointF& delta)
{
    if (!m_hasCurrentROI || !m_currentROI.rect.isValid()) {
        return;
    }

    // 基于角度的旋转处理：计算鼠标移动相对于ROI中心的角度变化
    QPointF center = m_currentROI.rect.center();
    QPointF oldPos = m_lastMousePos;
    QPointF newPos = m_lastMousePos + delta;

    // 计算两个位置相对于中心的角度
    qreal oldAngle = qAtan2(oldPos.y() - center.y(), oldPos.x() - center.x()) * 180.0 / M_PI;
    qreal newAngle = qAtan2(newPos.y() - center.y(), newPos.x() - center.x()) * 180.0 / M_PI;

    // 计算角度变化
    qreal angleChange = newAngle - oldAngle;

    // 处理角度跨越±180度的情况
    if (angleChange > 180) angleChange -= 360;
    if (angleChange < -180) angleChange += 360;

    m_currentROI.angle += angleChange;

    // 限制角度范围到 -180 到 180
    while (m_currentROI.angle > 180) m_currentROI.angle -= 360;
    while (m_currentROI.angle < -180) m_currentROI.angle += 360;

    qDebug() << "旋转处理 - 角度变化:" << angleChange << "新角度:" << m_currentROI.angle;
    emit roiChanged(m_viewName, m_currentROI.rect, m_currentROI.angle);
}

void PaintingOverlay::updateROICursor(ROIObject::HandleType handle)
{
    switch (handle) {
    case PaintingOverlay::ROIObject::TopLeft:
    case PaintingOverlay::ROIObject::BottomRight:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case PaintingOverlay::ROIObject::TopRight:
    case PaintingOverlay::ROIObject::BottomLeft:
        setCursor(Qt::SizeBDiagCursor);
        break;
    case PaintingOverlay::ROIObject::TopCenter:
    case PaintingOverlay::ROIObject::BottomCenter:
        setCursor(Qt::SizeVerCursor);
        break;
    case PaintingOverlay::ROIObject::LeftCenter:
    case PaintingOverlay::ROIObject::RightCenter:
        setCursor(Qt::SizeHorCursor);
        break;
    case PaintingOverlay::ROIObject::RotationHandle:
        setCursor(Qt::PointingHandCursor);
        break;
    case PaintingOverlay::ROIObject::MoveHandle:
        setCursor(Qt::SizeAllCursor);
        break;
    default:
        setCursor(Qt::ArrowCursor);
        break;
    }
}

void PaintingOverlay::drawROIButtons(QPainter& painter, const DrawingContext& ctx) const
{
    if (!m_hasCurrentROI || !m_currentROI.rect.isValid()) {
        return;
    }

    // 按钮尺寸和位置（与旋转手柄直径一致）
    double handleSize = 8.0 / ctx.scale;
    double rotationHandleDiameter = handleSize * 1.8; // 与旋转手柄背景直径一致
    double buttonSize = rotationHandleDiameter;
    double buttonSpacing = 10.0 / ctx.scale;

    // 按钮位置：ROI右下角外侧，增加距离避免与控制点冲突
    QPointF roiBottomRight = m_currentROI.rect.bottomRight();
    QPointF confirmButtonPos = roiBottomRight + QPointF(35, -buttonSize - buttonSpacing);
    QPointF cancelButtonPos = roiBottomRight + QPointF(35, -buttonSpacing);

    // 绘制确认按钮（绿色√）
    QRectF confirmRect(confirmButtonPos, QSizeF(buttonSize, buttonSize));
    painter.save();
    painter.setPen(createPen(Qt::darkGreen, 2, ctx.scale));
    
    // 根据悬浮状态选择确认按钮颜色（悬浮时变亮）
    QColor confirmBgColor = m_isHoveringConfirmButton ? QColor(180, 255, 180, 240) : QColor(144, 238, 144, 200);
    painter.setBrush(QBrush(confirmBgColor));
    painter.drawRoundedRect(confirmRect, 5, 5);

    // 绘制√符号（根据按钮大小自适应）
    double symbolSize = buttonSize * 0.3; // 符号大小为按钮的30%
    painter.setPen(createPen(Qt::darkGreen, 2, ctx.scale));
    QPointF checkStart = confirmRect.center() + QPointF(-symbolSize, 0);
    QPointF checkMid = confirmRect.center() + QPointF(-symbolSize * 0.25, symbolSize * 0.75);
    QPointF checkEnd = confirmRect.center() + QPointF(symbolSize, -symbolSize * 0.75);
    painter.drawLine(checkStart, checkMid);
    painter.drawLine(checkMid, checkEnd);
    painter.restore();

    // 绘制取消按钮（红色×）
    QRectF cancelRect(cancelButtonPos, QSizeF(buttonSize, buttonSize));
    painter.save();
    painter.setPen(createPen(Qt::darkRed, 2, ctx.scale));
    
    // 根据悬浮状态选择取消按钮颜色（悬浮时变亮）
    QColor cancelBgColor = m_isHoveringCancelButton ? QColor(255, 220, 220, 240) : QColor(255, 182, 193, 200);
    painter.setBrush(QBrush(cancelBgColor));
    painter.drawRoundedRect(cancelRect, 5, 5);

    // 绘制×符号（根据按钮大小自适应）
    double crossSymbolSize = buttonSize * 0.3; // 符号大小为按钮的30%
    painter.setPen(createPen(Qt::darkRed, 2, ctx.scale));
    QPointF crossTopLeft = cancelRect.center() + QPointF(-crossSymbolSize, -crossSymbolSize);
    QPointF crossBottomRight = cancelRect.center() + QPointF(crossSymbolSize, crossSymbolSize);
    QPointF crossTopRight = cancelRect.center() + QPointF(crossSymbolSize, -crossSymbolSize);
    QPointF crossBottomLeft = cancelRect.center() + QPointF(-crossSymbolSize, crossSymbolSize);
    painter.drawLine(crossTopLeft, crossBottomRight);
    painter.drawLine(crossTopRight, crossBottomLeft);
    painter.restore();
}

void PaintingOverlay::drawROIInfo(QPainter& painter, const DrawingContext& ctx) const
{
    if (!m_hasCurrentROI || !m_currentROI.rect.isValid()) {
        return;
    }

    // 计算ROI信息
    QRectF rect = m_currentROI.rect;
    double angle = m_currentROI.angle;

    // 使用缓存避免重复的字符串格式化（性能优化）
    QString infoText;
    if (rect != m_cachedROIRect || qAbs(angle - m_cachedROIAngle) > 0.01) {
        // 只有当ROI发生变化时才重新计算文本
        int width = qRound(rect.width());
        int height = qRound(rect.height());
        infoText = QString("尺寸: %1×%2  角度: %3°").arg(width).arg(height).arg(angle, 0, 'f', 1);

        // 更新缓存
        m_cachedROIInfoText = infoText;
        m_cachedROIRect = rect;
        m_cachedROIAngle = angle;
    } else {
        // 使用缓存的文本
        infoText = m_cachedROIInfoText;
    }

    // 字体设置
    QFont infoFont = ctx.font;
    infoFont.setPointSize(qMax(8, qRound(ctx.fontSize * 0.8))); // 稍小的字体

    // 计算文本尺寸
    QFontMetrics fm(infoFont);
    QRect textRect = fm.boundingRect(infoText);

    // 计算信息框尺寸
    int padding = 8;
    QSizeF infoBoxSize(textRect.width() + padding * 2, textRect.height() + padding * 2);

    // 信息框位置：ROI左下角下方，避免挡住旋转按钮
    QPointF infoPos = rect.bottomLeft() + QPointF(0, 10);

    // 确保信息框不超出视图边界
    // 这里可以根据需要添加边界检查

    QRectF infoRect(infoPos, infoBoxSize);

    // 绘制半透明背景
    painter.save();
    painter.setPen(QPen(Qt::darkGray, 1));
    painter.setBrush(QBrush(QColor(0, 0, 0, 150))); // 半透明黑色背景
    painter.drawRoundedRect(infoRect, 5, 5);

    // 绘制文本
    painter.setPen(QPen(Qt::white));
    painter.setFont(infoFont);

    // 绘制信息文本（一行）
    QPointF textPos = infoRect.topLeft() + QPointF(padding, padding + textRect.height());
    painter.drawText(textPos, infoText);

    painter.restore();
}

bool PaintingOverlay::isPointInROIButton(const QPointF& pos, bool& isConfirm) const
{
    if (!m_hasCurrentROI || !m_currentROI.rect.isValid()) {
        return false;
    }

    // 按钮尺寸和位置（与drawROIButtons保持一致）
    double handleSize = 8.0 / m_cachedDrawingContext.scale;
    double rotationHandleDiameter = handleSize * 1.8; // 与旋转手柄背景直径一致
    double buttonSize = rotationHandleDiameter;
    double buttonSpacing = 10.0 / m_cachedDrawingContext.scale;

    // 按钮位置：ROI右下角外侧，增加距离避免与控制点冲突
    QPointF roiBottomRight = m_currentROI.rect.bottomRight();
    QPointF confirmButtonPos = roiBottomRight + QPointF(35, -buttonSize - buttonSpacing);
    QPointF cancelButtonPos = roiBottomRight + QPointF(35, -buttonSpacing);

    QRectF confirmRect(confirmButtonPos, QSizeF(buttonSize, buttonSize));
    QRectF cancelRect(cancelButtonPos, QSizeF(buttonSize, buttonSize));

    if (confirmRect.contains(pos)) {
        isConfirm = true;
        return true;
    } else if (cancelRect.contains(pos)) {
        isConfirm = false;
        return true;
    }

    return false;
}

cv::Mat PaintingOverlay::validateAndPreprocessROI(const cv::Mat& roiImage) const
{
    // 基本验证
    if (roiImage.empty()) {
        qWarning() << "ROI图像为空";
        return cv::Mat();
    }

    // 检查图像尺寸
    if (roiImage.cols < 10 || roiImage.rows < 10) {
        qWarning() << "ROI图像太小，尺寸:" << roiImage.cols << "x" << roiImage.rows;
        return cv::Mat();
    }

    if (roiImage.cols > 2000 || roiImage.rows > 2000) {
        qWarning() << "ROI图像太大，尺寸:" << roiImage.cols << "x" << roiImage.rows;
        return cv::Mat();
    }

    try {
        cv::Mat processedImage = roiImage.clone();

        // 确保图像是3通道BGR格式（模板匹配通常需要）
        if (processedImage.channels() == 1) {
            cv::cvtColor(processedImage, processedImage, cv::COLOR_GRAY2BGR);
        } else if (processedImage.channels() == 4) {
            cv::cvtColor(processedImage, processedImage, cv::COLOR_BGRA2BGR);
        }

        // 检查图像质量（计算图像的方差，低方差表示图像过于平坦）
        cv::Mat grayImage;
        cv::cvtColor(processedImage, grayImage, cv::COLOR_BGR2GRAY);

        cv::Scalar meanValue, stdValue;
        cv::meanStdDev(grayImage, meanValue, stdValue);
        double variance = stdValue[0] * stdValue[0];

        // 如果方差太低，图像可能过于平坦，不适合作为模板
        if (variance < 100.0) {
            qWarning() << "ROI图像方差过低，可能不适合作为模板，方差:" << variance;
            // 不返回空，只是警告，让用户决定
        }

        qInfo() << "ROI图像验证通过，尺寸:" << processedImage.cols << "x" << processedImage.rows
                << "通道:" << processedImage.channels() << "方差:" << variance;

        return processedImage;

    } catch (const cv::Exception& e) {
        qCritical() << "预处理ROI图像时发生OpenCV异常：" << e.what();
    } catch (const std::exception& e) {
        qCritical() << "预处理ROI图像时发生异常：" << e.what();
    }

    return cv::Mat();
}

bool PaintingOverlay::saveTemplateData(const cv::Mat& templateImage, const QString& templateName, const QString& templateDir) const
{
    if (templateImage.empty()) {
        qWarning() << "无法保存空的模板图像";
        return false;
    }

    if (templateName.isEmpty()) {
        qWarning() << "模板名称不能为空";
        return false;
    }

    try {
        // 确定保存目录
        QString saveDir = templateDir;
        if (saveDir.isEmpty()) {
            saveDir = QCoreApplication::applicationDirPath() + "/templates";
        }

        // 创建目录（如果不存在）
        QDir dir;
        if (!dir.exists(saveDir)) {
            if (!dir.mkpath(saveDir)) {
                qCritical() << "无法创建模板目录:" << saveDir;
                return false;
            }
        }

        // 生成文件名（确保唯一性）
        QString baseName = templateName;
        // 移除文件名中的非法字符
        baseName = baseName.replace(QRegularExpression("[<>:\"/\\|?*]"), "_");

        QString imageFileName = QString("%1.png").arg(baseName);
        QString metaFileName = QString("%1.json").arg(baseName);
        QString halconFileName = QString("%1.shm").arg(baseName);

        QString imagePath = QDir(saveDir).filePath(imageFileName);
        QString metaPath = QDir(saveDir).filePath(metaFileName);
        QString halconPath = QDir(saveDir).filePath(halconFileName);

        // 如果文件已存在，添加时间戳
        if (QFile::exists(imagePath) || QFile::exists(metaPath) || QFile::exists(halconPath)) {
            QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
            imageFileName = QString("%1_%2.png").arg(baseName, timestamp);
            metaFileName = QString("%1_%2.json").arg(baseName, timestamp);
            halconFileName = QString("%1_%2.shm").arg(baseName, timestamp);
            imagePath = QDir(saveDir).filePath(imageFileName);
            metaPath = QDir(saveDir).filePath(metaFileName);
            halconPath = QDir(saveDir).filePath(halconFileName);
        }

        // 保存图像文件
        if (!cv::imwrite(imagePath.toStdString(), templateImage)) {
            qCritical() << "无法保存模板图像到:" << imagePath;
            return false;
        }

        // 额外：生成 Halcon 形状模板并保存 .shm
        bool halconOk = false;
        try {
            // 直接从保存的图像文件读取，避免指针寿命问题
            HalconCpp::HObject hoTemplate;
            HalconCpp::ReadImage(&hoTemplate, HalconCpp::HTuple(imagePath.toStdString().c_str()));

            HalconCpp::HTuple hvModelID;
            
            // 从配置文件读取参数
            TemplateMatchingConfig* config = TemplateMatchingConfig::instance();
            TemplateMatchingConfig::TemplateCreationParams params = config->getTemplateCreationParams();
            
            HalconCpp::CreateScaledShapeModel(hoTemplate,
                params.numLevels,
                HalconCpp::HTuple(params.angleStart).TupleRad(), 
                HalconCpp::HTuple(params.angleExtent).TupleRad(), 
                HalconCpp::HTuple(params.angleStep).TupleRad(),
                params.scaleMin, params.scaleMax, params.scaleStep,
                params.optimization.toStdString().c_str(), 
                params.metric.toStdString().c_str(), 
                params.contrast.toStdString().c_str(), 
                params.minContrast.toStdString().c_str(),
                &hvModelID);

            HalconCpp::WriteShapeModel(hvModelID, HalconCpp::HTuple(halconPath.toStdString().c_str()));
            HalconCpp::ClearShapeModel(hvModelID);
            halconOk = true;
        } catch (const HalconCpp::HException& e) {
            qWarning() << "Halcon模板创建/保存失败:" << e.ErrorMessage().TextA();
            // 不中断保存png/json流程
        }

        // 创建元数据
        QJsonObject metadata;
        metadata["templateName"] = templateName;
        metadata["imageFile"] = imageFileName;
        metadata["createdTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        metadata["imageSize"] = QJsonObject{
            {"width", templateImage.cols},
            {"height", templateImage.rows},
            {"channels", templateImage.channels()}
        };
        // 记录 Halcon 模型文件（即使创建失败也写入空值，保证兼容）
        metadata["halconModelFile"] = halconOk ? QJsonValue(halconFileName) : QJsonValue("");

        // 保存ROI信息（如果有）
        if (m_hasCurrentROI) {
            QJsonObject roiInfo;
            roiInfo["x"] = m_currentROI.rect.x();
            roiInfo["y"] = m_currentROI.rect.y();
            roiInfo["width"] = m_currentROI.rect.width();
            roiInfo["height"] = m_currentROI.rect.height();
            roiInfo["angle"] = m_currentROI.angle;
            metadata["originalROI"] = roiInfo;
        }

        // 保存元数据文件
        QJsonDocument doc(metadata);
        QFile metaFile(metaPath);
        if (!metaFile.open(QIODevice::WriteOnly)) {
            qCritical() << "无法创建元数据文件:" << metaPath;
            // 回滚已保存的图像和shm
            QFile::remove(imagePath);
            if (QFile::exists(halconPath)) QFile::remove(halconPath);
            return false;
        }

        metaFile.write(doc.toJson());
        metaFile.close();

        qInfo() << "模板保存成功:" << templateName;
        qInfo() << "图像文件:" << imagePath;
        qInfo() << "元数据文件:" << metaPath;
        if (halconOk) qInfo() << "Halcon模板文件:" << halconPath;

        return true;

    } catch (const cv::Exception& e) {
        qCritical() << "保存模板时发生OpenCV异常：" << e.what();
    } catch (const std::exception& e) {
        qCritical() << "保存模板时发生异常：" << e.what();
    }

    return false;
}

bool PaintingOverlay::createTemplateFromROI(const cv::Mat& sourceImage, const QString& templateName, const QString& templateDir) const
{
    qInfo() << "开始从ROI创建模板:" << templateName;

    // 步骤1：从ROI提取图像
    cv::Mat roiImage = extractROIImage(sourceImage);
    if (roiImage.empty()) {
        qWarning() << "无法从ROI提取图像";
        return false;
    }

    qInfo() << "ROI图像提取成功，尺寸:" << roiImage.cols << "x" << roiImage.rows;

    // 步骤2：验证和预处理图像
    cv::Mat processedImage = validateAndPreprocessROI(roiImage);
    if (processedImage.empty()) {
        qWarning() << "ROI图像验证或预处理失败";
        return false;
    }

    qInfo() << "ROI图像验证和预处理完成";

    // 步骤3：保存模板数据
    if (!saveTemplateData(processedImage, templateName, templateDir)) {
        qWarning() << "模板数据保存失败";
        return false;
    }

    qInfo() << "模板创建完成:" << templateName;
    return true;
}

QVector<TemplateInfo> PaintingOverlay::loadTemplatesFromDirectory(const QString& templateDir) const
{
    QVector<TemplateInfo> templates;

    // 确定模板目录
    QString searchDir = templateDir;
    if (searchDir.isEmpty()) {
        searchDir = QCoreApplication::applicationDirPath() + "/templates";
    }

    QDir dir(searchDir);
    if (!dir.exists()) {
        qWarning() << "模板目录不存在:" << searchDir;
        return templates;
    }

    // 查找所有PNG文件
    QStringList imageFilters;
    imageFilters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp";
    QStringList imageFiles = dir.entryList(imageFilters, QDir::Files);

    qInfo() << "在目录" << searchDir << "中找到" << imageFiles.size() << "个图像文件";

    for (const QString& imageFile : imageFiles) {
        QString imagePath = dir.filePath(imageFile);

        // 构造对应的JSON文件路径
        QString baseName = QFileInfo(imageFile).completeBaseName();
        QString metadataPath = dir.filePath(baseName + ".json");

        // 检查JSON文件是否存在
        if (QFile::exists(metadataPath)) {
            TemplateInfo templateInfo = loadSingleTemplate(imagePath, metadataPath);
            if (!templateInfo.name.isEmpty()) {
                templates.append(templateInfo);
                qDebug() << "成功加载模板:" << templateInfo.name;
            }
        } else {
            qWarning() << "找不到对应的元数据文件:" << metadataPath;
        }
    }

    qInfo() << "总共加载了" << templates.size() << "个有效模板";
    return templates;
}

TemplateInfo PaintingOverlay::loadSingleTemplate(const QString& imagePath, const QString& metadataPath) const
{
    TemplateInfo templateInfo;

    try {
        // 加载图像
        cv::Mat image = cv::imread(imagePath.toStdString());

        // 记录 Halcon 模型路径（与图片同目录）
        QFileInfo imgInfo(imagePath);
        QFileInfo metaInfo(metadataPath);
        QDir dir = metaInfo.dir();
        if (image.empty()) {
            qWarning() << "无法加载模板图像:" << imagePath;
            return templateInfo;
        }

        // 读取元数据文件
        QFile metaFile(metadataPath);
        if (!metaFile.open(QIODevice::ReadOnly)) {
            qWarning() << "无法打开元数据文件:" << metadataPath;
            return templateInfo;
        }

        QByteArray metaData = metaFile.readAll();
        metaFile.close();

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(metaData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "解析元数据文件失败:" << parseError.errorString();
            return templateInfo;
        }

        QJsonObject metadata = doc.object();

        // 填充模板信息
        templateInfo.name = metadata["templateName"].toString();
        templateInfo.imagePath = imagePath;
        templateInfo.metadataPath = metadataPath;
        templateInfo.templateImage = image.clone();
        templateInfo.createdTime = QDateTime::fromString(metadata["createdTime"].toString(), Qt::ISODate);
        templateInfo.isSelected = false; // 默认未选中

        // 解析原始ROI信息
        if (metadata.contains("originalROI")) {
            QJsonObject roiObj = metadata["originalROI"].toObject();
            templateInfo.originalROI = QRectF(
                roiObj["x"].toDouble(),
                roiObj["y"].toDouble(),
                roiObj["width"].toDouble(),
                roiObj["height"].toDouble()
            );
            templateInfo.originalAngle = roiObj["angle"].toDouble();
        }

        // 解析 Halcon 模型路径
        if (metadata.contains("halconModelFile")) {
            QString shmName = metadata["halconModelFile"].toString();
            if (!shmName.isEmpty()) {
                QString shmPath = dir.filePath(shmName);
                templateInfo.halconModelPath = shmPath;
            }
        }

        // 加载 Halcon 模型
        if (metadata.contains("halconModelFile")) {
            QString halconModelPath = dir.filePath(metadata["halconModelFile"].toString());
            if (QFile::exists(halconModelPath)) {
                try {
                    // 仅校验存在性，实际匹配时再加载
                    templateInfo.halconModelPath = halconModelPath;
                    qDebug() << "检测到 Halcon 模型文件:" << halconModelPath;
                } catch (const HalconCpp::HException& e) {
                    qWarning() << "加载 Halcon 模型失败:" << e.ErrorMessage().TextA();
                }
            } else {
                qWarning() << "找不到 Halcon 模型文件:" << halconModelPath;
            }
        }

        qDebug() << "成功解析模板:" << templateInfo.name
                 << "尺寸:" << image.cols << "x" << image.rows
                 << "创建时间:" << templateInfo.createdTime.toString();

    } catch (const cv::Exception& e) {
        qCritical() << "加载模板时发生OpenCV异常：" << e.what();
    } catch (const std::exception& e) {
        qCritical() << "加载模板时发生异常：" << e.what();
    }

    return templateInfo;
}

bool PaintingOverlay::startTemplateMatching(const QVector<TemplateInfo>& selectedTemplates,
                                               const TemplateMatchingConfig::TemplateMatchingParams* matchingParams)
{

    if (selectedTemplates.isEmpty()) {
        qWarning() << "无法启动模板匹配：没有选中的模板";
        return false;
    }

    try {
        // 清除之前的匹配结果
        m_currentMatches.clear();

        // 保存选中的模板
        m_loadedTemplates = selectedTemplates;
        
        // 保存匹配参数（如果提供）
        if (matchingParams) {
            TemplateMatchingConfig* config = TemplateMatchingConfig::instance();
            config->setTemplateMatchingParams(*matchingParams);
        }

        // 释放旧的 Halcon 模型缓存，避免句柄泄漏
        clearHalconModelCache("启动匹配");

        // 预加载选中模板的 Halcon 模型
        QTime preloadStartTime = QTime::currentTime();
        int preloadCount = 0;
        for (int idx = 0; idx < m_loadedTemplates.size(); ++idx) {
            TemplateInfo& t = m_loadedTemplates[idx];
            if (!t.isSelected) continue;
            QString key = !t.halconModelPath.isEmpty() ? t.halconModelPath
                           : (!t.imagePath.isEmpty() ? t.imagePath : t.name);
            if (m_halconModelCache.contains(key)) continue;

            HalconCpp::HTuple modelId;
            bool ok = false;
            try {
                if (!t.halconModelPath.isEmpty() && QFile::exists(t.halconModelPath)) {
                    HalconCpp::ReadShapeModel(HalconCpp::HTuple(t.halconModelPath.toStdString().c_str()), &modelId);
                    ok = true;
                }
            } catch (const HalconCpp::HException& e) {
                qWarning() << "预加载读取Halcon模型失败，将尝试临时创建:" << e.ErrorMessage().TextA();
            }

            if (!ok) {
                if (t.imagePath.isEmpty() || !QFile::exists(t.imagePath)) {
                    qWarning() << "模板缺少图像或模型，跳过预加载:" << t.name;
                    continue;
                }
                try {
                    HalconCpp::HObject hoTemplate;
                    HalconCpp::ReadImage(&hoTemplate, HalconCpp::HTuple(t.imagePath.toStdString().c_str()));
                    // 使用配置文件参数
                    TemplateMatchingConfig* config = TemplateMatchingConfig::instance();
                    TemplateMatchingConfig::TemplateCreationParams params = config->getTemplateCreationParams();
                    
                    HalconCpp::CreateScaledShapeModel(hoTemplate,
                        params.numLevels,
                        HalconCpp::HTuple(params.angleStart).TupleRad(), 
                        HalconCpp::HTuple(params.angleExtent).TupleRad(), 
                        HalconCpp::HTuple(params.angleStep).TupleRad(),
                        params.scaleMin, params.scaleMax, params.scaleStep,
                        params.optimization.toStdString().c_str(), 
                        params.metric.toStdString().c_str(), 
                        params.contrast.toStdString().c_str(), 
                        params.minContrast.toStdString().c_str(),
                        &modelId);
                    ok = true;
                } catch (const HalconCpp::HException& e) {
                    qWarning() << "预加载临时创建Halcon模型失败，跳过模板:" << t.name << e.ErrorMessage().TextA();
                    continue;
                }
            }

            if (ok) {
                // 缓存 Halcon 模型句柄
                m_halconModelCache.insert(key, std::make_shared<HalconCpp::HTuple>(modelId));
                preloadCount++;

                // 仅在还未提取过轮廓时，从模型中提取轮廓并缓存到模板信息
                if (t.modelContours.isEmpty()) {
                    t.modelContours = extractContoursFromHalconModel(modelId);
                    if (!t.modelContours.isEmpty()) {
                        qDebug() << "模板" << t.name << "已缓存" << t.modelContours.size() << "条模型轮廓";
                    }
                }
            }
        }
        qDebug() << "预加载 Halcon 模型" << preloadCount << "个，耗时:" << preloadStartTime.msecsTo(QTime::currentTime()) << "ms";

        // 启用匹配
        m_isMatchingEnabled = true;

        // 重置帧跳过计数器，确保立即开始匹配
        m_matchingFrameSkip = MATCHING_FRAME_INTERVAL - 1;

        qInfo() << "模板匹配已启动，加载了" << selectedTemplates.size() << "个模板";
        return true;

    } catch (const std::exception& e) {
        qCritical() << "启动模板匹配失败：" << e.what();
        return false;
    }
}

void PaintingOverlay::stopTemplateMatching()
{
    m_isMatchingEnabled = false;
    m_currentMatches.clear();
    qInfo() << "模板匹配已停止";

    // 释放 Halcon 模型缓存
    clearHalconModelCache("停止匹配");
}

QVector<TemplateMatchResult> PaintingOverlay::performMatching(const cv::Mat& sourceImage)
{
    QVector<TemplateMatchResult> results;

    if (!m_isMatchingEnabled || sourceImage.empty()) {
        return results;
    }

    if (m_loadedTemplates.isEmpty()) {
        return results;
    }

    // 记录匹配开始时间
    QTime matchingStartTime = QTime::currentTime();
    qDebug() << "======== 开始模板匹配，模板数量:" << m_loadedTemplates.size() << " ========";

    try {
        // 预处理源图像（转灰且8U）
        QTime preprocessStartTime = QTime::currentTime();
        cv::Mat processedSourceImage;
        if (sourceImage.channels() == 3) {
            cv::cvtColor(sourceImage, processedSourceImage, cv::COLOR_BGR2GRAY);
        } else {
            processedSourceImage = sourceImage.clone();
        }
        if (processedSourceImage.depth() != CV_8U) {
            processedSourceImage.convertTo(processedSourceImage, CV_8U);
        }
        int preprocessTime = preprocessStartTime.msecsTo(QTime::currentTime());
        qDebug() << "图像预处理耗时:" << preprocessTime << "ms";

        // 将源图转换为 Halcon 图像 - 优化：直接从内存创建，避免磁盘I/O
        QTime halconConvertStartTime = QTime::currentTime();
        HalconCpp::HObject hoSearch;
        try {
            // 直接从OpenCV Mat创建Halcon图像，避免文件I/O
            // 确保数据连续性
            cv::Mat continuousImage;
            if (processedSourceImage.isContinuous()) {
                continuousImage = processedSourceImage;
            } else {
                processedSourceImage.copyTo(continuousImage);
            }
            
            // 使用GenImage1直接从内存数据创建Halcon图像
            HalconCpp::GenImage1(&hoSearch, "byte", 
                               continuousImage.cols, continuousImage.rows,
                               reinterpret_cast<Hlong>(continuousImage.data));
                               
            qDebug() << "使用直接内存创建Halcon图像成功";
        } catch (const HalconCpp::HException& e) {
            qWarning() << "直接内存创建失败，回退到文件方式:" << e.ErrorMessage().TextA();
            
            // 回退方案：使用临时文件
            QString tmpDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
            if (tmpDir.isEmpty()) tmpDir = QDir::tempPath();
            QDir().mkpath(tmpDir);
            QString tmpSearchPath = QDir(tmpDir).filePath("__halcon_search__.png");
            cv::imwrite(tmpSearchPath.toStdString(), processedSourceImage);
            HalconCpp::ReadImage(&hoSearch, HalconCpp::HTuple(tmpSearchPath.toStdString().c_str()));
            QFile::remove(tmpSearchPath); // 立即清理临时文件
        }
        int halconConvertTime = halconConvertStartTime.msecsTo(QTime::currentTime());
        qDebug() << "Halcon图像转换耗时:" << halconConvertTime << "ms";
        
        // 记录模板遍历开始时间
        QTime templatesStartTime = QTime::currentTime();
        qDebug() << "-------- 开始遍历模板 --------";

        // 遍历模板，使用 Halcon 进行匹配
        int templateIndex = 0;
        for (const TemplateInfo& templateInfo : m_loadedTemplates) {
            if (!templateInfo.isSelected) continue;

            QTime templateStartTime = QTime::currentTime();
            qDebug() << "开始匹配模板" << (templateIndex + 1) << ":" << templateInfo.name;

            // 读取或创建模型（优先使用缓存）
            QTime modelLoadStartTime = QTime::currentTime();
            HalconCpp::HTuple hvModelID;
            bool modelReady = false;
            bool modelFromCache = false;

            QString cacheKey = !templateInfo.halconModelPath.isEmpty() ? templateInfo.halconModelPath
                               : (!templateInfo.imagePath.isEmpty() ? templateInfo.imagePath : templateInfo.name);

            auto cached = m_halconModelCache.find(cacheKey);
            if (cached != m_halconModelCache.end() && cached.value()) {
                hvModelID = *(cached.value());
                modelReady = true;
                modelFromCache = true;
            } else {
                try {
                    if (!templateInfo.halconModelPath.isEmpty() && QFile::exists(templateInfo.halconModelPath)) {
                        HalconCpp::ReadShapeModel(HalconCpp::HTuple(templateInfo.halconModelPath.toStdString().c_str()), &hvModelID);
                        modelReady = true;
                    }
                } catch (const HalconCpp::HException& e) {
                    qWarning() << "读取Halcon模型失败，将尝试临时创建:" << e.ErrorMessage().TextA();
                }

                // 若没有shm，退化为从模板图创建临时模型
                if (!modelReady) {
                    if (templateInfo.imagePath.isEmpty() || !QFile::exists(templateInfo.imagePath)) {
                        qWarning() << "模板缺少图像或模型，跳过:" << templateInfo.name;
                        continue;
                    }
                    try {
                        HalconCpp::HObject hoTemplate;
                        HalconCpp::ReadImage(&hoTemplate, HalconCpp::HTuple(templateInfo.imagePath.toStdString().c_str()));
                        // 使用配置文件参数
                        TemplateMatchingConfig* config = TemplateMatchingConfig::instance();
                        TemplateMatchingConfig::TemplateCreationParams params = config->getTemplateCreationParams();
                        
                        HalconCpp::CreateScaledShapeModel(hoTemplate,
                            params.numLevels,
                            HalconCpp::HTuple(params.angleStart).TupleRad(), 
                            HalconCpp::HTuple(params.angleExtent).TupleRad(), 
                            HalconCpp::HTuple(params.angleStep).TupleRad(),
                            params.scaleMin, params.scaleMax, params.scaleStep,
                            params.optimization.toStdString().c_str(), 
                            params.metric.toStdString().c_str(), 
                            params.contrast.toStdString().c_str(), 
                            params.minContrast.toStdString().c_str(),
                            &hvModelID);
                        modelReady = true;
                    } catch (const HalconCpp::HException& e) {
                        qWarning() << "临时创建Halcon模型失败，跳过模板:" << templateInfo.name << e.ErrorMessage().TextA();
                        continue;
                    }
                }

                if (modelReady) {
                    m_halconModelCache.insert(cacheKey, std::make_shared<HalconCpp::HTuple>(hvModelID));
                    modelFromCache = true; // 之后帧也将使用缓存；本帧不释放
                }
            }

            int modelLoadTime = modelLoadStartTime.msecsTo(QTime::currentTime());
            qDebug() << "模板" << templateInfo.name << "模型加载耗时:" << modelLoadTime << "ms";

            // 执行匹配
            QTime matchStartTime = QTime::currentTime();
            try {
                HalconCpp::HTuple hvRow, hvCol, hvAngle, hvScale, hvScore;
                
                // 从配置文件读取匹配参数
                TemplateMatchingConfig* config = TemplateMatchingConfig::instance();
                TemplateMatchingConfig::TemplateMatchingParams matchParams = config->getTemplateMatchingParams();
                qDebug() << "matchParams.angleStart:" << matchParams.angleStart;
                qDebug() << "matchParams.angleExtent:" << matchParams.angleExtent;
                qDebug() << "matchParams.scaleMin:" << matchParams.scaleMin;
                qDebug() << "matchParams.scaleMax:" << matchParams.scaleMax;
                qDebug() << "matchParams.minScore:" << matchParams.minScore;
                qDebug() << "matchParams.numMatches:" << matchParams.numMatches;
                qDebug() << "matchParams.maxOverlap:" << matchParams.maxOverlap;
                qDebug() << "matchParams.subPixel:" << matchParams.subPixel;
                qDebug() << "matchParams.numLevels:" << matchParams.numLevels;
                qDebug() << "matchParams.greediness:" << matchParams.greediness;

                HalconCpp::FindScaledShapeModel(hoSearch,
                    hvModelID,
                    HalconCpp::HTuple(matchParams.angleStart).TupleRad(), 
                    HalconCpp::HTuple(matchParams.angleExtent).TupleRad(),
                    matchParams.scaleMin, matchParams.scaleMax,
                    matchParams.minScore,
                    matchParams.numMatches > 0 ? matchParams.numMatches : 1,
                    matchParams.maxOverlap,
                    matchParams.subPixel.toStdString().c_str(),
                    matchParams.numLevels,
                    matchParams.greediness,
                    &hvRow, &hvCol, &hvAngle, &hvScale, &hvScore);

                int matchTime = matchStartTime.msecsTo(QTime::currentTime());
                const int count = static_cast<int>(hvScore.TupleLength());
                qDebug() << "模板" << templateInfo.name << "匹配耗时:" << matchTime << "ms，找到" << count << "个匹配";

                // 将结果转换为现有结构
                for (int i = 0; i < count; ++i) {
                    TemplateMatchResult match;
                    match.templateName = templateInfo.name;
                    match.confidence = static_cast<double>(hvScore[i]);
                    match.angle = static_cast<double>(hvAngle[i]);
                    match.scale = static_cast<double>(hvScale[i]);
                    // Halcon返回 row/column 对应 y/x
                    const double row = static_cast<double>(hvRow[i]);
                    const double col = static_cast<double>(hvCol[i]);
                    // 用模板尺寸估计边界框并按匹配比例缩放（如果有模板图像），否则给定默认尺寸
                    double w = templateInfo.templateImage.empty() ? 20.0 : templateInfo.templateImage.cols;
                    double h = templateInfo.templateImage.empty() ? 20.0 : templateInfo.templateImage.rows;
                    w *= match.scale;
                    h *= match.scale;
                    // 中心点
                    match.position = QPointF(col, row);
                    match.boundingRect = QRectF(col - w * 0.5, row - h * 0.5, w, h);
                    match.timestamp = QDateTime::currentDateTime();
                    results.append(match);
                }

                // 仅当模型非缓存时释放；缓存模型由 stopTemplateMatching/析构统一释放
                if (!modelFromCache) {
                    try {
                        HalconCpp::ClearShapeModel(hvModelID);
                    } catch (const HalconCpp::HException& e) {
                        qWarning() << "释放临时Halcon模型失败:" << e.ErrorMessage().TextA();
                    }
                }

            } catch (const HalconCpp::HException& e) {
                int matchTime = matchStartTime.msecsTo(QTime::currentTime());
                qWarning() << "模板" << templateInfo.name << "Halcon匹配失败，耗时:" << matchTime << "ms，错误:" << e.ErrorMessage().TextA();
            }

            int templateTotalTime = templateStartTime.msecsTo(QTime::currentTime());
            qDebug() << "模板" << templateInfo.name << "总耗时:" << templateTotalTime << "ms" 
                     << " [模型加载:" << modelLoadTime << "ms + 匹配执行:" << (templateTotalTime - modelLoadTime) << "ms]";
            templateIndex++;
        }
        
        int allTemplatesTime = templatesStartTime.msecsTo(QTime::currentTime());
        qDebug() << "-------- 所有模板处理完成，耗时:" << allTemplatesTime << "ms --------";

    } catch (const cv::Exception& e) {
        qCritical() << "执行模板匹配时发生OpenCV异常：" << e.what();
    } catch (const HalconCpp::HException& e) {
        qCritical() << "执行模板匹配时发生Halcon异常：" << e.ErrorMessage().TextA();
    } catch (const std::exception& e) {
        qCritical() << "执行模板匹配时发生异常：" << e.what();
    }

    int totalMatchingTime = matchingStartTime.msecsTo(QTime::currentTime());
    qDebug() << "======== 模板匹配完成，总耗时:" << totalMatchingTime << "ms，找到" << results.size() << "个匹配结果 ========";

    return results;
}

void PaintingOverlay::drawMatchResults(QPainter& painter, const DrawingContext& ctx) const
{
    if (!m_isMatchingEnabled || m_currentMatches.isEmpty()) {
        return;
    }

    painter.save();

    // 设置抗锯齿
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 绘制所有匹配结果
    for (const TemplateMatchResult& match : m_currentMatches) {
        drawSingleMatchResult(painter, match, ctx);
    }

    painter.restore();
}

void PaintingOverlay::drawSingleMatchResult(QPainter& painter, const TemplateMatchResult& match, const DrawingContext& ctx) const
{
    painter.save();

    // 在paintEvent中painter已经设置了正确的变换，可以直接使用图像坐标
    QRectF matchRect = match.boundingRect;
    QPointF matchCenter = match.position;

    // 根据置信度设置颜色（绿色到红色渐变）
    QColor matchColor;
    if (match.confidence >= 0.9) {
        matchColor = QColor(0, 255, 0);      // 高置信度：绿色
    } else if (match.confidence >= 0.8) {
        matchColor = QColor(128, 255, 0);    // 中高置信度：黄绿色
    } else if (match.confidence >= 0.7) {
        matchColor = QColor(255, 255, 0);    // 中等置信度：黄色
    } else {
        matchColor = QColor(255, 128, 0);    // 低置信度：橙色
    }

    // 设置画笔 - 使用与ROI框相同的线条粗细
    QPen pen = createPen(matchColor, 2, ctx.scale, false);  // 使用createPen函数进行缩放调整
    painter.setPen(pen);

    // 绘制匹配边界框（按角度旋转并以中心为原点绘制）
    painter.save();
    painter.translate(matchCenter);
    painter.rotate(-match.angle * 180.0 / M_PI);
    painter.drawRect(QRectF(-matchRect.width() * 0.5, -matchRect.height() * 0.5,
                            matchRect.width(), matchRect.height()));
    painter.restore();

    // 绘制模板轮廓（使用 Halcon 模型轮廓，按匹配的角度和缩放变换）
    // 为避免在绘制阶段做额外的 Halcon 调用，这里只使用已缓存的点集，并由 QPainter 做几何变换
    const TemplateInfo* matchedTemplate = nullptr;
    for (const TemplateInfo& t : m_loadedTemplates) {
        if (t.name == match.templateName) {
            matchedTemplate = &t;
            break;
        }
    }

    if (matchedTemplate && !matchedTemplate->modelContours.isEmpty()) {
        painter.save();
        painter.translate(matchCenter);
        painter.rotate(-match.angle * 180.0 / M_PI);
        painter.scale(match.scale, match.scale);

        for (const QPolygonF& poly : matchedTemplate->modelContours) {
            if (!poly.isEmpty()) {
                painter.drawPolyline(poly);
            }
        }

        painter.restore();
    }

    // 绘制中心点
    painter.setBrush(QBrush(matchColor));
    double centerSize = 4.0;
    painter.drawEllipse(matchCenter, centerSize, centerSize);

    // 绘制模板名称、置信度、角度和缩放信息
    QString infoText = QString("%1\n置信度: %2%\n角度: %3°\n缩放: %4")
                       .arg(match.templateName)
                       .arg(QString::number(match.confidence * 100, 'f', 1))
                       .arg(QString::number(match.angle * 180.0 / M_PI, 'f', 1))
                       .arg(QString::number(match.scale, 'f', 2));

    // 计算文本位置（在边界框上方，增加更多间距）
    QPointF textPos = matchRect.topLeft() + QPointF(0, -10);

    // 设置文本样式
    QFont font = ctx.font;
    font.setPointSize(qMax(8, qRound(ctx.fontSize * 0.8))); // 与ROI信息文字大小一致
    font.setBold(true);
    painter.setFont(font);

    // 绘制文本背景（增加更多边距）
    QFontMetrics fm(font);
    QRect textBounds = fm.boundingRect(infoText);
    const int marginX = 16;  // 水平边距
    const int marginY = 16;  // 垂直边距
    QRectF textBackground(textPos.x() - marginX, 
                         textPos.y() - textBounds.height() - marginY,
                         textBounds.width() + 2 * marginX, 
                         textBounds.height() + 2 * marginY);

    // 绘制圆角背景
    painter.setBrush(QBrush(QColor(0, 0, 0, 180))); // 半透明黑色背景
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(textBackground, 4, 4); // 圆角半径4像素

    // 绘制文本（调整位置以适应新的边距）
    painter.setPen(QPen(Qt::white));
    painter.drawText(textPos.x(), textPos.y() - marginY/2, infoText);

    painter.restore();
}

void PaintingOverlay::updateMatchResults(const QVector<TemplateMatchResult>& matches)
{
    m_currentMatches = matches;

    // 触发重绘以显示新的匹配结果
    update();

    qDebug() << "更新匹配结果，共" << matches.size() << "个匹配";
}

void PaintingOverlay::processFrameForMatching(const cv::Mat& frame)
{
    if (!m_isMatchingEnabled || frame.empty()) {
        return;
    }

    try {
        // 执行模板匹配
        QVector<TemplateMatchResult> matches = performMatching(frame);

        // 更新匹配结果并触发重绘
        updateMatchResults(matches);

    } catch (const std::exception& e) {
        qCritical() << "处理帧进行模板匹配时发生异常：" << e.what();
    }
}

void PaintingOverlay::clearHalconModelCache(const QString& logContext)
{
    try {
        for (auto it = m_halconModelCache.begin(); it != m_halconModelCache.end(); ++it) {
            if (it.value() && it.value().get()) {
                try {
                    HalconCpp::ClearShapeModel(*(it.value()));
                } catch (const HalconCpp::HException& e) {
                    QString context = logContext.isEmpty() ? "清理" : QString("清理(%1)").arg(logContext);
                    qWarning() << QString("%1Halcon模型失败:").arg(context) << e.ErrorMessage().TextA();
                } catch (const std::exception& e) {
                    QString context = logContext.isEmpty() ? "清理" : QString("清理(%1)").arg(logContext);
                    qWarning() << QString("%1Halcon模型时发生异常:").arg(context) << e.what();
                }
            }
        }
        m_halconModelCache.clear();
        
        if (!logContext.isEmpty()) {
            qDebug() << QString("Halcon模型缓存清理完成(%1)").arg(logContext);
        }
    } catch (const std::exception& e) {
        QString context = logContext.isEmpty() ? "清理缓存" : QString("清理缓存(%1)").arg(logContext);
        qWarning() << QString("%1时发生异常:").arg(context) << e.what();
    } catch (...) {
        // 记录未知异常的详细信息，避免程序崩溃
        if (!logContext.isEmpty() && logContext != "析构") {
            qWarning() << QString("PaintingOverlay::clearHalconModelCache - 清理Halcon模型缓存时发生未知异常(%1) - 函数: clearHalconModelCache").arg(logContext);
            // 尝试获取当前异常类型信息
            try {
                std::rethrow_exception(std::current_exception());
            } catch (const std::exception& ex) {
                qWarning() << QString("重新捕获的异常信息: %1").arg(ex.what());
            } catch (...) {
                qWarning() << "无法获取具体异常类型信息";
            }
        }
    }
}
