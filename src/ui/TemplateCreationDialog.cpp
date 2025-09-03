#include "TemplateCreationDialog.h"
#include <QMessageBox>

TemplateCreationDialog::TemplateCreationDialog(QWidget* parent)
    : QDialog(parent)
    , m_templateNameEdit(nullptr)
    , m_angleStartSpinBox(nullptr)
    , m_angleEndSpinBox(nullptr)
    , m_angleStepSpinBox(nullptr)
    , m_scaleStartSpinBox(nullptr)
    , m_scaleEndSpinBox(nullptr)
    , m_scaleStepSpinBox(nullptr)
    , m_numFeaturesSpinBox(nullptr)
    , m_weakThreshSpinBox(nullptr)
    , m_strongThreshSpinBox(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
{
    setWindowTitle("创建模板");
    setModal(true);
    resize(400, 500);
    
    setupUI();
    connectSignals();
}

void TemplateCreationDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 模板名称输入
    QGroupBox* nameGroup = new QGroupBox("模板名称", this);
    QFormLayout* nameLayout = new QFormLayout(nameGroup);
    
    m_templateNameEdit = new QLineEdit(this);
    m_templateNameEdit->setPlaceholderText("请输入模板名称");
    nameLayout->addRow("名称:", m_templateNameEdit);
    
    mainLayout->addWidget(nameGroup);
    
    // 角度范围设置
    QGroupBox* angleGroup = new QGroupBox("角度范围", this);
    QFormLayout* angleLayout = new QFormLayout(angleGroup);
    
    m_angleStartSpinBox = new QDoubleSpinBox(this);
    m_angleStartSpinBox->setRange(-180.0, 180.0);
    m_angleStartSpinBox->setValue(-45.0);
    m_angleStartSpinBox->setSuffix("°");
    angleLayout->addRow("起始角度:", m_angleStartSpinBox);
    
    m_angleEndSpinBox = new QDoubleSpinBox(this);
    m_angleEndSpinBox->setRange(-180.0, 180.0);
    m_angleEndSpinBox->setValue(45.0);
    m_angleEndSpinBox->setSuffix("°");
    angleLayout->addRow("结束角度:", m_angleEndSpinBox);
    
    m_angleStepSpinBox = new QDoubleSpinBox(this);
    m_angleStepSpinBox->setRange(1.0, 45.0);
    m_angleStepSpinBox->setValue(15.0);
    m_angleStepSpinBox->setSuffix("°");
    angleLayout->addRow("角度步长:", m_angleStepSpinBox);
    
    mainLayout->addWidget(angleGroup);
    
    // 缩放范围设置
    QGroupBox* scaleGroup = new QGroupBox("缩放范围", this);
    QFormLayout* scaleLayout = new QFormLayout(scaleGroup);
    
    m_scaleStartSpinBox = new QDoubleSpinBox(this);
    m_scaleStartSpinBox->setRange(0.1, 5.0);
    m_scaleStartSpinBox->setValue(0.9);
    m_scaleStartSpinBox->setDecimals(2);
    scaleLayout->addRow("起始缩放:", m_scaleStartSpinBox);
    
    m_scaleEndSpinBox = new QDoubleSpinBox(this);
    m_scaleEndSpinBox->setRange(0.1, 5.0);
    m_scaleEndSpinBox->setValue(1.1);
    m_scaleEndSpinBox->setDecimals(2);
    scaleLayout->addRow("结束缩放:", m_scaleEndSpinBox);
    
    m_scaleStepSpinBox = new QDoubleSpinBox(this);
    m_scaleStepSpinBox->setRange(0.01, 1.0);
    m_scaleStepSpinBox->setValue(0.1);
    m_scaleStepSpinBox->setDecimals(2);
    scaleLayout->addRow("缩放步长:", m_scaleStepSpinBox);
    
    mainLayout->addWidget(scaleGroup);
    
    // 其他参数设置
    QGroupBox* otherGroup = new QGroupBox("其他参数", this);
    QFormLayout* otherLayout = new QFormLayout(otherGroup);
    
    m_numFeaturesSpinBox = new QSpinBox(this);
    m_numFeaturesSpinBox->setRange(50, 500);
    m_numFeaturesSpinBox->setValue(100);
    otherLayout->addRow("特征点数量:", m_numFeaturesSpinBox);
    
    m_weakThreshSpinBox = new QDoubleSpinBox(this);
    m_weakThreshSpinBox->setRange(10.0, 100.0);
    m_weakThreshSpinBox->setValue(30.0);
    otherLayout->addRow("弱梯度阈值:", m_weakThreshSpinBox);
    
    m_strongThreshSpinBox = new QDoubleSpinBox(this);
    m_strongThreshSpinBox->setRange(30.0, 150.0);
    m_strongThreshSpinBox->setValue(60.0);
    otherLayout->addRow("强梯度阈值:", m_strongThreshSpinBox);
    
    mainLayout->addWidget(otherGroup);
    
    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_okButton = new QPushButton("确定", this);
    m_cancelButton = new QPushButton("取消", this);
    
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
}

void TemplateCreationDialog::connectSignals()
{
    // 连接模板名称输入框信号
    connect(m_templateNameEdit, &QLineEdit::textChanged,
            this, &TemplateCreationDialog::onTextChanged);
    
    // 连接参数变化信号进行验证
    connect(m_angleStartSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TemplateCreationDialog::validateParameters);
    connect(m_angleEndSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TemplateCreationDialog::validateParameters);
    connect(m_angleStepSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TemplateCreationDialog::validateParameters);
    
    connect(m_scaleStartSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TemplateCreationDialog::validateParameters);
    connect(m_scaleEndSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TemplateCreationDialog::validateParameters);
    connect(m_scaleStepSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TemplateCreationDialog::validateParameters);
    
    connect(m_numFeaturesSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &TemplateCreationDialog::validateParameters);
    connect(m_strongThreshSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TemplateCreationDialog::validateParameters);
    connect(m_weakThreshSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TemplateCreationDialog::validateParameters);
    
    // 连接按钮信号
    connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void TemplateCreationDialog::setParameters(const TemplateCreationParams& params)
{
    // 设置角度范围
    m_angleStartSpinBox->setValue(params.angle_range.begin);
    m_angleEndSpinBox->setValue(params.angle_range.end);
    m_angleStepSpinBox->setValue(params.angle_range.step);
    
    // 设置缩放范围
    m_scaleStartSpinBox->setValue(params.scale_range.begin);
    m_scaleEndSpinBox->setValue(params.scale_range.end);
    m_scaleStepSpinBox->setValue(params.scale_range.step);
    
    // 设置其他参数
    m_numFeaturesSpinBox->setValue(params.num_features);
    m_weakThreshSpinBox->setValue(params.weak_thresh);
    m_strongThreshSpinBox->setValue(params.strong_thresh);
}

TemplateCreationParams TemplateCreationDialog::getParameters() const
{
    TemplateCreationParams params;
    
    // 获取角度范围
    params.angle_range.begin = static_cast<float>(m_angleStartSpinBox->value());
    params.angle_range.end = static_cast<float>(m_angleEndSpinBox->value());
    params.angle_range.step = static_cast<float>(m_angleStepSpinBox->value());
    
    // 获取缩放范围
    params.scale_range.begin = static_cast<float>(m_scaleStartSpinBox->value());
    params.scale_range.end = static_cast<float>(m_scaleEndSpinBox->value());
    params.scale_range.step = static_cast<float>(m_scaleStepSpinBox->value());
    
    // 获取其他参数
    params.num_features = m_numFeaturesSpinBox->value();
    params.weak_thresh = static_cast<float>(m_weakThreshSpinBox->value());
    params.strong_thresh = static_cast<float>(m_strongThreshSpinBox->value());
    
    return params;
}

QString TemplateCreationDialog::getTemplateName() const
{
    return m_templateNameEdit->text().trimmed();
}

void TemplateCreationDialog::onTextChanged()
{
    validateParameters();
}

void TemplateCreationDialog::validateParameters()
{
    bool isValid = true;
    QString errorMsg;
    
    // 验证模板名称
    QString templateName = m_templateNameEdit->text().trimmed();
    if (templateName.isEmpty()) {
        isValid = false;
        errorMsg = "模板名称不能为空";
    }
    
    // 验证角度范围
    if (m_angleStartSpinBox->value() >= m_angleEndSpinBox->value()) {
        isValid = false;
        errorMsg = "角度起始值必须小于结束值";
    }
    
    // 验证缩放范围
    if (m_scaleStartSpinBox->value() >= m_scaleEndSpinBox->value()) {
        isValid = false;
        errorMsg = "缩放起始值必须小于结束值";
    }
    
    // 验证梯度阈值
    if (m_weakThreshSpinBox->value() >= m_strongThreshSpinBox->value()) {
        isValid = false;
        errorMsg = "弱梯度阈值必须小于强梯度阈值";
    }
    
    // 更新OK按钮状态
    m_okButton->setEnabled(isValid);
    
    // 显示错误信息（可选）
    if (!isValid && !errorMsg.isEmpty()) {
        m_okButton->setToolTip(errorMsg);
    } else {
        m_okButton->setToolTip("");
    }
}
