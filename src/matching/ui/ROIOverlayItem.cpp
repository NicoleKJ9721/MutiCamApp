#include "ROIOverlayItem.h"
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QtMath>
#include <QDebug>

ROIOverlayItem::ROIOverlayItem(QGraphicsItem* parent) 
    : QGraphicsItem(parent)
    , state_(Hidden)
    , rotationAngle_(0.0)
    , activeHandle_(NoHandle)
    , hoverHandle_(NoHandle)
    , isDragging_(false)
    , showMask_(true)
    , showHandles_(true)
{
    setZValue(1000);  // 确保在最顶层
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    
    // 初始化样式
    roiBorderPen_ = QPen(Qt::red, 2, Qt::DashLine);
    handlePen_ = QPen(Qt::blue, 1);
    handleBrush_ = QBrush(Qt::white);
    hoverBrush_ = QBrush(Qt::yellow);
    maskColor_ = QColor(0, 0, 0, 100);  // 半透明黑色
}

void ROIOverlayItem::setState(ROIState state) {
    if (state_ == state) return;

    qDebug() << "ROI状态变更：" << state_ << "->" << state;

    state_ = state;
    setVisible(state != Hidden);

    if (state == Editing) {
        showHandles_ = true;
        updateHandlePositions();
        setCursor(Qt::ArrowCursor);
    } else if (state == Locked) {
        showHandles_ = false;
        setCursor(Qt::ArrowCursor);
    }

    update();
    qDebug() << "ROI状态变更完成 - 状态:" << state << "可见性:" << isVisible();
}

void ROIOverlayItem::setROI(const QRectF& rect, qreal angle) {
    roiRect_ = rect;
    rotationAngle_ = angle;
    rotationCenter_ = rect.center();
    
    if (state_ == Editing) {
        updateHandlePositions();
    }
    
    update();
    emit roiChanged(roiRect_, rotationAngle_);
}

void ROIOverlayItem::showDefaultROI(const QSizeF& viewSize) {
    // 创建一个位于视图中心的正方形ROI
    qreal size = qMin(viewSize.width(), viewSize.height()) * 0.3;  // 视图大小的30%
    QPointF center(viewSize.width() / 2.0, viewSize.height() / 2.0);

    roiRect_ = QRectF(center.x() - size/2, center.y() - size/2, size, size);
    rotationAngle_ = 0.0;
    rotationCenter_ = roiRect_.center();

    qDebug() << "设置ROI前 - 视图大小:" << viewSize << "ROI矩形:" << roiRect_;

    setState(Editing);
    updateHandlePositions();

    qDebug() << "显示默认ROI完成 - 状态:" << state_ << "可见性:" << isVisible();
}

QPolygonF ROIOverlayItem::getRotatedPolygon() const {
    if (!roiRect_.isValid()) return QPolygonF();
    
    QPolygonF polygon(roiRect_);
    
    if (qAbs(rotationAngle_) > 0.01) {  // 如果有旋转
        QTransform transform;
        transform.translate(rotationCenter_.x(), rotationCenter_.y());
        transform.rotate(rotationAngle_);
        transform.translate(-rotationCenter_.x(), -rotationCenter_.y());
        polygon = transform.map(polygon);
    }
    
    return polygon;
}

void ROIOverlayItem::setShowMask(bool show) {
    showMask_ = show;
    update();
}

void ROIOverlayItem::setShowHandles(bool show) {
    showHandles_ = show;
    update();
}

QRectF ROIOverlayItem::boundingRect() const {
    if (state_ == Hidden || !roiRect_.isValid()) {
        return QRectF();
    }
    
    // 扩展边界以包含控制点和旋转手柄
    QRectF bounds = roiRect_.adjusted(-20, -50, 20, 20);
    return bounds;
}

void ROIOverlayItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option)
    Q_UNUSED(widget)
    
    if (state_ == Hidden || !roiRect_.isValid()) return;
    
    painter->setRenderHint(QPainter::Antialiasing);
    
    // 1. 绘制半透明遮罩(ROI外区域)
    if (showMask_ && scene()) {
        QRectF sceneRect = scene()->sceneRect();
        if (sceneRect.isEmpty()) {
            // 如果场景矩形为空，使用一个默认大小
            sceneRect = QRectF(-1000, -1000, 2000, 2000);
        }
        
        QPainterPath fullPath;
        fullPath.addRect(sceneRect);
        
        QPainterPath roiPath;
        QPolygonF rotatedROI = getRotatedPolygon();
        roiPath.addPolygon(rotatedROI);
        
        QPainterPath maskPath = fullPath.subtracted(roiPath);
        painter->fillPath(maskPath, maskColor_);
    }
    
    // 2. 绘制ROI边框
    painter->setPen(roiBorderPen_);
    painter->setBrush(Qt::NoBrush);
    
    QPolygonF rotatedROI = getRotatedPolygon();
    painter->drawPolygon(rotatedROI);
    
    // 3. 绘制中心点
    painter->setPen(QPen(Qt::red, 1));
    painter->setBrush(QBrush(Qt::red));
    painter->drawEllipse(rotationCenter_, 2, 2);
    
    // 4. 绘制控制点
    if (showHandles_ && state_ == Editing) {
        for (int i = 0; i < 8; ++i) {  // 8个调整控制点
            if (handleRects_[i].isValid()) {
                QBrush brush = (hoverHandle_ == i) ? hoverBrush_ : handleBrush_;
                painter->setPen(handlePen_);
                painter->setBrush(brush);
                painter->drawEllipse(handleRects_[i]);
            }
        }
        
        // 绘制旋转手柄
        if (handleRects_[RotationHandle].isValid()) {
            QBrush brush = (hoverHandle_ == RotationHandle) ? hoverBrush_ : QBrush(Qt::green);
            painter->setPen(QPen(Qt::green, 2));
            painter->setBrush(brush);
            painter->drawEllipse(handleRects_[RotationHandle]);
            
            // 绘制连接线
            painter->setPen(QPen(Qt::green, 1));
            painter->drawLine(rotationCenter_, handleRects_[RotationHandle].center());
        }
    }
}

QRectF ROIOverlayItem::getHandleRect(const QPointF& center, qreal size) const {
    qreal halfSize = size / 2.0;
    return QRectF(center.x() - halfSize, center.y() - halfSize, size, size);
}

void ROIOverlayItem::updateHandlePositions() {
    if (!roiRect_.isValid()) return;
    
    // 获取旋转后的四个角点
    QPolygonF rotatedROI = getRotatedPolygon();
    if (rotatedROI.size() < 4) return;
    
    // 四个角点
    handleRects_[TopLeft] = getHandleRect(rotatedROI[0]);
    handleRects_[TopRight] = getHandleRect(rotatedROI[1]);
    handleRects_[BottomRight] = getHandleRect(rotatedROI[2]);
    handleRects_[BottomLeft] = getHandleRect(rotatedROI[3]);
    
    // 四个边中点
    handleRects_[TopCenter] = getHandleRect((rotatedROI[0] + rotatedROI[1]) / 2.0);
    handleRects_[RightCenter] = getHandleRect((rotatedROI[1] + rotatedROI[2]) / 2.0);
    handleRects_[BottomCenter] = getHandleRect((rotatedROI[2] + rotatedROI[3]) / 2.0);
    handleRects_[LeftCenter] = getHandleRect((rotatedROI[3] + rotatedROI[0]) / 2.0);
    
    // 旋转手柄（在ROI上方）
    QPointF rotationHandlePos = rotationCenter_ + QPointF(0, -roiRect_.height()/2 - 30);
    handleRects_[RotationHandle] = getHandleRect(rotationHandlePos, 12);
}

ROIOverlayItem::HandleType ROIOverlayItem::getHandleAt(const QPointF& pos) const {
    for (int i = 0; i < 9; ++i) {  // 包括旋转手柄
        if (handleRects_[i].contains(pos)) {
            return static_cast<HandleType>(i);
        }
    }

    // 检查是否在ROI内部（移动手柄）
    QPolygonF rotatedROI = getRotatedPolygon();
    if (rotatedROI.containsPoint(pos, Qt::OddEvenFill)) {
        return MoveHandle;
    }

    return NoHandle;
}

void ROIOverlayItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (state_ != Editing) return;

    QPointF pos = event->pos();
    lastMousePos_ = pos;

    // 检查点击的控制点类型
    activeHandle_ = getHandleAt(pos);
    if (activeHandle_ != NoHandle) {
        isDragging_ = true;
        qDebug() << "开始拖拽控制点:" << activeHandle_;
    }

    update();
}

void ROIOverlayItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if (state_ != Editing) return;

    QPointF pos = event->pos();
    QPointF delta = pos - lastMousePos_;

    if (isDragging_ && activeHandle_ != NoHandle) {
        if (activeHandle_ == RotationHandle) {
            handleRotation(pos);
        } else if (activeHandle_ == MoveHandle) {
            // 移动整个ROI
            roiRect_.translate(delta);
            rotationCenter_ += delta;
            updateHandlePositions();
        } else {
            // 调整ROI大小
            handleResize(activeHandle_, delta);
        }
        update();
        emit roiChanged(roiRect_, rotationAngle_);
    }

    lastMousePos_ = pos;
}

void ROIOverlayItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    Q_UNUSED(event)

    if (isDragging_) {
        isDragging_ = false;
        activeHandle_ = NoHandle;
        qDebug() << "ROI编辑完成:" << roiRect_ << "角度:" << rotationAngle_;
        emit roiEditingFinished();
    }

    update();
}

void ROIOverlayItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event) {
    if (state_ != Editing) return;

    QPointF pos = event->pos();
    HandleType oldHover = hoverHandle_;
    hoverHandle_ = getHandleAt(pos);

    if (hoverHandle_ != oldHover) {
        updateCursor(hoverHandle_);
        update();
    }
}

void ROIOverlayItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    Q_UNUSED(event)

    if (hoverHandle_ != NoHandle) {
        hoverHandle_ = NoHandle;
        setCursor(Qt::ArrowCursor);
        update();
    }
}

void ROIOverlayItem::handleResize(HandleType handle, const QPointF& delta) {
    // 简化的调整大小实现
    QRectF newRect = roiRect_;

    switch (handle) {
    case TopLeft:
        newRect.setTopLeft(newRect.topLeft() + delta);
        break;
    case TopRight:
        newRect.setTopRight(newRect.topRight() + delta);
        break;
    case BottomLeft:
        newRect.setBottomLeft(newRect.bottomLeft() + delta);
        break;
    case BottomRight:
        newRect.setBottomRight(newRect.bottomRight() + delta);
        break;
    case TopCenter:
        newRect.setTop(newRect.top() + delta.y());
        break;
    case BottomCenter:
        newRect.setBottom(newRect.bottom() + delta.y());
        break;
    case LeftCenter:
        newRect.setLeft(newRect.left() + delta.x());
        break;
    case RightCenter:
        newRect.setRight(newRect.right() + delta.x());
        break;
    default:
        return;
    }

    // 确保最小尺寸
    if (newRect.width() > 20 && newRect.height() > 20) {
        roiRect_ = newRect;
        rotationCenter_ = roiRect_.center();
        updateHandlePositions();
    }
}

void ROIOverlayItem::handleRotation(const QPointF& currentPos) {
    // 计算旋转角度
    QPointF centerToMouse = currentPos - rotationCenter_;
    qreal angle = qRadiansToDegrees(qAtan2(centerToMouse.y(), centerToMouse.x())) + 90;

    // 限制角度范围到 -180 到 180
    while (angle > 180) angle -= 360;
    while (angle < -180) angle += 360;

    rotationAngle_ = angle;
    updateHandlePositions();
}

void ROIOverlayItem::updateCursor(HandleType handle) {
    switch (handle) {
    case TopLeft:
    case BottomRight:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case TopRight:
    case BottomLeft:
        setCursor(Qt::SizeBDiagCursor);
        break;
    case TopCenter:
    case BottomCenter:
        setCursor(Qt::SizeVerCursor);
        break;
    case LeftCenter:
    case RightCenter:
        setCursor(Qt::SizeHorCursor);
        break;
    case RotationHandle:
        setCursor(Qt::PointingHandCursor);
        break;
    case MoveHandle:
        setCursor(Qt::SizeAllCursor);
        break;
    default:
        setCursor(Qt::ArrowCursor);
        break;
    }
}
