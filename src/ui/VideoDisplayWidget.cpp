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
    m_sourceImageSize = pixmap.size();
    m_hasSourceImageSize = !m_sourceImageSize.isEmpty();
    m_sourceImageRect = QRect();
    m_hasSourceImageRect = false;
    update(); // 直接触发重绘
}

void VideoDisplayWidget::setVideoFrame(const QPixmap& pixmap, const QSize& sourceImageSize)
{
    m_videoFrame = pixmap;
    m_sourceImageSize = sourceImageSize;
    m_hasSourceImageSize = !m_sourceImageSize.isEmpty();
    m_sourceImageRect = QRect();
    m_hasSourceImageRect = false;
    update(); // 直接触发重绘
}

void VideoDisplayWidget::setVideoFrame(const QPixmap& pixmap, const QSize& sourceImageSize, const QRect& sourceImageRect)
{
    m_videoFrame = pixmap;
    m_sourceImageSize = sourceImageSize;
    m_hasSourceImageSize = !m_sourceImageSize.isEmpty();
    m_sourceImageRect = sourceImageRect;
    m_hasSourceImageRect = !m_sourceImageRect.isNull() && !m_sourceImageRect.isEmpty();
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
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
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

    const QSize logicalSize = (m_hasSourceImageSize && !m_sourceImageSize.isEmpty())
        ? m_sourceImageSize
        : m_videoFrame.size();

    const QRect sourceRect = (m_hasSourceImageRect && !m_sourceImageRect.isEmpty())
        ? m_sourceImageRect
        : QRect(QPoint(0, 0), logicalSize);

    QRectF targetRect(offset.x() + sourceRect.x() * scale,
                      offset.y() + sourceRect.y() * scale,
                      sourceRect.width() * scale,
                      sourceRect.height() * scale);

    // 使用正确的 drawPixmap 重载版本
    painter.drawPixmap(targetRect, m_videoFrame, m_videoFrame.rect());
}

QPointF VideoDisplayWidget::getImageOffset() const
{
    if (m_videoFrame.isNull()) {
        return QPointF(0, 0);
    }
    
    double scale = getScaleFactor();
    const QSize logicalSize = (m_hasSourceImageSize && !m_sourceImageSize.isEmpty())
        ? m_sourceImageSize
        : m_videoFrame.size();
    double scaledWidth = logicalSize.width() * scale;
    double scaledHeight = logicalSize.height() * scale;
    
    double offsetX = (width() - scaledWidth) / 2.0;
    double offsetY = (height() - scaledHeight) / 2.0;
    
    return QPointF(offsetX, offsetY);
}

double VideoDisplayWidget::getScaleFactor() const
{
    if (m_videoFrame.isNull()) {
        return 1.0;
    }
    
    const QSize logicalSize = (m_hasSourceImageSize && !m_sourceImageSize.isEmpty())
        ? m_sourceImageSize
        : m_videoFrame.size();

    if (logicalSize.isEmpty()) {
        return 1.0;
    }

    double scaleX = static_cast<double>(width()) / logicalSize.width();
    double scaleY = static_cast<double>(height()) / logicalSize.height();
    
    return qMin(scaleX, scaleY);
}

QSize VideoDisplayWidget::getImageSize() const
{
    if (m_videoFrame.isNull()) {
        return QSize(0, 0);
    }

    if (m_hasSourceImageSize && !m_sourceImageSize.isEmpty()) {
        return m_sourceImageSize;
    }

    return m_videoFrame.size();
}
