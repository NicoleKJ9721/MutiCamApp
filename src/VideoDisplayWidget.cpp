#include "VideoDisplayWidget.h"
#include "PaintingOverlay.h"
#include <QPainter>
#include <QDebug>

VideoDisplayWidget::VideoDisplayWidget(QWidget *parent)
    : QLabel(parent)
    , m_hasExternalTransform(false)
    , m_externalOffset(0.0, 0.0)
    , m_externalScaleFactor(1.0)
{
    // 高DPI显示优化设置
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setAutoFillBackground(false);
    setScaledContents(false);
}

void VideoDisplayWidget::setVideoFrame(const QPixmap& pixmap)
{
    m_videoFrame = pixmap;
    update(); // 直接触发重绘
}

void VideoDisplayWidget::setExternalTransform(const QPointF& offset, double scaleFactor)
{
    m_hasExternalTransform = true;
    m_externalOffset = offset;
    m_externalScaleFactor = scaleFactor;
    update(); // 触发重绘
}

void VideoDisplayWidget::resetExternalTransform()
{
    m_hasExternalTransform = false;
    m_externalOffset = QPointF(0.0, 0.0);
    m_externalScaleFactor = 1.0;
    update(); // 触发重绘
}

void VideoDisplayWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    if (m_videoFrame.isNull()) {
        return;
    }

    // 使用外部变换参数（如果有的话）
    QPointF offset;
    double scale;

    if (m_hasExternalTransform) {
        offset = m_externalOffset;
        scale = m_externalScaleFactor;
    } else {
        // 使用内部计算的偏移和缩放
        offset = getImageOffset();
        scale = getScaleFactor();
    }

    QRectF targetRect(offset.x(), offset.y(),
                     m_videoFrame.width() * scale,
                     m_videoFrame.height() * scale);

    // 使用正确的 drawPixmap 重载版本
    painter.drawPixmap(targetRect, m_videoFrame, m_videoFrame.rect());
}

QPointF VideoDisplayWidget::getImageOffset() const
{
    if (m_videoFrame.isNull()) {
        return QPointF(0, 0);
    }
    
    double scale = getScaleFactor();
    double scaledWidth = m_videoFrame.width() * scale;
    double scaledHeight = m_videoFrame.height() * scale;
    
    double offsetX = (width() - scaledWidth) / 2.0;
    double offsetY = (height() - scaledHeight) / 2.0;
    
    return QPointF(offsetX, offsetY);
}

double VideoDisplayWidget::getScaleFactor() const
{
    if (m_videoFrame.isNull()) {
        return 1.0;
    }
    
    double scaleX = static_cast<double>(width()) / m_videoFrame.width();
    double scaleY = static_cast<double>(height()) / m_videoFrame.height();
    
    return qMin(scaleX, scaleY);
}

QSize VideoDisplayWidget::getImageSize() const
{
    if (m_videoFrame.isNull()) {
        return QSize(0, 0);
    }
    
    return m_videoFrame.size();
}