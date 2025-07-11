#pragma once
#include "ui_MutiCamApp.h"
#include "domain/drawing/services/DrawingManager.h"
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QGroupBox>
#include <QTabWidget>
#include <QImage>
#include <QPixmap>
#include <QMouseEvent>
#include <QWheelEvent>
#include <memory>
#include <map>

// 前向声明
class CameraThread;

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
class QLineEdit;
class QGroupBox;
class QTabWidget;
QT_END_NAMESPACE

class MutiCamApp : public QMainWindow {
    Q_OBJECT
    
public:
    MutiCamApp(QWidget* parent = nullptr);
    ~MutiCamApp();
    
    // 调试控制方法
    void enableDebugOutput();   // 开启调试输出
    void disableDebugOutput();  // 关闭调试输出
    bool isDebugOutputEnabled() const;  // 检查调试状态

protected:
    // 事件过滤器用于处理鼠标事件
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    // 主界面绘图工具槽函数
    void onDrawPointClicked();
    void onDrawStraightClicked();
    void onDrawSimpleCircleClicked();
    void onDrawParallelClicked();
    void onDraw2LineClicked();
    void onDrawFineCircleClicked();
    void onCan1StepDrawClicked();
    void onClearDrawingsClicked();
    void onSaveImageClicked();
    
    // 网格控制槽函数
    void onCancelGridsClicked();
    
    // 自动检测槽函数
    void onLineDetClicked();
    void onCircleDetClicked();
    void onCan1StepDetClicked();
    
    // 测量控制槽函数
    void onStartMeasureClicked();
    void onStopMeasureClicked();
    
    // 垂直视图槽函数
    void onVerticalDrawParallelClicked();
    void onVerticalCalibrationClicked();
    void onVerticalDrawFineCircleClicked();
    void onVerticalCan1StepDrawClicked();
    void onVerticalDrawStraightClicked();
    void onVerticalClearDrawingsClicked();
    void onVerticalDrawPointClicked();
    void onVerticalDraw2LineClicked();
    void onVerticalDrawSimpleCircleClicked();
    void onVerticalLineDetClicked();
    void onVerticalCircleDetClicked();
    void onVerticalCan1StepDetClicked();
    void onVerticalCancelGridsClicked();
    void onVerticalSaveImageClicked();
    
    // 左侧视图槽函数
    void onLeftClearDrawingsClicked();
    void onLeftDrawFineCircleClicked();
    void onLeftDrawParallelClicked();
    void onLeftCan1StepDrawClicked();
    void onLeftDrawStraightClicked();
    void onLeftDrawPointClicked();
    void onLeftDrawSimpleCircleClicked();
    void onLeftDraw2LineClicked();
    void onLeftCalibrationClicked();
    void onLeftLineDetClicked();
    void onLeftCircleDetClicked();
    void onLeftCan1StepDetClicked();
    void onLeftCancelGridsClicked();
    void onLeftSaveImageClicked();
    
    // 对向视图槽函数
    void onFrontDrawPointClicked();
    void onFrontDrawStraightClicked();
    void onFrontDrawSimpleCircleClicked();
    void onFrontDrawParallelClicked();
    void onFrontDraw2LineClicked();
    void onFrontDrawFineCircleClicked();
    void onFrontCan1StepDrawClicked();
    void onFrontClearDrawingsClicked();
    void onFrontCalibrationClicked();
    void onFrontLineDetClicked();
    void onFrontCircleDetClicked();
    void onFrontCan1StepDetClicked();
    void onFrontCancelGridsClicked();
    void onFrontSaveImageClicked();
    
    // 相机控制槽函数
    void onVerticalFrameReady(const QImage& image);
    void onLeftFrameReady(const QImage& image);
    void onFrontFrameReady(const QImage& image);
    void onCameraError(const QString& error);
    void onCameraConnected(const QString& serialNumber);
    void onCameraDisconnected(const QString& serialNumber);

private:
    void setupUI();
    void connectSignals();
    void initializeViews();
    void setupDrawingManager();
    void setupMouseEventFilters();
    
    // 鼠标事件处理
    void handleLabelMouseEvent(QLabel* label, QMouseEvent* event);
    void handleLabelWheelEvent(QLabel* label, QWheelEvent* event);
    
    // 相机相关方法
    void startCameras();
    void stopCameras();
    void displayImage(const QImage& image, QLabel* label);
    void initializeCameraThreads();
    void cleanupCameraThreads();
    
    // 获取当前帧的方法
    QImage getCurrentFrameForView(QLabel* label);
    void displayImageWithDrawings(const QImage& image, QLabel* label);
    
    // 高质量图像显示方法
    void displayImageHighQuality(const QImage& image, QLabel* label, double zoomFactor = 1.0);
    
    // UI 控件指针
    Ui_MutiCamApp* ui;
    
    // 主要的图像显示标签
    QLabel* lbVerticalView;
    QLabel* lbLeftView; 
    QLabel* lbFrontView;
    QLabel* lbVerticalView2;
    QLabel* lbLeftView2;
    QLabel* lbFrontView2;
    
    // 网格密度输入框
    QLineEdit* leGridDensity;
    QLineEdit* leGridDensVertical;
    QLineEdit* leGridDensLeft;
    QLineEdit* leGridDensFront;
    
    // 绘图管理器
    std::unique_ptr<DrawingManager> m_drawingManager;
    
    // 相机控制
    std::unique_ptr<CameraThread> m_verticalCameraThread;
    std::unique_ptr<CameraThread> m_leftCameraThread;
    std::unique_ptr<CameraThread> m_frontCameraThread;
    
    // 相机序列号（用户指定的序列号）
    QString m_verticalCameraSN;
    QString m_leftCameraSN;
    QString m_frontCameraSN;
    
    // 相机状态
    bool m_camerasRunning;
    
    // 当前帧存储
    QImage m_currentVerticalFrame;
    QImage m_currentLeftFrame; 
    QImage m_currentFrontFrame;
    
    // 视图缩放状态
    std::map<QLabel*, double> m_viewZoomFactors;  // 每个视图的缩放因子
};