#pragma once
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <QRectF>
#include <QPointF>
#include <QPen>
#include <QBrush>
#include <QCursor>
#include <array>

class ROIOverlayItem : public QObject, public QGraphicsItem {
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
    
public:
    enum HandleType {
        NoHandle = -1,
        TopLeft, TopRight, BottomLeft, BottomRight,        // 角点
        TopCenter, BottomCenter, LeftCenter, RightCenter,  // 边中点
        RotationHandle,                                    // 旋转手柄
        MoveHandle                                         // 移动手柄(中心)
    };
    
    enum ROIState {
        Hidden,         // 隐藏状态
        Editing,        // 编辑中
        Locked          // 锁定状态
    };
    
    explicit ROIOverlayItem(QGraphicsItem* parent = nullptr);
    
    // 状态控制
    void setState(ROIState state);
    ROIState getState() const { return state_; }
    
    // ROI操作
    void setROI(const QRectF& rect, qreal angle = 0);
    QRectF getROI() const { return roiRect_; }
    qreal getRotation() const { return rotationAngle_; }
    QPolygonF getRotatedPolygon() const;
    
    // 显示默认ROI（正方形，位于中心）
    void showDefaultROI(const QSizeF& viewSize);
    
    // 显示控制
    void setShowMask(bool show);
    void setShowHandles(bool show);
    
    // QGraphicsItem接口
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    
private:
    // 状态变量
    ROIState state_;
    QRectF roiRect_;
    qreal rotationAngle_;
    QPointF rotationCenter_;
    
    // 控制点
    std::array<QRectF, 9> handleRects_;  // 8个调整点+1个旋转点
    HandleType activeHandle_;
    HandleType hoverHandle_;
    
    // 样式配置
    QPen roiBorderPen_;
    QPen handlePen_;
    QBrush handleBrush_;
    QBrush hoverBrush_;
    QColor maskColor_;
    
    // 交互状态
    QPointF lastMousePos_;
    bool isDragging_;
    bool showMask_;
    bool showHandles_;
    
    // 私有方法
    void updateHandlePositions();
    HandleType getHandleAt(const QPointF& pos) const;
    void handleResize(HandleType handle, const QPointF& delta);
    void handleRotation(const QPointF& currentPos);
    void updateCursor(HandleType handle);
    QRectF getHandleRect(const QPointF& center, qreal size = 8.0) const;
    
signals:
    void roiChanged(const QRectF& rect, qreal angle);
    void roiEditingFinished();
};
