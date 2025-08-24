#include "ZoomPanWidget.h"
#include "VideoDisplayWidget.h"
#include "PaintingOverlay.h"
#include "template_matching/RotatableROIOverlay.h"
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QDebug>
#include <QTimer>
#include <cmath>

ZoomPanWidget::ZoomPanWidget(QWidget *parent)
    : QWidget(parent)
    , m_videoWidget(nullptr)
    , m_paintingOverlay(nullptr)
    , m_roiOverlay(nullptr)
    , m_zoomFactor(1.0)
    , m_minZoomFactor(0.1)
    , m_maxZoomFactor(5.0)
    , m_zoomStep(0.1)
    , m_lastZoomCenter(0, 0)
    , m_panOffsetX(0.0)
    , m_panOffsetY(0.0)
    , m_maxPanOffsetX(0.0)
    , m_maxPanOffsetY(0.0)
    , m_isPanning(false)
    , m_spacePressed(false)
    , m_lastPanPoint(0, 0)
    , m_panAnimationTimer(nullptr)
    , m_keyPanActive(false)
    , m_keyPanVelocity(0.0, 0.0)
    , m_keyPanTimer(nullptr)
    , m_panEnabled(true)
{
    initializeComponents();
    setupLayout();
    connectSignals();
    
    // 设置焦点策略以接收键盘事件
    setFocusPolicy(Qt::StrongFocus);

    // 启用鼠标跟踪
    setMouseTracking(true);

    // 设置工具提示
    setToolTip("缩放平移操作说明：\n"
               "• 鼠标滚轮：缩放视图\n"
               "• 方向键：平移视图（需要先缩放）\n"
               "• 空格+鼠标左键：拖拽平移\n"
               "• 点击'重置缩放'按钮恢复默认视图");
}

void ZoomPanWidget::initializeComponents()
{
    // 创建VideoDisplayWidget
    m_videoWidget = new VideoDisplayWidget(this);
    
    // 创建定时器
    m_panAnimationTimer = new QTimer(this);
    m_panAnimationTimer->setSingleShot(false);
    m_panAnimationTimer->setInterval(16); // 60 FPS
    
    m_keyPanTimer = new QTimer(this);
    m_keyPanTimer->setSingleShot(false);
    m_keyPanTimer->setInterval(16); // 60 FPS
}

void ZoomPanWidget::setupLayout()
{
    // 创建布局
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    // 添加VideoDisplayWidget
    layout->addWidget(m_videoWidget);
    
    setLayout(layout);
}

void ZoomPanWidget::connectSignals()
{
    // 连接定时器信号
    connect(m_panAnimationTimer, &QTimer::timeout,
            this, &ZoomPanWidget::onPanAnimationTimer);
    
    connect(m_keyPanTimer, &QTimer::timeout,
            this, &ZoomPanWidget::onPanAnimationTimer);
}

void ZoomPanWidget::setVideoFrame(const QPixmap& pixmap)
{
    if (m_videoWidget) {
        m_videoWidget->setVideoFrame(pixmap);

        // 重新计算最大平移偏移
        calculateMaxPanOffset();

        // 约束当前平移偏移
        constrainPanOffset();

        // 更新变换
        updateVideoWidgetTransform();
        updatePaintingOverlayTransform();
    }
}

void ZoomPanWidget::setZoomFactor(double factor)
{
    double newFactor = qBound(m_minZoomFactor, factor, m_maxZoomFactor);
    if (qAbs(newFactor - m_zoomFactor) > 0.001) {
        m_zoomFactor = newFactor;
        
        // 重新计算最大平移偏移
        calculateMaxPanOffset();
        
        // 约束平移偏移
        constrainPanOffset();
        
        // 更新变换
        updateVideoWidgetTransform();
        updatePaintingOverlayTransform();
        
        // 发射信号
        emit viewTransformChanged(m_zoomFactor, getPanOffset());
    }
}

void ZoomPanWidget::resetZoom()
{
    setZoomFactor(1.0);
}

void ZoomPanWidget::resetPan()
{
    setPanOffset(QPointF(0.0, 0.0));
}

void ZoomPanWidget::resetView()
{
    m_zoomFactor = 1.0;
    m_panOffsetX = 0.0;
    m_panOffsetY = 0.0;
    
    updateVideoWidgetTransform();
    updatePaintingOverlayTransform();
    
    emit viewTransformChanged(m_zoomFactor, getPanOffset());
}

void ZoomPanWidget::setZoomRange(double minZoom, double maxZoom)
{
    m_minZoomFactor = qMax(0.01, minZoom);
    m_maxZoomFactor = qMax(m_minZoomFactor, maxZoom);

    // 确保当前缩放因子在范围内
    setZoomFactor(m_zoomFactor);
}

void ZoomPanWidget::setPanEnabled(bool enabled)
{
    m_panEnabled = enabled;

    // 如果禁用平移功能，重置相关状态
    if (!enabled) {
        m_spacePressed = false;
        m_isPanning = false;
        m_keyPanActive = false;
        setCursor(Qt::ArrowCursor);
        if (m_paintingOverlay) {
            m_paintingOverlay->setCursor(Qt::ArrowCursor);
        }
        if (m_keyPanTimer && m_keyPanTimer->isActive()) {
            m_keyPanTimer->stop();
        }
    }
}

void ZoomPanWidget::setPanOffset(const QPointF& offset)
{
    m_panOffsetX = offset.x();
    m_panOffsetY = offset.y();
    
    constrainPanOffset();
    
    updateVideoWidgetTransform();
    updatePaintingOverlayTransform();
    
    emit viewTransformChanged(m_zoomFactor, getPanOffset());
}

void ZoomPanWidget::setPaintingOverlay(PaintingOverlay* overlay)
{
    m_paintingOverlay = overlay;

    if (m_paintingOverlay) {
        // 设置PaintingOverlay覆盖整个ZoomPanWidget
        m_paintingOverlay->setGeometry(rect());
        m_paintingOverlay->raise(); // 确保在最上层
        m_paintingOverlay->show();

        // 更新变换参数
        updatePaintingOverlayTransform();
    }
}

void ZoomPanWidget::setROIOverlay(RotatableROIOverlay* overlay)
{
    m_roiOverlay = overlay;

    if (m_roiOverlay) {
        // 设置ROIOverlay覆盖整个ZoomPanWidget
        m_roiOverlay->setGeometry(rect());
        m_roiOverlay->setParent(this);
        
        // 设置坐标转换函数
        m_roiOverlay->setCoordinateTransform(
            [this](const QPoint& windowPos) { return windowToImageCoordinates(windowPos); },
            [this](const QPointF& imagePos) { return imageToWindowCoordinates(imagePos); }
        );
        
        // 初始状态为隐藏
        m_roiOverlay->setTemplateCreationMode(false);
        
        // 更新变换参数
        updateROIOverlayTransform();
    }
}

QPointF ZoomPanWidget::windowToImageCoordinates(const QPoint& windowPos) const
{
    if (!m_videoWidget) {
        return QPointF(windowPos);
    }
    
    // 获取有效的图像偏移和缩放因子
    QPointF imageOffset = getEffectiveImageOffset();
    double scaleFactor = getEffectiveScaleFactor();
    
    if (scaleFactor <= 0.0) {
        return QPointF(windowPos);
    }
    
    // 转换坐标
    double imageX = (windowPos.x() - imageOffset.x()) / scaleFactor;
    double imageY = (windowPos.y() - imageOffset.y()) / scaleFactor;
    
    return QPointF(imageX, imageY);
}

QPoint ZoomPanWidget::imageToWindowCoordinates(const QPointF& imagePos) const
{
    if (!m_videoWidget) {
        return imagePos.toPoint();
    }
    
    // 获取有效的图像偏移和缩放因子
    QPointF imageOffset = getEffectiveImageOffset();
    double scaleFactor = getEffectiveScaleFactor();
    
    // 转换坐标
    int windowX = static_cast<int>(imagePos.x() * scaleFactor + imageOffset.x());
    int windowY = static_cast<int>(imagePos.y() * scaleFactor + imageOffset.y());
    
    return QPoint(windowX, windowY);
}

QPointF ZoomPanWidget::getImageOffset() const
{
    return getEffectiveImageOffset();
}

double ZoomPanWidget::getScaleFactor() const
{
    return getEffectiveScaleFactor();
}

QSize ZoomPanWidget::getImageSize() const
{
    if (m_videoWidget) {
        return m_videoWidget->getImageSize();
    }
    return QSize(0, 0);
}

void ZoomPanWidget::wheelEvent(QWheelEvent* event)
{
    // 获取焦点以接收键盘事件
    setFocus();
    
    // 计算缩放变化
    QPoint numDegrees = event->angleDelta() / 8;
    if (!numDegrees.isNull()) {
        QPoint numSteps = numDegrees / 15;
        double zoomDelta = numSteps.y() * m_zoomStep;
        
        // 计算新的缩放因子
        double newZoomFactor = m_zoomFactor + zoomDelta;
        
        // 执行缩放
        performZoom(newZoomFactor, event->position().toPoint());
    }
    
    event->accept();
}

void ZoomPanWidget::keyPressEvent(QKeyEvent* event)
{
    // 如果平移功能被禁用，不处理任何键盘事件
    if (!m_panEnabled) {
        QWidget::keyPressEvent(event);
        return;
    }

    // 处理空格键
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        m_spacePressed = true;
        setCursor(Qt::OpenHandCursor);
        // 同步PaintingOverlay的光标
        if (m_paintingOverlay) {
            m_paintingOverlay->setCursor(Qt::OpenHandCursor);
        }
        qDebug() << "ZoomPanWidget: 空格键按下，设置光标为OpenHandCursor";
        event->accept();
        return;
    }
    
    // 处理方向键平移（仅在缩放状态下）
    if (m_zoomFactor > 1.0) {
        QPointF velocity(0.0, 0.0);
        double speed = 100.0 * m_zoomFactor; // 根据缩放因子调整速度
        
        switch (event->key()) {
        case Qt::Key_Left:
            velocity.setX(speed);
            break;
        case Qt::Key_Right:
            velocity.setX(-speed);
            break;
        case Qt::Key_Up:
            velocity.setY(speed);
            break;
        case Qt::Key_Down:
            velocity.setY(-speed);
            break;
        default:
            QWidget::keyPressEvent(event);
            return;
        }
        
        if (!velocity.isNull()) {
            startKeyboardPan(velocity);
            event->accept();
            return;
        }
    }
    
    QWidget::keyPressEvent(event);
}

void ZoomPanWidget::keyReleaseEvent(QKeyEvent* event)
{
    // 如果平移功能被禁用，不处理任何键盘事件
    if (!m_panEnabled) {
        QWidget::keyReleaseEvent(event);
        return;
    }

    // 处理空格键释放
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        m_spacePressed = false;
        if (!m_isPanning) {
            setCursor(Qt::ArrowCursor);
            // 同步PaintingOverlay的光标
            if (m_paintingOverlay) {
                m_paintingOverlay->setCursor(Qt::ArrowCursor);
            }
        }
        qDebug() << "ZoomPanWidget: 空格键释放，m_spacePressed=" << m_spacePressed;
        event->accept();
        return;
    }

    // 处理方向键释放
    if (m_keyPanActive) {
        switch (event->key()) {
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Up:
        case Qt::Key_Down:
            stopKeyboardPan();
            event->accept();
            return;
        }
    }

    QWidget::keyReleaseEvent(event);
}

void ZoomPanWidget::mousePressEvent(QMouseEvent* event)
{
    // 确保获得焦点以接收键盘事件
    setFocus();

    if (event->button() == Qt::LeftButton && m_spacePressed && m_panEnabled) {
        // 开始平移
        m_isPanning = true;
        m_lastPanPoint = event->pos();
        setCursor(Qt::ClosedHandCursor);
        // 同步PaintingOverlay的光标
        if (m_paintingOverlay) {
            m_paintingOverlay->setCursor(Qt::ClosedHandCursor);
        }
        qDebug() << "ZoomPanWidget: 开始平移，设置光标为ClosedHandCursor";
        event->accept();
    } else {
        // 转发鼠标事件
        emit mousePressed(event->pos());
        QWidget::mousePressEvent(event);
    }
}

void ZoomPanWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isPanning && m_spacePressed && m_panEnabled) {
        // 执行平移
        QPoint delta = event->pos() - m_lastPanPoint;
        performPan(delta.x(), delta.y());
        m_lastPanPoint = event->pos();
        event->accept();
    } else {
        // 转发鼠标事件
        emit mouseMoved(event->pos());
        QWidget::mouseMoveEvent(event);
    }
}

void ZoomPanWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_isPanning) {
        // 结束平移
        m_isPanning = false;
        Qt::CursorShape newCursor = m_spacePressed ? Qt::OpenHandCursor : Qt::ArrowCursor;
        setCursor(newCursor);
        // 同步PaintingOverlay的光标
        if (m_paintingOverlay) {
            m_paintingOverlay->setCursor(newCursor);
        }
        event->accept();
    } else {
        // 转发鼠标事件
        emit mouseReleased(event->pos());
        QWidget::mouseReleaseEvent(event);
    }
}

void ZoomPanWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    // 更新PaintingOverlay的几何信息
    if (m_paintingOverlay) {
        m_paintingOverlay->setGeometry(rect());
    }
    
    // 更新ROIOverlay的几何信息
    if (m_roiOverlay) {
        m_roiOverlay->setGeometry(rect());
    }

    // 重新计算最大平移偏移
    calculateMaxPanOffset();

    // 约束平移偏移
    constrainPanOffset();

    // 更新变换
    updateVideoWidgetTransform();
    updatePaintingOverlayTransform();
    updateROIOverlayTransform();
}

void ZoomPanWidget::onPanAnimationTimer()
{
    if (m_keyPanActive && !m_keyPanVelocity.isNull()) {
        // 键盘平移动画
        double deltaTime = 0.016; // 16ms
        double deltaX = m_keyPanVelocity.x() * deltaTime;
        double deltaY = m_keyPanVelocity.y() * deltaTime;

        performPan(deltaX, deltaY);
    }
}

// 私有方法实现
void ZoomPanWidget::updateVideoWidgetTransform()
{
    if (m_videoWidget) {
        // 将变换参数传递给VideoDisplayWidget
        QPointF offset = getEffectiveImageOffset();
        double scale = getEffectiveScaleFactor();
        m_videoWidget->setExternalTransform(offset, scale);
    }
}

void ZoomPanWidget::updatePaintingOverlayTransform()
{
    if (m_paintingOverlay) {
        // 更新PaintingOverlay的变换参数
        QPointF offset = getEffectiveImageOffset();
        double scale = getEffectiveScaleFactor();
        QSize imageSize = getImageSize();

        m_paintingOverlay->setTransforms(offset, scale, imageSize);
    }
}

void ZoomPanWidget::updateROIOverlayTransform()
{
    if (m_roiOverlay) {
        // ROI覆盖层的坐标转换已通过lambda函数实现
        // 这里可以添加其他需要更新的变换参数
        m_roiOverlay->update();
    }
}

void ZoomPanWidget::constrainPanOffset()
{
    // 约束平移偏移在有效范围内
    m_panOffsetX = qBound(-m_maxPanOffsetX, m_panOffsetX, m_maxPanOffsetX);
    m_panOffsetY = qBound(-m_maxPanOffsetY, m_panOffsetY, m_maxPanOffsetY);
}

void ZoomPanWidget::calculateMaxPanOffset()
{
    if (!m_videoWidget) {
        m_maxPanOffsetX = 0.0;
        m_maxPanOffsetY = 0.0;
        return;
    }

    QSize imageSize = m_videoWidget->getImageSize();
    if (imageSize.isEmpty()) {
        m_maxPanOffsetX = 0.0;
        m_maxPanOffsetY = 0.0;
        return;
    }

    // 计算缩放后的图像尺寸
    double scaledWidth = imageSize.width() * m_zoomFactor;
    double scaledHeight = imageSize.height() * m_zoomFactor;

    // 计算可平移的最大偏移
    m_maxPanOffsetX = qMax(0.0, (scaledWidth - width()) / 2.0);
    m_maxPanOffsetY = qMax(0.0, (scaledHeight - height()) / 2.0);
}

QPointF ZoomPanWidget::getEffectiveImageOffset() const
{
    if (!m_videoWidget) {
        return QPointF(0.0, 0.0);
    }

    // 获取基础偏移（居中显示）
    QPointF baseOffset = m_videoWidget->getImageOffset();

    // 应用缩放调整
    double baseScale = m_videoWidget->getScaleFactor();
    double scaleRatio = (m_zoomFactor * baseScale) / baseScale;

    // 计算缩放后的偏移调整
    QSize imageSize = m_videoWidget->getImageSize();
    if (!imageSize.isEmpty()) {
        double scaledWidth = imageSize.width() * baseScale * m_zoomFactor;
        double scaledHeight = imageSize.height() * baseScale * m_zoomFactor;

        // 重新计算居中偏移
        double centeredOffsetX = (width() - scaledWidth) / 2.0;
        double centeredOffsetY = (height() - scaledHeight) / 2.0;

        // 应用平移偏移
        return QPointF(centeredOffsetX + m_panOffsetX, centeredOffsetY + m_panOffsetY);
    }

    return QPointF(baseOffset.x() + m_panOffsetX, baseOffset.y() + m_panOffsetY);
}

double ZoomPanWidget::getEffectiveScaleFactor() const
{
    if (!m_videoWidget) {
        return 1.0;
    }

    // 组合基础缩放和用户缩放
    double baseScale = m_videoWidget->getScaleFactor();
    return baseScale * m_zoomFactor;
}

void ZoomPanWidget::performZoom(double newZoomFactor, const QPoint& zoomCenter)
{
    // 限制缩放范围
    newZoomFactor = qBound(m_minZoomFactor, newZoomFactor, m_maxZoomFactor);

    if (qAbs(newZoomFactor - m_zoomFactor) < 0.001) {
        return; // 没有变化
    }

    // 计算缩放前的图像坐标
    QPointF imageCoordBefore = windowToImageCoordinates(zoomCenter);

    // 更新缩放因子
    double oldZoomFactor = m_zoomFactor;
    m_zoomFactor = newZoomFactor;

    // 重新计算最大平移偏移
    calculateMaxPanOffset();

    // 计算缩放后的图像坐标（应该保持不变）
    QPointF imageCoordAfter = windowToImageCoordinates(zoomCenter);

    // 调整平移偏移以保持缩放中心不变
    QPointF deltaImage = imageCoordAfter - imageCoordBefore;
    double effectiveScale = getEffectiveScaleFactor() / m_zoomFactor; // 基础缩放
    m_panOffsetX -= deltaImage.x() * effectiveScale;
    m_panOffsetY -= deltaImage.y() * effectiveScale;

    // 约束平移偏移
    constrainPanOffset();

    // 更新变换
    updateVideoWidgetTransform();
    updatePaintingOverlayTransform();
    updateROIOverlayTransform();

    // 发射信号
    emit viewTransformChanged(m_zoomFactor, getPanOffset());
}

void ZoomPanWidget::performPan(double deltaX, double deltaY)
{
    m_panOffsetX += deltaX;
    m_panOffsetY += deltaY;

    constrainPanOffset();

    updateVideoWidgetTransform();
    updatePaintingOverlayTransform();
    updateROIOverlayTransform();

    emit viewTransformChanged(m_zoomFactor, getPanOffset());
}

void ZoomPanWidget::startKeyboardPan(const QPointF& velocity)
{
    m_keyPanVelocity = velocity;
    m_keyPanActive = true;

    if (!m_keyPanTimer->isActive()) {
        m_keyPanTimer->start();
    }
}

void ZoomPanWidget::stopKeyboardPan()
{
    m_keyPanActive = false;
    m_keyPanVelocity = QPointF(0.0, 0.0);

    if (m_keyPanTimer->isActive()) {
        m_keyPanTimer->stop();
    }
}
