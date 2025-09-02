#ifndef VIDEODISPLAYWIDGET_H
#define VIDEODISPLAYWIDGET_H

#include <QLabel>
#include <QPixmap>
#include <QPointF>

// 前向声明
class QPaintEvent;
class PaintingOverlay;

/**
 * @brief 简化的视频显示控件，专注于显示QPixmap
 * 
 * 这个控件现在只负责：
 * - 显示视频帧（QPixmap）
 * - 提供坐标转换辅助函数
 * - 与PaintingOverlay配合，烘焙静态绘图内容到背景缓存
 * 
 * 所有绘图逻辑已迁移到PaintingOverlay类中
 */

class VideoDisplayWidget : public QLabel
{
    Q_OBJECT

public:
    explicit VideoDisplayWidget(QWidget *parent = nullptr);
    
    // 设置视频帧
    void setVideoFrame(const QPixmap& pixmap);

    // 设置外部变换参数（由ZoomPanWidget调用）
    void setExternalTransform(const QPointF& offset, double scaleFactor);
    void resetExternalTransform();

    // 坐标转换辅助函数
    QPointF getImageOffset() const;
    double getScaleFactor() const;
    QSize getImageSize() const;
protected:
    void paintEvent(QPaintEvent *event) override;
    
private:
    QPixmap m_videoFrame;                           ///< 视频帧

    // 外部变换参数（由ZoomPanWidget设置）
    bool m_hasExternalTransform;                    ///< 是否有外部变换
    QPointF m_externalOffset;                       ///< 外部偏移
    double m_externalScaleFactor;                   ///< 外部缩放因子

};

#endif // VIDEODISPLAYWIDGET_H