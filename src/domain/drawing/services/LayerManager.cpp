#include "LayerManager.h"
#include <QPainter>
#include <QFont>
#include <QDebug>
#include <cmath>

LayerManager::LayerManager(QObject* parent)
    : QObject(parent) {
    qDebug() << "LayerManager 初始化完成";
}

LayerManager::~LayerManager() {
    clear();
}

void LayerManager::addDrawingObject(const DrawingObject& obj) {
    auto sharedObj = std::make_shared<DrawingObject>(obj);
    m_drawingObjects.push_back(sharedObj);
    qDebug() << "添加绘制对象:" << QString::fromStdString(getDrawingTypeName(obj.type));
}

void LayerManager::addDetectionObject(const DrawingObject& obj) {
    auto sharedObj = std::make_shared<DrawingObject>(obj);
    m_detectionObjects.push_back(sharedObj);
    qDebug() << "添加检测对象:" << QString::fromStdString(getDrawingTypeName(obj.type));
}

void LayerManager::setCurrentObject(std::shared_ptr<DrawingObject> obj) {
    m_currentObject = obj;
}

void LayerManager::commitCurrentDrawing() {
    if (m_currentObject) {
        if (m_currentObject->properties.isDetection) {
            addDetectionObject(*m_currentObject);
        } else {
            addDrawingObject(*m_currentObject);
        }
        m_currentObject.reset();
        qDebug() << "提交当前绘制对象";
    }
}

void LayerManager::clear() {
    m_drawingObjects.clear();
    m_detectionObjects.clear();
    m_selectedObjects.clear();
    m_currentObject.reset();
}

void LayerManager::clearDrawings() {
    // 只保留检测对象
    auto newDrawings = std::vector<std::shared_ptr<DrawingObject>>();
    for (const auto& obj : m_drawingObjects) {
        if (obj->properties.isDetection) {
            newDrawings.push_back(obj);
        }
    }
    m_drawingObjects = newDrawings;
}

void LayerManager::clearDetections() {
    m_detectionObjects.clear();
}

void LayerManager::clearSelection() {
    for (auto& obj : m_selectedObjects) {
        obj->properties.selected = false;
    }
    m_selectedObjects.clear();
}

bool LayerManager::undoLastDrawing() {
    // 从后向前查找第一个非检测对象
    for (auto it = m_drawingObjects.rbegin(); it != m_drawingObjects.rend(); ++it) {
        if (!(*it)->properties.isDetection) {
            m_drawingObjects.erase(std::next(it).base());
            return true;
        }
    }
    return false;
}

bool LayerManager::undoLastDetection() {
    // 从后向前查找第一个检测对象
    for (auto it = m_drawingObjects.rbegin(); it != m_drawingObjects.rend(); ++it) {
        if ((*it)->properties.isDetection) {
            m_drawingObjects.erase(std::next(it).base());
            return true;
        }
    }
    return false;
}

QImage LayerManager::renderFrame(const QImage& baseFrame) {
    if (baseFrame.isNull()) {
        return QImage();
    }
    
    // 创建副本进行绘制
    QImage resultFrame = baseFrame.copy();
    
    // 渲染所有绘制对象
    for (const auto& obj : m_drawingObjects) {
        if (obj->properties.visible) {
            renderObject(resultFrame, *obj);
        }
    }
    
    // 渲染所有检测对象
    for (const auto& obj : m_detectionObjects) {
        if (obj->properties.visible) {
            renderObject(resultFrame, *obj);
        }
    }
    
    // 渲染当前正在绘制的对象
    if (m_currentObject && m_currentObject->properties.visible) {
        renderObject(resultFrame, *m_currentObject);
    }
    
    return resultFrame;
}

void LayerManager::renderObject(QImage& frame, const DrawingObject& obj) {
    switch (obj.type) {
        case DrawingType::POINT:
            drawPoint(frame, obj);
            break;
        case DrawingType::LINE:
            drawLine(frame, obj);
            break;
        case DrawingType::CIRCLE:
            drawCircle(frame, obj);
            break;
        case DrawingType::LINE_SEGMENT:
            drawLineSegment(frame, obj);
            break;
        default:
            qDebug() << "未实现的绘制类型:" << QString::fromStdString(getDrawingTypeName(obj.type));
            break;
    }
}

void LayerManager::drawPoint(QImage& frame, const DrawingObject& obj) {
    if (obj.points.empty()) {
        return;
    }
    
    QPainter painter(&frame);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    const QPoint& point = obj.points[0];
    const auto& props = obj.properties;
    
    // 如果被选中，先绘制高亮效果
    if (props.selected) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 255));  // 蓝色高亮
        painter.drawEllipse(point, props.radius + 8, props.radius + 8);
    }
    
    // 绘制点（实心圆）
    painter.setPen(Qt::NoPen);
    painter.setBrush(props.color);
    painter.drawEllipse(point, props.radius, props.radius);
    
    // 显示坐标信息
    QString coordText = QString("(%1, %2)").arg(point.x()).arg(point.y());
    
    // 设置字体
    QFont font = painter.font();
    double fontScale = calculateFontScale(frame.height());
    font.setPointSizeF(fontScale * 24);  // 基础字体大小从12pt增加到24pt
    painter.setFont(font);
    
    // 计算文本位置（移动到点的右上方）
    int textX = point.x() + 15;
    int textY = point.y() - 15;
    
    // 获取文本矩形
    QFontMetrics fm(font);
    QRect textRect = fm.boundingRect(coordText);
    textRect.moveTo(textX, textY - textRect.height());
    
    // 绘制文本背景
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 180));  // 半透明黑色背景
    painter.drawRect(textRect.adjusted(-8, -4, 8, 4));
    
    // 绘制文本
    painter.setPen(props.color);
    painter.setBrush(Qt::NoBrush);
    painter.drawText(textX, textY, coordText);
}

void LayerManager::drawLine(QImage& frame, const DrawingObject& obj) {
    // 暂时留空，后续实现
    Q_UNUSED(frame)
    Q_UNUSED(obj)
}

void LayerManager::drawCircle(QImage& frame, const DrawingObject& obj) {
    // 暂时留空，后续实现
    Q_UNUSED(frame)
    Q_UNUSED(obj)
}

void LayerManager::drawLineSegment(QImage& frame, const DrawingObject& obj) {
    // 暂时留空，后续实现
    Q_UNUSED(frame)
    Q_UNUSED(obj)
}

std::vector<std::shared_ptr<DrawingObject>> LayerManager::hitTest(const QPoint& point, int tolerance) {
    std::vector<std::shared_ptr<DrawingObject>> hitObjects;
    
    // 检测绘制对象
    for (const auto& obj : m_drawingObjects) {
        switch (obj->type) {
            case DrawingType::POINT:
                if (!obj->points.empty() && hitTestPoint(point, obj->points[0], tolerance)) {
                    hitObjects.push_back(obj);
                }
                break;
            default:
                // 其他类型的碰撞检测后续实现
                break;
        }
    }
    
    return hitObjects;
}

void LayerManager::selectObject(std::shared_ptr<DrawingObject> obj, bool ctrlPressed) {
    if (!ctrlPressed) {
        clearSelection();
    }
    
    obj->properties.selected = true;
    
    // 检查是否已在选择列表中
    auto it = std::find(m_selectedObjects.begin(), m_selectedObjects.end(), obj);
    if (it == m_selectedObjects.end()) {
        m_selectedObjects.push_back(obj);
    }
}

void LayerManager::deleteSelectedObjects() {
    for (const auto& selectedObj : m_selectedObjects) {
        // 从绘制对象列表中移除
        auto it = std::find(m_drawingObjects.begin(), m_drawingObjects.end(), selectedObj);
        if (it != m_drawingObjects.end()) {
            m_drawingObjects.erase(it);
        }
        
        // 从检测对象列表中移除
        auto it2 = std::find(m_detectionObjects.begin(), m_detectionObjects.end(), selectedObj);
        if (it2 != m_detectionObjects.end()) {
            m_detectionObjects.erase(it2);
        }
    }
    
    m_selectedObjects.clear();
}

bool LayerManager::hitTestPoint(const QPoint& testPoint, const QPoint& point, int tolerance) {
    int dx = testPoint.x() - point.x();
    int dy = testPoint.y() - point.y();
    return (dx * dx + dy * dy) <= (tolerance * tolerance);
}

bool LayerManager::hitTestLine(const QPoint& point, const QPoint& lineStart, const QPoint& lineEnd, int tolerance) {
    // 点到线段的距离计算，后续实现
    Q_UNUSED(point)
    Q_UNUSED(lineStart)
    Q_UNUSED(lineEnd)
    Q_UNUSED(tolerance)
    return false;
}

bool LayerManager::hitTestCircle(const QPoint& point, const QPoint& center, const QPoint& radiusPoint, int tolerance) {
    // 点到圆的距离计算，后续实现
    Q_UNUSED(point)
    Q_UNUSED(center)
    Q_UNUSED(radiusPoint)
    Q_UNUSED(tolerance)
    return false;
}

double LayerManager::calculateFontScale(int imageHeight) {
    // Python版本算法：font_scale = max(0.9, height / 70 / 30)
    // 转换为C++：适应高分辨率图像
    double fontScale = std::max(0.9, static_cast<double>(imageHeight) / 70.0 / 30.0);
    
    // 对于高分辨率图像，进一步增大字体
    if (imageHeight > 1000) {
        fontScale *= 1.8;  // 高分辨率图像字体增大80%
    } else if (imageHeight > 500) {
        fontScale *= 1.4;  // 中等分辨率图像字体增大40%
    }
    
    return fontScale;
}

QColor LayerManager::qColorToOpenCV(const QColor& color) {
    // Qt使用ARGB，OpenCV使用BGR
    return QColor(color.blue(), color.green(), color.red(), color.alpha());
} 