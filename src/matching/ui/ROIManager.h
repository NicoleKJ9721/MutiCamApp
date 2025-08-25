#pragma once
#include <QObject>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QWidget>
#include <opencv2/opencv.hpp>
#include "ROIOverlayItem.h"
#include "ROIControlButtons.h"

class PaintingOverlay;

/**
 * @brief ROI管理器，基于PaintingOverlay实现ROI功能
 * 
 * 这个类在PaintingOverlay上添加ROI功能，
 * 使用QGraphicsView/QGraphicsScene架构
 */
class ROIManager : public QObject {
    Q_OBJECT
    
public:
    explicit ROIManager(PaintingOverlay* paintingOverlay, QObject* parent = nullptr);
    ~ROIManager();
    
    // ROI操作
    void startROICreation();
    void cancelROICreation();
    void finishROICreation();
    
    // ROI状态查询
    bool hasROI() const;
    bool isEditing() const;
    
    // ROI数据获取
    QRectF getROI() const;
    qreal getROIRotation() const;
    QPolygonF getROIPolygon() const;
    cv::Rect getROIRect() const;  // 转换为OpenCV格式
    
    // 显示控制
    void setROIVisible(bool visible);
    void setROIMaskVisible(bool visible);
    
private slots:
    void onROIChanged(const QRectF& rect, qreal angle);
    void onROIEditingFinished();
    void onConfirmClicked();
    void onCancelClicked();
    void onPaintingOverlayResized();
    
private:
    // 核心组件
    PaintingOverlay* paintingOverlay_;
    QGraphicsView* overlayView_;
    QGraphicsScene* overlayScene_;
    ROIOverlayItem* roiItem_;
    ROIControlButtons* controlButtons_;
    
    // 状态管理
    bool isInitialized_;
    
    // 私有方法
    void initializeOverlay();
    void updateOverlayGeometry();
    void updateControlButtonsPosition();
    
signals:
    void roiCreated(const QRectF& rect, qreal angle);
    void roiChanged(const QRectF& rect, qreal angle);
    void roiFinished();
    void roiCancelled();
};
