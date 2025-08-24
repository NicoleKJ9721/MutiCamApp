#pragma once

#include <QWidget>
#include <QPoint>
#include <QRect>
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QTransform>
#include <QPolygonF>
#include <QRectF>
#include <vector>

/**
 * @brief 可旋转ROI覆盖层
 * 
 * 透明覆盖层，提供8个控制点的矩形ROI和1个旋转控制点
 * 支持拖拽调整大小、位置和旋转角度
 */
class RotatableROIOverlay : public QWidget
{
    Q_OBJECT

public:
    enum ControlPoint {
        None = -1,
        TopLeft = 0,
        TopCenter = 1,
        TopRight = 2,
        MiddleRight = 3,
        BottomRight = 4,
        BottomCenter = 5,
        BottomLeft = 6,
        MiddleLeft = 7,
        RotationHandle = 8
    };

    explicit RotatableROIOverlay(QWidget *parent = nullptr);
    ~RotatableROIOverlay() = default;

    // ROI控制
    void setROI(const QRectF& roi);
    QRectF getROI() const { return m_roi; }
    
    void setRotation(float angle);
    float getRotation() const { return m_rotationAngle; }
    
    // 显示控制
    void setVisible(bool visible);
    bool isVisible() const { return m_visible; }
    
    void setEnabled(bool enabled);
    bool isEnabled() const { return m_enabled; }
    
    // 模式控制 - 与PaintingOverlay互斥
    void setTemplateCreationMode(bool enabled);
    
    // 样式设置
    void setBorderColor(const QColor& color) { m_borderColor = color; update(); }
    void setFillColor(const QColor& color) { m_fillColor = color; update(); }
    void setControlPointColor(const QColor& color) { m_controlPointColor = color; update(); }
    void setRotationHandleColor(const QColor& color) { m_rotationHandleColor = color; update(); }
    
    void setBorderWidth(int width) { m_borderWidth = width; update(); }
    void setControlPointSize(int size) { m_controlPointSize = size; update(); }
    
    // 确认/取消按钮
    void showConfirmButtons(bool show) { m_showConfirmButtons = show; update(); }
    
    // 坐标转换接口（需要ZoomPanWidget提供）
    void setCoordinateTransform(std::function<QPointF(const QPoint&)> windowToImage,
                               std::function<QPoint(const QPointF&)> imageToWindow);
    
    // 获取变换后的ROI多边形
    QPolygonF getTransformedROI() const;
    
    // 重置到默认状态
    void reset();

signals:
    void roiChanged(const QRectF& roi, float rotation);
    void roiEditingStarted();
    void roiEditingFinished();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    // 几何计算
    QPolygonF calculateROIPolygon() const;
    std::vector<QPointF> calculateControlPoints() const;
    QPointF calculateRotationHandle() const;
    
    // 碰撞检测
    ControlPoint hitTestControlPoint(const QPoint& pos) const;
    bool hitTestROI(const QPoint& pos) const;
    
    // 变换操作
    void updateROIFromControlPoint(ControlPoint point, const QPointF& newPos);
    void updateRotationFromHandle(const QPointF& handlePos);
    void moveROI(const QPointF& delta);
    
    // 约束处理
    void constrainROI();
    QRectF constrainToParent(const QRectF& rect) const;
    
    // 绘制辅助
    void drawROI(QPainter& painter) const;
    void drawControlPoints(QPainter& painter) const;
    void drawRotationHandle(QPainter& painter) const;
    void drawROIInfo(QPainter& painter) const;
    void drawConfirmButtons(QPainter& painter) const;
    
    // 按钮位置更新
    void updateConfirmButtonPositions();
    
    // 坐标转换
    QPointF windowToImage(const QPoint& windowPos) const;
    QPoint imageToWindow(const QPointF& imagePos) const;

private:
    // ROI几何
    QRectF m_roi;                           ///< ROI矩形（图像坐标）
    float m_rotationAngle;                  ///< 旋转角度（度）
    QPointF m_roiCenter;                    ///< ROI中心点（图像坐标）
    
    // 交互状态
    bool m_visible;                         ///< 是否可见
    bool m_enabled;                         ///< 是否启用交互
    bool m_dragging;                        ///< 是否正在拖拽
    ControlPoint m_activeControlPoint;      ///< 当前激活的控制点
    QPoint m_lastMousePos;                  ///< 上次鼠标位置
    QPointF m_dragStartROI;                 ///< 拖拽开始时的ROI位置
    float m_dragStartRotation;              ///< 拖拽开始时的旋转角度
    
    // 样式设置
    QColor m_borderColor;                   ///< 边框颜色
    QColor m_fillColor;                     ///< 填充颜色
    QColor m_controlPointColor;             ///< 控制点颜色
    QColor m_rotationHandleColor;           ///< 旋转控制点颜色
    int m_borderWidth;                      ///< 边框宽度
    int m_controlPointSize;                 ///< 控制点大小
    
    // 坐标转换函数
    std::function<QPointF(const QPoint&)> m_windowToImage;
    std::function<QPoint(const QPointF&)> m_imageToWindow;
    
    // 确认/取消按钮
    bool m_showConfirmButtons;              ///< 是否显示确认/取消按钮
    QRectF m_confirmButtonRect;             ///< 确认按钮区域
    QRectF m_cancelButtonRect;              ///< 取消按钮区域
    
    // 常量
    static constexpr float ROTATION_HANDLE_DISTANCE = 30.0f;  ///< 旋转控制点距离
    static constexpr int BUTTON_SIZE = 24;                    ///< 按钮大小
    static constexpr int BUTTON_MARGIN = 8;                   ///< 按钮间距
    static constexpr float HIT_TEST_TOLERANCE = 8.0f;        ///< 碰撞检测容差
    static constexpr float MIN_ROI_SIZE = 10.0f;             ///< 最小ROI尺寸
};
