#ifndef LAYER_MANAGER_H
#define LAYER_MANAGER_H

#include "../entities/DrawingTypes.h"
#include <QObject>
#include <QImage>
#include <memory>
#include <vector>

/**
 * 图层管理器
 * 负责管理绘图对象的存储、渲染和操作
 * 对应Python版本的LayerManager类
 */
class LayerManager : public QObject {
    Q_OBJECT

public:
    explicit LayerManager(QObject* parent = nullptr);
    virtual ~LayerManager();

    // 绘图对象管理
    void addDrawingObject(const DrawingObject& obj);
    void addDetectionObject(const DrawingObject& obj);
    void setCurrentObject(std::shared_ptr<DrawingObject> obj);
    void commitCurrentDrawing();
    
    // 清理操作
    void clear();
    void clearDrawings();
    void clearDetections();
    void clearSelection();
    
    // 撤销操作
    bool undoLastDrawing();
    bool undoLastDetection();
    
    // 选择操作
    std::vector<std::shared_ptr<DrawingObject>> hitTest(const QPoint& point, int tolerance = 10);
    void selectObject(std::shared_ptr<DrawingObject> obj, bool ctrlPressed = false);
    void deleteSelectedObjects();
    
    // 渲染操作
    QImage renderFrame(const QImage& baseFrame);
    void renderObject(QImage& frame, const DrawingObject& obj);
    
    // 访问器
    const std::vector<std::shared_ptr<DrawingObject>>& getDrawingObjects() const { return m_drawingObjects; }
    const std::vector<std::shared_ptr<DrawingObject>>& getDetectionObjects() const { return m_detectionObjects; }
    const std::vector<std::shared_ptr<DrawingObject>>& getSelectedObjects() const { return m_selectedObjects; }
    std::shared_ptr<DrawingObject> getCurrentObject() const { return m_currentObject; }
    
    // 像素比例
    void setPixelScale(double scale) { m_pixelScale = scale; }
    double getPixelScale() const { return m_pixelScale; }

private:
    // 绘制方法
    void drawPoint(QImage& frame, const DrawingObject& obj);
    void drawLine(QImage& frame, const DrawingObject& obj);
    void drawCircle(QImage& frame, const DrawingObject& obj);
    void drawLineSegment(QImage& frame, const DrawingObject& obj);
    
    // 碰撞检测
    bool hitTestPoint(const QPoint& testPoint, const QPoint& point, int tolerance);
    bool hitTestLine(const QPoint& point, const QPoint& lineStart, const QPoint& lineEnd, int tolerance);
    bool hitTestCircle(const QPoint& point, const QPoint& center, const QPoint& radiusPoint, int tolerance);
    
    // 辅助方法
    double calculateFontScale(int imageHeight);
    QColor qColorToOpenCV(const QColor& color);

private:
    std::vector<std::shared_ptr<DrawingObject>> m_drawingObjects;    // 手动绘制对象
    std::vector<std::shared_ptr<DrawingObject>> m_detectionObjects;  // 自动检测对象
    std::vector<std::shared_ptr<DrawingObject>> m_selectedObjects;   // 选中的对象
    std::shared_ptr<DrawingObject> m_currentObject;                  // 当前正在绘制的对象
    double m_pixelScale = 1.0;                                       // 像素比例
};

#endif // LAYER_MANAGER_H 