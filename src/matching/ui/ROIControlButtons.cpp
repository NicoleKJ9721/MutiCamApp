#include "ROIControlButtons.h"
#include <QDebug>

ROIControlButtons::ROIControlButtons(QGraphicsItem* parent)
    : QGraphicsItem(parent)
    , confirmHovered_(false)
    , cancelHovered_(false)
    , isVisible_(false)
{
    setZValue(1001); // 确保在ROI之上
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
}

void ROIControlButtons::setPosition(const QPointF& roiBottomRight) {
    // 在ROI右下角偏移位置显示按钮
    QPointF offset(10, 10);
    QPointF buttonPos = roiBottomRight + offset;
    
    confirmButtonRect_ = QRectF(buttonPos, QSizeF(30, 30));
    cancelButtonRect_ = QRectF(buttonPos + QPointF(40, 0), QSizeF(30, 30));
    
    update();
}

void ROIControlButtons::setVisible(bool visible) {
    isVisible_ = visible;
    QGraphicsItem::setVisible(visible);
    update();
}

QRectF ROIControlButtons::boundingRect() const {
    if (!isVisible_) return QRectF();
    return confirmButtonRect_.united(cancelButtonRect_).adjusted(-5, -5, 5, 5);
}

void ROIControlButtons::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option)
    Q_UNUSED(widget)
    
    if (!isVisible_) return;
    
    painter->setRenderHint(QPainter::Antialiasing);
    
    drawConfirmButton(painter);
    drawCancelButton(painter);
}

void ROIControlButtons::drawConfirmButton(QPainter* painter) {
    // 绘制确认按钮(√)
    QColor confirmColor = confirmHovered_ ? QColor(0, 200, 0) : QColor(0, 150, 0);
    painter->setBrush(confirmColor);
    painter->setPen(QPen(Qt::white, 2));
    painter->drawEllipse(confirmButtonRect_);
    
    // 绘制√符号
    QPen checkPen(Qt::white, 3);
    painter->setPen(checkPen);
    QPointF center = confirmButtonRect_.center();
    painter->drawLine(center + QPointF(-8, 0), center + QPointF(-3, 5));
    painter->drawLine(center + QPointF(-3, 5), center + QPointF(8, -5));
}

void ROIControlButtons::drawCancelButton(QPainter* painter) {
    // 绘制取消按钮(×)
    QColor cancelColor = cancelHovered_ ? QColor(200, 0, 0) : QColor(150, 0, 0);
    painter->setBrush(cancelColor);
    painter->setPen(QPen(Qt::white, 2));
    painter->drawEllipse(cancelButtonRect_);
    
    // 绘制×符号
    QPen crossPen(Qt::white, 3);
    painter->setPen(crossPen);
    QPointF cancelCenter = cancelButtonRect_.center();
    painter->drawLine(cancelCenter + QPointF(-6, -6), cancelCenter + QPointF(6, 6));
    painter->drawLine(cancelCenter + QPointF(-6, 6), cancelCenter + QPointF(6, -6));
}

void ROIControlButtons::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    QPointF pos = event->pos();
    if (confirmButtonRect_.contains(pos)) {
        qDebug() << "ROI确认按钮被点击";
        emit confirmClicked();
    } else if (cancelButtonRect_.contains(pos)) {
        qDebug() << "ROI取消按钮被点击";
        emit cancelClicked();
    }
}

void ROIControlButtons::hoverMoveEvent(QGraphicsSceneHoverEvent* event) {
    QPointF pos = event->pos();
    bool oldConfirmHover = confirmHovered_;
    bool oldCancelHover = cancelHovered_;
    
    confirmHovered_ = confirmButtonRect_.contains(pos);
    cancelHovered_ = cancelButtonRect_.contains(pos);
    
    if (confirmHovered_ != oldConfirmHover || cancelHovered_ != oldCancelHover) {
        update();
    }
}

void ROIControlButtons::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    Q_UNUSED(event)
    
    if (confirmHovered_ || cancelHovered_) {
        confirmHovered_ = false;
        cancelHovered_ = false;
        update();
    }
}
