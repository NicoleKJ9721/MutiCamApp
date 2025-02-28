#include "drawingmanager.h"
#include <QMouseEvent>
#include <QDebug>

// 初始化静态成员变量
int DrawingObject::nextId = 0;

// DrawingObject 实现
DrawingObject::DrawingObject(DrawingType type) 
    : type(type), selected(false), visible(true)
{
    id = nextId++;
}

// LayerManager 实现
LayerManager::LayerManager(QObject *parent) 
    : QObject(parent), currentObject(nullptr)
{
}

void LayerManager::startDrawing(DrawingType type, const QMap<QString, QVariant>& properties)
{
    // 创建新的绘制对象
    currentObject = new DrawingObject(type);
    currentObject->properties = properties;
}

void LayerManager::updateCurrentObject(const QPoint& point)
{
    if (currentObject) {
        // 根据不同类型的绘制对象，更新点
        if (currentObject->type == DrawingType::POINT) {
            if (currentObject->points.isEmpty()) {
                currentObject->points.append(point);
            } else {
                currentObject->points[0] = point;
            }
        } else {
            // 对于其他类型，添加或更新点
            if (currentObject->points.size() <= 1) {
                currentObject->points.append(point);
            } else {
                currentObject->points.last() = point;
            }
        }
    }
}

void LayerManager::commitDrawing()
{
    if (currentObject) {
        drawingObjects.append(currentObject);
        currentObject = nullptr;
    }
}

cv::Mat LayerManager::renderFrame(const cv::Mat& frame)
{
    cv::Mat result = frame.clone();
    
    // 渲染所有已提交的绘制对象
    for (DrawingObject* obj : drawingObjects) {
        if (obj->visible) {
            renderObject(result, obj);
        }
    }
    
    // 渲染当前正在绘制的对象
    if (currentObject) {
        renderObject(result, currentObject);
    }
    
    return result;
}

void LayerManager::renderObject(cv::Mat& frame, DrawingObject* obj)
{
    if (!obj) return;
    
    // 根据对象类型调用相应的绘制方法
    switch (obj->type) {
        case DrawingType::POINT:
            drawPoint(frame, obj);
            break;
        // 其他类型的绘制方法将在后续实现
        default:
            break;
    }
}

void LayerManager::drawPoint(cv::Mat& frame, DrawingObject* obj)
{
    if (obj->points.isEmpty()) return;
    
    QPoint p = obj->points.first();
    
    // 获取属性
    QColor color = obj->properties.value("color", QColor(0, 255, 0)).value<QColor>();
    int radius = obj->properties.value("radius", 10).toInt();
    
    cv::Scalar cvColor(color.blue(), color.green(), color.red());
    
    // 如果对象被选中，先绘制高亮效果
    if (obj->selected) {
        cv::circle(frame, cv::Point(p.x(), p.y()), radius + 4, cv::Scalar(0, 0, 255), -1);  // 使用红色作为选中高亮
    }
    
    // 绘制点（实心圆）
    cv::circle(frame, cv::Point(p.x(), p.y()), radius, cvColor, -1);
    
    // 显示坐标信息
    QString coordText = QString("(%1, %2)").arg(p.x()).arg(p.y());
    
    // 绘制文本（带背景）
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    double fontScale = 1.2;
    int thickness = 2;
    int padding = 8;
    
    // 计算文本位置（移动到点的右上方）
    int textX = p.x() + 10;
    int textY = p.y() - 10;
    
    // 获取文本大小
    cv::Size textSize = cv::getTextSize(coordText.toStdString(), fontFace, fontScale, thickness, nullptr);
    
    // 绘制文本背景
    cv::rectangle(frame,
                 cv::Point(textX - padding, textY - textSize.height - padding),
                 cv::Point(textX + textSize.width + padding, textY + padding),
                 cv::Scalar(0, 0, 0),
                 -1);
    
    // 绘制文本
    cv::putText(frame, coordText.toStdString(), cv::Point(textX, textY), fontFace, fontScale, cv::Scalar(255, 255, 255), thickness);
}

void LayerManager::clearDrawings()
{
    // 删除所有绘制对象
    qDeleteAll(drawingObjects);
    drawingObjects.clear();
    
    // 删除当前绘制对象
    if (currentObject) {
        delete currentObject;
        currentObject = nullptr;
    }
}

void LayerManager::clearSelection()
{
    // 清除所有对象的选中状态
    for (DrawingObject* obj : drawingObjects) {
        obj->selected = false;
    }
}

DrawingObject* LayerManager::findObjectAtPoint(const QPoint& point, int tolerance)
{
    // 从后向前查找（后绘制的对象优先选中）
    for (int i = drawingObjects.size() - 1; i >= 0; --i) {
        DrawingObject* obj = drawingObjects[i];
        
        if (obj->type == DrawingType::POINT) {
            if (!obj->points.isEmpty()) {
                QPoint p = obj->points.first();
                int radius = obj->properties.value("radius", 10).toInt();
                
                // 计算点击位置到点的距离
                int dx = point.x() - p.x();
                int dy = point.y() - p.y();
                int distanceSquared = dx * dx + dy * dy;
                
                // 如果距离小于半径加容差，则选中该对象
                if (distanceSquared <= (radius + tolerance) * (radius + tolerance)) {
                    return obj;
                }
            }
        }
        // 其他类型的对象选择将在后续实现
    }
    
    return nullptr;
}

// MeasurementManager 实现
MeasurementManager::MeasurementManager(QObject *parent)
    : QObject(parent), drawing(false), drawMode(DrawingType::NONE)
{
    layerManager = new LayerManager(this);
}

MeasurementManager::~MeasurementManager()
{
    delete layerManager;
}

void MeasurementManager::startPointMeasurement()
{
    drawMode = DrawingType::POINT;
    drawing = false;
}

void MeasurementManager::startLineMeasurement()
{
    drawMode = DrawingType::LINE;
    drawing = false;
}

void MeasurementManager::startCircleMeasurement()
{
    drawMode = DrawingType::CIRCLE;
    drawing = false;
}

void MeasurementManager::startLineSegmentMeasurement()
{
    drawMode = DrawingType::LINE_SEGMENT;
    drawing = false;
}

void MeasurementManager::startParallelMeasurement()
{
    drawMode = DrawingType::PARALLEL;
    drawing = false;
}

void MeasurementManager::startCircleLineMeasurement()
{
    drawMode = DrawingType::CIRCLE_LINE;
    drawing = false;
}

void MeasurementManager::startTwoLinesMeasurement()
{
    drawMode = DrawingType::TWO_LINES;
    drawing = false;
}

void MeasurementManager::startLineDetection()
{
    drawMode = DrawingType::LINE_DETECT;
    drawing = false;
}

void MeasurementManager::startCircleDetection()
{
    drawMode = DrawingType::CIRCLE_DETECT;
    drawing = false;
}

void MeasurementManager::clearMeasurements()
{
    layerManager->clearDrawings();
}

cv::Mat MeasurementManager::handleMousePress(const QPoint& eventPos, const cv::Mat& currentFrame)
{
    try {
        if (drawMode == DrawingType::POINT) {
            // 点绘制模式
            drawing = true;
            QMap<QString, QVariant> properties;
            properties["color"] = QColor(0, 255, 0);  // 绿色
            properties["radius"] = 10;  // 点的半径
            
            layerManager->startDrawing(drawMode, properties);
            layerManager->updateCurrentObject(eventPos);
            layerManager->commitDrawing();  // 立即提交点的绘制
            drawing = false;
            return layerManager->renderFrame(currentFrame);
        } else {
            // 其他模式的处理将在后续实现
            return currentFrame;
        }
    } catch (const std::exception& e) {
        qDebug() << "处理鼠标按下事件时出错:" << e.what();
        return currentFrame;
    }
}

cv::Mat MeasurementManager::handleMouseMove(const QPoint& eventPos, const cv::Mat& currentFrame)
{
    try {
        if (drawing && layerManager->currentObject) {
            layerManager->updateCurrentObject(eventPos);
            return layerManager->renderFrame(currentFrame);
        }
        return currentFrame;
    } catch (const std::exception& e) {
        qDebug() << "处理鼠标移动事件时出错:" << e.what();
        return currentFrame;
    }
}

cv::Mat MeasurementManager::handleMouseRelease(const QPoint& eventPos, const cv::Mat& currentFrame)
{
    try {
        if (drawMode == DrawingType::POINT) {
            // 点绘制模式已在按下时处理
            return layerManager->renderFrame(currentFrame);
        } else if (drawing && layerManager->currentObject) {
            // 其他模式的处理将在后续实现
            return currentFrame;
        }
        return currentFrame;
    } catch (const std::exception& e) {
        qDebug() << "处理鼠标释放事件时出错:" << e.what();
        return currentFrame;
    }
}

cv::Mat MeasurementManager::handleMouseRightClick(const QPoint& eventPos, const cv::Mat& currentFrame)
{
    try {
        // 右键点击时，退出绘制模式，进入选择模式
        drawing = false;
        
        // 查找点击位置的对象
        DrawingObject* clickedObj = layerManager->findObjectAtPoint(eventPos);
        
        // 清除之前的选择
        layerManager->clearSelection();
        
        // 如果找到对象，则选中它
        if (clickedObj) {
            clickedObj->selected = true;
        }
        
        return layerManager->renderFrame(currentFrame);
    } catch (const std::exception& e) {
        qDebug() << "处理鼠标右键点击事件时出错:" << e.what();
        return currentFrame;
    }
}

// DrawingManager 实现
DrawingManager::DrawingManager(QObject *parent)
    : QObject(parent), activeView(nullptr), activeMeasurement(nullptr), selectionMode(false)
{
}

DrawingManager::~DrawingManager()
{
    // 删除所有测量管理器
    qDeleteAll(measurementManagers);
}

void DrawingManager::initialize()
{
    // 清空现有的测量管理器
    qDeleteAll(measurementManagers);
    measurementManagers.clear();
    viewPairs.clear();
}

void DrawingManager::startPointMeasurement()
{
    qDebug() << "启动点测量";
    // 退出选择模式
    selectionMode = false;
    for (auto it = measurementManagers.begin(); it != measurementManagers.end(); ++it) {
        it.value()->startPointMeasurement();
        it.key()->setCursor(Qt::CrossCursor);
    }
}

void DrawingManager::startLineMeasurement()
{
    // 退出选择模式
    selectionMode = false;
    for (auto it = measurementManagers.begin(); it != measurementManagers.end(); ++it) {
        it.value()->startLineMeasurement();
        it.key()->setCursor(Qt::CrossCursor);
    }
}

void DrawingManager::startCircleMeasurement()
{
    // 退出选择模式
    selectionMode = false;
    for (auto it = measurementManagers.begin(); it != measurementManagers.end(); ++it) {
        it.value()->startCircleMeasurement();
        it.key()->setCursor(Qt::CrossCursor);
    }
}

void DrawingManager::startLineSegmentMeasurement()
{
    // 退出选择模式
    selectionMode = false;
    for (auto it = measurementManagers.begin(); it != measurementManagers.end(); ++it) {
        it.value()->startLineSegmentMeasurement();
        it.key()->setCursor(Qt::CrossCursor);
    }
}

void DrawingManager::startParallelMeasurement()
{
    // 退出选择模式
    selectionMode = false;
    for (auto it = measurementManagers.begin(); it != measurementManagers.end(); ++it) {
        it.value()->startParallelMeasurement();
        it.key()->setCursor(Qt::CrossCursor);
    }
}

void DrawingManager::startCircleLineMeasurement()
{
    // 退出选择模式
    selectionMode = false;
    for (auto it = measurementManagers.begin(); it != measurementManagers.end(); ++it) {
        it.value()->startCircleLineMeasurement();
        it.key()->setCursor(Qt::CrossCursor);
    }
}

void DrawingManager::startTwoLinesMeasurement()
{
    // 退出选择模式
    selectionMode = false;
    for (auto it = measurementManagers.begin(); it != measurementManagers.end(); ++it) {
        it.value()->startTwoLinesMeasurement();
        it.key()->setCursor(Qt::CrossCursor);
    }
}

void DrawingManager::startLineDetection()
{
    // 退出选择模式
    selectionMode = false;
    for (auto it = measurementManagers.begin(); it != measurementManagers.end(); ++it) {
        it.value()->startLineDetection();
        it.key()->setCursor(Qt::CrossCursor);
    }
}

void DrawingManager::startCircleDetection()
{
    // 退出选择模式
    selectionMode = false;
    for (auto it = measurementManagers.begin(); it != measurementManagers.end(); ++it) {
        it.value()->startCircleDetection();
        it.key()->setCursor(Qt::CrossCursor);
    }
}

void DrawingManager::clearDrawings(QLabel* label)
{
    if (label) {
        // 清空特定视图的绘制
        auto it = measurementManagers.find(label);
        if (it != measurementManagers.end()) {
            it.value()->clearMeasurements();
        }
    } else {
        // 清空所有视图的绘制
        for (auto it = measurementManagers.begin(); it != measurementManagers.end(); ++it) {
            it.value()->clearMeasurements();
        }
    }
}

void DrawingManager::enterSelectionMode()
{
    // 进入选择模式
    selectionMode = true;
    for (auto it = measurementManagers.begin(); it != measurementManagers.end(); ++it) {
        it.key()->setCursor(Qt::ArrowCursor);
    }
}

void DrawingManager::handleMousePress(QMouseEvent* event, QLabel* label)
{
    if (!label || !measurementManagers.contains(label)) {
        return;
    }
    
    activeView = label;
    activeMeasurement = measurementManagers[label];
    
    // 获取当前帧
    QVariant frameVariant = label->property("currentFrame");
    if (!frameVariant.isValid()) {
        return;
    }
    
    cv::Mat currentFrame = frameVariant.value<cv::Mat>();
    if (currentFrame.empty()) {
        return;
    }
    
    // 转换鼠标坐标
    QPoint imagePos = convertMouseToImageCoords(event->pos(), label, currentFrame);
    
    // 处理鼠标事件
    if (event->button() == Qt::LeftButton) {
        // 左键点击
        if (selectionMode) {
            // 在选择模式下，左键用于选择对象
            // 清除之前的选择
            activeMeasurement->layerManager->clearSelection();
            
            // 查找点击位置的对象
            DrawingObject* clickedObj = activeMeasurement->layerManager->findObjectAtPoint(imagePos);
            
            // 如果找到对象，则选中它
            if (clickedObj) {
                clickedObj->selected = true;
            }
            
            // 重新渲染帧
            cv::Mat displayFrame = activeMeasurement->layerManager->renderFrame(currentFrame);
            if (!displayFrame.empty()) {
                QImage img;
                // 根据图像通道数选择正确的格式
                if (displayFrame.channels() == 1) {
                    // 灰度图像
                    img = QImage(displayFrame.data, displayFrame.cols, displayFrame.rows, displayFrame.step, QImage::Format_Grayscale8);
                } else if (displayFrame.channels() == 3) {
                    // 彩色图像
                    img = QImage(displayFrame.data, displayFrame.cols, displayFrame.rows, displayFrame.step, QImage::Format_RGB888).rgbSwapped(); // BGR转RGB
                }
                
                // 保持QLabel大小不变，只缩放图像
                QSize labelSize = label->size();
                QPixmap pixmap = QPixmap::fromImage(img);
                label->setPixmap(pixmap.scaled(
                    labelSize, 
                    Qt::KeepAspectRatio, 
                    Qt::SmoothTransformation));
                
                // 保存当前帧
                label->setProperty("currentDisplayFrame", QVariant::fromValue(displayFrame));
            }
        } else {
            // 在绘制模式下，左键用于绘制
            cv::Mat displayFrame = activeMeasurement->handleMousePress(imagePos, currentFrame);
            if (!displayFrame.empty()) {
                QImage img;
                // 根据图像通道数选择正确的格式
                if (displayFrame.channels() == 1) {
                    // 灰度图像
                    img = QImage(displayFrame.data, displayFrame.cols, displayFrame.rows, displayFrame.step, QImage::Format_Grayscale8);
                } else if (displayFrame.channels() == 3) {
                    // 彩色图像
                    img = QImage(displayFrame.data, displayFrame.cols, displayFrame.rows, displayFrame.step, QImage::Format_RGB888).rgbSwapped(); // BGR转RGB
                }
                
                // 保持QLabel大小不变，只缩放图像
                QSize labelSize = label->size();
                QPixmap pixmap = QPixmap::fromImage(img);
                label->setPixmap(pixmap.scaled(
                    labelSize, 
                    Qt::KeepAspectRatio, 
                    Qt::SmoothTransformation));
                
                // 保存当前帧
                label->setProperty("currentDisplayFrame", QVariant::fromValue(displayFrame));
            }
        }
    } else if (event->button() == Qt::RightButton) {
        // 右键点击
        if (selectionMode) {
            // 在选择模式下，右键用于显示上下文菜单
            // 查找是否有选中的对象
            bool hasSelection = false;
            for (DrawingObject* obj : activeMeasurement->layerManager->drawingObjects) {
                if (obj->selected) {
                    hasSelection = true;
                    break;
                }
            }
            
            if (hasSelection) {
                // 创建右键菜单
                createContextMenu(label, event->pos());
            }
        } else {
            // 在绘制模式下，右键用于进入选择模式
            enterSelectionMode();
            
            // 清除之前的选择
            activeMeasurement->layerManager->clearSelection();
            
            // 重新渲染帧
            cv::Mat displayFrame = activeMeasurement->layerManager->renderFrame(currentFrame);
            if (!displayFrame.empty()) {
                QImage img;
                // 根据图像通道数选择正确的格式
                if (displayFrame.channels() == 1) {
                    // 灰度图像
                    img = QImage(displayFrame.data, displayFrame.cols, displayFrame.rows, displayFrame.step, QImage::Format_Grayscale8);
                } else if (displayFrame.channels() == 3) {
                    // 彩色图像
                    img = QImage(displayFrame.data, displayFrame.cols, displayFrame.rows, displayFrame.step, QImage::Format_RGB888).rgbSwapped(); // BGR转RGB
                }
                
                // 保持QLabel大小不变，只缩放图像
                QSize labelSize = label->size();
                QPixmap pixmap = QPixmap::fromImage(img);
                label->setPixmap(pixmap.scaled(
                    labelSize, 
                    Qt::KeepAspectRatio, 
                    Qt::SmoothTransformation));
                
                // 保存当前帧
                label->setProperty("currentDisplayFrame", QVariant::fromValue(displayFrame));
            }
        }
    }
}

void DrawingManager::handleMouseMove(QMouseEvent* event, QLabel* label)
{
    if (!label || !measurementManagers.contains(label) || !activeMeasurement) {
        return;
    }
    
    // 如果处于选择模式，不处理鼠标移动
    if (selectionMode) {
        return;
    }
    
    // 获取当前帧
    QVariant frameVariant = label->property("currentFrame");
    if (!frameVariant.isValid()) {
        return;
    }
    
    cv::Mat currentFrame = frameVariant.value<cv::Mat>();
    if (currentFrame.empty()) {
        return;
    }
    
    // 转换鼠标坐标
    QPoint imagePos = convertMouseToImageCoords(event->pos(), label, currentFrame);
    
    // 处理鼠标移动
    cv::Mat displayFrame = activeMeasurement->handleMouseMove(imagePos, currentFrame);
    if (!displayFrame.empty()) {
        QImage img;
        // 根据图像通道数选择正确的格式
        if (displayFrame.channels() == 1) {
            // 灰度图像
            img = QImage(displayFrame.data, displayFrame.cols, displayFrame.rows, displayFrame.step, QImage::Format_Grayscale8);
        } else if (displayFrame.channels() == 3) {
            // 彩色图像
            img = QImage(displayFrame.data, displayFrame.cols, displayFrame.rows, displayFrame.step, QImage::Format_RGB888).rgbSwapped(); // BGR转RGB
        }
        
        // 保持QLabel大小不变，只缩放图像
        QSize labelSize = label->size();
        QPixmap pixmap = QPixmap::fromImage(img);
        label->setPixmap(pixmap.scaled(
            labelSize, 
            Qt::KeepAspectRatio, 
            Qt::SmoothTransformation));
        
        // 保存当前帧
        label->setProperty("currentDisplayFrame", QVariant::fromValue(displayFrame));
    }
}

void DrawingManager::handleMouseRelease(QMouseEvent* event, QLabel* label)
{
    if (!label || !measurementManagers.contains(label) || !activeMeasurement) {
        return;
    }
    
    // 如果处于选择模式，不处理鼠标释放
    if (selectionMode) {
        return;
    }
    
    // 获取当前帧
    QVariant frameVariant = label->property("currentFrame");
    if (!frameVariant.isValid()) {
        return;
    }
    
    cv::Mat currentFrame = frameVariant.value<cv::Mat>();
    if (currentFrame.empty()) {
        return;
    }
    
    // 转换鼠标坐标
    QPoint imagePos = convertMouseToImageCoords(event->pos(), label, currentFrame);
    
    // 处理鼠标释放
    if (event->button() == Qt::LeftButton) {
        cv::Mat displayFrame = activeMeasurement->handleMouseRelease(imagePos, currentFrame);
        if (!displayFrame.empty()) {
            QImage img;
            // 根据图像通道数选择正确的格式
            if (displayFrame.channels() == 1) {
                // 灰度图像
                img = QImage(displayFrame.data, displayFrame.cols, displayFrame.rows, displayFrame.step, QImage::Format_Grayscale8);
            } else if (displayFrame.channels() == 3) {
                // 彩色图像
                img = QImage(displayFrame.data, displayFrame.cols, displayFrame.rows, displayFrame.step, QImage::Format_RGB888).rgbSwapped(); // BGR转RGB
            }
            
            // 保持QLabel大小不变，只缩放图像
            QSize labelSize = label->size();
            QPixmap pixmap = QPixmap::fromImage(img);
            label->setPixmap(pixmap.scaled(
                labelSize, 
                Qt::KeepAspectRatio, 
                Qt::SmoothTransformation));
            
            // 保存当前帧
            label->setProperty("currentDisplayFrame", QVariant::fromValue(displayFrame));
        }
    }
}

void DrawingManager::createContextMenu(QLabel* label, const QPoint& pos)
{
    if (!label || !measurementManagers.contains(label)) {
        return;
    }
    
    MeasurementManager* manager = measurementManagers[label];
    
    // 查找是否有选中的对象
    bool hasSelection = false;
    for (DrawingObject* obj : manager->layerManager->drawingObjects) {
        if (obj->selected) {
            hasSelection = true;
            break;
        }
    }
    
    if (hasSelection) {
        // 创建右键菜单
        QMenu contextMenu;
        QAction* deleteAction = contextMenu.addAction("删除");
        
        // 连接删除动作
        connect(deleteAction, &QAction::triggered, [this, label]() {
            this->deleteSelectedObject(label);
        });
        
        // 显示菜单
        contextMenu.exec(label->mapToGlobal(pos));
    }
}

void DrawingManager::deleteSelectedObject(QLabel* label)
{
    if (!label || !measurementManagers.contains(label)) {
        return;
    }
    
    MeasurementManager* manager = measurementManagers[label];
    
    // 获取当前帧
    QVariant frameVariant = label->property("currentFrame");
    if (!frameVariant.isValid()) {
        return;
    }
    
    cv::Mat currentFrame = frameVariant.value<cv::Mat>();
    if (currentFrame.empty()) {
        return;
    }
    
    // 查找选中的对象
    QVector<DrawingObject*>& objects = manager->layerManager->drawingObjects;
    for (int i = 0; i < objects.size(); ++i) {
        if (objects[i]->selected) {
            // 删除选中的对象
            delete objects[i];
            objects.removeAt(i);
            break;
        }
    }
    
    // 重新渲染帧
    cv::Mat displayFrame = manager->layerManager->renderFrame(currentFrame);
    if (!displayFrame.empty()) {
        QImage img;
        // 根据图像通道数选择正确的格式
        if (displayFrame.channels() == 1) {
            // 灰度图像
            img = QImage(displayFrame.data, displayFrame.cols, displayFrame.rows, displayFrame.step, QImage::Format_Grayscale8);
        } else if (displayFrame.channels() == 3) {
            // 彩色图像
            img = QImage(displayFrame.data, displayFrame.cols, displayFrame.rows, displayFrame.step, QImage::Format_RGB888).rgbSwapped(); // BGR转RGB
        }
        
        // 保持QLabel大小不变，只缩放图像
        QSize labelSize = label->size();
        QPixmap pixmap = QPixmap::fromImage(img);
        label->setPixmap(pixmap.scaled(
            labelSize, 
            Qt::KeepAspectRatio, 
            Qt::SmoothTransformation));
        
        // 保存当前帧
        label->setProperty("currentDisplayFrame", QVariant::fromValue(displayFrame));
    }
}

QPoint DrawingManager::convertMouseToImageCoords(const QPoint& pos, QLabel* label, const cv::Mat& currentFrame)
{
    int imgHeight = currentFrame.rows;
    int imgWidth = currentFrame.cols;
    int labelWidth = label->width();
    int labelHeight = label->height();
    
    double ratio = qMin(static_cast<double>(labelWidth) / imgWidth, static_cast<double>(labelHeight) / imgHeight);
    int displayWidth = static_cast<int>(imgWidth * ratio);
    int displayHeight = static_cast<int>(imgHeight * ratio);
    
    int xOffset = (labelWidth - displayWidth) / 2;
    int yOffset = (labelHeight - displayHeight) / 2;
    
    int imageX = static_cast<int>((pos.x() - xOffset) * imgWidth / displayWidth);
    int imageY = static_cast<int>((pos.y() - yOffset) * imgHeight / displayHeight);
    
    imageX = qMax(0, qMin(imageX, imgWidth - 1));
    imageY = qMax(0, qMin(imageY, imgHeight - 1));
    
    return QPoint(imageX, imageY);
}

MeasurementManager* DrawingManager::getMeasurementManager(QLabel* label)
{
    return measurementManagers.value(label, nullptr);
}

void DrawingManager::syncDrawings(QLabel* sourceLabel, QLabel* targetLabel)
{
    if (!sourceLabel || !targetLabel || !measurementManagers.contains(sourceLabel) || !measurementManagers.contains(targetLabel)) {
        return;
    }
    
    MeasurementManager* sourceManager = measurementManagers[sourceLabel];
    MeasurementManager* targetManager = measurementManagers[targetLabel];
    
    // 同步绘制对象
    targetManager->layerManager->drawingObjects.clear();
    for (DrawingObject* obj : sourceManager->layerManager->drawingObjects) {
        DrawingObject* newObj = new DrawingObject(obj->type);
        newObj->points = obj->points;
        newObj->properties = obj->properties;
        newObj->selected = obj->selected;
        newObj->visible = obj->visible;
        
        targetManager->layerManager->drawingObjects.append(newObj);
    }
    
    // 获取目标标签的当前帧
    QVariant frameVariant = targetLabel->property("currentFrame");
    if (!frameVariant.isValid()) {
        return;
    }
    
    cv::Mat currentFrame = frameVariant.value<cv::Mat>();
    if (currentFrame.empty()) {
        return;
    }
    
    // 重新渲染目标帧
    cv::Mat displayFrame = targetManager->layerManager->renderFrame(currentFrame);
    if (!displayFrame.empty()) {
        QImage img;
        // 根据图像通道数选择正确的格式
        if (displayFrame.channels() == 1) {
            // 灰度图像
            img = QImage(displayFrame.data, displayFrame.cols, displayFrame.rows, displayFrame.step, QImage::Format_Grayscale8);
        } else if (displayFrame.channels() == 3) {
            // 彩色图像
            img = QImage(displayFrame.data, displayFrame.cols, displayFrame.rows, displayFrame.step, QImage::Format_RGB888).rgbSwapped(); // BGR转RGB
        }
        
        // 保持QLabel大小不变，只缩放图像
        QSize labelSize = targetLabel->size();
        QPixmap pixmap = QPixmap::fromImage(img);
        targetLabel->setPixmap(pixmap.scaled(
            labelSize, 
            Qt::KeepAspectRatio, 
            Qt::SmoothTransformation));
        
        // 保存当前帧
        targetLabel->setProperty("currentDisplayFrame", QVariant::fromValue(displayFrame));
    }
}

void DrawingManager::addMeasurementManager(QLabel* label, MeasurementManager* manager)
{
    if (label && manager) {
        measurementManagers[label] = manager;
    }
}

void DrawingManager::addViewPair(QLabel* sourceLabel, QLabel* targetLabel)
{
    if (sourceLabel && targetLabel) {
        viewPairs[sourceLabel] = targetLabel;
    }
}

// 添加新方法，用于在图像更新时保持绘制内容
void DrawingManager::updateDisplay(QLabel* label, const cv::Mat& newFrame)
{
    if (!label || !measurementManagers.contains(label)) {
        return;
    }
    
    MeasurementManager* manager = measurementManagers[label];
    
    // 保存新帧作为当前帧
    label->setProperty("currentFrame", QVariant::fromValue(newFrame));
    
    // 如果没有绘制对象，直接显示新帧
    if (manager->layerManager->drawingObjects.isEmpty()) {
        QImage img;
        // 根据图像通道数选择正确的格式
        if (newFrame.channels() == 1) {
            // 灰度图像
            img = QImage(newFrame.data, newFrame.cols, newFrame.rows, newFrame.step, QImage::Format_Grayscale8);
        } else if (newFrame.channels() == 3) {
            // 彩色图像
            img = QImage(newFrame.data, newFrame.cols, newFrame.rows, newFrame.step, QImage::Format_RGB888).rgbSwapped(); // BGR转RGB
        }
        
        // 保持QLabel大小不变，只缩放图像
        QSize labelSize = label->size();
        QPixmap pixmap = QPixmap::fromImage(img);
        label->setPixmap(pixmap.scaled(
            labelSize, 
            Qt::KeepAspectRatio, 
            Qt::SmoothTransformation));
        return;
    }
    
    // 在新帧上重新渲染所有绘制对象
    cv::Mat displayFrame = manager->layerManager->renderFrame(newFrame);
    if (!displayFrame.empty()) {
        QImage img;
        // 根据图像通道数选择正确的格式
        if (displayFrame.channels() == 1) {
            // 灰度图像
            img = QImage(displayFrame.data, displayFrame.cols, displayFrame.rows, displayFrame.step, QImage::Format_Grayscale8);
        } else if (displayFrame.channels() == 3) {
            // 彩色图像
            img = QImage(displayFrame.data, displayFrame.cols, displayFrame.rows, displayFrame.step, QImage::Format_RGB888).rgbSwapped(); // BGR转RGB
        }
        
        // 保持QLabel大小不变，只缩放图像
        QSize labelSize = label->size();
        QPixmap pixmap = QPixmap::fromImage(img);
        label->setPixmap(pixmap.scaled(
            labelSize, 
            Qt::KeepAspectRatio, 
            Qt::SmoothTransformation));
        
        // 保存当前显示帧
        label->setProperty("currentDisplayFrame", QVariant::fromValue(displayFrame));
        
        // 如果处于选择模式，设置鼠标为箭头
        if (selectionMode) {
            label->setCursor(Qt::ArrowCursor);
        }
    }
} 