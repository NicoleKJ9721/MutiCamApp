#include "PaintingOverlay.h"
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
    , m_hasValidMousePos(false)
    , m_selectionEnabled(true)
    , m_scaleFactor(1.0)
    , m_imageSize(QSize())
    , m_drawingContextValid(false)
    , m_hasCurrentLineSegment(false)
    , m_hasCurrentFineCircle(false)
{
    // 关键：设置透明背景，并让鼠标事件穿透到下层（如果需要）
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setMouseTracking(true);
}

void PaintingOverlay::startDrawing(DrawingTool tool)
{
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
    m_currentPoints.clear();

    // 停止绘制模式时重新启用选择模式并清除选择状态
    m_selectionEnabled = true;
    clearSelection();

    setCursor(Qt::ArrowCursor);
    update();
}

void PaintingOverlay::clearAllDrawings()
{
    m_points.clear();
    m_lines.clear();
    m_circles.clear();
    m_fineCircles.clear();
    m_parallels.clear();
    m_twoLines.clear();
    m_drawingHistory.clear();
    clearSelection();
    update();
}

void PaintingOverlay::undoLastDrawing()
{
    if (m_drawingHistory.isEmpty()) {
        return;
    }
    
    DrawingAction action = m_drawingHistory.pop();
    
    switch (action.type) {
        case DrawingAction::AddPoint:
            if (action.index < m_points.size()) {
                m_points.removeAt(action.index);
            }
            break;
        case DrawingAction::AddLine:
            if (action.index < m_lines.size()) {
                m_lines.removeAt(action.index);
            }
            break;
        case DrawingAction::AddCircle:
            if (action.index < m_circles.size()) {
                m_circles.removeAt(action.index);
            }
            break;
        case DrawingAction::AddFineCircle:
            if (action.index < m_fineCircles.size()) {
                m_fineCircles.removeAt(action.index);
            }
            break;
        case DrawingAction::AddParallel:
            if (action.index < m_parallels.size()) {
                m_parallels.removeAt(action.index);
            }
            break;
        case DrawingAction::AddTwoLines:
            if (action.index < m_twoLines.size()) {
                m_twoLines.removeAt(action.index);
            }
            break;
        default:
            break;
    }
    
    update();
}

void PaintingOverlay::setTransforms(const QPointF& offset, double scale, const QSize& imageSize)
{
    m_imageOffset = offset;
    m_scaleFactor = scale;
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

        // 3. 【关键】绘制当前正在预览的图形（在最上层）
        if (m_isDrawingMode) {
            drawCurrentPreview(painter, ctx);
        }
        
        painter.restore();
    }
}

void PaintingOverlay::mousePressEvent(QMouseEvent *event)
{
    // 处理右键点击：如果在绘制模式且没有选中任何图元，退出绘制模式
    if (event->button() == Qt::RightButton) {
        if (m_isDrawingMode) {
            // 检查是否有选中的对象
            bool hasSelection = !m_selectedPoints.isEmpty() || !m_selectedLines.isEmpty() ||
                               !m_selectedCircles.isEmpty() || !m_selectedFineCircles.isEmpty() ||
                               !m_selectedParallels.isEmpty() || !m_selectedTwoLines.isEmpty() ||
                               !m_selectedLineSegments.isEmpty() || !m_selectedParallelMiddleLines.isEmpty();

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



    if (m_selectionEnabled) {
        // 处理选择逻辑
        bool ctrlPressed = (event->modifiers() & Qt::ControlModifier) != 0;
        handleSelectionClick(imagePos, ctrlPressed);
        return;
    }
    
    if (!m_isDrawingMode) {
        return;
    }
    
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
        default:
            break;
    }
}

void PaintingOverlay::mouseMoveEvent(QMouseEvent *event)
{
    // 事件节流：限制更新频率以提升性能
    static QTime lastUpdateTime = QTime::currentTime();
    QTime currentTime = QTime::currentTime();
    
    if (lastUpdateTime.msecsTo(currentTime) < 16) { // 约60fps
        return;
    }
    lastUpdateTime = currentTime;
    
    QPointF imagePos = widgetToImage(event->pos());
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

    // 只在绘图模式或有当前绘制对象时更新
    if (m_isDrawingMode || m_hasCurrentLine || m_hasCurrentCircle ||
        m_hasCurrentFineCircle || m_hasCurrentParallel || m_hasCurrentTwoLines) {
        update(); // 更新预览
    }
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
                       !m_selectedLineSegments.isEmpty() || !m_selectedParallelMiddleLines.isEmpty();
    
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
    
    // 显示菜单并处理选择
    QAction *selectedAction = contextMenu.exec(event->globalPos());
    
    if (selectedAction == deleteAction) {
        deleteSelectedObjects();
    } else if (selectedAction == pointToPointAction) {
        createLineFromSelectedPoints();
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
    action.index = m_points.size() - 1;
    commitDrawingAction(action);
}

void PaintingOverlay::handleLineDrawingClick(const QPointF& pos)
{
    if (!m_hasCurrentLine) {
        // 开始新线段
        m_currentLine.points.clear();
        m_currentLine.points.append(pos);
        m_currentLine.start = pos;
        m_hasCurrentLine = true;
    } else {
        // 完成线段
        m_currentLine.points.append(pos);
        m_currentLine.end = pos;
        m_currentLine.label = QString("L%1").arg(m_lines.size() + 1);
        
        // 计算长度
        QPointF diff = m_currentLine.end - m_currentLine.start;
        m_currentLine.length = qSqrt(diff.x() * diff.x() + diff.y() * diff.y());
        
        m_lines.append(m_currentLine);
        
        DrawingAction action;
        action.type = DrawingAction::AddLine;
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
    const double innerRadius = 20 * heightScale;
    const double textpadding = qMax(4.0, ctx.fontSize * 2);
    
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
    
    // 绘制当前正在绘制的直线（实时预览）
    if (m_hasCurrentLine && !m_currentLine.points.isEmpty()) {
        drawSingleLine(painter, m_currentLine, true, ctx);
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
    
    // 使用简单可靠的方向向量延伸方法，让QPainter自动裁剪
    // 向后延伸足够长的距离（在图像坐标系中，5000像素足以覆盖任何合理的显示区域）
    QPointF extendedStart = start - direction * 5000.0;
    // 向前延伸足够长的距离
    QPointF extendedEnd = end + direction * 5000.0;
    
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
    double textPadding = qMax(4.0, ctx.fontSize * 2);
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
        const double centerMarkRadius = 15 * heightScale;

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
    double textPadding = qMax(4.0, ctx.fontSize * 2);
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

        // 绘制圆心标记（使用绿色）
        double centerMarkRadius = qMax(4.0, 6.0 * ctx.scale);
        painter.setPen(ctx.whitePen);
        painter.setBrush(ctx.greenBrush);
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
    double textPadding = qMax(4.0, ctx.fontSize * 2);
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

        // 绘制第二条平行线（使用绿色）
        painter.setPen(ctx.greenPen);
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
        
        // 当有2个点时，显示第三个点的预览
        if (twoLines.points.size() == 2 && m_hasValidMousePos) {
            const QPointF& previewPos = m_currentMousePos;
            
            // 绘制预览点 - 使用虚线样式的圆环，不显示序号
            // 使用绿色画笔
            painter.setPen(ctx.greenPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(previewPos, pointOuterRadius, pointOuterRadius);
        }
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
            
            // 使用绿色画笔
            painter.setPen(ctx.greenPen);
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
            
            // 使用绿色画笔
            painter.setPen(ctx.greenPen);
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
    // 内部进行补偿，确保在屏幕上看起来是 targetScreenSize 大小
    font.setPointSizeF(qMax(8.0, targetScreenSize / scale));
    font.setBold(true);
    return font;
}

double PaintingOverlay::calculateFontSize() const
{
    // Calculate font size based on user view scaling, consistent with parallel line logic
    if (m_imageSize.isEmpty()) {
        return 8.0; // Default font size
    }

    // Use the same logic as parallel lines for consistent user experience
    double scaleFactor = m_scaleFactor;
    const double heightScale = std::max(0.8, std::min(scaleFactor, 4.0));
    const double fontSizeBase = std::max(0.8, 0.6 + (0.8 * heightScale));

    // Scale up to match the expected font size range for UI rendering
    return fontSizeBase * 8.0;
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

void PaintingOverlay::drawTextWithBackground(QPainter& painter, const QPointF& pos, const QString& text, const QColor& textColor, const QColor& bgColor) const
{
    QFontMetrics fm(painter.font());
    QRect textRect = fm.boundingRect(text);
    textRect.moveCenter(pos.toPoint());
    
    // 绘制背景
    painter.fillRect(textRect.adjusted(-2, -1, 2, 1), bgColor);
    
    // 绘制文本
    QPen textPen(textColor);
    painter.setPen(textPen);
    painter.drawText(textRect, Qt::AlignCenter, text);
}

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
    lineSegment.label = QString("LS%1").arg(m_lineSegments.size() + 1);

    // 计算长度
    QPointF p1 = lineSegment.points[0];
    QPointF p2 = lineSegment.points[1];
    lineSegment.length = sqrt(pow(p2.x() - p1.x(), 2) + pow(p2.y() - p1.y(), 2));
    lineSegment.showLength = true;

    m_lineSegments.append(lineSegment);

    // 记录历史
    DrawingAction action;
    action.type = DrawingAction::AddLineSegment;
    action.index = m_lineSegments.size() - 1;
    commitDrawingAction(action);

    // 清除选择状态
    clearSelection();

    // 发出信号
    QString result = QString("线段: 长度 %.1f").arg(lineSegment.length);
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
    if (twoLinesIndex >= 0 && !foundSelection) {
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
        // 修改逻辑：点击圆内部或圆周附近都能选中
        if (distance <= radius + tolerance) {
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

        double distance = calculateDistancePointToLine(testPos, lineSegment.points[0], lineSegment.points[1]);
        if (distance <= tolerance) {
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
        // 点击圆内部或圆周附近都能选中
        if (distance <= fineCircle.radius + tolerance) {
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