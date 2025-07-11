#include "DrawingManager.h"
#include <QDebug>
#include <QApplication>
#include <QCursor>

// 前向声明
class MutiCamApp;

DrawingManager::DrawingManager(QObject* parent)
    : QObject(parent) {
    qDebug() << "DrawingManager 初始化完成";
}

DrawingManager::~DrawingManager() {
    qDebug() << "DrawingManager 析构";
}

void DrawingManager::setupView(QLabel* label, const QString& name) {
    if (!label) {
        qWarning() << "设置视图失败：label为空";
        return;
    }
    
    // 创建图层管理器
    auto layerManager = std::make_unique<LayerManager>(this);
    m_layerManagers[label] = std::move(layerManager);
    
    // 设置鼠标追踪
    setupMouseTracking(label);
    
    qDebug() << "设置视图完成:" << name;
}

void DrawingManager::startPointMeasurement() {
    qDebug() << "启动点测量模式";
    m_currentDrawingMode = DrawingType::POINT;
    m_isDrawing = false;
    
    // 设置所有视图的鼠标光标为十字
    for (auto& pair : m_layerManagers) {
        pair.first->setCursor(Qt::CrossCursor);
    }
}

void DrawingManager::startLineMeasurement() {
    qDebug() << "启动直线测量模式";
    m_currentDrawingMode = DrawingType::LINE;
    m_isDrawing = false;
    
    for (auto& pair : m_layerManagers) {
        pair.first->setCursor(Qt::CrossCursor);
    }
}

void DrawingManager::startCircleMeasurement() {
    qDebug() << "启动圆形测量模式";
    m_currentDrawingMode = DrawingType::CIRCLE;
    m_isDrawing = false;
    
    for (auto& pair : m_layerManagers) {
        pair.first->setCursor(Qt::CrossCursor);
    }
}

void DrawingManager::startLineSegmentMeasurement() {
    qDebug() << "启动线段测量模式";
    m_currentDrawingMode = DrawingType::LINE_SEGMENT;
    m_isDrawing = false;
    
    for (auto& pair : m_layerManagers) {
        pair.first->setCursor(Qt::CrossCursor);
    }
}

void DrawingManager::clearDrawings(QLabel* label) {
    if (label) {
        // 清空特定视图
        auto it = m_layerManagers.find(label);
        if (it != m_layerManagers.end()) {
            it->second->clearDrawings();
            qDebug() << "清空特定视图的绘制";
        }
    } else {
        // 清空所有视图
        for (auto& pair : m_layerManagers) {
            pair.second->clearDrawings();
        }
        qDebug() << "清空所有视图的绘制";
    }
}

void DrawingManager::undoLastDrawing() {
    if (m_activeLayerManager) {
        if (m_activeLayerManager->undoLastDrawing()) {
            qDebug() << "撤销上一次绘制";
        }
    }
}

void DrawingManager::handleMousePress(QMouseEvent* event, QLabel* label) {
    // 这个方法现在只是一个包装器，实际逻辑在带Frame的方法中
    QImage emptyFrame; // 空帧，将在MutiCamApp中被正确的帧替换
    handleMousePressWithFrame(event, label, emptyFrame);
}

void DrawingManager::handleMousePressWithFrame(QMouseEvent* event, QLabel* label, const QImage& currentFrame) {
    if (!event || !label) {
        return;
    }
    
    setActiveView(label);
    
    // 检查是否是右键点击
    if (event->button() == Qt::RightButton) {
        // 如果正在绘制，退出绘制状态
        if (m_isDrawing) {
            m_isDrawing = false;
            m_activeLayerManager->setCurrentObject(nullptr);
            qDebug() << "右键退出绘制状态";
        }
        return;
    }
    
    // 只处理左键点击的绘制操作
    if (event->button() == Qt::LeftButton && m_activeLayerManager) {
        // 使用传入的当前帧，如果为空则创建测试图像
        QImage frameToUse = currentFrame;
        if (frameToUse.isNull()) {
            frameToUse = QImage(640, 480, QImage::Format_RGB888);
            frameToUse.fill(QColor(50, 50, 50)); // 深灰色背景
            qDebug() << "使用测试图像，尺寸:" << frameToUse.size();
        } else {
            qDebug() << "使用真实图像，尺寸:" << frameToUse.size();
        }
        
        QPoint imagePos = convertMouseToImageCoords(event->pos(), label, frameToUse);
        
        if (m_currentDrawingMode == DrawingType::POINT) {
            // 点绘制模式 - 立即创建并提交点
            DrawingProperties props;
            props.color = QColor(0, 255, 0);  // 绿色
            props.radius = 10;                // 半径10像素
            
            DrawingObject pointObj(DrawingType::POINT);
            pointObj.points.push_back(imagePos);
            pointObj.properties = props;
            
            m_activeLayerManager->addDrawingObject(pointObj);
            
            qDebug() << "创建点:" << imagePos << "在图像" << frameToUse.size();
            
            // 通知视图更新（不直接更新，让主窗口处理）
            emit viewUpdated(label, frameToUse);
        }
    }
}

void DrawingManager::handleMouseMove(QMouseEvent* event, QLabel* label) {
    QImage emptyFrame;
    handleMouseMoveWithFrame(event, label, emptyFrame);
}

void DrawingManager::handleMouseMoveWithFrame(QMouseEvent* event, QLabel* label, const QImage& currentFrame) {
    Q_UNUSED(event)
    Q_UNUSED(label)
    Q_UNUSED(currentFrame)
    // 鼠标移动处理，后续根据需要实现
}

void DrawingManager::handleMouseRelease(QMouseEvent* event, QLabel* label) {
    QImage emptyFrame;
    handleMouseReleaseWithFrame(event, label, emptyFrame);
}

void DrawingManager::handleMouseReleaseWithFrame(QMouseEvent* event, QLabel* label, const QImage& currentFrame) {
    Q_UNUSED(event)
    Q_UNUSED(label)
    Q_UNUSED(currentFrame)
    // 鼠标释放处理，后续根据需要实现
}

void DrawingManager::handleMouseDoubleClick(QMouseEvent* event, QLabel* label) {
    Q_UNUSED(event)
    Q_UNUSED(label)
    // 双击处理，后续根据需要实现
}

LayerManager* DrawingManager::getLayerManager(QLabel* label) {
    auto it = m_layerManagers.find(label);
    if (it != m_layerManagers.end()) {
        return it->second.get();
    }
    return nullptr;
}

void DrawingManager::updateView(QLabel* label, const QImage& frame) {
    if (!label || frame.isNull()) {
        return;
    }
    
    // 将QImage转换为QPixmap并设置到QLabel
    QPixmap pixmap = QPixmap::fromImage(frame);
    label->setPixmap(pixmap);
    
    emit viewUpdated(label, frame);
}

QPoint DrawingManager::convertMouseToImageCoords(const QPoint& mousePos, QLabel* label, const QImage& currentFrame) {
    if (!label || currentFrame.isNull()) {
        return mousePos;
    }
    
    int imgWidth = currentFrame.width();
    int imgHeight = currentFrame.height();
    int labelWidth = label->width();
    int labelHeight = label->height();
    
    // 计算缩放比例（保持宽高比）
    double ratio = std::min(static_cast<double>(labelWidth) / imgWidth, 
                           static_cast<double>(labelHeight) / imgHeight);
    
    int displayWidth = static_cast<int>(imgWidth * ratio);
    int displayHeight = static_cast<int>(imgHeight * ratio);
    
    // 计算偏移量（居中显示）
    int xOffset = (labelWidth - displayWidth) / 2;
    int yOffset = (labelHeight - displayHeight) / 2;
    
    // 转换坐标
    int imageX = static_cast<int>((mousePos.x() - xOffset) / ratio);
    int imageY = static_cast<int>((mousePos.y() - yOffset) / ratio);
    
    // 确保坐标在图像范围内
    imageX = std::max(0, std::min(imageX, imgWidth - 1));
    imageY = std::max(0, std::min(imageY, imgHeight - 1));
    
    return QPoint(imageX, imageY);
}

void DrawingManager::setActiveView(QLabel* label) {
    m_activeView = label;
    m_activeLayerManager = getLayerManager(label);
}

void DrawingManager::setupMouseTracking(QLabel* label) {
    if (!label) {
        return;
    }
    
    // 启用鼠标追踪
    label->setMouseTracking(true);
    
    // 安装事件过滤器（如果需要的话）
    // label->installEventFilter(this);
}

void DrawingManager::setPairedViews(QLabel* view1, QLabel* view2) {
    if (view1 && view2) {
        m_pairedViews[view1] = view2;
        m_pairedViews[view2] = view1;
        qDebug() << "设置视图配对:" << view1->objectName() << "<->" << view2->objectName();
    }
}

QLabel* DrawingManager::getPairedView(QLabel* view) {
    auto it = m_pairedViews.find(view);
    if (it != m_pairedViews.end()) {
        return it->second;
    }
    return nullptr;
}

void DrawingManager::syncToAllPairedViews(QLabel* sourceView, const QImage& currentFrame) {
    if (!sourceView) {
        return;
    }
    
    // 获取源视图的图层管理器
    LayerManager* sourceLayerManager = getLayerManager(sourceView);
    if (!sourceLayerManager) {
        return;
    }
    
    // 获取配对视图
    QLabel* pairedView = getPairedView(sourceView);
    if (!pairedView) {
        return;
    }
    
    // 获取配对视图的图层管理器
    LayerManager* pairedLayerManager = getLayerManager(pairedView);
    if (!pairedLayerManager) {
        return;
    }
    
    // 同步绘制对象（复制源视图的绘制对象到配对视图）
    const auto& sourceDrawingObjects = sourceLayerManager->getDrawingObjects();
    
    // 清空配对视图的绘制对象
    pairedLayerManager->clearDrawings();
    
    // 复制所有绘制对象
    for (const auto& obj : sourceDrawingObjects) {
        pairedLayerManager->addDrawingObject(*obj);
    }
    
    qDebug() << "同步绘制对象:" << sourceView->objectName() << "->" << pairedView->objectName() 
             << "，对象数量:" << sourceDrawingObjects.size();
} 