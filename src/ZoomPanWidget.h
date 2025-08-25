#ifndef ZOOMPANWIDGET_H
#define ZOOMPANWIDGET_H

#include <QWidget>
#include <QPointF>
#include <QSize>
#include <QPixmap>
#include <QTimer>

// 前向声明
class VideoDisplayWidget;
class PaintingOverlay;
class QWheelEvent;
class QKeyEvent;
class QMouseEvent;

/**
 * @brief 缩放平移控件
 * 
 * 包装VideoDisplayWidget，提供缩放和平移功能：
 * - 鼠标滚轮缩放
 * - 键盘方向键平移
 * - 空格+鼠标左键平移
 * - 坐标转换接口
 */
class ZoomPanWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ZoomPanWidget(QWidget *parent = nullptr);
    ~ZoomPanWidget() = default;

    // VideoDisplayWidget接口代理
    void setVideoFrame(const QPixmap& pixmap);
    VideoDisplayWidget* getVideoDisplayWidget() const { return m_videoWidget; }

    // 缩放平移控制
    void setZoomFactor(double factor);
    double getZoomFactor() const { return m_zoomFactor; }
    void resetZoom();
    void resetPan();
    void resetView();

    // 缩放范围设置
    void setZoomRange(double minZoom, double maxZoom);
    double getMinZoom() const { return m_minZoomFactor; }
    double getMaxZoom() const { return m_maxZoomFactor; }

    // 平移功能控制
    void setPanEnabled(bool enabled);
    bool isPanEnabled() const { return m_panEnabled; }

    // 平移偏移
    void setPanOffset(const QPointF& offset);
    QPointF getPanOffset() const { return QPointF(m_panOffsetX, m_panOffsetY); }

    // 坐标转换接口（供PaintingOverlay使用）
    QPointF windowToImageCoordinates(const QPoint& windowPos) const;
    QPoint imageToWindowCoordinates(const QPointF& imagePos) const;
    QPointF getImageOffset() const;
    double getScaleFactor() const;
    QSize getImageSize() const;

    // PaintingOverlay集成
    void setPaintingOverlay(PaintingOverlay* overlay);
    PaintingOverlay* getPaintingOverlay() const { return m_paintingOverlay; }

    // 状态查询
    bool isSpacePressed() const { return m_spacePressed; }
    bool isPanning() const { return m_isPanning; }

signals:
    /**
     * @brief 视图变换改变信号（缩放或平移）
     * @param zoomFactor 当前缩放因子
     * @param panOffset 当前平移偏移
     */
    void viewTransformChanged(double zoomFactor, const QPointF& panOffset);

    /**
     * @brief 鼠标事件转发信号
     */
    void mousePressed(const QPoint& pos);
    void mouseMoved(const QPoint& pos);
    void mouseReleased(const QPoint& pos);

protected:
    // 事件处理
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    /**
     * @brief 平移动画定时器槽函数
     */
    void onPanAnimationTimer();

private:
    // 核心组件
    VideoDisplayWidget* m_videoWidget;      ///< 被包装的视频显示控件
    PaintingOverlay* m_paintingOverlay;     ///< 关联的绘图覆盖层

    // 缩放相关
    double m_zoomFactor;                    ///< 当前缩放因子
    double m_minZoomFactor;                 ///< 最小缩放因子
    double m_maxZoomFactor;                 ///< 最大缩放因子
    double m_zoomStep;                      ///< 缩放步长
    QPoint m_lastZoomCenter;                ///< 上次缩放中心点

    // 平移相关
    double m_panOffsetX;                    ///< X轴平移偏移
    double m_panOffsetY;                    ///< Y轴平移偏移
    double m_maxPanOffsetX;                 ///< X轴最大平移偏移
    double m_maxPanOffsetY;                 ///< Y轴最大平移偏移

    // 鼠标拖拽平移
    bool m_isPanning;                       ///< 是否正在平移
    bool m_spacePressed;                    ///< 空格键是否按下
    QPoint m_lastPanPoint;                  ///< 上次平移点
    QTimer* m_panAnimationTimer;            ///< 平移动画定时器

    // 键盘平移
    bool m_keyPanActive;                    ///< 键盘平移是否激活
    QPointF m_keyPanVelocity;               ///< 键盘平移速度
    QTimer* m_keyPanTimer;                  ///< 键盘平移定时器

    // 功能控制
    bool m_panEnabled;                      ///< 是否启用平移功能

    // 内部方法
    void initializeComponents();
    void setupLayout();
    void connectSignals();
    void updateVideoWidgetTransform();
    void updatePaintingOverlayTransform();
    void constrainPanOffset();
    void calculateMaxPanOffset();
    QPointF getEffectiveImageOffset() const;
    double getEffectiveScaleFactor() const;

    // 缩放辅助方法
    void performZoom(double newZoomFactor, const QPoint& zoomCenter);
    void smoothZoom(double targetZoom, const QPoint& zoomCenter);

    // 平移辅助方法
    void performPan(double deltaX, double deltaY);
    void smoothPan(const QPointF& targetOffset);
    void startKeyboardPan(const QPointF& velocity);
    void stopKeyboardPan();
};

#endif // ZOOMPANWIDGET_H
