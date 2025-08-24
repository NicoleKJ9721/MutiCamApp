#include "RotatableROIOverlay.h"
#include <QPainter>
#include <QApplication>
#include <QDebug>
#include <QtMath>
#include <algorithm>

RotatableROIOverlay::RotatableROIOverlay(QWidget *parent)
    : QWidget(parent)
    , m_roi(100, 100, 200, 150)
    , m_rotationAngle(0.0f)
    , m_visible(false)
    , m_enabled(true)
    , m_dragging(false)
    , m_activeControlPoint(None)
    , m_dragStartRotation(0.0f)
    , m_borderColor(Qt::red)
    , m_fillColor(QColor(255, 0, 0, 30))
    , m_controlPointColor(Qt::red)
    , m_rotationHandleColor(Qt::blue)
    , m_borderWidth(2)
    , m_controlPointSize(8)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setFocusPolicy(Qt::StrongFocus);
    
    m_roiCenter = m_roi.center();
}

void RotatableROIOverlay::setROI(const QRectF& roi)
{
    if (m_roi != roi) {
        m_roi = roi;
        m_roiCenter = roi.center();
        update();
        emit roiChanged(m_roi, m_rotationAngle);
    }
}

void RotatableROIOverlay::setRotation(float angle)
{
    // 规范化角度到 [-180, 180] 范围
    while (angle > 180.0f) angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    
    if (qAbs(m_rotationAngle - angle) > 0.1f) {
        m_rotationAngle = angle;
        update();
        emit roiChanged(m_roi, m_rotationAngle);
    }
}

void RotatableROIOverlay::setVisible(bool visible)
{
    if (m_visible != visible) {
        m_visible = visible;
        QWidget::setVisible(visible);
        update();
    }
}

void RotatableROIOverlay::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        update();
    }
}

void RotatableROIOverlay::setTemplateCreationMode(bool enabled)
{
    setVisible(enabled);
    setEnabled(enabled);
    
    // 设置鼠标事件透传属性
    setAttribute(Qt::WA_TransparentForMouseEvents, !enabled);
    
    if (enabled) {
        // 进入模板创建模式，获取焦点
        setFocus();
        raise(); // 确保在最上层
    } else {
        // 退出模板创建模式，重置状态
        reset();
        clearFocus();
    }
}

void RotatableROIOverlay::setCoordinateTransform(std::function<QPointF(const QPoint&)> windowToImage,
                                                std::function<QPoint(const QPointF&)> imageToWindow)
{
    m_windowToImage = windowToImage;
    m_imageToWindow = imageToWindow;
}

QPolygonF RotatableROIOverlay::getTransformedROI() const
{
    return calculateROIPolygon();
}

void RotatableROIOverlay::reset()
{
    m_roi = QRectF(100, 100, 200, 150);
    m_rotationAngle = 0.0f;
    m_roiCenter = m_roi.center();
    m_dragging = false;
    m_activeControlPoint = None;
    update();
    emit roiChanged(m_roi, m_rotationAngle);
}

void RotatableROIOverlay::paintEvent(QPaintEvent* event)
{
    if (!m_visible) {
        return;
    }
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // 绘制ROI
    drawROI(painter);
    
    // 绘制控制点
    if (m_enabled) {
        drawControlPoints(painter);
        drawRotationHandle(painter);
    }
    
    // 绘制ROI信息
    drawROIInfo(painter);
}

void RotatableROIOverlay::mousePressEvent(QMouseEvent* event)
{
    if (!m_visible || !m_enabled) {
        QWidget::mousePressEvent(event);
        return;
    }
    
    if (event->button() == Qt::LeftButton) {
        m_lastMousePos = event->pos();
        m_activeControlPoint = hitTestControlPoint(event->pos());
        
        if (m_activeControlPoint != None) {
            m_dragging = true;
            m_dragStartROI = m_roi.topLeft();
            m_dragStartRotation = m_rotationAngle;
            emit roiEditingStarted();
            setCursor(Qt::ClosedHandCursor);
        } else if (hitTestROI(event->pos())) {
            m_dragging = true;
            m_activeControlPoint = None;
            m_dragStartROI = m_roi.topLeft();
            emit roiEditingStarted();
            setCursor(Qt::ClosedHandCursor);
        }
        
        update();
    }
    
    QWidget::mousePressEvent(event);
}

void RotatableROIOverlay::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_visible || !m_enabled) {
        QWidget::mouseMoveEvent(event);
        return;
    }
    
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        QPointF currentImagePos = windowToImage(event->pos());
        QPointF lastImagePos = windowToImage(m_lastMousePos);
        QPointF delta = currentImagePos - lastImagePos;
        
        if (m_activeControlPoint == None) {
            // 移动整个ROI
            moveROI(delta);
        } else if (m_activeControlPoint == RotationHandle) {
            // 旋转操作
            updateRotationFromHandle(currentImagePos);
        } else {
            // 调整控制点
            updateROIFromControlPoint(m_activeControlPoint, currentImagePos);
        }
        
        m_lastMousePos = event->pos();
        update();
    } else {
        // 更新鼠标光标
        ControlPoint hitPoint = hitTestControlPoint(event->pos());
        if (hitPoint != None) {
            if (hitPoint == RotationHandle) {
                setCursor(Qt::PointingHandCursor);
            } else {
                setCursor(Qt::SizeAllCursor);
            }
        } else if (hitTestROI(event->pos())) {
            setCursor(Qt::OpenHandCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    }
    
    QWidget::mouseMoveEvent(event);
}

void RotatableROIOverlay::mouseReleaseEvent(QMouseEvent* event)
{
    if (!m_visible || !m_enabled) {
        QWidget::mouseReleaseEvent(event);
        return;
    }
    
    if (event->button() == Qt::LeftButton && m_dragging) {
        m_dragging = false;
        m_activeControlPoint = None;
        setCursor(Qt::ArrowCursor);
        emit roiEditingFinished();
        update();
    }
    
    QWidget::mouseReleaseEvent(event);
}

void RotatableROIOverlay::keyPressEvent(QKeyEvent* event)
{
    if (!m_visible || !m_enabled) {
        QWidget::keyPressEvent(event);
        return;
    }
    
    const float moveStep = 1.0f;
    const float rotateStep = 1.0f;
    
    switch (event->key()) {
        case Qt::Key_Left:
            moveROI(QPointF(-moveStep, 0));
            break;
        case Qt::Key_Right:
            moveROI(QPointF(moveStep, 0));
            break;
        case Qt::Key_Up:
            moveROI(QPointF(0, -moveStep));
            break;
        case Qt::Key_Down:
            moveROI(QPointF(0, moveStep));
            break;
        case Qt::Key_R:
            if (event->modifiers() & Qt::ControlModifier) {
                setRotation(m_rotationAngle + rotateStep);
            } else {
                setRotation(m_rotationAngle - rotateStep);
            }
            break;
        case Qt::Key_Escape:
            reset();
            break;
        default:
            QWidget::keyPressEvent(event);
            return;
    }
    
    update();
}

void RotatableROIOverlay::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    constrainROI();
}

QPolygonF RotatableROIOverlay::calculateROIPolygon() const
{
    QPolygonF polygon;
    
    // 创建未旋转的矩形
    polygon << m_roi.topLeft()
            << m_roi.topRight()
            << m_roi.bottomRight()
            << m_roi.bottomLeft();
    
    // 如果有旋转，应用变换
    if (qAbs(m_rotationAngle) > 0.1f) {
        QTransform transform;
        transform.translate(m_roiCenter.x(), m_roiCenter.y());
        transform.rotate(m_rotationAngle);
        transform.translate(-m_roiCenter.x(), -m_roiCenter.y());
        polygon = transform.map(polygon);
    }
    
    return polygon;
}

std::vector<QPointF> RotatableROIOverlay::calculateControlPoints() const
{
    std::vector<QPointF> points(8);
    
    // 计算8个控制点的位置（相对于ROI）
    points[TopLeft] = m_roi.topLeft();
    points[TopCenter] = QPointF(m_roi.center().x(), m_roi.top());
    points[TopRight] = m_roi.topRight();
    points[MiddleRight] = QPointF(m_roi.right(), m_roi.center().y());
    points[BottomRight] = m_roi.bottomRight();
    points[BottomCenter] = QPointF(m_roi.center().x(), m_roi.bottom());
    points[BottomLeft] = m_roi.bottomLeft();
    points[MiddleLeft] = QPointF(m_roi.left(), m_roi.center().y());
    
    // 应用旋转变换
    if (qAbs(m_rotationAngle) > 0.1f) {
        QTransform transform;
        transform.translate(m_roiCenter.x(), m_roiCenter.y());
        transform.rotate(m_rotationAngle);
        transform.translate(-m_roiCenter.x(), -m_roiCenter.y());
        
        for (auto& point : points) {
            point = transform.map(point);
        }
    }
    
    return points;
}

QPointF RotatableROIOverlay::calculateRotationHandle() const
{
    QPointF topCenter = QPointF(m_roi.center().x(), m_roi.top());
    QPointF handle = topCenter + QPointF(0, -ROTATION_HANDLE_DISTANCE);
    
    // 应用旋转变换
    if (qAbs(m_rotationAngle) > 0.1f) {
        QTransform transform;
        transform.translate(m_roiCenter.x(), m_roiCenter.y());
        transform.rotate(m_rotationAngle);
        transform.translate(-m_roiCenter.x(), -m_roiCenter.y());
        handle = transform.map(handle);
    }
    
    return handle;
}

RotatableROIOverlay::ControlPoint RotatableROIOverlay::hitTestControlPoint(const QPoint& pos) const
{
    QPointF imagePos = windowToImage(pos);
    
    // 测试旋转控制点
    QPointF rotationHandle = calculateRotationHandle();
    if (QLineF(imagePos, rotationHandle).length() <= HIT_TEST_TOLERANCE) {
        return RotationHandle;
    }
    
    // 测试8个控制点
    auto controlPoints = calculateControlPoints();
    for (int i = 0; i < 8; ++i) {
        if (QLineF(imagePos, controlPoints[i]).length() <= HIT_TEST_TOLERANCE) {
            return static_cast<ControlPoint>(i);
        }
    }
    
    return None;
}

bool RotatableROIOverlay::hitTestROI(const QPoint& pos) const
{
    QPointF imagePos = windowToImage(pos);
    QPolygonF roiPolygon = calculateROIPolygon();
    return roiPolygon.containsPoint(imagePos, Qt::OddEvenFill);
}

void RotatableROIOverlay::updateROIFromControlPoint(ControlPoint point, const QPointF& newPos)
{
    QRectF newROI = m_roi;
    
    // 根据控制点类型调整ROI
    switch (point) {
        case TopLeft:
            newROI.setTopLeft(newPos);
            break;
        case TopCenter:
            newROI.setTop(newPos.y());
            break;
        case TopRight:
            newROI.setTopRight(newPos);
            break;
        case MiddleRight:
            newROI.setRight(newPos.x());
            break;
        case BottomRight:
            newROI.setBottomRight(newPos);
            break;
        case BottomCenter:
            newROI.setBottom(newPos.y());
            break;
        case BottomLeft:
            newROI.setBottomLeft(newPos);
            break;
        case MiddleLeft:
            newROI.setLeft(newPos.x());
            break;
        default:
            return;
    }
    
    // 确保ROI有效
    if (newROI.width() >= MIN_ROI_SIZE && newROI.height() >= MIN_ROI_SIZE) {
        m_roi = newROI.normalized();
        m_roiCenter = m_roi.center();
        constrainROI();
        emit roiChanged(m_roi, m_rotationAngle);
    }
}

void RotatableROIOverlay::updateRotationFromHandle(const QPointF& handlePos)
{
    QPointF center = m_roiCenter;
    QPointF vector = handlePos - center;
    
    // 计算角度（相对于垂直向上方向）
    float angle = qRadiansToDegrees(qAtan2(vector.x(), -vector.y()));
    
    setRotation(angle);
}

void RotatableROIOverlay::moveROI(const QPointF& delta)
{
    QRectF newROI = m_roi.translated(delta);
    newROI = constrainToParent(newROI);
    
    if (newROI != m_roi) {
        m_roi = newROI;
        m_roiCenter = m_roi.center();
        emit roiChanged(m_roi, m_rotationAngle);
    }
}

void RotatableROIOverlay::constrainROI()
{
    m_roi = constrainToParent(m_roi);
    m_roiCenter = m_roi.center();
}

QRectF RotatableROIOverlay::constrainToParent(const QRectF& rect) const
{
    if (!m_imageToWindow || !m_windowToImage) {
        return rect;
    }
    
    // 获取父控件的图像边界
    QRectF parentBounds = QRectF(windowToImage(QPoint(0, 0)), 
                                windowToImage(QPoint(width(), height())));
    
    QRectF constrainedRect = rect;
    
    // 确保ROI在边界内
    if (constrainedRect.left() < parentBounds.left()) {
        constrainedRect.moveLeft(parentBounds.left());
    }
    if (constrainedRect.top() < parentBounds.top()) {
        constrainedRect.moveTop(parentBounds.top());
    }
    if (constrainedRect.right() > parentBounds.right()) {
        constrainedRect.moveRight(parentBounds.right());
    }
    if (constrainedRect.bottom() > parentBounds.bottom()) {
        constrainedRect.moveBottom(parentBounds.bottom());
    }
    
    return constrainedRect;
}

void RotatableROIOverlay::drawROI(QPainter& painter) const
{
    QPolygonF roiPolygon = calculateROIPolygon();
    
    // 转换到窗口坐标
    QPolygonF windowPolygon;
    for (const QPointF& point : roiPolygon) {
        windowPolygon << imageToWindow(point);
    }
    
    // 绘制填充
    painter.setBrush(QBrush(m_fillColor));
    painter.setPen(Qt::NoPen);
    painter.drawPolygon(windowPolygon);
    
    // 绘制边框
    QPen borderPen(m_borderColor, m_borderWidth);
    borderPen.setCosmetic(true);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(borderPen);
    painter.drawPolygon(windowPolygon);
}

void RotatableROIOverlay::drawControlPoints(QPainter& painter) const
{
    auto controlPoints = calculateControlPoints();
    
    QPen pen(m_controlPointColor, 1);
    pen.setCosmetic(true);
    painter.setPen(pen);
    painter.setBrush(QBrush(m_controlPointColor));
    
    for (const QPointF& point : controlPoints) {
        QPoint windowPoint = imageToWindow(point);
        painter.drawEllipse(windowPoint, m_controlPointSize/2, m_controlPointSize/2);
    }
}

void RotatableROIOverlay::drawRotationHandle(QPainter& painter) const
{
    QPointF rotationHandle = calculateRotationHandle();
    QPoint windowHandle = imageToWindow(rotationHandle);
    QPoint windowCenter = imageToWindow(m_roiCenter);
    
    // 绘制连接线
    QPen linePen(m_rotationHandleColor, 1, Qt::DashLine);
    linePen.setCosmetic(true);
    painter.setPen(linePen);
    painter.drawLine(windowCenter, windowHandle);
    
    // 绘制旋转控制点
    QPen handlePen(m_rotationHandleColor, 2);
    handlePen.setCosmetic(true);
    painter.setPen(handlePen);
    painter.setBrush(QBrush(m_rotationHandleColor));
    painter.drawEllipse(windowHandle, m_controlPointSize, m_controlPointSize);
}

void RotatableROIOverlay::drawROIInfo(QPainter& painter) const
{
    // 绘制ROI信息文本
    QString info = QString("ROI: %1x%2, Angle: %3°")
                   .arg(qRound(m_roi.width()))
                   .arg(qRound(m_roi.height()))
                   .arg(m_rotationAngle, 0, 'f', 1);
    
    QPoint textPos = imageToWindow(m_roi.topLeft()) + QPoint(5, -5);
    
    QPen textPen(Qt::white);
    painter.setPen(textPen);
    painter.setFont(QFont("Arial", 10));
    
    // 绘制文本背景
    QFontMetrics fm(painter.font());
    QRect textRect = fm.boundingRect(info);
    textRect.moveTopLeft(textPos);
    textRect.adjust(-2, -2, 2, 2);
    
    painter.fillRect(textRect, QColor(0, 0, 0, 128));
    painter.drawText(textPos, info);
}

QPointF RotatableROIOverlay::windowToImage(const QPoint& windowPos) const
{
    if (m_windowToImage) {
        return m_windowToImage(windowPos);
    }
    return QPointF(windowPos);
}

QPoint RotatableROIOverlay::imageToWindow(const QPointF& imagePos) const
{
    if (m_imageToWindow) {
        return m_imageToWindow(imagePos);
    }
    return imagePos.toPoint();
}
