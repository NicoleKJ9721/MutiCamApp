#pragma once
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <QRectF>
#include <QPointF>
#include <QPen>
#include <QBrush>

class ROIControlButtons : public QObject, public QGraphicsItem {
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
    
public:
    explicit ROIControlButtons(QGraphicsItem* parent = nullptr);
    
    // 设置按钮位置（相对于ROI右下角）
    void setPosition(const QPointF& roiBottomRight);
    
    // 显示/隐藏按钮
    void setVisible(bool visible);
    
    // QGraphicsItem接口
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    
private:
    QRectF confirmButtonRect_;  // √按钮区域
    QRectF cancelButtonRect_;   // ×按钮区域
    bool confirmHovered_;
    bool cancelHovered_;
    bool isVisible_;
    
    void drawConfirmButton(QPainter* painter);
    void drawCancelButton(QPainter* painter);
    
signals:
    void confirmClicked();
    void cancelClicked();
};
