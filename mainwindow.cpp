#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cvmattype.h"
#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_isUpdatingUI(false)
{
    // 注册cv::Mat类型到Qt元对象系统
    qRegisterMetaType<cv::Mat>("cv::Mat");
    
    ui->setupUi(this);
    
    // 创建设置管理器
    m_settingsManager = new SettingsManager(this);
    
    // 连接信号和槽
    connect(m_settingsManager, &SettingsManager::settingsLoaded,
            this, &MainWindow::updateUI);
            
    // 设置所有控件的连接
    setupConnections();
            
    // 加载设置
    m_settingsManager->loadSettings();
    
    // 设置图像显示标签的大小策略
    // 主界面标签
    ui->lbVerticalView->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->lbLeftView->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->lbFrontView->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    
    // 标签页标签
    ui->lbVerticalView_2->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->lbLeftView_2->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->lbFrontView_2->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    
    // 设置标签的对齐方式为居中
    ui->lbVerticalView->setAlignment(Qt::AlignCenter);
    ui->lbLeftView->setAlignment(Qt::AlignCenter);
    ui->lbFrontView->setAlignment(Qt::AlignCenter);
    ui->lbVerticalView_2->setAlignment(Qt::AlignCenter);
    ui->lbLeftView_2->setAlignment(Qt::AlignCenter);
    ui->lbFrontView_2->setAlignment(Qt::AlignCenter);
    
    // 初始化相机管理器
    initCameraManager();
    
    // 初始化绘图管理器
    initDrawingManager();
    
    // 初始UI状态
    ui->btnStartMeasure->setEnabled(true);
    ui->btnStopMeasure->setEnabled(false);
    
    // 为所有图像标签安装事件过滤器
    ui->lbVerticalView->installEventFilter(this);
    ui->lbLeftView->installEventFilter(this);
    ui->lbFrontView->installEventFilter(this);
    ui->lbVerticalView_2->installEventFilter(this);
    ui->lbLeftView_2->installEventFilter(this);
    ui->lbFrontView_2->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    // 停止图像抓取
    if (m_cameraManager)
    {
        m_cameraManager->stopAllGrabbing();
        m_cameraManager->disconnectAllCameras();
    }
    
    delete ui;
}

void MainWindow::setupConnections()
{
    // 连接所有LineEdit控件的editingFinished信号
    // 相机参数控件
    connect(ui->ledVerCamSN, &QLineEdit::editingFinished, this, &MainWindow::handleEditingFinished);
    connect(ui->ledLeftCamSN, &QLineEdit::editingFinished, this, &MainWindow::handleEditingFinished);
    connect(ui->ledFrontCamSN, &QLineEdit::editingFinished, this, &MainWindow::handleEditingFinished);
    
    // 线条检测参数控件
    connect(ui->ledCannyLineLow, &QLineEdit::editingFinished, this, &MainWindow::handleEditingFinished);
    connect(ui->ledCannyLineHigh, &QLineEdit::editingFinished, this, &MainWindow::handleEditingFinished);
    connect(ui->ledLineDetThreshold, &QLineEdit::editingFinished, this, &MainWindow::handleEditingFinished);
    connect(ui->ledLineDetMinLength, &QLineEdit::editingFinished, this, &MainWindow::handleEditingFinished);
    connect(ui->ledLineDetMaxGap, &QLineEdit::editingFinished, this, &MainWindow::handleEditingFinished);
    
    // 圆形检测参数控件
    connect(ui->ledCannyCircleLow, &QLineEdit::editingFinished, this, &MainWindow::handleEditingFinished);
    connect(ui->ledCannyCircleHigh, &QLineEdit::editingFinished, this, &MainWindow::handleEditingFinished);
    connect(ui->ledCircleDetParam2, &QLineEdit::editingFinished, this, &MainWindow::handleEditingFinished);
    
    // UI参数控件
    connect(ui->ledUIWidth, &QLineEdit::editingFinished, this, &MainWindow::handleEditingFinished);
    connect(ui->ledUIHeight, &QLineEdit::editingFinished, this, &MainWindow::handleEditingFinished);
    
    // 特别为UI宽高参数添加连接，当它们编辑完成时调整窗口大小
    connect(ui->ledUIWidth, &QLineEdit::editingFinished, this, &MainWindow::updateWindowSize);
    connect(ui->ledUIHeight, &QLineEdit::editingFinished, this, &MainWindow::updateWindowSize);
}

void MainWindow::updateUI(const QMap<QString, QVariant>& allSettings)
{
    // 标记开始更新UI，避免循环更新
    m_isUpdatingUI = true;
    
    // 保存当前设置
    m_currentSettings = allSettings;
    
    // 更新相机相关控件
    ui->ledVerCamSN->setText(allSettings["Camera/VerCamSN"].toString());
    ui->ledLeftCamSN->setText(allSettings["Camera/LeftCamSN"].toString());
    ui->ledFrontCamSN->setText(allSettings["Camera/FrontCamSN"].toString());
    
    // 更新线条检测相关控件
    ui->ledCannyLineLow->setText(allSettings["LineDetection/CannyLineLow"].toString());
    ui->ledCannyLineHigh->setText(allSettings["LineDetection/CannyLineHigh"].toString());
    ui->ledLineDetThreshold->setText(allSettings["LineDetection/LineDetThreshold"].toString());
    ui->ledLineDetMinLength->setText(allSettings["LineDetection/LineDetMinLength"].toString());
    ui->ledLineDetMaxGap->setText(allSettings["LineDetection/LineDetMaxGap"].toString());
    
    // 更新圆形检测相关控件
    ui->ledCannyCircleLow->setText(allSettings["CircleDetection/CannyCircleLow"].toString());
    ui->ledCannyCircleHigh->setText(allSettings["CircleDetection/CannyCircleHigh"].toString());
    ui->ledCircleDetParam2->setText(allSettings["CircleDetection/CircleDetParam2"].toString());
    
    // 更新UI相关控件
    ui->ledUIWidth->setText(allSettings["UI/Width"].toString());
    ui->ledUIHeight->setText(allSettings["UI/Height"].toString());
    
    // 更新窗口大小以匹配设置
    updateWindowSize();
    
    // 标记UI更新完成
    m_isUpdatingUI = false;
}

QMap<QString, QVariant> MainWindow::collectSettingsFromUI()
{
    QMap<QString, QVariant> settings = m_currentSettings;
    
    // 收集相机相关设置
    settings["Camera/VerCamSN"] = ui->ledVerCamSN->text();
    settings["Camera/LeftCamSN"] = ui->ledLeftCamSN->text();
    settings["Camera/FrontCamSN"] = ui->ledFrontCamSN->text();
    
    // 收集线条检测相关设置
    settings["LineDetection/CannyLineLow"] = ui->ledCannyLineLow->text().toInt();
    settings["LineDetection/CannyLineHigh"] = ui->ledCannyLineHigh->text().toInt();
    settings["LineDetection/LineDetThreshold"] = ui->ledLineDetThreshold->text().toInt();
    settings["LineDetection/LineDetMinLength"] = ui->ledLineDetMinLength->text().toInt();
    settings["LineDetection/LineDetMaxGap"] = ui->ledLineDetMaxGap->text().toInt();
    
    // 收集圆形检测相关设置
    settings["CircleDetection/CannyCircleLow"] = ui->ledCannyCircleLow->text().toInt();
    settings["CircleDetection/CannyCircleHigh"] = ui->ledCannyCircleHigh->text().toInt();
    settings["CircleDetection/CircleDetParam2"] = ui->ledCircleDetParam2->text().toInt();
    
    // 收集UI相关设置
    settings["UI/Width"] = ui->ledUIWidth->text().toInt();
    settings["UI/Height"] = ui->ledUIHeight->text().toInt();
    
    return settings;
}

void MainWindow::saveSettings()
{
    // 收集当前UI中的所有设置
    QMap<QString, QVariant> currentSettings = collectSettingsFromUI();
    
    // 保存设置
    m_settingsManager->saveSettings(currentSettings);
}

// 通用的编辑完成槽函数
void MainWindow::handleEditingFinished()
{
    // 如果正在更新UI，则不处理编辑完成信号
    if (m_isUpdatingUI) return;
    
    // 可以在这里加入一些调试信息
    QLineEdit* sender = qobject_cast<QLineEdit*>(QObject::sender());
    if (sender) {
        qDebug() << "控件" << sender->objectName() << "编辑完成，正在保存设置...";
    }
    
    saveSettings();
}

// 重写窗口大小改变事件
void MainWindow::resizeEvent(QResizeEvent *event)
{
    // 先调用父类的实现
    QMainWindow::resizeEvent(event);
    
    // 如果正在更新UI，则不更新参数（避免循环）
    if (m_isUpdatingUI) return;
    
    // 获取当前窗口大小
    QSize newSize = event->size();
    
    // 更新UI参数控件
    ui->ledUIWidth->setText(QString::number(newSize.width()));
    ui->ledUIHeight->setText(QString::number(newSize.height()));
    
    // 如果相机管理器存在，刷新图像显示
    if (m_cameraManager) {
        // 对于每个相机位置，如果有图像，重新调整大小并显示
        for (int pos = 0; pos <= 2; pos++) {
            CameraPosition position = static_cast<CameraPosition>(pos);
            if (m_cameraManager->isCameraConnected(position) && m_cameraManager->isCameraGrabbing(position)) {
                // 这里不需要做特殊处理，因为CameraManager::handleImageReady会处理图像缩放
                // 只需确保标签大小策略正确即可
            }
        }
    }
    
    // 保存设置
    saveSettings();
}

// 根据UI参数更新窗口大小
void MainWindow::updateWindowSize()
{
    // 获取UI参数中的宽高
    int width = ui->ledUIWidth->text().toInt();
    int height = ui->ledUIHeight->text().toInt();
    
    // 确保宽高合理
    // if (width < 640) width = 640;
    // if (height < 480) height = 480;
    
    // 调整窗口大小
    resize(width, height);
}

// 初始化相机管理器
void MainWindow::initCameraManager()
{
    // 创建相机管理器
    m_cameraManager = new CameraManager(this);
    
    // 连接信号和槽
    connect(m_cameraManager, &CameraManager::cameraConnectionChanged,
            this, &MainWindow::handleCameraConnectionChanged);
    connect(m_cameraManager, &CameraManager::cameraGrabbingChanged,
            this, &MainWindow::handleCameraGrabbingChanged);
    connect(m_cameraManager, &CameraManager::logMessage,
            this, &MainWindow::handleLogMessage);
    
    // 设置显示标签
    m_cameraManager->setDisplayLabel(VerticalCamera, ui->lbVerticalView, ui->lbVerticalView_2);
    m_cameraManager->setDisplayLabel(LeftCamera, ui->lbLeftView, ui->lbLeftView_2);
    m_cameraManager->setDisplayLabel(FrontCamera, ui->lbFrontView, ui->lbFrontView_2);
}

// 连接相机
void MainWindow::connectCameras()
{
    // 确保图像显示区域的大小策略正确
    ui->lbVerticalView->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->lbLeftView->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->lbFrontView->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->lbVerticalView_2->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->lbLeftView_2->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->lbFrontView_2->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    
    // 枚举设备
    if (!m_cameraManager->enumDevices())
    {
        QMessageBox::warning(this, "警告", "枚举设备失败，请检查相机连接");
        return;
    }
    
    // 获取相机序列号
    QString verCamSN = ui->ledVerCamSN->text();
    QString leftCamSN = ui->ledLeftCamSN->text();
    QString frontCamSN = ui->ledFrontCamSN->text();
    
    // 连接相机
    bool verConnected = m_cameraManager->connectCamera(VerticalCamera, verCamSN);
    bool leftConnected = m_cameraManager->connectCamera(LeftCamera, leftCamSN);
    bool frontConnected = m_cameraManager->connectCamera(FrontCamera, frontCamSN);
    
    // 检查是否至少有一个相机连接成功
    if (!verConnected && !leftConnected && !frontConnected)
    {
        QMessageBox::warning(this, "警告", "所有相机连接失败，请检查相机序列号和连接");
        return;
    }
    
    // 开始采集
    m_cameraManager->startAllGrabbing();
    
    // 更新按钮状态
    ui->btnStartMeasure->setEnabled(false);
    ui->btnStopMeasure->setEnabled(true);
}

// 开始测量按钮点击事件
void MainWindow::on_btnStartMeasure_clicked()
{
    // 连接相机并开始采集
    connectCameras();
}

// 停止测量按钮点击事件
void MainWindow::on_btnStopMeasure_clicked()
{
    // 停止采集并断开相机连接
    m_cameraManager->stopAllGrabbing();
    m_cameraManager->disconnectAllCameras();
    
    // 更新按钮状态
    ui->btnStartMeasure->setEnabled(true);
    ui->btnStopMeasure->setEnabled(false);
}

// 处理相机连接状态变化
void MainWindow::handleCameraConnectionChanged(CameraPosition position, bool isConnected)
{
    QString positionName;
    switch (position)
    {
    case VerticalCamera:
        positionName = "垂直相机";
        break;
    case LeftCamera:
        positionName = "左侧相机";
        break;
    case FrontCamera:
        positionName = "前置相机";
        break;
    default:
        positionName = "未知相机";
        break;
    }
    
    QString status = isConnected ? "已连接" : "已断开";
    ui->statusbar->showMessage(QString("%1 %2").arg(positionName).arg(status), 3000);
}

// 处理相机采集状态变化
void MainWindow::handleCameraGrabbingChanged(CameraPosition position, bool isGrabbing)
{
    QString positionName;
    switch (position)
    {
    case VerticalCamera:
        positionName = "垂直相机";
        break;
    case LeftCamera:
        positionName = "左侧相机";
        break;
    case FrontCamera:
        positionName = "前置相机";
        break;
    default:
        positionName = "未知相机";
        break;
    }
    
    QString status = isGrabbing ? "开始采集" : "停止采集";
    ui->statusbar->showMessage(QString("%1 %2").arg(positionName).arg(status), 3000);
}

// 处理日志消息
void MainWindow::handleLogMessage(const QString& message)
{
    // 在状态栏显示日志消息
    ui->statusbar->showMessage(message, 3000);
    
    // 也可以将日志消息添加到日志窗口或文件中
    qDebug() << message;
}

void MainWindow::initDrawingManager()
{
    // 创建绘图管理器
    m_drawingManager = new DrawingManager(this);
    
    // 为每个视图创建测量管理器
    MeasurementManager* verticalMeasurement = new MeasurementManager(this);
    MeasurementManager* leftMeasurement = new MeasurementManager(this);
    MeasurementManager* frontMeasurement = new MeasurementManager(this);
    MeasurementManager* verticalTabMeasurement = new MeasurementManager(this);
    MeasurementManager* leftTabMeasurement = new MeasurementManager(this);
    MeasurementManager* frontTabMeasurement = new MeasurementManager(this);
    
    // 添加测量管理器到绘图管理器
    m_drawingManager->addMeasurementManager(ui->lbVerticalView, verticalMeasurement);
    m_drawingManager->addMeasurementManager(ui->lbLeftView, leftMeasurement);
    m_drawingManager->addMeasurementManager(ui->lbFrontView, frontMeasurement);
    m_drawingManager->addMeasurementManager(ui->lbVerticalView_2, verticalTabMeasurement);
    m_drawingManager->addMeasurementManager(ui->lbLeftView_2, leftTabMeasurement);
    m_drawingManager->addMeasurementManager(ui->lbFrontView_2, frontTabMeasurement);
    
    // 添加视图对，用于同步绘制
    m_drawingManager->addViewPair(ui->lbVerticalView, ui->lbVerticalView_2);
    m_drawingManager->addViewPair(ui->lbLeftView, ui->lbLeftView_2);
    m_drawingManager->addViewPair(ui->lbFrontView, ui->lbFrontView_2);
    
    // 将绘图管理器关联到图像标签
    ui->lbVerticalView->setProperty("drawingManager", QVariant::fromValue(m_drawingManager));
    ui->lbLeftView->setProperty("drawingManager", QVariant::fromValue(m_drawingManager));
    ui->lbFrontView->setProperty("drawingManager", QVariant::fromValue(m_drawingManager));
    ui->lbVerticalView_2->setProperty("drawingManager", QVariant::fromValue(m_drawingManager));
    ui->lbLeftView_2->setProperty("drawingManager", QVariant::fromValue(m_drawingManager));
    ui->lbFrontView_2->setProperty("drawingManager", QVariant::fromValue(m_drawingManager));
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    // 处理图像标签的鼠标事件
    if (watched == ui->lbVerticalView || watched == ui->lbLeftView || watched == ui->lbFrontView ||
        watched == ui->lbVerticalView_2 || watched == ui->lbLeftView_2 || watched == ui->lbFrontView_2)
    {
        QLabel* label = qobject_cast<QLabel*>(watched);
        
        if (event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            m_drawingManager->handleMousePress(mouseEvent, label);
            return true;
        }
        else if (event->type() == QEvent::MouseMove)
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            m_drawingManager->handleMouseMove(mouseEvent, label);
            return true;
        }
        else if (event->type() == QEvent::MouseButtonRelease)
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            m_drawingManager->handleMouseRelease(mouseEvent, label);
            return true;
        }
    }
    
    return QMainWindow::eventFilter(watched, event);
}

cv::Mat MainWindow::getCurrentFrameForView(QLabel* label)
{
    QVariant frameVariant = label->property("currentFrame");
    if (frameVariant.isValid())
    {
        return frameVariant.value<cv::Mat>();
    }
    return cv::Mat();
}

void MainWindow::updateView(QLabel* label, const cv::Mat& frame)
{
    if (!frame.empty())
    {
        QImage img(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
        img = img.rgbSwapped();
        label->setPixmap(QPixmap::fromImage(img));
        
        // 保存当前帧
        label->setProperty("currentDisplayFrame", QVariant::fromValue(frame));
    }
}

// 绘图相关槽函数
void MainWindow::on_btnDrawPoint_Ver_clicked()
{
    m_drawingManager->startPointMeasurement();
}

void MainWindow::on_btnDrawPoint_Left_clicked()
{
    m_drawingManager->startPointMeasurement();
}

void MainWindow::on_btnDrawPoint_Front_clicked()
{
    m_drawingManager->startPointMeasurement();
}

void MainWindow::on_btnDrawPoint_clicked()
{
    m_drawingManager->startPointMeasurement();
}
