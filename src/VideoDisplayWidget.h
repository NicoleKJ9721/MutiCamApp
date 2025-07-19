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
    
    // 移除 updateStaticDrawings - 不再需要
    
    // 坐标转换辅助函数
    QPointF getImageOffset() const;
    double getScaleFactor() const;
    QSize getImageSize() const;
protected:
    void paintEvent(QPaintEvent *event) override;
    
private:
    QPixmap m_videoFrame;                           ///< 视频帧
    // 移除 m_compositionBuffer - 不再需要

};

#endif // VIDEODISPLAYWIDGET_H