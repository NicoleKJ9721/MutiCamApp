#include "ROIManager.h"
#include "../../PaintingOverlay.h"
#include <QResizeEvent>
#include <QDebug>

ROIManager::ROIManager(PaintingOverlay* paintingOverlay, QObject* parent)
    : QObject(parent)
    , paintingOverlay_(paintingOverlay)
    , overlayView_(nullptr)
    , overlayScene_(nullptr)
    , roiItem_(nullptr)
    , controlButtons_(nullptr)
    , isInitialized_(false)
{
    if (!paintingOverlay_) {
        qWarning() << "ROIManager: paintingOverlay不能为空";
        return;
    }
    
    initializeOverlay();
}

ROIManager::~ROIManager() {
    if (overlayView_) {
        overlayView_->deleteLater();
    }
}

void ROIManager::initializeOverlay() {
    if (isInitialized_ || !paintingOverlay_) return;
    
    qDebug() << "ROIManager: 开始初始化，PaintingOverlay地址:" << paintingOverlay_;
    
    // 创建QGraphicsView作为透明覆盖层
    overlayView_ = new QGraphicsView(paintingOverlay_);
    overlayView_->setStyleSheet("background: transparent; border: none;");
    overlayView_->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    overlayView_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    overlayView_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    overlayView_->setFrameStyle(QFrame::NoFrame);
    overlayView_->setRenderHint(QPainter::Antialiasing);
    
    // 创建场景
    overlayScene_ = new QGraphicsScene(overlayView_);
    overlayView_->setScene(overlayScene_);
    
    // 创建ROI项
    roiItem_ = new ROIOverlayItem();
    overlayScene_->addItem(roiItem_);
    
    // 创建控制按钮
    controlButtons_ = new ROIControlButtons();
    overlayScene_->addItem(controlButtons_);
    
    // 连接信号
    connect(roiItem_, &ROIOverlayItem::roiChanged,
            this, &ROIManager::onROIChanged);
    connect(roiItem_, &ROIOverlayItem::roiEditingFinished,
            this, &ROIManager::onROIEditingFinished);
    connect(controlButtons_, &ROIControlButtons::confirmClicked,
            this, &ROIManager::onConfirmClicked);
    connect(controlButtons_, &ROIControlButtons::cancelClicked,
            this, &ROIManager::onCancelClicked);
    
    // 初始化几何 - 强制设置正确的大小
    QRect paintingRect = paintingOverlay_->rect();
    qDebug() << "初始化时PaintingOverlay矩形:" << paintingRect;

    overlayView_->setGeometry(paintingRect);
    overlayScene_->setSceneRect(paintingRect);

    qDebug() << "初始化后OverlayView几何:" << overlayView_->geometry();
    qDebug() << "初始化后场景矩形:" << overlayScene_->sceneRect();

    // 默认隐藏
    overlayView_->setVisible(false);
    
    isInitialized_ = true;
    qDebug() << "ROIManager初始化完成";
}

void ROIManager::startROICreation() {
    if (!isInitialized_ || !roiItem_) {
        qDebug() << "ROIManager未初始化或roiItem为空";
        return;
    }

    qDebug() << "开始ROI创建";
    qDebug() << "PaintingOverlay大小:" << paintingOverlay_->size();
    qDebug() << "PaintingOverlay几何:" << paintingOverlay_->geometry();
    qDebug() << "OverlayView几何（更新前）:" << overlayView_->geometry();

    // 强制更新几何
    QRect paintingRect = paintingOverlay_->rect();
    overlayView_->setGeometry(paintingRect);
    overlayScene_->setSceneRect(paintingRect);

    qDebug() << "OverlayView几何（更新后）:" << overlayView_->geometry();
    qDebug() << "场景矩形:" << overlayScene_->sceneRect();

    // 显示默认正方形ROI
    QSize viewSize = paintingOverlay_->size();
    roiItem_->showDefaultROI(QSizeF(viewSize));

    // 显示控制按钮
    updateControlButtonsPosition();
    controlButtons_->setVisible(true);

    // 显示覆盖层
    overlayView_->setVisible(true);
    overlayView_->raise();  // 确保在最顶层

    qDebug() << "ROI状态:" << roiItem_->getState();
    qDebug() << "OverlayView可见性:" << overlayView_->isVisible();
    qDebug() << "ROI项可见性:" << roiItem_->isVisible();
}

void ROIManager::cancelROICreation() {
    if (!isInitialized_ || !roiItem_) return;
    
    roiItem_->setState(ROIOverlayItem::Hidden);
    controlButtons_->setVisible(false);
    overlayView_->setVisible(false);
    
    emit roiCancelled();
    qDebug() << "取消ROI创建";
}

void ROIManager::finishROICreation() {
    if (!isInitialized_ || !roiItem_) return;
    
    if (roiItem_->getState() == ROIOverlayItem::Editing) {
        roiItem_->setState(ROIOverlayItem::Locked);
        controlButtons_->setVisible(false);
        emit roiFinished();
        qDebug() << "完成ROI创建";
    }
}

bool ROIManager::hasROI() const {
    return isInitialized_ && roiItem_ && 
           roiItem_->getState() != ROIOverlayItem::Hidden &&
           roiItem_->getROI().isValid();
}

bool ROIManager::isEditing() const {
    return isInitialized_ && roiItem_ && 
           roiItem_->getState() == ROIOverlayItem::Editing;
}

QRectF ROIManager::getROI() const {
    if (!hasROI()) return QRectF();
    return roiItem_->getROI();
}

qreal ROIManager::getROIRotation() const {
    if (!hasROI()) return 0.0;
    return roiItem_->getRotation();
}

QPolygonF ROIManager::getROIPolygon() const {
    if (!hasROI()) return QPolygonF();
    return roiItem_->getRotatedPolygon();
}

cv::Rect ROIManager::getROIRect() const {
    QRectF roi = getROI();
    if (!roi.isValid()) return cv::Rect();
    
    return cv::Rect(
        static_cast<int>(roi.x()),
        static_cast<int>(roi.y()),
        static_cast<int>(roi.width()),
        static_cast<int>(roi.height())
    );
}

void ROIManager::setROIVisible(bool visible) {
    if (!isInitialized_) return;
    
    if (visible && hasROI()) {
        overlayView_->setVisible(true);
    } else {
        overlayView_->setVisible(false);
    }
}

void ROIManager::setROIMaskVisible(bool visible) {
    if (!isInitialized_ || !roiItem_) return;
    
    roiItem_->setShowMask(visible);
}

void ROIManager::onROIChanged(const QRectF& rect, qreal angle) {
    updateControlButtonsPosition();
    emit roiChanged(rect, angle);
}

void ROIManager::onROIEditingFinished() {
    updateControlButtonsPosition();
}

void ROIManager::onConfirmClicked() {
    emit roiCreated(getROI(), getROIRotation());
}

void ROIManager::onCancelClicked() {
    cancelROICreation();
}

void ROIManager::onPaintingOverlayResized() {
    updateOverlayGeometry();
}

void ROIManager::updateOverlayGeometry() {
    if (!isInitialized_ || !overlayView_ || !paintingOverlay_) return;

    QRect paintingRect = paintingOverlay_->rect();
    qDebug() << "更新覆盖层几何 - PaintingOverlay矩形:" << paintingRect;

    // 使覆盖层与PaintingOverlay大小一致
    overlayView_->setGeometry(paintingRect);

    // 更新场景矩形
    if (overlayScene_) {
        overlayScene_->setSceneRect(paintingRect);
        qDebug() << "场景矩形设置为:" << overlayScene_->sceneRect();
    }

    qDebug() << "覆盖层几何更新完成 - OverlayView几何:" << overlayView_->geometry();
}

void ROIManager::updateControlButtonsPosition() {
    if (!isInitialized_ || !controlButtons_ || !roiItem_) return;
    
    if (roiItem_->getState() == ROIOverlayItem::Editing) {
        QRectF roi = roiItem_->getROI();
        if (roi.isValid()) {
            controlButtons_->setPosition(roi.bottomRight());
        }
    }
}
