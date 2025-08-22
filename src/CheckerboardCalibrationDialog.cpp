#include "CheckerboardCalibrationDialog.h"
#include <QApplication>

CheckerboardCalibrationDialog::CheckerboardCalibrationDialog(QWidget *parent)
    : QDialog(parent)
    , m_presetCombo(nullptr)
    , m_cornersXSpinBox(nullptr)
    , m_cornersYSpinBox(nullptr)
    , m_squareSizeSpinBox(nullptr)
    , m_unitCombo(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
{
    initializeUI();
}

CheckerboardCalibrationDialog::CheckerboardParams CheckerboardCalibrationDialog::getParams() const
{
    return m_params;
}

void CheckerboardCalibrationDialog::initializeUI()
{
    setWindowTitle("棋盘格标定参数");
    setModal(true);
    setFixedSize(400, 350);
    
    // 创建主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // 添加说明标签
    QLabel* descLabel = new QLabel("棋盘格标定使用说明：\n"
                                  "1. 确保棋盘格图案清晰，黑白对比明显\n"
                                  "2. 棋盘格完全在视野范围内，占据50-80%视野\n"
                                  "3. 棋盘格应贴在平整表面，避免弯曲变形\n"
                                  "4. 光照均匀，避免反光和阴影\n"
                                  "5. 内角点数 = (棋盘格行数-1) × (列数-1)", this);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("color: #666; font-size: 10px; background-color: #f5f5f5; "
                            "padding: 8px; border: 1px solid #ddd; border-radius: 4px;");
    mainLayout->addWidget(descLabel);
    
    // 添加预设选择组
    QGroupBox* presetGroup = createPresetGroup();
    mainLayout->addWidget(presetGroup);
    
    // 添加参数输入组
    QGroupBox* paramGroup = createParameterGroup();
    mainLayout->addWidget(paramGroup);
    
    // 添加弹簧，将按钮推到底部
    mainLayout->addStretch();
    
    // 添加按钮组
    QWidget* buttonWidget = createButtonGroup();
    mainLayout->addWidget(buttonWidget);
    
    // 连接信号
    connect(m_okButton, &QPushButton::clicked, this, &CheckerboardCalibrationDialog::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &CheckerboardCalibrationDialog::onCancelClicked);
    connect(m_presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &CheckerboardCalibrationDialog::onPresetChanged);
    
    // 设置默认值
    updateParameters(9, 7, 1.0, "mm");
}

QGroupBox* CheckerboardCalibrationDialog::createPresetGroup()
{
    QGroupBox* groupBox = new QGroupBox("常用预设", this);
    QVBoxLayout* layout = new QVBoxLayout(groupBox);
    
    m_presetCombo = new QComboBox(this);
    m_presetCombo->addItem("自定义");
    m_presetCombo->addItem("标准棋盘格 9x7 (1mm方格)");
    m_presetCombo->addItem("标准棋盘格 8x6 (1mm方格)");
    m_presetCombo->addItem("大尺寸棋盘格 9x7 (5mm方格)");
    m_presetCombo->addItem("小尺寸棋盘格 9x7 (0.5mm方格)");
    
    layout->addWidget(m_presetCombo);
    
    return groupBox;
}

QGroupBox* CheckerboardCalibrationDialog::createParameterGroup()
{
    QGroupBox* groupBox = new QGroupBox("标定参数", this);
    QFormLayout* layout = new QFormLayout(groupBox);
    
    // X方向内角点数
    m_cornersXSpinBox = new QSpinBox(this);
    m_cornersXSpinBox->setRange(3, 20);
    m_cornersXSpinBox->setValue(9);
    layout->addRow("X方向内角点数:", m_cornersXSpinBox);
    
    // Y方向内角点数
    m_cornersYSpinBox = new QSpinBox(this);
    m_cornersYSpinBox->setRange(3, 20);
    m_cornersYSpinBox->setValue(7);
    layout->addRow("Y方向内角点数:", m_cornersYSpinBox);
    
    // 方格尺寸
    m_squareSizeSpinBox = new QDoubleSpinBox(this);
    m_squareSizeSpinBox->setRange(0.001, 1000.0);
    m_squareSizeSpinBox->setDecimals(3);
    m_squareSizeSpinBox->setValue(1.0);
    layout->addRow("方格尺寸:", m_squareSizeSpinBox);
    
    // 单位
    m_unitCombo = new QComboBox(this);
    m_unitCombo->addItems({"mm", "μm", "cm", "inch"});
    layout->addRow("单位:", m_unitCombo);
    
    // 添加说明
    QLabel* noteLabel = new QLabel("注意：内角点数 = 棋盘格行数-1 × 列数-1", this);
    noteLabel->setStyleSheet("color: #888; font-size: 9px; font-style: italic;");
    layout->addRow(noteLabel);
    
    return groupBox;
}

QWidget* CheckerboardCalibrationDialog::createButtonGroup()
{
    QWidget* buttonWidget = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(buttonWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // 添加弹簧，将按钮推到右侧
    layout->addStretch();
    
    // 取消按钮
    m_cancelButton = new QPushButton("取消", this);
    m_cancelButton->setMinimumSize(80, 30);
    layout->addWidget(m_cancelButton);
    
    // 确定按钮
    m_okButton = new QPushButton("开始检测", this);
    m_okButton->setMinimumSize(80, 30);
    m_okButton->setDefault(true);
    layout->addWidget(m_okButton);
    
    return buttonWidget;
}

void CheckerboardCalibrationDialog::updateParameters(int cornersX, int cornersY, double squareSize, const QString& unit)
{
    m_cornersXSpinBox->setValue(cornersX);
    m_cornersYSpinBox->setValue(cornersY);
    m_squareSizeSpinBox->setValue(squareSize);
    
    int unitIndex = m_unitCombo->findText(unit);
    if (unitIndex >= 0) {
        m_unitCombo->setCurrentIndex(unitIndex);
    }
}

void CheckerboardCalibrationDialog::onPresetChanged()
{
    int index = m_presetCombo->currentIndex();
    
    switch (index) {
        case 1: // 标准棋盘格 9x7 (1mm方格)
            updateParameters(9, 7, 1.0, "mm");
            break;
        case 2: // 标准棋盘格 8x6 (1mm方格)
            updateParameters(8, 6, 1.0, "mm");
            break;
        case 3: // 大尺寸棋盘格 9x7 (5mm方格)
            updateParameters(9, 7, 5.0, "mm");
            break;
        case 4: // 小尺寸棋盘格 9x7 (0.5mm方格)
            updateParameters(9, 7, 0.5, "mm");
            break;
        default: // 自定义
            break;
    }
}

void CheckerboardCalibrationDialog::onOkClicked()
{
    // 获取当前参数
    m_params.cornersX = m_cornersXSpinBox->value();
    m_params.cornersY = m_cornersYSpinBox->value();
    m_params.squareSize = m_squareSizeSpinBox->value();
    m_params.unit = m_unitCombo->currentText();
    
    accept();
}

void CheckerboardCalibrationDialog::onCancelClicked()
{
    reject();
}
