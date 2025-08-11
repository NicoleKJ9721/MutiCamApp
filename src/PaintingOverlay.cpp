#include "PaintingOverlay.h"
#include "MutiCamApp.h"
#include <QApplication>
#include <QDebug>
#include <QtMath>
#include <QMessageBox>
#include <QTime>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

PaintingOverlay::PaintingOverlay(QWidget *parent)
    : QWidget(parent)
    , m_isDrawingMode(false)
    , m_currentDrawingTool(DrawingTool::None)
    , m_hasCurrentLine(false)
    , m_hasCurrentCircle(false)
    , m_hasCurrentParallel(false)
    , m_hasCurrentTwoLines(false)
    , m_hasCurrentROI(false)
    , m_hasValidMousePos(false)
    , m_selectionEnabled(true)
    , m_scaleFactor(1.0)
    , m_imageSize(QSize())
    , m_drawingContextValid(false)
    , m_hasCurrentLineSegment(false)
    , m_hasCurrentFineCircle(false)
    , m_gridSpacing(0)              // 默认不显示网格
    , m_gridColor(Qt::red)          // 红色网格
    , m_gridStyle(Qt::DashLine)     // 虚线样式
    , m_gridWidth(1)                // 线宽1像素
    , m_gridCacheValid(false)       // 网格缓存初始无效
    , m_lastGridImageSize(QSize())  // 初始图像尺寸
    , m_lastGridSpacing(0)          // 初始网格间距
    , m_edgeDetector(nullptr)
    , m_shapeDetector(nullptr)
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

    // 清理缓存的图像数据
    if (!m_lastProcessedFrame.empty()) {
        m_lastProcessedFrame.release();
    }

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
    m_hasCurrentROI = false;
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
    QSet<int> twoLinesToRemove, roisToRemove;

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

        // 2. 绘制所有已完成的图形（在高亮之上）
        drawPoints(painter, ctx);
        drawLines(painter, ctx);
        drawLineSegments(painter, ctx);
        drawCircles(painter, ctx);
        drawFineCircles(painter, ctx);
        drawParallels(painter, ctx);
        drawTwoLines(painter, ctx);
        drawROIs(painter, ctx);

        // 3. 【关键】绘制当前正在预览的图形（在最上层）
        if (m_isDrawingMode) {
            drawCurrentPreview(painter, ctx);
        }
        
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
        if (m_isDrawingMode) {
            // 检查是否有选中的对象
            bool hasSelection = !m_selectedPoints.isEmpty() || !m_selectedLines.isEmpty() ||
                               !m_selectedCircles.isEmpty() || !m_selectedFineCircles.isEmpty() ||
                               !m_selectedParallels.isEmpty() || !m_selectedTwoLines.isEmpty() ||
                               !m_selectedLineSegments.isEmpty() || !m_selectedParallelMiddleLines.isEmpty() ||
                               !m_selectedROIs.isEmpty();

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

    if (m_selectionEnabled) {
        // 处理选择逻辑
        bool ctrlPressed = (event->modifiers() & Qt::ControlModifier) != 0;
        handleSelectionClick(imagePos, ctrlPressed);
        return;
    }
    
    if (!m_isDrawingMode) {
        // 静默返回，避免频繁的鼠标事件日志
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
        default:
            break;
    }
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
    if (m_hasCurrentROI && !m_currentROI.isCompleted) {
        if (m_currentROI.points.size() == 1) {
            // ROI绘制：添加第二个点进行实时预览
            m_currentROI.points.append(imagePos);
        } else if (m_currentROI.points.size() >= 2) {
            // ROI绘制：更新第二个点进行实时预览
            m_currentROI.points[1] = imagePos;
        }
    }

    // 只在绘图模式或有当前绘制对象时更新
    if (m_isDrawingMode || m_hasCurrentLine || m_hasCurrentCircle ||
        m_hasCurrentFineCircle || m_hasCurrentParallel || m_hasCurrentTwoLines || m_hasCurrentROI) {
        update(); // 更新预览
    }
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
                       !m_selectedLineSegments.isEmpty() || !m_selectedParallelMiddleLines.isEmpty() ||
                       !m_selectedBisectorLines.isEmpty() || !m_selectedROIs.isEmpty();
    
    if (!hasSelection) {
        QWidget::contextMenuEvent(event);
        return;
    }
    
    // 创建右键菜单
    QMenu contextMenu(this);
    
    // 添加删除选项
    QAction *deleteAction = contextMenu.addAction("删除");
    deleteAction->setIcon(QIcon("../icon/delete.svg"));
    
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

    // 点与线距离测量
    if (m_selectedPoints.size() == 1 && m_selectedLines.size() == 1 &&
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

    // 线与圆关系分析
    if (m_selectedLines.size() == 1 && m_selectedCircles.size() == 1 &&
        m_selectedPoints.isEmpty() && m_selectedFineCircles.isEmpty()) {
        lineToCircleAction = contextMenu.addAction("线与圆关系");
    }

    // 线与精细圆关系分析
    QAction *lineToFineCircleAction = nullptr;
    if (m_selectedLines.size() == 1 && m_selectedFineCircles.size() == 1 &&
        m_selectedPoints.isEmpty() && m_selectedCircles.isEmpty()) {
        lineToFineCircleAction = contextMenu.addAction("线与精细圆关系");
    }

    // 两线段夹角测量
    if (m_selectedLineSegments.size() == 2 && m_selectedPoints.isEmpty() &&
        m_selectedLines.isEmpty() && m_selectedCircles.isEmpty()) {
        lineAngleAction = contextMenu.addAction("线段夹角");
    }

    // 点与平行线中线距离测量
    if (m_selectedPoints.size() == 1 && m_selectedParallelMiddleLines.size() == 1 &&
        m_selectedLines.isEmpty() && m_selectedCircles.isEmpty()) {
        pointToMidlineAction = contextMenu.addAction("点与平行线中线距离");
    }

    // 点与角平分线距离测量
    if (m_selectedPoints.size() == 1 && m_selectedBisectorLines.size() == 1 &&
        m_selectedLines.isEmpty() && m_selectedCircles.isEmpty()) {
        pointToBisectorAction = contextMenu.addAction("点与角平分线距离");
    }

    // 两条直线夹角测量
    QAction *twoLinesAngleAction = nullptr;
    if (m_selectedLines.size() == 2 && m_selectedPoints.isEmpty() &&
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
        performComplexMeasurement("点与线距离");
    } else if (selectedAction == pointToCircleAction) {
        performComplexMeasurement("点与圆距离");
    } else if (selectedAction == pointToFineCircleAction) {
        performComplexMeasurement("点与精细圆距离");
    } else if (selectedAction == lineToCircleAction) {
        performComplexMeasurement("线与圆关系");
    } else if (selectedAction == lineToFineCircleAction) {
        performComplexMeasurement("线与精细圆关系");
    } else if (selectedAction == lineAngleAction) {
        performComplexMeasurement("线段夹角");
    } else if (selectedAction == twoLinesAngleAction) {
        performComplexMeasurement("两线夹角");
    } else if (selectedAction == pointToMidlineAction) {
        performComplexMeasurement("点与平行线中线距离");
    } else if (selectedAction == pointToBisectorAction) {
        performComplexMeasurement("点与角平分线距离");
    }
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
                QString result = QString("圆形: 半径 %.1f, 面积 %.1f, 周长 %.1f")
                                .arg(m_currentCircle.radius).arg(area).arg(circumference);

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
                QString result = QString("精细圆: 半径 %.1f, 面积 %.1f, 周长 %.1f")
                                .arg(m_currentFineCircle.radius).arg(area).arg(circumference);
                
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
                QString result = QString("平行线: 距离 %.1f, 角度 %.1f°")
                                .arg(m_currentParallel.distance).arg(m_currentParallel.angle);

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
                QString result = QString("两线夹角: %.1f°")
                                .arg(m_currentTwoLines.angle);

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
    const double heightScale = std::max(1.0, std::min(ctx.scale, 4.0));
    const double innerRadius = 12 * heightScale;
    const double textpadding = qMax(4.0, ctx.fontSize * 0.5);  // 动态padding，字体大小的一半
    
    for (int i = 0; i < m_points.size(); ++i) {
        const QPointF& point = m_points[i].position;  // 直接使用图像坐标
        // 绘制点（绿色实心圆）- 使用预创建的画刷
        painter.setPen(Qt::NoPen);
        painter.setBrush(ctx.greenBrush);
        painter.drawEllipse(point, innerRadius, innerRadius);
        
        // 绘制坐标文本（使用统一的布局思维）
        QString coordText = QString("(%1,%2)")
            .arg(static_cast<int>(point.x()))
            .arg(static_cast<int>(point.y()));
        
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
    
    // 使用预创建的画笔或根据需要创建特定颜色的画笔
    int desiredThickness = qMax(2, static_cast<int>(line.thickness * 2.0 * ctx.scale));

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
    // 绘制起始点标记（动态缩放的小圆点）
    double pointRadius = qMax(2.0, 3.0 * ctx.scale);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(line.color));
    painter.drawEllipse(start, pointRadius, pointRadius);
    
    // Calculate and display angle information
    double angle = calculateLineAngle(start, end);
    
    // 格式化角度文本（包含度数符号）
    QString angleText = QString::asprintf("%.1f°", angle);
    
    // 动态计算文本布局参数
    double textOffset = qMax(8.0, 10.0 * ctx.scale);
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
    double textOffset = qMax(10.0, 15.0 * ctx.scale);
    int connectionLineWidth = qMax(1, static_cast<int>(2.0 * ctx.scale));
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
    
    // 如果有3个点，绘制圆形（包括预览状态）
    if (circle.points.size() >= 3) {
        QPointF centerImage;
        double radiusImage;

        if (circle.isCompleted) {
            // 已完成的圆形，使用存储的圆心和半径
            centerImage = circle.center;
            radiusImage = circle.radius;
        } else {
            // 预览状态，实时计算圆心和半径
            if (!calculateCircleFromThreePoints(circle.points, centerImage, radiusImage)) {
                // 如果计算失败，不绘制圆形
                return;
            }
        }

        // 绘制圆形（使用绿色，预览时使用虚线）
        if (circle.isCompleted) {
            painter.setPen(ctx.greenPen);
        } else {
            painter.setPen(ctx.greenDashedPen); // 预览时使用虚线
        }
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(centerImage, radiusImage, radiusImage);
        
        // 动态计算圆心标记尺寸（与画点功能相同）
        const double heightScale = std::max(1.0, std::min(ctx.scale, 4.0));
        const double centerMarkRadius = 12 * heightScale;

        // 绘制圆心 - 红色实心圆
        painter.setPen(Qt::NoPen);
        painter.setBrush(ctx.redBrush);
        painter.drawEllipse(centerImage, centerMarkRadius, centerMarkRadius);

        // 只在圆形完成后显示文字信息
        if (circle.isCompleted) {
            // 显示圆心坐标和半径 - 使用与点相同的布局方式
            QString centerText = QString::asprintf("(%.1f,%.1f)", circle.center.x(), circle.center.y());
            QString radiusText = QString::asprintf("R=%.1f", circle.radius);

            // 计算将文本框定位到圆心右上角所需的偏移量（与点的布局一致）
            QFontMetrics fm(ctx.font);
            QRect centerTextBoundingRect = fm.boundingRect(centerText);
            double centerBgHeight = centerTextBoundingRect.height() + 2 * textPadding;

            // 第一个文本框偏移量（与点相同：右上角）
            QPointF centerOffset(centerMarkRadius, -centerMarkRadius - centerBgHeight);

            // 1. 绘制第一个文本框（坐标文本）- 使用绿色
            drawTextWithBackground(painter, centerImage, centerText, ctx.font, Qt::green, Qt::black, textPadding, bgBorderWidth, centerOffset);

            // 2. 计算第二个文本框的位置（紧挨着第一个文本框下方）
            QRect radiusTextBoundingRect = fm.boundingRect(radiusText);
            double radiusBgHeight = radiusTextBoundingRect.height() + 2 * textPadding;
            QPointF radiusOffset(centerMarkRadius, -centerMarkRadius - centerBgHeight + centerBgHeight);

            // 绘制第二个文本框（半径文本）- 使用绿色
            drawTextWithBackground(painter, centerImage, radiusText, ctx.font, Qt::green, Qt::black, textPadding, bgBorderWidth, radiusOffset);
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
    double textOffset = qMax(8.0, 10.0 * ctx.scale);
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

        // 绘制圆心标记（使用红色，与简单圆保持一致）
        // 动态计算圆心标记尺寸（与简单圆功能相同）
        const double heightScale = std::max(1.0, std::min(ctx.scale, 4.0));
        const double centerMarkRadius = 12 * heightScale;
        painter.setPen(Qt::NoPen);
        painter.setBrush(ctx.redBrush);
        painter.drawEllipse(centerImage, centerMarkRadius, centerMarkRadius);
        
        // 显示圆心坐标和半径信息
        QString centerText = QString::asprintf("(%.1f, %.1f)", fineCircle.center.x(), fineCircle.center.y());
        QString radiusText = QString::asprintf("%.1f", fineCircle.radius);
        
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
    double textOffset = qMax(10.0, 15.0 * ctx.scale);
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
        QString distanceText = QString::asprintf("%.1f", parallel.distance) + "px";
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

// 两线
void PaintingOverlay::drawSingleTwoLines(QPainter& painter, const TwoLinesObject& twoLines, const DrawingContext& ctx) const
{
    // 使用预创建的字体和画笔
    painter.setFont(ctx.font);
    
    // 计算动态尺寸参数
    double fontSize = ctx.fontSize;
    double pointInnerRadius = qMax(3.0, 5.0 * ctx.scale);
    double pointOuterRadius = qMax(5.0, 8.0 * ctx.scale);
    int pointPenWidth = qMax(1, static_cast<int>(2.0 * ctx.scale));
    double textOffset = qMax(10.0, 15.0 * ctx.scale);
    double textPadding = qMax(4.0, ctx.fontSize * 0.5);  // 动态padding，字体大小的一半
    int bgBorderWidth = 1;
    double desiredThickness = twoLines.thickness * 2.0 * ctx.scale;
    int thickLine = qMax(2, static_cast<int>(desiredThickness));
    double intersectionRadius = qMax(16.0, 40.0 * ctx.scale);
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
        QString coordText = QString::asprintf("(%.1f,%.1f)", twoLines.intersection.x(), twoLines.intersection.y());
        
        // 动态计算文本布局参数，直接使用期望的屏幕像素值
        double textOffset = qMax(8.0, 10.0 * ctx.scale);
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

        case DrawingTool::ROI_LineDetect:
        case DrawingTool::ROI_CircleDetect:
            if (m_hasCurrentROI && !m_currentROI.points.isEmpty()) {
                drawSingleROI(painter, m_currentROI, ctx);
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
            double radius = 25.0 * std::max(1.0, std::min(ctx.scale, 4.0));
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
            if (!circle.isVisible || circle.points.size() < 3) continue;

            // 计算圆心和半径
            QPointF center;
            double radius;
            if (calculateCircleFromThreePoints(circle.points, center, radius)) {
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

QPen PaintingOverlay::createPen(const QColor& color, int width, double scale, bool dashed) const
{
    QPen pen(color);
    pen.setWidth(qMax(1, static_cast<int>(width / scale)));
    if (dashed) {
        pen.setStyle(Qt::DashLine);
    }
    return pen;
}

QFont PaintingOverlay::createFont(int targetScreenSize, double scale) const
{
    QFont font;
    // {{ AURA-X: Modify - 基于图像分辨率的字体大小，不受显示缩放影响. Approval: 寸止(ID:font_size_fix). }}
    // 直接使用目标大小，不进行缩放补偿，因为字体大小已经基于图像分辨率计算
    font.setPointSizeF(qMax(8.0, static_cast<double>(targetScreenSize)));
    font.setBold(true);
    return font;
}

double PaintingOverlay::calculateFontSize() const
{
    // {{ AURA-X: Modify - 基于图像分辨率计算字体大小，而不是显示缩放比例. Approval: 寸止(ID:font_size_fix). }}
    if (m_imageSize.isEmpty()) {
        return 8.0; // Default font size
    }

    // 基于图像实际分辨率计算字体大小，确保保存时文字大小合适
    // 使用图像高度作为基准，2448x2048分辨率下字体大小约为30像素
    double imageHeight = m_imageSize.height();
    double baseFontSize = imageHeight / 66.67;  // 基础字体大小：图像高度的1.5%

    // 限制字体大小范围，确保可读性
    double fontSize = qMax(14.0, qMin(baseFontSize, 50.0));

    qDebug() << "字体大小计算：图像尺寸" << m_imageSize << "，基础字体大小" << baseFontSize << "，最终字体大小" << fontSize;

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
    // 1. 计算尺寸
    QFontMetrics fm(font);
    QRect textBoundingRect = fm.boundingRect(text);

    // 2. 创建一个以(0,0)为左上角的、包含内边距的总内容框
    QRectF contentRectWithPadding = QRectF(0, 0, textBoundingRect.width(), textBoundingRect.height())
                                     .adjusted(-padding, -padding, padding, padding);

    // 3. 将内容框移动到最终位置
    contentRectWithPadding.moveTopLeft(anchorPoint + offset);

    // 4. 绘制背景和边框
    painter.setPen(createPen(textColor, borderWidth, m_scaleFactor)); // 使用文本颜色作为边框颜色
    painter.setBrush(QBrush(bgColor));
    painter.drawRect(contentRectWithPadding);

    // 5. 在内容框内居中绘制文本
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
    lineSegment.label = QString("长度: %1, 角度: %2°").arg(lineSegment.length, 0, 'f', 1).arg(angleDegrees, 0, 'f', 1);

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
    QString result = QString("线段: 长度 %1, 角度 %2°").arg(lineSegment.length, 0, 'f', 1).arg(angleDegrees, 0, 'f', 1);
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
        newLineSegment.thickness = 1.0;
        newLineSegment.isDashed = false;
        newLineSegment.isCompleted = true;
        newLineSegment.label = QString("LineSegment_%1").arg(m_lineSegments.size() + 1);
        
        // 添加到线段列表
        m_lineSegments.append(newLineSegment);
        
        // 提交绘图动作
        DrawingAction action;
        action.type = DrawingAction::AddLineSegment;
        action.index = m_lineSegments.size() - 1;
        commitDrawingAction(action);
        
        // 清除当前点
        m_currentPoints.clear();
        
        // 停止绘制
        stopDrawing();
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
    
    // 创建线段画笔
    int desiredThickness = qMax(2, static_cast<int>(lineSegment.thickness * 2.0 * ctx.scale));
    QPen linePen = createPen(lineSegment.color, desiredThickness, ctx.scale);
    linePen.setCapStyle(Qt::RoundCap);
    
    if (lineSegment.isDashed) {
        linePen.setStyle(Qt::DashLine);
    }
    
    // 绘制线段（直接连接两点，不延伸）
    painter.setPen(linePen);
    painter.drawLine(start, end);
    
    // 绘制端点标记
    double pointRadius = qMax(2.0, 3.0 * ctx.scale);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(lineSegment.color));
    painter.drawEllipse(start, pointRadius, pointRadius);
    painter.drawEllipse(end, pointRadius, pointRadius);
    
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

            lengthText = QString("长度: %1").arg(length, 0, 'f', 1);
            angleText = QString("角度: %1°").arg(angle, 0, 'f', 1);
        }
    } else if (lengthText.isEmpty()) {
        // 只缺少长度信息，使用标签内容
        lengthText = lineSegment.label;
    }

    // 计算线段中点作为文本位置
    QPointF midPoint = (start + end) / 2.0;

    // 动态计算文本布局参数
    double textOffset = qMax(8.0, 10.0 * ctx.scale);
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
        if (!circle.isVisible || circle.points.size() < 3) continue;
        
        QPointF center;
        double radius;
        if (!calculateCircleFromThreePoints(circle.points, center, radius)) {
            continue;
        }
        
        double distance = sqrt(pow(testPos.x() - center.x(), 2) + pow(testPos.y() - center.y(), 2));
        // 修改逻辑：只有在圆周附近才能选中
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
        if (m_selectedPoints.size() == 1 && m_selectedLines.size() == 1) {
            int pointIndex = *m_selectedPoints.begin();
            int lineIndex = *m_selectedLines.begin();

            if (pointIndex >= 0 && pointIndex < m_points.size() &&
                lineIndex >= 0 && lineIndex < m_lines.size()) {

                const PointObject& point = m_points[pointIndex];
                const LineObject& line = m_lines[lineIndex];

                if (line.points.size() >= 2) {
                    // 计算垂足点
                    QPointF footPoint = calculatePerpendicularFoot(
                        point.position, line.points[0], line.points[1]);

                    // 计算距离
                    double distance = calculatePointToLineDistance(
                        point.position, line.points[0], line.points[1]);

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
                    perpendicular.label = QString("距离: %1").arg(distance, 0, 'f', 2);

                    // 添加到线段列表
                    m_lineSegments.append(perpendicular);

                    // 记录历史
                    DrawingAction action;
                    action.type = DrawingAction::AddLineSegment;
                    action.index = m_lineSegments.size() - 1;
                    commitDrawingAction(action);

                    // 发送测量完成信号
                    QString result = QString("点到直线距离: %1 像素").arg(distance, 0, 'f', 2);
                    emit measurementCompleted(m_viewName, result);

                    // 清除选择并更新显示
                    clearSelection();
                    emit drawingDataChanged(m_viewName);
                    update();
                }
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
                        toCircumference.label = QString("距离: %1").arg(distanceToCircumference, 0, 'f', 2);

                        m_lineSegments.append(toCircumference);

                        // 记录历史
                        DrawingAction action;
                        action.type = DrawingAction::AddLineSegment;
                        action.index = m_lineSegments.size() - 1;
                        commitDrawingAction(action);

                        // 发送测量完成信号
                        QString result = QString("点到圆距离: %1").arg(distanceToCircumference, 0, 'f', 2);
                        emit measurementCompleted(m_viewName, result);

                        // 清除选择并更新显示
                        clearSelection();
                        emit drawingDataChanged(m_viewName);
                        update();
                    }
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
                        toCircumference.label = QString("距离: %1").arg(distanceToCircumference, 0, 'f', 2);

                        m_lineSegments.append(toCircumference);

                        // 记录历史
                        DrawingAction action;
                        action.type = DrawingAction::AddLineSegment;
                        action.index = m_lineSegments.size() - 1;
                        commitDrawingAction(action);

                        // 发送测量完成信号
                        QString result = QString("点到精细圆距离: %1").arg(distanceToCircumference, 0, 'f', 2);
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
        if (m_selectedLines.size() == 1 && m_selectedCircles.size() == 1) {
            int lineIndex = *m_selectedLines.begin();
            int circleIndex = *m_selectedCircles.begin();

            if (lineIndex >= 0 && lineIndex < m_lines.size() &&
                circleIndex >= 0 && circleIndex < m_circles.size()) {

                const LineObject& line = m_lines[lineIndex];
                const CircleObject& circle = m_circles[circleIndex];

                if (line.points.size() >= 2 && circle.isCompleted) {
                    // 计算圆心到直线的垂足
                    QPointF footPoint = calculatePerpendicularFoot(
                        circle.center, line.points[0], line.points[1]);

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
                        double distanceToCenter = calculatePointToLineDistance(circle.center, line.points[0], line.points[1]);
                        double distanceToCircle = abs(distanceToCenter - circle.radius);
                        perpendicular.length = distanceToCircle;
                        perpendicular.label = QString("距离: %1").arg(distanceToCircle, 0, 'f', 2);

                        m_lineSegments.append(perpendicular);

                        // 记录历史
                        DrawingAction action;
                        action.type = DrawingAction::AddLineSegment;
                        action.index = m_lineSegments.size() - 1;
                        commitDrawingAction(action);

                        // 分析关系并发送信号
                        QString relationResult = analyzeLineCircleRelation(
                            line.points[0], line.points[1], circle.center, circle.radius);
                        emit measurementCompleted(m_viewName, relationResult);
                    }

                    // 清除选择并更新显示
                    clearSelection();
                    emit drawingDataChanged(m_viewName);
                    update();
                }
            }
        }
    } else if (measurementType == "线与精细圆关系") {
        if (m_selectedLines.size() == 1 && m_selectedFineCircles.size() == 1) {
            QList<int> lineIndices = m_selectedLines.values();
            QList<int> fineCircleIndices = m_selectedFineCircles.values();
            int lineIndex = lineIndices[0];
            int fineCircleIndex = fineCircleIndices[0];

            if (lineIndex >= 0 && lineIndex < m_lines.size() &&
                fineCircleIndex >= 0 && fineCircleIndex < m_fineCircles.size()) {

                const LineObject& line = m_lines[lineIndex];
                const FineCircleObject& fineCircle = m_fineCircles[fineCircleIndex];

                if (line.points.size() >= 2 && fineCircle.isCompleted) {
                    // 计算圆心到直线的垂足
                    QPointF footPoint = calculatePerpendicularFoot(
                        fineCircle.center, line.points[0], line.points[1]);

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
                        double distanceToCenter = calculatePointToLineDistance(fineCircle.center, line.points[0], line.points[1]);
                        double distanceToCircle = abs(distanceToCenter - fineCircle.radius);
                        perpendicular.length = distanceToCircle;
                        perpendicular.label = QString("距离: %1").arg(distanceToCircle, 0, 'f', 2);

                        m_lineSegments.append(perpendicular);

                        // 记录历史
                        DrawingAction action;
                        action.type = DrawingAction::AddLineSegment;
                        action.index = m_lineSegments.size() - 1;
                        commitDrawingAction(action);

                        // 分析关系并发送信号
                        QString relationResult = analyzeLineCircleRelation(
                            line.points[0], line.points[1], fineCircle.center, fineCircle.radius);
                        emit measurementCompleted(m_viewName, relationResult);
                    }

                    // 清除选择并更新显示
                    clearSelection();
                    emit drawingDataChanged(m_viewName);
                    update();
                }
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
                    double angle = calculateLineSegmentAngle(
                        line1.points[0], line1.points[1],
                        line2.points[0], line2.points[1]);

                    // 发送测量完成信号
                    QString result = QString("线段夹角: %1 度").arg(angle, 0, 'f', 2);
                    emit measurementCompleted(m_viewName, result);

                    // 清除选择并更新显示
                    clearSelection();
                    update();
                }
            }
        }
    } else if (measurementType == "点与平行线中线距离") {
        if (m_selectedPoints.size() == 1 && m_selectedParallelMiddleLines.size() == 1) {
            QList<int> pointIndices = m_selectedPoints.values();
            QList<int> parallelIndices = m_selectedParallelMiddleLines.values();
            int pointIndex = pointIndices[0];
            int parallelIndex = parallelIndices[0];

            if (pointIndex >= 0 && pointIndex < m_points.size() &&
                parallelIndex >= 0 && parallelIndex < m_parallels.size()) {

                const PointObject& point = m_points[pointIndex];
                const ParallelObject& parallel = m_parallels[parallelIndex];

                if (parallel.isCompleted && parallel.points.size() >= 3) {
                    // 计算点到平行线中线的距离
                    double distance = calculatePointToLineDistance(point.position, parallel.midStart, parallel.midEnd);

                    // 计算垂足
                    QPointF footPoint = calculatePerpendicularFoot(point.position, parallel.midStart, parallel.midEnd);

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
                    perpendicular.label = QString("距离: %1").arg(distance, 0, 'f', 2);

                    // 添加到线段列表
                    m_lineSegments.append(perpendicular);

                    // 记录历史
                    DrawingAction action;
                    action.type = DrawingAction::AddLineSegment;
                    action.index = m_lineSegments.size() - 1;
                    commitDrawingAction(action);

                    // 发送测量完成信号
                    QString result = QString("点到平行线中线距离: %1").arg(distance, 0, 'f', 2);
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
                    perpendicular.label = QString("距离: %1").arg(distance, 0, 'f', 2);

                    // 添加到线段列表
                    m_lineSegments.append(perpendicular);

                    // 记录历史
                    DrawingAction action;
                    action.type = DrawingAction::AddLineSegment;
                    action.index = m_lineSegments.size() - 1;
                    commitDrawingAction(action);

                    // 发送测量完成信号
                    QString result = QString("点到角平分线距离: %1").arg(distance, 0, 'f', 2);
                    emit measurementCompleted(m_viewName, result);

                    // 清除选择并更新显示
                    clearSelection();
                    emit drawingDataChanged(m_viewName);
                    update();
                }
            }
        }
    } else if (measurementType == "两线夹角") {
        if (m_selectedLines.size() == 2) {
            QList<int> indices = m_selectedLines.values();
            int index1 = indices[0];
            int index2 = indices[1];

            if (index1 >= 0 && index1 < m_lines.size() &&
                index2 >= 0 && index2 < m_lines.size()) {

                const LineObject& line1 = m_lines[index1];
                const LineObject& line2 = m_lines[index2];

                if (line1.points.size() >= 2 && line2.points.size() >= 2) {
                    // 计算两条直线的交点
                    QPointF intersection;
                    bool hasIntersection = calculateLineIntersection(
                        line1.points[0], line1.points[1],
                        line2.points[0], line2.points[1],
                        intersection);

                    if (hasIntersection) {
                        // 计算夹角
                        double angle = calculateLineSegmentAngle(
                            line1.points[0], line1.points[1],
                            line2.points[0], line2.points[1]);

                        // 创建一个TwoLinesObject来显示交点和夹角
                        TwoLinesObject twoLinesDisplay;
                        twoLinesDisplay.points.append(line1.points[0]);
                        twoLinesDisplay.points.append(line1.points[1]);
                        twoLinesDisplay.points.append(line2.points[0]);
                        twoLinesDisplay.points.append(line2.points[1]);
                        twoLinesDisplay.intersection = intersection;
                        twoLinesDisplay.angle = angle;
                        twoLinesDisplay.isCompleted = true;
                        twoLinesDisplay.color = Qt::blue;
                        twoLinesDisplay.thickness = 2.0;
                        twoLinesDisplay.isVisible = true;
                        twoLinesDisplay.label = QString("夹角: %.1f°").arg(angle);

                        // 添加到两线列表
                        m_twoLines.append(twoLinesDisplay);

                        // 记录历史
                        DrawingAction action;
                        action.type = DrawingAction::AddTwoLines;
                        action.index = m_twoLines.size() - 1;
                        commitDrawingAction(action);

                        // 发送测量完成信号
                        QString result = QString("两线夹角: %.1f°\n交点坐标: (%.1f, %.1f)")
                                        .arg(angle).arg(intersection.x()).arg(intersection.y());
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

    return QString("直线与圆%1\n圆心到直线距离: %2 像素\n圆半径: %3 像素")
           .arg(relation).arg(distanceToCenter, 0, 'f', 2).arg(radius, 0, 'f', 2);
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
    cosAngle = std::max(-1.0, std::min(1.0, cosAngle));

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

void PaintingOverlay::drawSingleROI(QPainter& painter, const ROIObject& roi, const DrawingContext& ctx) const
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
        double textPadding = qMax(4.0, ctx.fontSize * 0.5);  // 动态padding，字体大小的一半
        int bgBorderWidth = 1;
        QPointF labelPos = rect.topLeft() + QPointF(5, -5);
        drawTextWithBackground(painter, labelPos, displayText, ctx.font, roi.color, Qt::white, textPadding, bgBorderWidth, QPointF(0, 0));
    }

    // 如果正在检测，绘制检测类型提示
    if (roi.isDetecting) {
        QString typeText;
        if (roi.detectionType == DrawingTool::ROI_LineDetect) {
            typeText = "直线检测";
        } else if (roi.detectionType == DrawingTool::ROI_CircleDetect) {
            typeText = "圆形检测";
        }

        if (!typeText.isEmpty()) {
            // 使用统一的文字样式参数
            double textPadding = qMax(4.0, ctx.fontSize * 0.5);  // 动态padding，字体大小的一半
            int bgBorderWidth = 1;
            QPointF typePos = rect.center();
            drawTextWithBackground(painter, typePos, typeText, ctx.font, Qt::yellow, Qt::black, textPadding, bgBorderWidth, QPointF(0, 0));
        }
    }
}

void PaintingOverlay::handleROIDrawingClick(const QPointF& pos)
{
    qDebug() << "handleROIDrawingClick called at position:" << pos;
    qDebug() << "m_hasCurrentROI:" << m_hasCurrentROI << "points size:" << (m_hasCurrentROI ? m_currentROI.points.size() : 0);
    if (!m_hasCurrentROI) {
        // 开始绘制ROI
        m_currentROI = ROIObject();
        m_currentROI.points.append(pos);
        m_currentROI.detectionType = m_currentDrawingTool;
        m_currentROI.label = QString("ROI_%1").arg(m_rois.size() + 1);
        m_currentROI.color = Qt::red;        // 使用红色更明显
        m_currentROI.thickness = 3;          // 更粗的线条
        m_hasCurrentROI = true;

        qDebug() << "开始绘制ROI，起点：" << pos;
    } else if (m_currentROI.points.size() >= 1 && !m_currentROI.isCompleted) {
        // 完成ROI绘制
        qDebug() << "准备完成ROI绘制，当前points数量：" << m_currentROI.points.size();
        if (m_currentROI.points.size() == 1) {
            m_currentROI.points.append(pos);
        } else {
            m_currentROI.points[1] = pos;  // 更新第二个点
        }
        m_currentROI.isCompleted = true;

        qDebug() << "完成ROI绘制，终点：" << pos;

        // 添加到ROI列表
        m_rois.append(m_currentROI);

        // 记录历史
        DrawingAction action;
        action.type = DrawingAction::AddROI;
        action.source = DrawingAction::ManualDrawing;  // 手动绘制ROI
        action.index = m_rois.size() - 1;
        commitDrawingAction(action);

        // 立即执行自动检测
        performAutoDetection(m_currentROI);

        // 清除当前ROI数据
        clearCurrentROIData();

        // 发出信号
        emit drawingCompleted(m_viewName);
        emit drawingDataChanged(m_viewName);
    } else {
        qDebug() << "ROI点击未处理 - hasCurrentROI:" << m_hasCurrentROI
                 << "points size:" << (m_hasCurrentROI ? m_currentROI.points.size() : 0)
                 << "isCompleted:" << (m_hasCurrentROI ? m_currentROI.isCompleted : false);
    }

    update();
}

int PaintingOverlay::hitTestROI(const QPointF& testPos, double tolerance) const
{
    for (int i = 0; i < m_rois.size(); ++i) {
        const ROIObject& roi = m_rois[i];
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

void PaintingOverlay::setCurrentROIData(const ROIObject& currentROI)
{
    m_currentROI = currentROI;
    m_hasCurrentROI = true;
}

void PaintingOverlay::clearCurrentROIData()
{
    m_currentROI = ROIObject();
    m_hasCurrentROI = false;
}

void PaintingOverlay::setROIsData(const QVector<ROIObject>& rois)
{
    m_rois = rois;
    update();
}

void PaintingOverlay::performAutoDetection(const ROIObject& roi)
{
    if (!m_edgeDetector || !m_shapeDetector) {
        qDebug() << "图像处理器未初始化";
        return;
    }

    // 检测频率限制（避免过于频繁的检测）
    if (m_lastDetectionTime.isValid() && m_lastDetectionTime.msecsTo(QTime::currentTime()) < 500) {
        qDebug() << "检测频率过高，跳过本次检测";
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
    detectedLineObj.thickness = 3;
    detectedLineObj.label = QString("自动检测直线 (长度: %.1f px, 角度: %.1f°, 置信度: %.1f)")
                           .arg(bestLine.length).arg(bestLine.angle).arg(bestLine.confidence);
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

    // 发出信号
    QString result = QString("自动直线检测成功：长度 %.1f 像素，角度 %.1f° (共检测到%2条直线)")
                    .arg(bestLine.length).arg(bestLine.angle).arg(detectedLines.size());
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
    detectedCircleObj.thickness = 3;
    detectedCircleObj.center = QPointF(bestCircle.center.x(), bestCircle.center.y());
    detectedCircleObj.radius = bestCircle.radius;
    detectedCircleObj.label = QString("自动检测圆形 (半径: %1 px, 中心: (%2,%3), 置信度: %.1f)")
                             .arg(bestCircle.radius)
                             .arg(bestCircle.center.x()).arg(bestCircle.center.y())
                             .arg(bestCircle.confidence);

    // 添加到圆形列表
    m_circles.append(detectedCircleObj);

    // 记录历史
    DrawingAction action;
    action.type = DrawingAction::AddCircle;
    action.source = DrawingAction::AutoDetection;  // 自动检测
    action.index = m_circles.size() - 1;
    commitDrawingAction(action);

    // 发出信号
    QString result = QString("自动圆形检测成功：半径 %1 像素，置信度 %.1f (共检测到%2个圆形)")
                    .arg(bestCircle.radius).arg(bestCircle.confidence).arg(detectedCircles.size());
    emit measurementCompleted(m_viewName, result);
    emit drawingDataChanged(m_viewName);

    int elapsedMs = startTime.msecsTo(QTime::currentTime());
    qDebug() << "圆形检测完成：" << result;
    qDebug() << QString("检测统计 - 候选圆形: %1, 最佳圆形中心: (%2,%3), 耗时: %4ms")
                .arg(detectedCircles.size()).arg(bestCircle.center.x()).arg(bestCircle.center.y()).arg(elapsedMs);
    update();
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

            // 2. 绘制所有已完成的图形（在高亮之上）
            drawPoints(painter, ctx);
            drawLines(painter, ctx);
            drawLineSegments(painter, ctx);
            drawCircles(painter, ctx);
            drawFineCircles(painter, ctx);
            drawParallels(painter, ctx);
            drawTwoLines(painter, ctx);
            drawROIs(painter, ctx);

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