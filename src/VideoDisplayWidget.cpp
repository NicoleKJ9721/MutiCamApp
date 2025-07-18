#include "VideoDisplayWidget.h"
#include "MutiCamApp.h"
#include <QPainter>
#include <QDebug>
#include <QPainterPath>
#include <QApplication>
#include <QVector2D>
#include <cmath>
// {{ AURA-X: Add - 右键菜单相关头文件. Approval: 寸止(ID:context_menu_delete). }}
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>

VideoDisplayWidget::VideoDisplayWidget(QWidget *parent)
    : QLabel(parent)
    , m_isDrawingMode(false)
    , m_currentDrawingTool(DrawingTool::None)
    , m_cachedScaleFactor(1.0)
    , m_cachedImageOffset(0, 0)
    , m_cachedWidgetSize(0, 0)
    , m_cachedImageSize(0, 0)
    , m_hasCurrentLine(false)
    , m_hasCurrentCircle(false)
    , m_hasCurrentFineCircle(false)
    , m_hasCurrentParallel(false)
    , m_hasCurrentTwoLines(false)
    , m_hasValidMousePos(false)
    , m_drawingContextValid(false)
    , m_lastContextWidgetSize(0, 0)
    , m_lastContextImageSize(0, 0)
    , m_lastMouseMoveTime(std::chrono::steady_clock::now())
{
    // 启用抗锯齿和高质量渲染
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setScaledContents(false);
    
    // 启用鼠标跟踪以支持鼠标移动事件
    setMouseTracking(true);
}

void VideoDisplayWidget::setVideoFrame(const QPixmap& pixmap)
{
    m_videoFrame = pixmap;
    
    // 如果图像尺寸发生变化，清除缓存
    if (m_cachedImageSize != pixmap.size()) {
        m_cachedImageSize = pixmap.size();
        m_cachedScaleFactor = -1; // 标记需要重新计算
        // {{ AURA-X: Add - 图像尺寸变化时使DrawingContext缓存失效. Approval: 寸止(ID:context_cache). }}
        m_drawingContextValid = false; // 标记DrawingContext需要重新计算
    }
    
    // 触发重绘（所有绘制工作统一在paintEvent中完成）
    update();
}

void VideoDisplayWidget::setViewName(const QString& viewName)
{
    m_viewName = viewName;
}

void VideoDisplayWidget::setPointsData(const QVector<QPointF>& points)
{
    m_points = points;
    update();
}

void VideoDisplayWidget::setLinesData(const QVector<LineObject>& lines)
{
    m_lines = lines;
    update();
}

void VideoDisplayWidget::setLineSegmentsData(const QVector<LineSegmentObject>& lineSegments)
{
    m_lineSegments = lineSegments;
    update();
}

void VideoDisplayWidget::setCirclesData(const QVector<CircleObject>& circles)
{
    m_circles = circles;
    update();
}

void VideoDisplayWidget::setFineCirclesData(const QVector<FineCircleObject>& fineCircles)
{
    m_fineCircles = fineCircles;
    update();
}

void VideoDisplayWidget::setParallelLinesData(const QVector<ParallelObject>& parallels)
{
    m_parallels = parallels;
    update();
}

void VideoDisplayWidget::setTwoLinesData(const QVector<TwoLinesObject>& twoLines)
{
    m_twoLines = twoLines;
    update();
}

void VideoDisplayWidget::setCurrentLineData(const LineObject& currentLine)
{
    m_currentLine = currentLine;
    m_hasCurrentLine = true;
    update(); // 触发重绘
}

void VideoDisplayWidget::clearCurrentLineData()
{
    m_hasCurrentLine = false;
    update(); // 触发重绘
}

void VideoDisplayWidget::setCurrentCircleData(const CircleObject& currentCircle)
{
    m_currentCircle = currentCircle;
    m_hasCurrentCircle = true;
    update(); // 触发重绘
}

void VideoDisplayWidget::clearCurrentCircleData()
{
    m_hasCurrentCircle = false;
    update(); // 触发重绘
}

void VideoDisplayWidget::setCurrentFineCircleData(const FineCircleObject& currentFineCircle)
{
    m_currentFineCircle = currentFineCircle;
    m_hasCurrentFineCircle = true;
    update(); // 触发重绘
}

void VideoDisplayWidget::clearCurrentFineCircleData()
{
    m_hasCurrentFineCircle = false;
    update(); // 触发重绘
}

void VideoDisplayWidget::setCurrentParallelData(const ParallelObject& currentParallel)
{
    m_currentParallel = currentParallel;
    m_hasCurrentParallel = true;
    update(); // 触发重绘
}

void VideoDisplayWidget::clearCurrentParallelData()
{
    m_currentParallel = ParallelObject();
    m_hasCurrentParallel = false;
    // {{ AURA-X: Add - 清除鼠标预览状态. Approval: 寸止(ID:preview_fix). }}
    m_hasValidMousePos = false;
    update(); // 触发重绘
}

void VideoDisplayWidget::setCurrentTwoLinesData(const TwoLinesObject& currentTwoLines)
{
    m_currentTwoLines = currentTwoLines;
    m_hasCurrentTwoLines = true;
    update(); // 触发重绘
}

void VideoDisplayWidget::clearCurrentTwoLinesData()
{
    m_currentTwoLines = TwoLinesObject();
    m_hasCurrentTwoLines = false;
    // {{ AURA-X: Add - 清除鼠标预览状态. Approval: 寸止(ID:preview_fix). }}
    m_hasValidMousePos = false;
    update();
}

void VideoDisplayWidget::startDrawing(DrawingTool tool)
{
    m_isDrawingMode = true;
    m_currentDrawingTool = tool;
    
    // 启动绘图模式时自动禁用选择模式，确保两个模式互斥
    m_selectionEnabled = false;
    clearSelection();
    
    // 清除所有当前绘制状态
    clearCurrentLineData();
    clearCurrentCircleData();
    clearCurrentFineCircleData();
    clearCurrentParallelData();
    clearCurrentTwoLinesData();
    
    // 设置鼠标光标
    setCursor(Qt::CrossCursor);
}

void VideoDisplayWidget::stopDrawing()
{
    m_isDrawingMode = false;
    m_currentDrawingTool = DrawingTool::None;
    
    // 清除所有当前绘制状态
    clearCurrentLineData();
    clearCurrentCircleData();
    clearCurrentFineCircleData();
    clearCurrentParallelData();
    clearCurrentTwoLinesData();
    
    // 停止绘制模式时重新启用选择模式
    m_selectionEnabled = true;
    
    // 恢复默认鼠标光标
    setCursor(Qt::ArrowCursor);
}

void VideoDisplayWidget::mousePressEvent(QMouseEvent *event)
{
    // 处理右键点击：如果在绘制模式且没有选中任何图元，退出绘制模式
    if (event->button() == Qt::RightButton) {
        if (m_isDrawingMode) {
            // 检查是否有选中的对象
            bool hasSelection = !m_selectedPoints.isEmpty() || !m_selectedLines.isEmpty() || 
                               !m_selectedCircles.isEmpty() || !m_selectedFineCircles.isEmpty() || 
                               !m_selectedParallels.isEmpty() || !m_selectedTwoLines.isEmpty() || 
                               !m_selectedParallelMiddleLines.isEmpty() || !m_selectedLineSegments.isEmpty();
            
            if (!hasSelection) {
                stopDrawing();
                update();
                return;
            }
        }
        QLabel::mousePressEvent(event);
        return;
    }
    
    if (event->button() != Qt::LeftButton) {
        QLabel::mousePressEvent(event);
        return;
    }
    
    // 将窗口坐标转换为图像坐标
    QPointF imagePos = widgetToImage(event->pos());
    
    // 检查点击是否在图像范围内
    if (imagePos.x() < 0 || imagePos.y() < 0 || 
        imagePos.x() >= m_videoFrame.width() || imagePos.y() >= m_videoFrame.height()) {
        return;
    }
    
    // 如果启用了选择模式，进行命中测试
    if (m_selectionEnabled) {
        handleSelectionClick(imagePos, event->modifiers() & Qt::ControlModifier);
        return;
    }
    
    // 如果不在绘制模式，直接返回
    if (!m_isDrawingMode) {
        QLabel::mousePressEvent(event);
        return;
    }
    
    // 根据当前绘制工具处理点击
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
        default:
            break;
    }
}

void VideoDisplayWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_isDrawingMode) {
        QLabel::mouseMoveEvent(event);
        return;
    }
    
    // {{ AURA-X: Add - 事件节流，限制处理频率到60FPS. Approval: 寸止(ID:event_throttling). }}
    // 事件节流开始 - 限制刷新频率，例如最高60FPS (大约16ms一次)
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastMouseMoveTime).count();
    
    if (elapsed < 16) { 
        return; // 时间太短，忽略这次事件
    }
    m_lastMouseMoveTime = now; // 更新时间戳
    // 事件节流结束
    
    // 将窗口坐标转换为图像坐标
    QPointF imagePos = widgetToImage(event->pos());
    
    // 检查鼠标是否在图像范围内
    if (imagePos.x() < 0 || imagePos.y() < 0 || 
        imagePos.x() >= m_videoFrame.width() || imagePos.y() >= m_videoFrame.height()) {
        QLabel::mouseMoveEvent(event);
        return;
    }
    
    // 根据当前绘制工具处理鼠标移动（实时预览）
    bool needUpdate = false;
    
    switch (m_currentDrawingTool) {
        case DrawingTool::Line:
            if (m_hasCurrentLine && m_currentLine.points.size() >= 1) {
                // 直线绘制：更新第二个点进行实时预览
                if (m_currentLine.points.size() == 1) {
                    m_currentLine.points.append(imagePos);
                } else if (m_currentLine.points.size() >= 2) {
                    m_currentLine.points[1] = imagePos;
                }
                needUpdate = true;
            }
            break;
            
        case DrawingTool::Parallel:
            if (m_hasCurrentParallel) {
                if (m_currentParallel.points.size() == 1) {
                    // 第一个点到第二个点的预览：更新鼠标位置
                    m_currentMousePos = imagePos;
                    m_hasValidMousePos = true;
                    needUpdate = true;
                } else if (m_currentParallel.points.size() >= 2) {
                    // 平行线绘制：只有在有2个点后才进行第三个点的预览
                    if (m_currentParallel.points.size() == 2) {
                        // 添加第三个点进行预览
                        m_currentParallel.points.append(imagePos);
                        m_currentParallel.isPreview = true;
                    } else if (m_currentParallel.points.size() >= 3) {
                        // 更新第三个点的预览位置
                        m_currentParallel.points[2] = imagePos;
                    }
                    needUpdate = true;
                }
            }
            break;
            
        case DrawingTool::TwoLines:
            if (m_hasCurrentTwoLines) {
                if (m_currentTwoLines.points.size() == 1) {
                    // {{ AURA-X: Modify - 第一条线的第二个点预览，使用鼠标位置. Approval: 寸止(ID:click_fix). }}
                    // 第一条线的第二个点预览
                    m_currentMousePos = imagePos;
                    m_hasValidMousePos = true;
                    needUpdate = true;
                } else if (m_currentTwoLines.points.size() == 2) {
                    // {{ AURA-X: Add - 第三个点预览，使用鼠标位置. Approval: 寸止(ID:third_point_preview). }}
                    // 第三个点预览
                    m_currentMousePos = imagePos;
                    m_hasValidMousePos = true; 
                    needUpdate = true;
                } else if (m_currentTwoLines.points.size() == 3) {
                    // {{ AURA-X: Modify - 第二条线的第二个点预览，使用鼠标位置. Approval: 寸止(ID:click_fix). }}
                    // 第二条线的第二个点预览
                    m_currentMousePos = imagePos;
                    m_hasValidMousePos = true;
                    needUpdate = true;
                }
            }
            break;
            
        default:
            // 其他绘制工具暂时不需要实时预览
            break;
    }
    
    if (needUpdate) {
        update(); // 触发重绘
    }
    
    QLabel::mouseMoveEvent(event);
}

void VideoDisplayWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QLabel::mouseReleaseEvent(event);
}

// {{ AURA-X: Add - 控件尺寸变化时使DrawingContext缓存失效. Approval: 寸止(ID:context_cache). }}
void VideoDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
    
    // 控件尺寸变化时，使DrawingContext缓存失效
    m_drawingContextValid = false;
    
    // 同时清除其他缓存
    m_cachedScaleFactor = -1;
}

void VideoDisplayWidget::clearAllDrawings()
{
    m_points.clear();
    m_lines.clear();
    m_lineSegments.clear();
    m_circles.clear();
    m_fineCircles.clear();
    m_parallels.clear();
    m_twoLines.clear();
    m_hasCurrentLine = false;
    m_hasCurrentCircle = false;
    m_hasCurrentFineCircle = false;
    m_hasCurrentParallel = false;
    m_hasCurrentTwoLines = false;
    // {{ AURA-X: Add - 清除鼠标预览状态. Approval: 寸止(ID:preview_fix). }}
    m_hasValidMousePos = false;
    // {{ AURA-X: Add - 清除选择状态. Approval: 寸止(ID:selection_feature). }}
    clearSelection();
    // {{ AURA-X: Add - 清除历史记录栈. Approval: 寸止(ID:undo_feature). }}
    m_drawingHistory.clear();
    emit drawingDataChanged(m_viewName);
    update();
}

void VideoDisplayWidget::undoLastDrawing()
{
    if (m_drawingHistory.isEmpty()) {
        return;
    }

    DrawingAction lastAction = m_drawingHistory.pop();
    
    // 根据动作类型，从对应的列表中移除最后一个添加的对象
    switch (lastAction.type) {
        case ActionType::Point:
            if (!m_points.isEmpty()) m_points.removeLast();
            break;
        case ActionType::Line:
            if (!m_lines.isEmpty()) m_lines.removeLast();
            break;
        case ActionType::LineSegment:
            if (!m_lineSegments.isEmpty()) m_lineSegments.removeLast();
            break;
        case ActionType::Circle:
            if (!m_circles.isEmpty()) m_circles.removeLast();
            break;
        case ActionType::FineCircle:
            if (!m_fineCircles.isEmpty()) m_fineCircles.removeLast();
            break;
        case ActionType::Parallel:
            if (!m_parallels.isEmpty()) m_parallels.removeLast();
            break;
        case ActionType::TwoLines:
            if (!m_twoLines.isEmpty()) m_twoLines.removeLast();
            break;
    }
    
    clearSelection();
    emit drawingDataChanged(m_viewName);
    update();
}

void VideoDisplayWidget::commitDrawingAction(ActionType type, const QString& result)
{
    // 1. 根据类型将当前对象添加到对应的最终列表中
    switch (type) {
        case ActionType::Point:
            // 点类型不需要从当前对象添加，因为已经直接添加到m_points
            break;
        case ActionType::Line:
            m_lines.append(m_currentLine);
            break;
        case ActionType::LineSegment:
            // LineSegment通常直接添加到m_lineSegments，这里可能不需要特殊处理
            break;
        case ActionType::Circle:
            m_circles.append(m_currentCircle);
            break;
        case ActionType::FineCircle:
            m_fineCircles.append(m_currentFineCircle);
            break;
        case ActionType::Parallel:
            m_parallels.append(m_currentParallel);
            break;
        case ActionType::TwoLines:
            m_twoLines.append(m_currentTwoLines);
            break;
    }
    
    // 2. 记录历史（获取对应列表的最新索引）
    int index = 0;
    switch (type) {
        case ActionType::Point:
            index = m_points.size() - 1;
            break;
        case ActionType::Line:
            index = m_lines.size() - 1;
            break;
        case ActionType::LineSegment:
            index = m_lineSegments.size() - 1;
            break;
        case ActionType::Circle:
            index = m_circles.size() - 1;
            break;
        case ActionType::FineCircle:
            index = m_fineCircles.size() - 1;
            break;
        case ActionType::Parallel:
            index = m_parallels.size() - 1;
            break;
        case ActionType::TwoLines:
            index = m_twoLines.size() - 1;
            break;
    }
    m_drawingHistory.push({type, index});
    
    // 3. 发出信号
    emit measurementCompleted(m_viewName, result);
    emit drawingDataChanged(m_viewName);
    
    // 4. 清理当前状态
    switch (type) {
        case ActionType::Point:
            // 点类型不需要清理当前状态
            break;
        case ActionType::Line:
            clearCurrentLineData();
            break;
        case ActionType::LineSegment:
            // LineSegment通常不需要清理当前状态
            break;
        case ActionType::Circle:
            clearCurrentCircleData();
            break;
        case ActionType::FineCircle:
            clearCurrentFineCircleData();
            break;
        case ActionType::Parallel:
            clearCurrentParallelData();
            break;
        case ActionType::TwoLines:
            clearCurrentTwoLinesData();
            break;
    }
}

void VideoDisplayWidget::updateDrawings()
{
    update();
}

// {{ AURA-X: Add - 绘图状态同步方法实现. Approval: 寸止(ID:drawing_sync). }}
VideoDisplayWidget::DrawingState VideoDisplayWidget::getDrawingState() const
{
    DrawingState state;
    state.points = m_points;
    state.lines = m_lines;
    state.lineSegments = m_lineSegments;
    state.circles = m_circles;
    state.fineCircles = m_fineCircles;
    state.parallels = m_parallels;
    state.twoLines = m_twoLines;
    return state;
}

void VideoDisplayWidget::setDrawingState(const DrawingState& state)
{
    m_points = state.points;
    m_lines = state.lines;
    m_lineSegments = state.lineSegments;
    m_circles = state.circles;
    m_fineCircles = state.fineCircles;
    m_parallels = state.parallels;
    m_twoLines = state.twoLines;
    
    // 清除当前绘制状态
    m_hasCurrentLine = false;
    m_hasCurrentCircle = false;
    m_hasCurrentFineCircle = false;
    m_hasCurrentParallel = false;
    m_hasCurrentTwoLines = false;
    m_hasValidMousePos = false;
    
    // 清除选择状态
    clearSelection();
    
    // 重建历史记录栈（简化版本，只记录当前存在的对象）
    m_drawingHistory.clear();
    
    // 发出数据变化信号
    emit drawingDataChanged(m_viewName);
    update();
}

void VideoDisplayWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    
    // 启用抗锯齿和高质量渲染
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    
    // 1. 绘制背景视频帧
    if (!m_videoFrame.isNull()) {
        // 计算图像在控件中的位置和缩放
        QPointF offset = getImageOffset();
        double scale = getScaleFactor();
        
        QRectF targetRect(offset.x(), offset.y(), 
                         m_videoFrame.width() * scale, 
                         m_videoFrame.height() * scale);
        
        painter.drawPixmap(targetRect, m_videoFrame, m_videoFrame.rect());
    }
    
    // 2. 绘制前景几何图形（硬件加速优化）
    if (!m_videoFrame.isNull()) {
        // {{ AURA-X: Optimize - 缓存变换参数避免重复函数调用，使用QTransform提升性能. Approval: 寸止(ID:performance). }}
        // 在paintEvent开头计算一次变换参数，避免重复函数调用开销
        const double scale = getScaleFactor();
        const QPointF offset = getImageOffset();
        
        painter.save();
        
        // 使用QTransform统一管理坐标变换，性能更优
        QTransform transform;
        transform.translate(offset.x(), offset.y());
        transform.scale(scale, scale);
        painter.setTransform(transform);
        
        // 在变换后的坐标系中设置裁剪区域，使用图像坐标
        painter.setClipRect(m_videoFrame.rect());
        
        // {{ AURA-X: Optimize - 使用缓存的DrawingContext避免重复创建对象. Approval: 寸止(ID:context_cache). }}
        // 检查是否需要更新DrawingContext缓存
        if (needsDrawingContextUpdate()) {
            updateDrawingContext();
        }
        
        // 使用缓存的DrawingContext，避免重复创建对象
        const DrawingContext& ctx = m_cachedDrawingContext;
        
        // 现在坐标系原点就是图像左上角，所有绘制函数可以直接使用图像坐标
        // 传递绘制上下文，避免重复创建对象
        drawPoints(painter, ctx);
        drawLines(painter, ctx);
        drawLineSegments(painter, ctx);
        drawCircles(painter, ctx);
        drawFineCircles(painter, ctx);
        drawParallelLines(painter, ctx);
        drawTwoLines(painter, ctx);
        
        // 绘制选中状态高亮
        drawSelectionHighlight(painter, ctx);
        
        painter.restore();
    }
}

// 点
void VideoDisplayWidget::drawPoints(QPainter& painter, const DrawingContext& ctx)
{
    if (m_points.isEmpty()) return;
    
    // {{ AURA-X: Optimize - 使用DrawingContext避免重复创建对象. Approval: 寸止(ID:performance_optimization). }}
    // 使用预创建的对象，提升性能
    
    // 预计算常量（匹配Python版本）
    const double heightScale = std::max(1.0, std::min(ctx.scale, 4.0));
    const double innerRadius = 20 * heightScale;  // {{ AURA-X: Modify - 增大点的半径以提高可见性. Approval: 寸止(ID:point_radius_fix). }}
    const double textpadding = qMax(4.0, ctx.fontSize * 2);
    
    for (int i = 0; i < m_points.size(); ++i) {
        const QPointF& point = m_points[i];  // 直接使用图像坐标
        
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

// 直线
void VideoDisplayWidget::drawSingleLine(QPainter& painter, const LineObject& line, bool isCurrentDrawing, const DrawingContext& ctx)
{
    if (line.points.size() < 2) {
        return;
    }
    
    const QPointF& start = line.points[0];
    const QPointF& end = line.points[1];
    
    // {{ AURA-X: Optimize - 移除重复坐标转换，直接使用图像坐标. Approval: 寸止(ID:performance). }}
    // 直接使用图像坐标，QPainter变换矩阵自动处理缩放和平移
    
    // Calculate direction vector for infinite line extension
    QPointF direction = end - start;
    qreal length = sqrt(direction.x() * direction.x() + direction.y() * direction.y());
    
    if (length < 1.0) {
        return; // 两点过于接近，无法确定方向
    }
    
    // 使用简单可靠的方向向量延伸方法，让QPainter自动裁剪
    // 向后延伸足够长的距离（在图像坐标系中，5000像素足以覆盖任何合理的显示区域）
    QPointF extendedStart = start - direction * 5000.0;
    // 向前延伸足够长的距离
    QPointF extendedEnd = end + direction * 5000.0;
    
    // {{ AURA-X: Optimize - 使用DrawingContext避免重复创建画笔. Approval: 寸止(ID:performance_optimization). }}
    // 使用预创建的画笔或根据需要创建特定颜色的画笔
    int desiredThickness = qMax(2, static_cast<int>(line.thickness * 2.0 * ctx.scale));
    
    QPen linePen = createPen(line.color, desiredThickness, ctx.scale);
    linePen.setCapStyle(Qt::RoundCap);
    
    if (line.isDashed) {
        // Draw dashed line
        linePen.setStyle(Qt::DashLine);
        painter.setPen(linePen);
        painter.drawLine(extendedStart, extendedEnd);
    } else {
        // Draw extended line to boundary
        painter.setPen(linePen);
        painter.drawLine(extendedStart, extendedEnd);
    }
    
    // {{ AURA-X: Optimize - 使用更Qt风格的端点绘制方式，直接使用期望的屏幕像素值 }}
    // Draw user-clicked original endpoints (match Python style)
    // 绘制起始点标记（动态缩放的小圆点）
    double pointRadius = qMax(2.0, 3.0 * ctx.scale);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(line.color));
    painter.drawEllipse(start, pointRadius, pointRadius);
    
    // Calculate and display angle information (使用与平行线功能一致的简洁方式)
    double angle = calculateLineAngle(start, end);
    
    // 格式化角度文本（包含度数符号）
    QString angleText = QString::asprintf("%.1f°", angle);
    
    // 动态计算文本布局参数
    double textOffset = qMax(8.0, 10.0 * ctx.scale);
    double textPadding = qMax(4.0, ctx.fontSize * 2);
    int bgBorderWidth = 1;
    
    // 计算角度文本框定位到起始点右上方所需的精确偏移量
    QFontMetrics fm(ctx.font);
    QRect textBoundingRect = fm.boundingRect(angleText);
    double bgHeight = textBoundingRect.height() + 2 * textPadding;
    QPointF angleTextOffset(textOffset, -textOffset - bgHeight);
    
    // 使用完整版drawTextWithBackground函数
    drawTextWithBackground(painter, start, angleText, ctx.font, line.color, Qt::black, textPadding, bgBorderWidth, angleTextOffset);
}

void VideoDisplayWidget::drawLines(QPainter& painter, const DrawingContext& ctx)
{
    // {{ AURA-X: Add - 添加当前正在绘制的直线实时预览功能. Approval: 寸止(ID:5). }}
    // 绘制已完成的直线
    for (const auto& line : m_lines) {
        drawSingleLine(painter, line, false, ctx);
    }
    
    // 绘制当前正在绘制的直线（实时预览）
    if (m_hasCurrentLine && !m_currentLine.points.isEmpty()) {
        drawSingleLine(painter, m_currentLine, true, ctx);
    }
}

void VideoDisplayWidget::drawLineSegments(QPainter& painter, const DrawingContext& ctx)
{
    // 绘制所有线段
    for (const auto& lineSegment : m_lineSegments) {
        drawSingleLineSegment(painter, lineSegment, ctx);
    }
}

void VideoDisplayWidget::drawSingleLineSegment(QPainter& painter, const LineSegmentObject& lineSegment, const DrawingContext& ctx)
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
    
    // 计算并显示长度信息
    double length = sqrt(pow(end.x() - start.x(), 2) + pow(end.y() - start.y(), 2));
    QString lengthText = QString::asprintf("L=%.1f", length);
    
    // 计算线段中点作为文本位置
    QPointF midPoint = (start + end) / 2.0;
    
    // 动态计算文本布局参数
    double textOffset = qMax(8.0, 10.0 * ctx.scale);
    double textPadding = qMax(4.0, ctx.fontSize * 2);
    int bgBorderWidth = 1;
    
    // 计算长度文本框定位到中点右上方所需的精确偏移量
    QFontMetrics fm(ctx.font);
    QRect textBoundingRect = fm.boundingRect(lengthText);
    double bgHeight = textBoundingRect.height() + 2 * textPadding;
    QPointF lengthTextOffset(textOffset, -textOffset - bgHeight);
    
    // 使用完整版drawTextWithBackground函数
    drawTextWithBackground(painter, midPoint, lengthText, ctx.font, lineSegment.color, Qt::black, textPadding, bgBorderWidth, lengthTextOffset);
}

void VideoDisplayWidget::drawCircles(QPainter& painter, const DrawingContext& ctx)
{
    // 绘制已完成的圆形
    for (const auto& circle : m_circles) {
        drawSingleCircle(painter, circle, ctx);
    }
    
    // 绘制当前正在绘制的圆形（实时预览）
    if (m_hasCurrentCircle && !m_currentCircle.points.isEmpty()) {
        drawSingleCircle(painter, m_currentCircle, ctx);
    }
}

// 简单圆
void VideoDisplayWidget::drawSingleCircle(QPainter& painter, const CircleObject& circle, const DrawingContext& ctx)
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
    double textPadding = qMax(4.0, ctx.fontSize * 2);
    int bgBorderWidth = 1;
    
    // 只在绘制过程中显示辅助点，圆形完成后不显示
    if (!circle.isCompleted) {
        // {{ AURA-X: Optimize - 移除重复坐标转换，直接使用图像坐标. Approval: 寸止(ID:performance). }}
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
    
    // 如果圆形已完成，绘制圆形
    if (circle.isCompleted && circle.points.size() >= 3) {
        const QPointF& centerImage = circle.center;
        double radiusImage = circle.radius;
        
        // 手动计算与 drawSingleLine 一致的线条粗细
        int desiredThickness = qMax(2, static_cast<int>(circle.thickness * 2.0 * ctx.scale));
        
        // 根据颜色选择合适的画笔
        QPen circlePen;
        if (circle.color == Qt::red) {
            circlePen = ctx.redPen;
        } else if (circle.color == Qt::green) {
            circlePen = ctx.greenPen;
        } else if (circle.color == Qt::blue) {
            circlePen = ctx.bluePen;
        } else {
            circlePen = ctx.blackPen;
        }
        
        // 绘制圆形
        painter.setPen(circlePen);
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(centerImage, radiusImage, radiusImage);
        
        // 动态计算圆心标记尺寸
        double centerMarkRadius = qMax(3.0, 6.0 * ctx.scale);
        
        // 绘制圆心 - 白色边框 + 红色实心圆
        painter.setPen(ctx.whitePen);
        painter.setBrush(ctx.redBrush);
        painter.drawEllipse(centerImage, centerMarkRadius, centerMarkRadius);
        
        // 显示圆心坐标和半径 - 让两个文本框紧挨着排列
        QString centerText = QString::asprintf("(%.1f,%.1f)", circle.center.x(), circle.center.y());
        QString radiusText = QString::asprintf("R=%.1f", circle.radius);
        
        // 使用已计算的文本布局参数，直接使用期望的屏幕像素值
        
        // 1. 绘制第一个文本框（坐标文本）- 使用圆形颜色
        drawTextWithBackground(painter, centerImage, centerText, ctx.font, circle.color, Qt::black, textPadding, bgBorderWidth, QPointF(textOffset, -textOffset));
        
        // 2. 计算第二个文本框的位置（紧挨着第一个文本框下方）
        QFontMetrics fm(ctx.font);
        QRect centerTextRect = fm.boundingRect(centerText);
        double centerTextHeight = centerTextRect.height() + 2 * textPadding;
        QPointF radiusAnchorPoint = centerImage + QPointF(textOffset, -textOffset + centerTextHeight);
        drawTextWithBackground(painter, radiusAnchorPoint, radiusText, ctx.font, circle.color, Qt::black, textPadding, bgBorderWidth, QPointF(0, 0));
    }
}

void VideoDisplayWidget::drawFineCircles(QPainter& painter, const DrawingContext& ctx)
{
    // 绘制已完成的精细圆
    for (const auto& fineCircle : m_fineCircles) {
        drawSingleFineCircle(painter, fineCircle, ctx);
    }
    
    // 绘制当前正在绘制的精细圆（实时预览）
    if (m_hasCurrentFineCircle && !m_currentFineCircle.points.isEmpty()) {
        drawSingleFineCircle(painter, m_currentFineCircle, ctx);
    }
}

// 精细圆
void VideoDisplayWidget::drawSingleFineCircle(QPainter& painter, const FineCircleObject& fineCircle, const DrawingContext& ctx)
{
    // 使用预创建的字体和画笔
    painter.setFont(ctx.font);
    
    // 计算动态尺寸参数
    double fontSize = ctx.fontSize;
    double pointInnerRadius = qMax(3.0, 5.0 * ctx.scale);
    double pointOuterRadius = qMax(5.0, 8.0 * ctx.scale);
    double pointPenWidth = qMax(1.0, 2.0 * ctx.scale);
    double textOffset = qMax(8.0, 10.0 * ctx.scale);
    int connectionLineWidth = qMax(1, static_cast<int>(2.0 * ctx.scale));
    double textPadding = qMax(4.0, ctx.fontSize * 2);
    int bgBorderWidth = 1;
    
    // 只在绘制过程中显示辅助点，精细圆完成后不显示
    if (!fineCircle.isCompleted) {
        // 绘制精细圆的点
        for (int i = 0; i < fineCircle.points.size(); ++i) {
            const QPointF& imagePos = fineCircle.points[i];
            
            // 绘制点 - 实心圆 + 空心圆环 - 使用精细圆颜色
            painter.setPen(Qt::NoPen);
            // 根据颜色选择合适的画刷
            if (fineCircle.color == Qt::red) {
                painter.setBrush(ctx.redBrush);
            } else if (fineCircle.color == Qt::green) {
                painter.setBrush(ctx.greenBrush);
            } else if (fineCircle.color == Qt::blue) {
                painter.setBrush(ctx.blueBrush);
            } else {
                painter.setBrush(ctx.blackBrush);
            }
            painter.drawEllipse(imagePos, pointInnerRadius, pointInnerRadius);
            
            // 绘制空心圆环
            // 根据颜色选择合适的画笔
            if (fineCircle.color == Qt::red) {
                painter.setPen(ctx.redPen);
            } else if (fineCircle.color == Qt::green) {
                painter.setPen(ctx.greenPen);
            } else if (fineCircle.color == Qt::blue) {
                painter.setPen(ctx.bluePen);
            } else {
                painter.setPen(ctx.blackPen);
            }
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(imagePos, pointOuterRadius, pointOuterRadius);
            
            // 显示序号（与drawSingleCircle保持一致，不带背景）
            QString pointText = QString::number(i + 1);
            QPointF textPos = imagePos + QPointF(textOffset, textOffset);
            
            // 绘制文本（使用相同颜色的画笔）
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
        
        // 手动计算线条粗细，确保与其他绘制功能一致
        int finalThickness = qMax(1, static_cast<int>(fineCircle.thickness * 2.0 * ctx.scale));
        
        // 绘制精细圆 - 根据颜色选择合适的画笔
        if (fineCircle.color == Qt::red) {
            painter.setPen(ctx.redPen);
        } else if (fineCircle.color == Qt::green) {
            painter.setPen(ctx.greenPen);
        } else if (fineCircle.color == Qt::blue) {
            painter.setPen(ctx.bluePen);
        } else {
            painter.setPen(ctx.blackPen);
        }
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(centerImage, radiusImage, radiusImage);
        
        // 绘制圆心标记（使用红色以区别于简单圆）
        double centerMarkRadius = qMax(4.0, 6.0 * ctx.scale);
        painter.setPen(ctx.whitePen);
        painter.setBrush(ctx.redBrush);
        painter.drawEllipse(centerImage, centerMarkRadius, centerMarkRadius);
        
        // 显示圆心坐标和半径信息（使用已计算的字体参数）
        double textPadding = qMax(16.0, ctx.fontSize * 2);
        int bgBorderWidth = 1; // 背景边框宽度
        
        // 使用正确的字符串格式化
        QString centerText = QString::asprintf("(%.1f, %.1f)", fineCircle.center.x(), fineCircle.center.y());
        QString radiusText = QString::asprintf("%.1f", fineCircle.radius);
        
        // {{ AURA-X: Modify - 修复精细圆第二个文本框位置，与简单圆保持一致. Confirmed via 寸止 }}
        // 1. 绘制第一个文本框（坐标文本）- 使用精细圆颜色
        drawTextWithBackground(painter, centerImage, centerText, ctx.font, fineCircle.color, Qt::black, textPadding, bgBorderWidth, QPointF(textOffset, -textOffset));
        
        // 2. 计算第二个文本框的位置（紧挨着第一个文本框下方）
        QFontMetrics fm(ctx.font);
        QRect centerTextRect = fm.boundingRect(centerText);
        double centerTextHeight = centerTextRect.height() + 2 * textPadding;
        QPointF radiusAnchorPoint = centerImage + QPointF(textOffset, -textOffset + centerTextHeight);
        drawTextWithBackground(painter, radiusAnchorPoint, radiusText, ctx.font, fineCircle.color, Qt::black, textPadding, bgBorderWidth, QPointF(0, 0));
    }
}

void VideoDisplayWidget::drawParallelLines(QPainter& painter, const DrawingContext& ctx)
{
    // 绘制已完成的平行线
    for (const auto& parallel : m_parallels) {
        drawSingleParallel(painter, parallel, ctx);
    }
    
    // 绘制当前正在绘制的平行线（实时预览）
    if (m_hasCurrentParallel && !m_currentParallel.points.isEmpty()) {
        drawSingleParallel(painter, m_currentParallel, ctx);
    }
}

// 平行线
void VideoDisplayWidget::drawSingleParallel(QPainter& painter, const ParallelObject& parallel, const DrawingContext& ctx)
{
    // 使用预创建的字体和画笔
    painter.setFont(ctx.font);
    
    // 计算动态尺寸参数
    double fontSize = ctx.fontSize;
    double pointInnerRadius = qMax(3.0, 5.0 * ctx.scale);
    double pointOuterRadius = qMax(5.0, 8.0 * ctx.scale);
    int pointPenWidth = qMax(1, static_cast<int>(2.0 * ctx.scale));
    double textOffset = qMax(10.0, 15.0 * ctx.scale);
    double textPadding = qMax(4.0, ctx.fontSize * 2);
    int bgBorderWidth = 1;
    double desiredThickness = parallel.thickness * 2.0 * ctx.scale;
    int thickLine = qMax(2, static_cast<int>(desiredThickness));
    int midLineWidth = qMax(1, static_cast<int>(2.0 * ctx.scale));
    
    // 只在绘制过程中显示辅助点，平行线完成后不显示
    if (!parallel.isCompleted) {
        // 绘制平行线的点
        for (int i = 0; i < parallel.points.size(); ++i) {
            const QPointF& imagePos = parallel.points[i];
            
            // {{ AURA-X: Modify - 预览点不显示序号. Approval: 寸止(ID:no_preview_text). }}
            // 判断是否为预览点（第三个点且处于预览状态）
            bool isPreviewPoint = (i == 2 && parallel.isPreview);
            
            if (isPreviewPoint) {
                // 预览点使用虚线样式的圆环，不显示序号
                // 根据颜色选择合适的画笔（虚线版本）
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
            } else {
                // 正常点绘制 - 实心圆 + 空心圆环 + 序号
                painter.setPen(Qt::NoPen);
                // 根据颜色选择合适的画刷
                if (parallel.color == Qt::red) {
                    painter.setBrush(ctx.redBrush);
                } else if (parallel.color == Qt::green) {
                    painter.setBrush(ctx.greenBrush);
                } else if (parallel.color == Qt::blue) {
                    painter.setBrush(ctx.blueBrush);
                } else {
                    painter.setBrush(ctx.blackBrush);
                }
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
                
                // 绘制点的序号（与drawSingleCircle保持一致，不带背景）
                QString pointText = QString::number(i + 1);
                QPointF textPos = imagePos + QPointF(textOffset, textOffset);
                
                // 绘制文本（使用相同颜色的画笔）
                painter.setBrush(Qt::NoBrush);
                painter.drawText(textPos, pointText);
            }
        }
    }
    
    // 绘制第一条线（延伸到图像边界）
    if (parallel.points.size() >= 2) {
        QPointF extStart1, extEnd1;
        calculateExtendedLine(parallel.points[0], parallel.points[1], extStart1, extEnd1);
        
        // 使用已计算的线条粗细参数，直接使用期望的屏幕像素值
        
        // 当只有两个点时（第一条线预览），使用虚线
            if (parallel.points.size() == 2) {
                // 根据颜色选择合适的画笔（虚线版本）
                if (parallel.color == Qt::red) {
                    painter.setPen(ctx.redPen);
                } else if (parallel.color == Qt::green) {
                    painter.setPen(ctx.greenPen);
                } else if (parallel.color == Qt::blue) {
                    painter.setPen(ctx.bluePen);
                } else {
                    painter.setPen(ctx.blackPen);
                }
            } else {
                // 有第三个点后：第一条线使用实线
                if (parallel.color == Qt::red) {
                    painter.setPen(ctx.redPen);
                } else if (parallel.color == Qt::green) {
                    painter.setPen(ctx.greenPen);
                } else if (parallel.color == Qt::blue) {
                    painter.setPen(ctx.bluePen);
                } else {
                    painter.setPen(ctx.blackPen);
                }
            }
        painter.drawLine(extStart1, extEnd1);
    } else if (parallel.points.size() == 1 && m_hasValidMousePos) {
        // {{ AURA-X: Add - 添加第一个点到鼠标位置的预览线. Approval: 寸止(ID:preview_fix). }}
        // 只有一个点时，绘制从第一个点到当前鼠标位置的预览线
        QPointF extStart1, extEnd1;
        calculateExtendedLine(parallel.points[0], m_currentMousePos, extStart1, extEnd1);
        
        // 使用虚线表示预览状态
        // 根据颜色选择合适的画笔（虚线版本）
        if (parallel.color == Qt::red) {
            painter.setPen(ctx.redPen);
        } else if (parallel.color == Qt::green) {
            painter.setPen(ctx.greenPen);
        } else if (parallel.color == Qt::blue) {
            painter.setPen(ctx.bluePen);
        } else {
            painter.setPen(ctx.blackPen);
        }
        painter.drawLine(extStart1, extEnd1);
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
        
        // 使用已计算的线条粗细参数，补偿变换矩阵
        
        // 根据完成状态决定第二条线的样式：未完成时使用虚线（预览状态），完成后使用实线
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
        painter.drawLine(extStart2, extEnd2);
        
        // 如果平行线已完成，绘制中线（红色虚线）
        if (parallel.isCompleted) {
            // 【核心修正】计算真正的中线
            
            // 重新获取第二条线的端点，以便计算
            QPointF direction = parallel.points[1] - parallel.points[0];
            QPointF p3 = parallel.points[2];
            QPointF p4 = p3 + direction;

            // 1. 计算中线的两个点
            QPointF mid_start = (parallel.points[0] + p3) / 2.0;
            QPointF mid_end = (parallel.points[1] + p4) / 2.0;

            // 2. 将中线延伸到图像边界
            QPointF extMidStart, extMidEnd;
            calculateExtendedLine(mid_start, mid_end, extMidStart, extMidEnd);

            // 3. 绘制中线，补偿变换矩阵
            
            // 绘制中线（红色虚线）
            painter.setPen(ctx.redDashedPen); // 红色虚线
            painter.drawLine(extMidStart, extMidEnd); // 绘制延伸后的中线
            
            // 显示距离和角度信息 - 使用drawTextWithBackground辅助函数
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
}

void VideoDisplayWidget::drawTwoLines(QPainter& painter, const DrawingContext& ctx)
{
    // 绘制已完成的两线
    for (const auto& twoLine : m_twoLines) {
        drawSingleTwoLines(painter, twoLine, ctx);
    }
    
    // 绘制当前正在绘制的两线（实时预览）
    if (m_hasCurrentTwoLines && !m_currentTwoLines.points.isEmpty()) {
        drawSingleTwoLines(painter, m_currentTwoLines, ctx);
    }
}

// 两线
void VideoDisplayWidget::drawSingleTwoLines(QPainter& painter, const TwoLinesObject& twoLines, const DrawingContext& ctx)
{
    // 使用预创建的字体和画笔
    painter.setFont(ctx.font);
    
    // 计算动态尺寸参数
    double fontSize = ctx.fontSize;
    double pointInnerRadius = qMax(3.0, 5.0 * ctx.scale);
    double pointOuterRadius = qMax(5.0, 8.0 * ctx.scale);
    int pointPenWidth = qMax(1, static_cast<int>(2.0 * ctx.scale));
    double textOffset = qMax(10.0, 15.0 * ctx.scale);
    double textPadding = qMax(4.0, ctx.fontSize * 2);
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
            // 根据颜色选择合适的画刷
            if (twoLines.color == Qt::red) {
                painter.setBrush(ctx.redBrush);
            } else if (twoLines.color == Qt::green) {
                painter.setBrush(ctx.greenBrush);
            } else if (twoLines.color == Qt::blue) {
                painter.setBrush(ctx.blueBrush);
            } else {
                painter.setBrush(ctx.blackBrush);
            }
            painter.drawEllipse(imagePos, pointInnerRadius, pointInnerRadius);
            
            // 绘制空心圆环
            // 根据颜色选择合适的画笔
            if (twoLines.color == Qt::red) {
                painter.setPen(ctx.redPen);
            } else if (twoLines.color == Qt::green) {
                painter.setPen(ctx.greenPen);
            } else if (twoLines.color == Qt::blue) {
                painter.setPen(ctx.bluePen);
            } else {
                painter.setPen(ctx.blackPen);
            }
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(imagePos, pointOuterRadius, pointOuterRadius);
            
            // 绘制点的序号（与drawSingleParallel保持一致，不带背景）
            QString pointText = QString::number(i + 1);
            QPointF textPos = imagePos + QPointF(textOffset, textOffset);
            
            // 绘制文本（使用相同颜色的画笔）
            painter.setBrush(Qt::NoBrush);
            painter.drawText(textPos, pointText);
        }
        
        // {{ AURA-X: Add - 第三个点预览支持. Approval: 寸止(ID:third_point_preview). }}
        // 当有2个点时，显示第三个点的预览
        if (twoLines.points.size() == 2 && m_hasValidMousePos) {
            const QPointF& previewPos = m_currentMousePos;
            
            // 绘制预览点 - 使用虚线样式的圆环，不显示序号
            // 根据颜色选择合适的画笔（虚线版本）
            if (twoLines.color == Qt::red) {
                painter.setPen(ctx.redPen);
            } else if (twoLines.color == Qt::green) {
                painter.setPen(ctx.greenPen);
            } else if (twoLines.color == Qt::blue) {
                painter.setPen(ctx.bluePen);
            } else {
                painter.setPen(ctx.blackPen);
            }
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(previewPos, pointOuterRadius, pointOuterRadius);
        }
    }
    
    // {{ AURA-X: Add - 支持第一条线的鼠标预览. Approval: 寸止(ID:preview_fix). }}
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
            
            // 使用已计算的线条粗细参数，直接使用期望的屏幕像素值
            
            // 根据颜色选择合适的画笔
            if (twoLines.color == Qt::red) {
                painter.setPen(ctx.redPen);
            } else if (twoLines.color == Qt::green) {
                painter.setPen(ctx.greenPen);
            } else if (twoLines.color == Qt::blue) {
                painter.setPen(ctx.bluePen);
            } else {
                painter.setPen(ctx.blackPen);
            }
            painter.drawLine(extStart1, extEnd1);
        }
    }
    
    // {{ AURA-X: Add - 支持第二条线的鼠标预览. Approval: 寸止(ID:preview_fix). }}
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
            
            // 绘制第二条线，直接使用期望的屏幕像素值
            
            // 根据颜色选择合适的画笔
            if (twoLines.color == Qt::red) {
                painter.setPen(ctx.redPen);
            } else if (twoLines.color == Qt::green) {
                painter.setPen(ctx.greenPen);
            } else if (twoLines.color == Qt::blue) {
                painter.setPen(ctx.bluePen);
            } else {
                painter.setPen(ctx.blackPen);
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
        double textPadding = qMax(16.0, ctx.fontSize * 2);
        int bgBorderWidth = 1;
        
        // 检查交点是否在视图范围内，如果不在则使用两线中间区域作为文字锚点
        QSize imageSize = m_videoFrame.size();
        QPointF textAnchorPoint = intersection;
        
        if (!imageSize.isEmpty()) {
            bool intersectionInView = (intersection.x() >= 0 && intersection.x() <= imageSize.width() && 
                                     intersection.y() >= 0 && intersection.y() <= imageSize.height());
            
            if (!intersectionInView) {
                // 计算两条线在视图内的中点作为文字锚点
                QPointF line1Center = (twoLines.points[0] + twoLines.points[1]) / 2.0;
                QPointF line2Center = (twoLines.points[2] + twoLines.points[3]) / 2.0;
                textAnchorPoint = (line1Center + line2Center) / 2.0;
            }
        }
        
        // 【终极API设计】使用新的优雅API进行相对布局
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

QPen VideoDisplayWidget::createPen(const QColor& color, int targetScreenWidth, double scale, bool dashed)
{
    QPen pen(color);
    // 内部进行补偿，确保在屏幕上看起来是 targetScreenWidth 像素宽
    pen.setWidthF(qMax(1.0, targetScreenWidth / scale));
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    
    if (dashed) {
        pen.setStyle(Qt::DashLine);
        // 设置虚线模式
        QVector<qreal> dashPattern;
        dashPattern << 5 << 3; // 5像素实线，3像素空白
        pen.setDashPattern(dashPattern);
    }
    
    return pen;
}

QFont VideoDisplayWidget::createFont(int targetScreenSize, double scale)
{
    QFont font;
    // 内部进行补偿，确保在屏幕上看起来是 targetScreenSize 大小
    font.setPointSizeF(qMax(8.0, targetScreenSize / scale));
    font.setBold(true);
    return font;
}



double VideoDisplayWidget::getScaleFactor() const
{
    if (m_videoFrame.isNull()) return 1.0;
    
    // 检查是否需要重新计算
    if (m_cachedScaleFactor < 0 || m_cachedWidgetSize != size()) {
        m_cachedWidgetSize = size();
        
        QSize imageSize = m_videoFrame.size();
        QSize widgetSize = size();
        
        double scaleX = static_cast<double>(widgetSize.width()) / imageSize.width();
        double scaleY = static_cast<double>(widgetSize.height()) / imageSize.height();
        
        // 使用较小的缩放因子以保持宽高比
        m_cachedScaleFactor = qMin(scaleX, scaleY);
    }
    
    return m_cachedScaleFactor;
}

QPointF VideoDisplayWidget::getImageOffset() const
{
    if (m_videoFrame.isNull()) return QPointF(0, 0);
    
    double scale = getScaleFactor();
    QSize imageSize = m_videoFrame.size();
    QSize widgetSize = size();
    
    double scaledWidth = imageSize.width() * scale;
    double scaledHeight = imageSize.height() * scale;
    
    double offsetX = (widgetSize.width() - scaledWidth) / 2.0;
    double offsetY = (widgetSize.height() - scaledHeight) / 2.0;
    
    return QPointF(offsetX, offsetY);
}

void VideoDisplayWidget::handlePointDrawingClick(const QPointF& imagePos)
{
    // 直接添加点到点集合中
    m_points.append(imagePos);
    
    // {{ AURA-X: Refactor - 使用通用提交逻辑简化代码. Approval: 寸止(ID:code_refactor). }}
    // 使用通用提交逻辑
    QString result = QString("点: (%.1f, %.1f)").arg(imagePos.x()).arg(imagePos.y());
    commitDrawingAction(ActionType::Point, result);
    
    update();
}

void VideoDisplayWidget::handleLineDrawingClick(const QPointF& imagePos)
{
    if (!m_hasCurrentLine) {
        // 开始新的直线
        m_currentLine = LineObject();
        m_currentLine.points.append(imagePos);
        m_hasCurrentLine = true;
    } else {
        // 完成直线
        m_currentLine.points.append(imagePos);
        m_currentLine.isCompleted = true;
        
        // {{ AURA-X: Refactor - 使用通用提交逻辑简化代码. Approval: 寸止(ID:code_refactor). }}
        // 计算角度并使用通用提交逻辑
        double angle = calculateLineAngle(m_currentLine.points[0], m_currentLine.points[1]);
        QString result = QString("直线: 角度 %.1f°").arg(angle);
        commitDrawingAction(ActionType::Line, result);
    }
    
    update();
}

void VideoDisplayWidget::handleCircleDrawingClick(const QPointF& imagePos)
{
    if (!m_hasCurrentCircle) {
        // 开始新的圆形
        m_currentCircle = CircleObject();
        m_currentCircle.points.append(imagePos);
        m_hasCurrentCircle = true;
    } else if (m_currentCircle.points.size() < 3) {
        // 添加第二个或第三个点
        m_currentCircle.points.append(imagePos);
        
        if (m_currentCircle.points.size() == 3) {
            // 计算圆心和半径
            if (calculateCircleFromThreePoints(m_currentCircle.points[0], 
                                             m_currentCircle.points[1], 
                                             m_currentCircle.points[2], 
                                             m_currentCircle.center, 
                                             m_currentCircle.radius)) {
                m_currentCircle.isCompleted = true;
                
                // {{ AURA-X: Refactor - 使用通用提交逻辑简化代码. Approval: 寸止(ID:code_refactor). }}
                // 计算圆的面积和周长并使用通用提交逻辑
                double area = M_PI * m_currentCircle.radius * m_currentCircle.radius;
                double circumference = 2 * M_PI * m_currentCircle.radius;
                QString result = QString("圆形: 半径 %.1f, 面积 %.1f, 周长 %.1f")
                                .arg(m_currentCircle.radius).arg(area).arg(circumference);
                commitDrawingAction(ActionType::Circle, result);
            } else {
                // 三点共线，重新开始
                clearCurrentCircleData();
            }
        }
    }
    update();
}

void VideoDisplayWidget::handleFineCircleDrawingClick(const QPointF& imagePos)
{
    if (!m_hasCurrentFineCircle) {
        // 开始新的精细圆
        m_currentFineCircle = FineCircleObject();
        m_currentFineCircle.points.append(imagePos);
        m_hasCurrentFineCircle = true;
    } else if (m_currentFineCircle.points.size() < 5) {
        // 添加点直到有5个点
        m_currentFineCircle.points.append(imagePos);
        
        if (m_currentFineCircle.points.size() == 5) {
            // 使用五点拟合圆
            if (calculateCircleFromFivePoints(m_currentFineCircle.points, 
                                            m_currentFineCircle.center, 
                                            m_currentFineCircle.radius)) {
                m_currentFineCircle.isCompleted = true;
                
                // {{ AURA-X: Refactor - 使用通用提交逻辑简化代码. Approval: 寸止(ID:code_refactor). }}
                // 计算精细圆的面积和周长并使用通用提交逻辑
                double area = M_PI * m_currentFineCircle.radius * m_currentFineCircle.radius;
                double circumference = 2 * M_PI * m_currentFineCircle.radius;
                QString result = QString("精细圆: 半径 %.1f, 面积 %.1f, 周长 %.1f")
                                .arg(m_currentFineCircle.radius).arg(area).arg(circumference);
                commitDrawingAction(ActionType::FineCircle, result);
            } else {
                // 拟合失败，重新开始
                clearCurrentFineCircleData();
            }
        }
    }
    update();
}

void VideoDisplayWidget::handleParallelDrawingClick(const QPointF& imagePos)
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
            // {{ AURA-X: Add - 清除鼠标预览状态. Approval: 寸止(ID:preview_fix). }}
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
                m_currentParallel.isCompleted = true;
                
                // {{ AURA-X: Refactor - 使用通用提交逻辑简化代码. Approval: 寸止(ID:code_refactor). }}
                // 使用通用提交逻辑
                QString result = QString("平行线: 距离 %.1f, 角度 %.1f°")
                                .arg(m_currentParallel.distance).arg(m_currentParallel.angle);
                commitDrawingAction(ActionType::Parallel, result);
            } else {
                // 计算失败，重新开始
                clearCurrentParallelData();
            }
        }
    }
    update();
}

void VideoDisplayWidget::handleTwoLinesDrawingClick(const QPointF& imagePos)
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
            // {{ AURA-X: Modify - 修复点击逻辑，确保每次点击都正确添加点并清除预览状态. Approval: 寸止(ID:click_fix). }}
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
                
                // {{ AURA-X: Refactor - 使用通用提交逻辑简化代码. Approval: 寸止(ID:code_refactor). }}
                // 使用通用提交逻辑
                QString result = QString("两线夹角: %.1f°")
                                .arg(m_currentTwoLines.angle);
                commitDrawingAction(ActionType::TwoLines, result);
            } else {
                // 两线平行，重新开始
                clearCurrentTwoLinesData();
            }
        }
    }
    update();
}

QPointF VideoDisplayWidget::imageToWidget(const QPointF& imagePos) const
{
    double scale = getScaleFactor();
    QPointF offset = getImageOffset();
    
    return QPointF(imagePos.x() * scale + offset.x(), 
                   imagePos.y() * scale + offset.y());
}

QPointF VideoDisplayWidget::widgetToImage(const QPointF& widgetPos) const
{
    double scale = getScaleFactor();
    QPointF offset = getImageOffset();
    
    return QPointF((widgetPos.x() - offset.x()) / scale, 
                   (widgetPos.y() - offset.y()) / scale);
}

double VideoDisplayWidget::calculateFontSize() const
{
    // Calculate font size based on user view scaling, consistent with parallel line logic
    if (m_videoFrame.isNull()) {
        return 8.0; // Default font size
    }
    
    // Use the same logic as parallel lines for consistent user experience
    double scaleFactor = getScaleFactor();
    const double heightScale = std::max(0.8, std::min(scaleFactor, 4.0));
    const double fontSizeBase = std::max(0.8, 0.6 + (0.8 * heightScale));
    
    // Scale up to match the expected font size range for UI rendering
    return fontSizeBase * 8.0;
}

double VideoDisplayWidget::calculateLineAngle(const QPointF& start, const QPointF& end) const
{
    double dx = end.x() - start.x();
    double dy = end.y() - start.y();
    
    // Calculate angle (radians to degrees)
    double angleRad = atan2(dy, dx);
    double angleDeg = angleRad * 180.0 / M_PI;
    
    // Ensure angle is in 0-180 degree range
    if (angleDeg < 0) {
        angleDeg += 180.0;
    }
    if (angleDeg >= 180.0) {
        angleDeg -= 180.0;
    }
    
    return angleDeg;
}

void VideoDisplayWidget::calculateExtendedLine(const QPointF& start, const QPointF& end, QPointF& extStart, QPointF& extEnd) const
{
    // 获取图像尺寸
    QSize imageSize = m_videoFrame.size();
    if (imageSize.isEmpty()) {
        extStart = start;
        extEnd = end;
        return;
    }
    
    double imageWidth = imageSize.width();
    double imageHeight = imageSize.height();
    
    // 计算直线方向向量
    double dx = end.x() - start.x();
    double dy = end.y() - start.y();
    
    // 避免除零错误
    if (abs(dx) < 1e-6 && abs(dy) < 1e-6) {
        extStart = start;
        extEnd = end;
        return;
    }
    
    // 计算延伸参数
    double t1 = -1000.0; // 向后延伸
    double t2 = 1000.0;  // 向前延伸
    
    // 与图像边界的交点计算
    if (abs(dx) > 1e-6) {
        // 与左边界 (x=0) 的交点
        double t_left = (0 - start.x()) / dx;
        // 与右边界 (x=imageWidth) 的交点
        double t_right = (imageWidth - start.x()) / dx;
        
        t1 = qMax(t1, qMin(t_left, t_right));
        t2 = qMin(t2, qMax(t_left, t_right));
    }
    
    if (abs(dy) > 1e-6) {
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

void VideoDisplayWidget::drawTextWithBackground(QPainter& painter,
                                                const QPointF& anchorPoint,
                                                const QString& text,
                                                const QFont& font,
                                                const QColor& textColor,
                                                const QColor& bgColor,
                                                double padding,
                                                double borderWidth,
                                                const QPointF& offset)
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
    painter.setPen(createPen(textColor, borderWidth, getScaleFactor())); // 使用文本颜色作为边框颜色
    painter.setBrush(QBrush(bgColor));
    painter.drawRect(contentRectWithPadding);

    // 5. 在内容框内居中绘制文本
    painter.setPen(createPen(textColor, 1, getScaleFactor()));
    painter.setFont(font);
    painter.drawText(contentRectWithPadding, Qt::AlignCenter, text);
}



// 【终极API设计】计算文本背景矩形的辅助函数
QRectF VideoDisplayWidget::calculateTextWithBackgroundRect(const QPointF& anchorPoint, const QString& text, 
                                                           const QFont& font, double padding, const QPointF& offset)
{
    QFontMetrics fm(font);
    QRect textBoundingRect = fm.boundingRect(text);
    QRectF contentRectWithPadding = QRectF(0, 0, textBoundingRect.width(), textBoundingRect.height()).adjusted(-padding, -padding, padding, padding);
    contentRectWithPadding.moveTopLeft(anchorPoint + offset);
    return contentRectWithPadding;
}

// 【终极API设计】在预先计算好的矩形中绘制文本
void VideoDisplayWidget::drawTextInRect(QPainter& painter, const QRectF& rect, const QString& text,
                                        const QFont& font, const QColor& textColor, const QColor& bgColor,
                                        double borderWidth)
{
    // 绘制背景和边框
    painter.setPen(createPen(textColor, borderWidth, getScaleFactor())); // 使用文本颜色作为边框颜色
    painter.setBrush(QBrush(bgColor));
    painter.drawRect(rect);
    
    // 在矩形内居中绘制文本
    painter.setPen(createPen(textColor, 1, getScaleFactor()));
    painter.setFont(font);
    painter.drawText(rect, Qt::AlignCenter, text);
}

bool VideoDisplayWidget::calculateCircleFromThreePoints(const QPointF& p1, const QPointF& p2, const QPointF& p3,
                                                         QPointF& center, double& radius) const
{
    double x1 = p1.x(), y1 = p1.y();
    double x2 = p2.x(), y2 = p2.y();
    double x3 = p3.x(), y3 = p3.y();
    
    // 计算行列式
    double det = 2 * (x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2));
    
    if (abs(det) < 1e-10) {
        // 三点共线
        return false;
    }
    
    // 计算圆心坐标
    double cx = ((x1*x1 + y1*y1) * (y2 - y3) + (x2*x2 + y2*y2) * (y3 - y1) + (x3*x3 + y3*y3) * (y1 - y2)) / det;
    double cy = ((x1*x1 + y1*y1) * (x3 - x2) + (x2*x2 + y2*y2) * (x1 - x3) + (x3*x3 + y3*y3) * (x2 - x1)) / det;
    
    center = QPointF(cx, cy);
    
    // 计算半径
    radius = sqrt((cx - x1) * (cx - x1) + (cy - y1) * (cy - y1));
    
    return true;
}

bool VideoDisplayWidget::calculateCircleFromFivePoints(const QVector<QPointF>& points,
                                                        QPointF& center, double& radius) const
{
    if (points.size() != 5) {
        return false;
    }
    
    // 使用最小二乘法拟合圆
    // 设圆的方程为: (x-a)^2 + (y-b)^2 = r^2
    // 展开为: x^2 + y^2 - 2ax - 2by + (a^2 + b^2 - r^2) = 0
    // 设 A = -2a, B = -2b, C = a^2 + b^2 - r^2
    // 则方程为: x^2 + y^2 + Ax + By + C = 0
    
    // 构建线性方程组求解A, B, C
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
    
    // 构建矩阵方程 M * [A B C]^T = V
    double m11 = sumX2, m12 = sumXY, m13 = sumX;
    double m21 = sumXY, m22 = sumY2, m23 = sumY;
    double m31 = sumX, m32 = sumY, m33 = n;
    
    double v1 = -(sumX3 + sumXY2);
    double v2 = -(sumX2Y + sumY3);
    double v3 = -(sumX2 + sumY2);
    
    // 使用克拉默法则求解
    double det = m11 * (m22 * m33 - m23 * m32) - m12 * (m21 * m33 - m23 * m31) + m13 * (m21 * m32 - m22 * m31);
    
    if (abs(det) < 1e-10) {
        return false;
    }
    
    double A = (v1 * (m22 * m33 - m23 * m32) - m12 * (v2 * m33 - m23 * v3) + m13 * (v2 * m32 - m22 * v3)) / det;
    double B = (m11 * (v2 * m33 - m23 * v3) - v1 * (m21 * m33 - m23 * m31) + m13 * (m21 * v3 - v2 * m31)) / det;
    
    center = QPointF(-A / 2, -B / 2);
    
    // 计算半径（使用第一个点）
    QPointF p0 = points[0];
    radius = sqrt((center.x() - p0.x()) * (center.x() - p0.x()) + (center.y() - p0.y()) * (center.y() - p0.y()));
    
    return true;
}

bool VideoDisplayWidget::calculateLineIntersection(const QPointF& p1, const QPointF& p2, const QPointF& p3, const QPointF& p4, QPointF& intersection) const
{
    double x1 = p1.x(), y1 = p1.y();
    double x2 = p2.x(), y2 = p2.y();
    double x3 = p3.x(), y3 = p3.y();
    double x4 = p4.x(), y4 = p4.y();
    
    double denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    
    if (abs(denom) < 1e-10) {
        // 两线平行
        return false;
    }
    
    double t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denom;
    
    intersection = QPointF(x1 + t * (x2 - x1), y1 + t * (y2 - y1));
    
    return true;
}

double VideoDisplayWidget::calculateDistancePointToLine(const QPointF& point, const QPointF& lineStart, const QPointF& lineEnd) const
{
    double A = lineEnd.y() - lineStart.y();
    double B = lineStart.x() - lineEnd.x();
    double C = lineEnd.x() * lineStart.y() - lineStart.x() * lineEnd.y();
    
    double distance = abs(A * point.x() + B * point.y() + C) / sqrt(A * A + B * B);
    
    return distance;
}

// {{ AURA-X: Add - DrawingContext缓存管理方法实现. Approval: 寸止(ID:context_cache). }}
void VideoDisplayWidget::updateDrawingContext()
{
    // 计算当前绘制参数
    double scale = getScaleFactor();
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
    m_cachedDrawingContext.grayPen = createPen(Qt::gray, 1, scale);
    m_cachedDrawingContext.redDashedPen = createPen(Qt::red, 2, scale, true); // 红色虚线画笔
    
    // 创建所有画刷
    m_cachedDrawingContext.greenBrush = QBrush(Qt::green);
    m_cachedDrawingContext.blackBrush = QBrush(Qt::black);
    m_cachedDrawingContext.whiteBrush = QBrush(Qt::white);
    m_cachedDrawingContext.redBrush = QBrush(Qt::red);
    m_cachedDrawingContext.blueBrush = QBrush(Qt::blue);
    
    // 更新缓存状态
    m_drawingContextValid = true;
    m_lastContextWidgetSize = size();
    m_lastContextImageSize = m_videoFrame.size();
}

bool VideoDisplayWidget::needsDrawingContextUpdate() const
{
    // 检查是否需要更新DrawingContext
    if (!m_drawingContextValid) {
        return true;
    }
    
    // 检查控件尺寸是否变化
    if (m_lastContextWidgetSize != size()) {
        return true;
    }
    
    // 检查图像尺寸是否变化
    if (m_lastContextImageSize != m_videoFrame.size()) {
        return true;
    }
    
    return false;
}

// {{ AURA-X: Add - 图元选择功能实现. Approval: 寸止(ID:selection_feature). }}
// 启用选择模式
void VideoDisplayWidget::enableSelection(bool enabled)
{
    m_selectionEnabled = enabled;
    if (enabled) {
        // 启用选择模式时，停止绘制模式
        stopDrawing();
        setCursor(Qt::ArrowCursor);
    }
    update();
}

// 检查是否启用了选择模式
bool VideoDisplayWidget::isSelectionEnabled() const
{
    return m_selectionEnabled;
}

// 清除选择状态
void VideoDisplayWidget::clearSelection()
{
    m_selectedPoints.clear();
    m_selectedLines.clear();
    m_selectedLineSegments.clear();
    m_selectedCircles.clear();
    m_selectedFineCircles.clear();
    m_selectedParallels.clear();
    m_selectedTwoLines.clear();
    m_selectedParallelMiddleLines.clear();
    update();
}

// 获取选中图元信息
QString VideoDisplayWidget::getSelectedObjectInfo() const
{
    QStringList info;
    
    if (!m_selectedPoints.isEmpty()) {
        info << QString("选中了 %1 个点").arg(m_selectedPoints.size());
    }
    if (!m_selectedLines.isEmpty()) {
        info << QString("选中了 %1 条直线").arg(m_selectedLines.size());
    }
    if (!m_selectedLineSegments.isEmpty()) {
        info << QString("选中了 %1 条线段").arg(m_selectedLineSegments.size());
    }
    if (!m_selectedCircles.isEmpty()) {
        info << QString("选中了 %1 个圆").arg(m_selectedCircles.size());
    }
    if (!m_selectedFineCircles.isEmpty()) {
        info << QString("选中了 %1 个精细圆").arg(m_selectedFineCircles.size());
    }
    if (!m_selectedParallels.isEmpty()) {
        info << QString("选中了 %1 组平行线").arg(m_selectedParallels.size());
    }
    if (!m_selectedTwoLines.isEmpty()) {
        info << QString("选中了 %1 个两线夹角").arg(m_selectedTwoLines.size());
    }
    if (!m_selectedParallelMiddleLines.isEmpty()) {
        info << QString("选中了 %1 条平行线中线").arg(m_selectedParallelMiddleLines.size());
    }
    
    if (info.isEmpty()) {
        return "未选中任何图元";
    }
    
    return info.join(", ");
}

// 处理选择点击
void VideoDisplayWidget::handleSelectionClick(const QPointF& imagePos, bool ctrlPressed)
{
    bool foundHit = false;
    
    // 命中测试顺序：点 -> 线 -> 圆 -> 精细圆 -> 平行线 -> 两线夹角
    
    // 1. 测试点
    for (int i = 0; i < m_points.size(); ++i) {
        if (hitTestPoint(imagePos, i)) {
            selectObject("point", i);
            foundHit = true;
            break;
        }
    }
    
    if (!foundHit) {
        // 2. 测试直线
        for (int i = 0; i < m_lines.size(); ++i) {
            if (hitTestLine(imagePos, i)) {
                selectObject("line", i);
                foundHit = true;
                break;
            }
        }
    }
    
    if (!foundHit) {
        // 3. 测试线段
        for (int i = 0; i < m_lineSegments.size(); ++i) {
            if (hitTestLineSegment(imagePos, i)) {
                selectObject("line_segment", i);
                foundHit = true;
                break;
            }
        }
    }
    
    if (!foundHit) {
        // 4. 测试圆
        for (int i = 0; i < m_circles.size(); ++i) {
            if (hitTestCircle(imagePos, i)) {
                selectObject("circle", i);
                foundHit = true;
                break;
            }
        }
    }
    
    if (!foundHit) {
        // 4. 测试精细圆
        for (int i = 0; i < m_fineCircles.size(); ++i) {
            if (hitTestFineCircle(imagePos, i)) {
                selectObject("fine_circle", i);
                foundHit = true;
                break;
            }
        }
    }
    
    if (!foundHit) {
        // 5. 测试平行线（包括中线）
        for (int i = 0; i < m_parallels.size(); ++i) {
            QString hitResult = hitTestParallel(imagePos, i);
            if (!hitResult.isEmpty()) {
                if (hitResult == "middle_line") {
                    selectObject("parallel_middle_line", i, true);
                } else {
                    selectObject("parallel", i);
                }
                foundHit = true;
                break;
            }
        }
    }
    
    if (!foundHit) {
        // 6. 测试两线夹角
        for (int i = 0; i < m_twoLines.size(); ++i) {
            if (hitTestTwoLines(imagePos, i)) {
                selectObject("two_lines", i);
                foundHit = true;
                break;
            }
        }
    }
    
    // 如果没有命中任何图元且没有按Ctrl，清除选择
    if (!foundHit && !ctrlPressed) {
        clearSelection();
    }
    
    // 发送选择信息到状态栏
    emit selectionChanged(getSelectedObjectInfo());
}

// 选择图元
void VideoDisplayWidget::selectObject(const QString& type, int index, bool isMiddleLine)
{
    if (type == "point") {
        if (m_selectedPoints.contains(index)) {
            m_selectedPoints.remove(index);
        } else {
            m_selectedPoints.insert(index);
        }
    } else if (type == "line") {
        if (m_selectedLines.contains(index)) {
            m_selectedLines.remove(index);
        } else {
            m_selectedLines.insert(index);
        }
    } else if (type == "circle") {
        if (m_selectedCircles.contains(index)) {
            m_selectedCircles.remove(index);
        } else {
            m_selectedCircles.insert(index);
        }
    } else if (type == "fine_circle") {
        if (m_selectedFineCircles.contains(index)) {
            m_selectedFineCircles.remove(index);
        } else {
            m_selectedFineCircles.insert(index);
        }
    } else if (type == "parallel") {
        if (m_selectedParallels.contains(index)) {
            m_selectedParallels.remove(index);
        } else {
            m_selectedParallels.insert(index);
        }
    } else if (type == "two_lines") {
        if (m_selectedTwoLines.contains(index)) {
            m_selectedTwoLines.remove(index);
        } else {
            m_selectedTwoLines.insert(index);
        }
    } else if (type == "parallel_middle_line") {
        if (m_selectedParallelMiddleLines.contains(index)) {
            m_selectedParallelMiddleLines.remove(index);
        } else {
            m_selectedParallelMiddleLines.insert(index);
        }
    } else if (type == "line_segment") {
        if (m_selectedLineSegments.contains(index)) {
            m_selectedLineSegments.remove(index);
        } else {
            m_selectedLineSegments.insert(index);
        }
    }
    
    update();
}

// 命中测试：点
bool VideoDisplayWidget::hitTestPoint(const QPointF& testPos, int index) const
{
    if (index < 0 || index >= m_points.size()) return false;
    const QPointF& point = m_points[index];
    double distance = sqrt(pow(testPos.x() - point.x(), 2) + pow(testPos.y() - point.y(), 2));
    return distance <= 20.0; // 20像素范围
}

// 命中测试：直线
bool VideoDisplayWidget::hitTestLine(const QPointF& testPos, int index) const
{
    if (index < 0 || index >= m_lines.size()) return false;
    const LineObject& line = m_lines[index];
    if (line.points.size() < 2) return false;
    
    double distance = calculateDistancePointToLine(testPos, line.points[0], line.points[1]);
    return distance <= 10.0; // 10像素范围
}

// 命中测试：线段
bool VideoDisplayWidget::hitTestLineSegment(const QPointF& testPos, int index) const
{
    if (index < 0 || index >= m_lineSegments.size()) return false;
    const LineSegmentObject& lineSegment = m_lineSegments[index];
    if (!lineSegment.isCompleted) return false;
    
    // 计算点到线段的距离
    if (lineSegment.points.size() < 2) return false;
    const QPointF& start = lineSegment.points[0];
    const QPointF& end = lineSegment.points[1];
    
    // 计算线段长度的平方
    double segmentLengthSq = pow(end.x() - start.x(), 2) + pow(end.y() - start.y(), 2);
    if (segmentLengthSq < 1e-6) return false; // 线段长度为0
    
    // 计算投影参数t
    double t = ((testPos.x() - start.x()) * (end.x() - start.x()) + 
                (testPos.y() - start.y()) * (end.y() - start.y())) / segmentLengthSq;
    
    // 限制t在[0,1]范围内，确保投影点在线段上
    t = qMax(0.0, qMin(1.0, t));
    
    // 计算投影点
    QPointF projection(start.x() + t * (end.x() - start.x()),
                      start.y() + t * (end.y() - start.y()));
    
    // 计算测试点到投影点的距离
    double distance = sqrt(pow(testPos.x() - projection.x(), 2) + 
                          pow(testPos.y() - projection.y(), 2));
    
    return distance <= 10.0; // 10像素范围
}

// 命中测试：圆
bool VideoDisplayWidget::hitTestCircle(const QPointF& testPos, int index) const
{
    if (index < 0 || index >= m_circles.size()) return false;
    const CircleObject& circle = m_circles[index];
    if (circle.points.size() < 3) return false;
    
    QPointF center;
    double radius;
    if (!calculateCircleFromThreePoints(circle.points[0], circle.points[1], circle.points[2], center, radius)) {
        return false;
    }
    
    double distance = sqrt(pow(testPos.x() - center.x(), 2) + pow(testPos.y() - center.y(), 2));
    return abs(distance - radius) <= 10.0; // 圆周附近10像素范围
}

// 命中测试：精细圆
bool VideoDisplayWidget::hitTestFineCircle(const QPointF& testPos, int index) const
{
    if (index < 0 || index >= m_fineCircles.size()) return false;
    const FineCircleObject& fineCircle = m_fineCircles[index];
    if (fineCircle.points.size() < 5) return false;
    
    QPointF center;
    double radius;
    if (!calculateCircleFromFivePoints(fineCircle.points, center, radius)) {
        return false;
    }
    
    double distance = sqrt(pow(testPos.x() - center.x(), 2) + pow(testPos.y() - center.y(), 2));
    return abs(distance - radius) <= 10.0; // 圆周附近10像素范围
}

// 命中测试：平行线
QString VideoDisplayWidget::hitTestParallel(const QPointF& testPos, int index) const
{
    if (index < 0 || index >= m_parallels.size()) return "";
    const ParallelObject& parallel = m_parallels[index];
    if (parallel.points.size() < 3) return "";
    
    const QPointF& p1 = parallel.points[0];
    const QPointF& p2 = parallel.points[1];
    const QPointF& p3 = parallel.points[2];
    
    // 测试第一条线
    double dist1 = calculateDistancePointToLine(testPos, p1, p2);
    if (dist1 <= 10.0) {
        return "line1";
    }
    
    // 计算第二条线的点
    QPointF direction = p2 - p1;
    QPointF p4 = p3 + direction;
    
    // 测试第二条线
    double dist2 = calculateDistancePointToLine(testPos, p3, p4);
    if (dist2 <= 10.0) {
        return "line2";
    }
    
    // 测试中线
    QPointF midP1 = (p1 + p3) / 2.0;
    QPointF midP2 = (p2 + p4) / 2.0;
    double distMid = calculateDistancePointToLine(testPos, midP1, midP2);
    if (distMid <= 10.0) {
        return "middle_line";
    }
    
    return "";
}

// 命中测试：两线夹角
bool VideoDisplayWidget::hitTestTwoLines(const QPointF& testPos, int index) const
{
    if (index < 0 || index >= m_twoLines.size()) return false;
    const TwoLinesObject& twoLines = m_twoLines[index];
    if (twoLines.points.size() < 4) return false;
    
    // 测试第一条线
    double dist1 = calculateDistancePointToLine(testPos, twoLines.points[0], twoLines.points[1]);
    if (dist1 <= 10.0) {
        return true;
    }
    
    // 测试第二条线
    double dist2 = calculateDistancePointToLine(testPos, twoLines.points[2], twoLines.points[3]);
    if (dist2 <= 10.0) {
        return true;
    }
    
    return false;
}

// 绘制选择高亮
void VideoDisplayWidget::drawSelectionHighlight(QPainter& painter, const DrawingContext& ctx)
{
    // {{ AURA-X: Modify - 将选择高亮改为蓝色描边，提升视觉效果. Approval: 寸止(ID:blue_selection_highlight). }}
    // 创建高亮画笔（蓝色，较粗）
    QPen highlightPen = createPen(Qt::blue, 4, ctx.scale);
    painter.setPen(highlightPen);
    painter.setBrush(Qt::NoBrush);
    
    // 高亮选中的点
    for (int index : m_selectedPoints) {
        if (index >= 0 && index < m_points.size()) {
            const QPointF& point = m_points[index];
            double radius = 25.0 * std::max(1.0, std::min(ctx.scale, 4.0));
            painter.drawEllipse(point, radius, radius);
        }
    }
    
    // 高亮选中的直线
    for (int index : m_selectedLines) {
        if (index >= 0 && index < m_lines.size()) {
            const LineObject& line = m_lines[index];
            if (line.points.size() >= 2) {
                // 绘制延伸的高亮线
                QPointF direction = line.points[1] - line.points[0];
                QPointF extendedStart = line.points[0] - direction * 5000.0;
                QPointF extendedEnd = line.points[1] + direction * 5000.0;
                painter.drawLine(extendedStart, extendedEnd);
            }
        }
    }
    
    // 高亮选中的圆
    for (int index : m_selectedCircles) {
        if (index >= 0 && index < m_circles.size()) {
            const CircleObject& circle = m_circles[index];
            if (circle.points.size() >= 3) {
                QPointF center;
                double radius;
                if (calculateCircleFromThreePoints(circle.points[0], circle.points[1], circle.points[2], center, radius)) {
                    painter.drawEllipse(center, radius, radius);
                }
            }
        }
    }
    
    // 高亮选中的精细圆
    for (int index : m_selectedFineCircles) {
        if (index >= 0 && index < m_fineCircles.size()) {
            const FineCircleObject& fineCircle = m_fineCircles[index];
            if (fineCircle.points.size() >= 5) {
                QPointF center;
                double radius;
                if (calculateCircleFromFivePoints(fineCircle.points, center, radius)) {
                    painter.drawEllipse(center, radius, radius);
                }
            }
        }
    }
    
    // 高亮选中的平行线
    for (int index : m_selectedParallels) {
        if (index >= 0 && index < m_parallels.size()) {
            const ParallelObject& parallel = m_parallels[index];
            if (parallel.points.size() >= 3) {
                const QPointF& p1 = parallel.points[0];
                const QPointF& p2 = parallel.points[1];
                const QPointF& p3 = parallel.points[2];
                
                QPointF direction = p2 - p1;
                QPointF p4 = p3 + direction;
                
                // 高亮两条平行线
                QPointF extendedStart1 = p1 - direction * 5000.0;
                QPointF extendedEnd1 = p2 + direction * 5000.0;
                painter.drawLine(extendedStart1, extendedEnd1);
                
                QPointF extendedStart2 = p3 - direction * 5000.0;
                QPointF extendedEnd2 = p4 + direction * 5000.0;
                painter.drawLine(extendedStart2, extendedEnd2);
            }
        }
    }
    
    // 高亮选中的平行线中线
    for (int index : m_selectedParallelMiddleLines) {
        if (index >= 0 && index < m_parallels.size()) {
            const ParallelObject& parallel = m_parallels[index];
            if (parallel.points.size() >= 3) {
                const QPointF& p1 = parallel.points[0];
                const QPointF& p2 = parallel.points[1];
                const QPointF& p3 = parallel.points[2];
                
                QPointF direction = p2 - p1;
                QPointF p4 = p3 + direction;
                
                // 高亮中线
                QPointF midP1 = (p1 + p3) / 2.0;
                QPointF midP2 = (p2 + p4) / 2.0;
                QPointF extendedMidStart = midP1 - direction * 5000.0;
                QPointF extendedMidEnd = midP2 + direction * 5000.0;
                painter.drawLine(extendedMidStart, extendedMidEnd);
            }
        }
    }
    
    // 高亮选中的两线夹角
    for (int index : m_selectedTwoLines) {
        if (index >= 0 && index < m_twoLines.size()) {
            const TwoLinesObject& twoLines = m_twoLines[index];
            if (twoLines.points.size() >= 4) {
                // 高亮第一条线
                QPointF direction1 = twoLines.points[1] - twoLines.points[0];
                QPointF extendedStart1 = twoLines.points[0] - direction1 * 5000.0;
                QPointF extendedEnd1 = twoLines.points[1] + direction1 * 5000.0;
                painter.drawLine(extendedStart1, extendedEnd1);
                
                // 高亮第二条线
                QPointF direction2 = twoLines.points[3] - twoLines.points[2];
                QPointF extendedStart2 = twoLines.points[2] - direction2 * 5000.0;
                QPointF extendedEnd2 = twoLines.points[3] + direction2 * 5000.0;
                painter.drawLine(extendedStart2, extendedEnd2);
            }
        }
    }
    
    // 高亮选中的线段
    for (int index : m_selectedLineSegments) {
        if (index >= 0 && index < m_lineSegments.size()) {
            const LineSegmentObject& lineSegment = m_lineSegments[index];
            if (lineSegment.isCompleted && lineSegment.points.size() >= 2) {
                painter.drawLine(lineSegment.points[0], lineSegment.points[1]);
            }
        }
    }
}

// 选择状态改变槽函数
void VideoDisplayWidget::onSelectionChanged()
{
    // 更新显示
    update();
    
    // 发送选择信息到状态栏
    QString info = getSelectedObjectInfo();
    emit selectionChanged(info);
}

// {{ AURA-X: Add - 删除选中对象功能. Approval: 寸止(ID:context_menu_delete). }}
void VideoDisplayWidget::deleteSelectedObjects()
{
    bool dataChanged = false;
    
    // 删除选中的点（从后往前删除，避免索引变化）
    QList<int> pointIndices = m_selectedPoints.values();
    std::sort(pointIndices.rbegin(), pointIndices.rend());
    for (int index : pointIndices) {
        if (index >= 0 && index < m_points.size()) {
            m_points.removeAt(index);
            dataChanged = true;
        }
    }
    
    // 删除选中的直线
    QList<int> lineIndices = m_selectedLines.values();
    std::sort(lineIndices.rbegin(), lineIndices.rend());
    for (int index : lineIndices) {
        if (index >= 0 && index < m_lines.size()) {
            m_lines.removeAt(index);
            dataChanged = true;
        }
    }
    
    // 删除选中的圆
    QList<int> circleIndices = m_selectedCircles.values();
    std::sort(circleIndices.rbegin(), circleIndices.rend());
    for (int index : circleIndices) {
        if (index >= 0 && index < m_circles.size()) {
            m_circles.removeAt(index);
            dataChanged = true;
        }
    }
    
    // 删除选中的精细圆
    QList<int> fineCircleIndices = m_selectedFineCircles.values();
    std::sort(fineCircleIndices.rbegin(), fineCircleIndices.rend());
    for (int index : fineCircleIndices) {
        if (index >= 0 && index < m_fineCircles.size()) {
            m_fineCircles.removeAt(index);
            dataChanged = true;
        }
    }
    
    // 删除选中的平行线
    QList<int> parallelIndices = m_selectedParallels.values();
    std::sort(parallelIndices.rbegin(), parallelIndices.rend());
    for (int index : parallelIndices) {
        if (index >= 0 && index < m_parallels.size()) {
            m_parallels.removeAt(index);
            dataChanged = true;
        }
    }
    
    // 删除选中的两线夹角
    QList<int> twoLinesIndices = m_selectedTwoLines.values();
    std::sort(twoLinesIndices.rbegin(), twoLinesIndices.rend());
    for (int index : twoLinesIndices) {
        if (index >= 0 && index < m_twoLines.size()) {
            m_twoLines.removeAt(index);
            dataChanged = true;
        }
    }
    
    // 删除选中的线段
    QList<int> lineSegmentIndices = m_selectedLineSegments.values();
    std::sort(lineSegmentIndices.rbegin(), lineSegmentIndices.rend());
    for (int index : lineSegmentIndices) {
        if (index >= 0 && index < m_lineSegments.size()) {
            m_lineSegments.removeAt(index);
            dataChanged = true;
        }
    }
    
    // 清除选择状态
    clearSelection();
    
    // 如果有数据变化，发出信号并更新显示
    if (dataChanged) {
        emit drawingDataChanged(m_viewName);
        update();
    }
}

// {{ AURA-X: Add - 右键菜单事件处理. Approval: 寸止(ID:context_menu_delete). }}
void VideoDisplayWidget::contextMenuEvent(QContextMenuEvent *event)
{
    // 只在选择模式下显示右键菜单
    if (!m_selectionEnabled) {
        QLabel::contextMenuEvent(event);
        return;
    }
    
    // 检查是否有选中的对象
    bool hasSelection = !m_selectedPoints.isEmpty() || !m_selectedLines.isEmpty() || 
                       !m_selectedCircles.isEmpty() || !m_selectedFineCircles.isEmpty() || 
                       !m_selectedParallels.isEmpty() || !m_selectedTwoLines.isEmpty() || 
                       !m_selectedParallelMiddleLines.isEmpty() || !m_selectedLineSegments.isEmpty();
    
    if (!hasSelection) {
        QLabel::contextMenuEvent(event);
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
        m_selectedParallels.isEmpty() && m_selectedTwoLines.isEmpty() && 
        m_selectedParallelMiddleLines.isEmpty()) {
        pointToPointAction = contextMenu.addAction("点与点");
    }
    
    // 显示菜单并处理选择
    QAction *selectedAction = contextMenu.exec(event->globalPos());
    
    if (selectedAction == deleteAction) {
        deleteSelectedObjects();
    } else if (selectedAction == pointToPointAction) {
        createLineFromSelectedPoints();
    }
}

void VideoDisplayWidget::createLineFromSelectedPoints()
{
    if (m_selectedPoints.size() != 2) {
        return;
    }
    
    // 获取选中的两个点
    QList<int> selectedIndices(m_selectedPoints.begin(), m_selectedPoints.end());
    QPointF point1 = m_points[selectedIndices[0]];
    QPointF point2 = m_points[selectedIndices[1]];
    
    // 创建新的线段对象
    LineSegmentObject newLineSegment;
    newLineSegment.points.append(point1);
    newLineSegment.points.append(point2);
    newLineSegment.isCompleted = true;
    newLineSegment.color = Qt::green;  // 统一绿色
    newLineSegment.thickness = 2.0;  // 默认粗细
    newLineSegment.isDashed = false; // 默认实线
    
    // 添加到线段列表
    m_lineSegments.append(newLineSegment);
    
    // 清除选择状态
    m_selectedPoints.clear();
    
    // 更新显示
    update();
}