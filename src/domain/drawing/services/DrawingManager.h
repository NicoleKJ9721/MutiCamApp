#ifndef DRAWING_MANAGER_H
#define DRAWING_MANAGER_H

#include "../entities/DrawingTypes.h"
#include "LayerManager.h"
#include <QObject>
#include <QLabel>
#include <QMouseEvent>
#include <QImage>
#include <memory>
#include <map>

/**
 * 绘图管理器
 * 负责处理用户交互和绘图模式管理
 * 对应Python版本的DrawingManager类
 */
class DrawingManager : public QObject {
    Q_OBJECT

public:
    explicit DrawingManager(QObject* parent = nullptr);
    virtual ~DrawingManager();

    // 设置主窗口引用
    void setMainWindow(QWidget* mainWindow) { m_mainWindow = mainWindow; }

    // 视图设置
    void setupView(QLabel* label, const QString& name);
    
    // 绘图模式控制
    void startPointMeasurement();
    void startLineMeasurement();
    void startCircleMeasurement();
    void startLineSegmentMeasurement();
    void clearDrawings(QLabel* label = nullptr);
    void undoLastDrawing();
    
    // 鼠标事件处理
    void handleMousePress(QMouseEvent* event, QLabel* label);
    void handleMouseMove(QMouseEvent* event, QLabel* label);
    void handleMouseRelease(QMouseEvent* event, QLabel* label);
    void handleMouseDoubleClick(QMouseEvent* event, QLabel* label);
    
    // 鼠标事件处理（带当前帧参数）
    void handleMousePressWithFrame(QMouseEvent* event, QLabel* label, const QImage& currentFrame);
    void handleMouseMoveWithFrame(QMouseEvent* event, QLabel* label, const QImage& currentFrame);
    void handleMouseReleaseWithFrame(QMouseEvent* event, QLabel* label, const QImage& currentFrame);
    
    // 视图管理
    LayerManager* getLayerManager(QLabel* label);
    void updateView(QLabel* label, const QImage& frame);
    
    // 视图配对管理
    void setPairedViews(QLabel* view1, QLabel* view2);
    QLabel* getPairedView(QLabel* view);
    void syncToAllPairedViews(QLabel* sourceView, const QImage& currentFrame);
    
    // 坐标转换
    QPoint convertMouseToImageCoords(const QPoint& mousePos, QLabel* label, const QImage& currentFrame);

signals:
    void viewUpdated(QLabel* label, const QImage& frame);

private:
    // 内部方法
    void setActiveView(QLabel* label);
    void setupMouseTracking(QLabel* label);
    
private:
    std::map<QLabel*, std::unique_ptr<LayerManager>> m_layerManagers;  // 每个视图的图层管理器
    std::map<QLabel*, QLabel*> m_pairedViews;                          // 视图配对关系
    DrawingType m_currentDrawingMode = DrawingType::POINT;             // 当前绘图模式
    QLabel* m_activeView = nullptr;                                    // 当前活动视图
    LayerManager* m_activeLayerManager = nullptr;                      // 当前活动的图层管理器
    bool m_isDrawing = false;                                          // 是否正在绘制
    QWidget* m_mainWindow = nullptr;                                   // 主窗口引用
};

#endif // DRAWING_MANAGER_H 