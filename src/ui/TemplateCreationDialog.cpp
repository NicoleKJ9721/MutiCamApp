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
    , m_errorLabel(nullptr)
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
    mainLayout->setSpacing(10);
    
    // 模板名称输入
    QGroupBox* nameGroup = new QGroupBox("模板名称", this);
    nameGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QFormLayout* nameLayout = new QFormLayout(nameGroup);
    nameLayout->setContentsMargins(10, 10, 10, 10);
    
    m_templateNameEdit = new QLineEdit(this);
    m_templateNameEdit->setPlaceholderText("请输入模板名称");
    m_templateNameEdit->setToolTip("模板的唯一标识名称。\n用于保存和识别不同的模板文件。\n建议使用有意义的描述性名称。");
    nameLayout->addRow("名称:", m_templateNameEdit);
    
    mainLayout->addWidget(nameGroup);
    
    // 角度范围设置
    QGroupBox* angleGroup = new QGroupBox("角度范围", this);
    angleGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QFormLayout* angleLayout = new QFormLayout(angleGroup);
    angleLayout->setContentsMargins(10, 10, 10, 10);
    
    m_angleStartSpinBox = new QDoubleSpinBox(this);
    m_angleStartSpinBox->setRange(-180.0, 180.0);
    m_angleStartSpinBox->setValue(-45.0);
    m_angleStartSpinBox->setSuffix("°");
    m_angleStartSpinBox->setToolTip("模板匹配时允许的最小旋转角度。\n负值表示逆时针旋转。\n范围越大，匹配越灵活，但速度会变慢。\n推荐值：-45°到-90°");
    angleLayout->addRow("起始角度:", m_angleStartSpinBox);
    
    m_angleEndSpinBox = new QDoubleSpinBox(this);
    m_angleEndSpinBox->setRange(-180.0, 180.0);
    m_angleEndSpinBox->setValue(45.0);
    m_angleEndSpinBox->setSuffix("°");
    m_angleEndSpinBox->setToolTip("模板匹配时允许的最大旋转角度。\n正值表示顺时针旋转。\n必须大于起始角度。\n推荐值：45°到90°");
    angleLayout->addRow("结束角度:", m_angleEndSpinBox);
    
    m_angleStepSpinBox = new QDoubleSpinBox(this);
    m_angleStepSpinBox->setRange(1.0, 45.0);
    m_angleStepSpinBox->setValue(15.0);
    m_angleStepSpinBox->setSuffix("°");
    m_angleStepSpinBox->setToolTip("模板旋转时的角度间隔。\n步长越小，角度识别越精确，但创建和匹配时间会显著增加。\n推荐值：10°-20°，精确场合可用5°");
    angleLayout->addRow("角度步长:", m_angleStepSpinBox);
    
    mainLayout->addWidget(angleGroup);
    
    // 缩放范围设置
    QGroupBox* scaleGroup = new QGroupBox("缩放范围", this);
    scaleGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QFormLayout* scaleLayout = new QFormLayout(scaleGroup);
    scaleLayout->setContentsMargins(10, 10, 10, 10);
    
    m_scaleStartSpinBox = new QDoubleSpinBox(this);
    m_scaleStartSpinBox->setRange(0.1, 5.0);
    m_scaleStartSpinBox->setValue(0.9);
    m_scaleStartSpinBox->setDecimals(2);
    m_scaleStartSpinBox->setToolTip("模板匹配时允许的最小缩放比例。\n1.0表示原始大小，小于1.0表示缩小。\n范围越大，对尺寸变化的适应性越强。\n推荐值：0.8-0.95");
    scaleLayout->addRow("起始缩放:", m_scaleStartSpinBox);
    
    m_scaleEndSpinBox = new QDoubleSpinBox(this);
    m_scaleEndSpinBox->setRange(0.1, 5.0);
    m_scaleEndSpinBox->setValue(1.1);
    m_scaleEndSpinBox->setDecimals(2);
    m_scaleEndSpinBox->setToolTip("模板匹配时允许的最大缩放比例。\n大于1.0表示放大，必须大于起始缩放。\n过大的范围会增加误匹配风险。\n推荐值：1.05-1.2");
    scaleLayout->addRow("结束缩放:", m_scaleEndSpinBox);
    
    m_scaleStepSpinBox = new QDoubleSpinBox(this);
    m_scaleStepSpinBox->setRange(0.01, 1.0);
    m_scaleStepSpinBox->setValue(0.1);
    m_scaleStepSpinBox->setDecimals(2);
    m_scaleStepSpinBox->setToolTip("缩放变化的步长间隔。\n步长越小，尺寸识别越精确，但处理时间会增加。\n推荐值：0.05-0.1，精确场合可用0.02");
    scaleLayout->addRow("缩放步长:", m_scaleStepSpinBox);
    
    mainLayout->addWidget(scaleGroup);
    
    // 其他参数设置
    QGroupBox* otherGroup = new QGroupBox("其他参数", this);
    otherGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QFormLayout* otherLayout = new QFormLayout(otherGroup);
    otherLayout->setContentsMargins(10, 10, 10, 10);
    
    m_numFeaturesSpinBox = new QSpinBox(this);
    m_numFeaturesSpinBox->setRange(50, 500);
    m_numFeaturesSpinBox->setValue(100);
    m_numFeaturesSpinBox->setToolTip("模板中提取的特征点数量。\n数量越多，模板越稳定可靠，但创建和匹配速度会变慢。\n过少可能导致匹配不稳定，过多会影响性能。\n推荐值：100-200，复杂图案可用300+");
    otherLayout->addRow("特征点数量:", m_numFeaturesSpinBox);
    
    m_weakThreshSpinBox = new QDoubleSpinBox(this);
    m_weakThreshSpinBox->setRange(10.0, 100.0);
    m_weakThreshSpinBox->setValue(30.0);
    m_weakThreshSpinBox->setToolTip("弱梯度阈值，用于检测较弱的边缘特征。\n值越低，检测到的边缘越多，但可能包含噪声。\n必须小于强梯度阈值。\n推荐值：20-40，噪声环境可适当提高");
    otherLayout->addRow("弱梯度阈值:", m_weakThreshSpinBox);
    
    m_strongThreshSpinBox = new QDoubleSpinBox(this);
    m_strongThreshSpinBox->setRange(30.0, 150.0);
    m_strongThreshSpinBox->setValue(60.0);
    m_strongThreshSpinBox->setToolTip("强梯度阈值，用于确定最可靠的边缘特征。\n值越高，找到的边缘越少但越清晰可靠。\n必须大于弱梯度阈值。\n推荐值：50-80，清晰图像可用更高值");
    otherLayout->addRow("强梯度阈值:", m_strongThreshSpinBox);
    
    mainLayout->addWidget(otherGroup);
    
    // 错误提示标签
    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet("color: red; font-weight: bold;");
    m_errorLabel->setWordWrap(true);
    m_errorLabel->hide(); // 初始隐藏
    m_errorLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    mainLayout->addWidget(m_errorLabel);
    
    // 添加弹性空间，将按钮推到底部
    mainLayout->addStretch();
    
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
    
    // 清除所有输入框的错误样式
    m_templateNameEdit->setStyleSheet("");
    m_angleStartSpinBox->setStyleSheet("");
    m_angleEndSpinBox->setStyleSheet("");
    m_angleStepSpinBox->setStyleSheet("");
    m_scaleStartSpinBox->setStyleSheet("");
    m_scaleEndSpinBox->setStyleSheet("");
    m_scaleStepSpinBox->setStyleSheet("");
    m_numFeaturesSpinBox->setStyleSheet("");
    m_weakThreshSpinBox->setStyleSheet("");
    m_strongThreshSpinBox->setStyleSheet("");
    
    // 验证模板名称
    QString templateName = m_templateNameEdit->text().trimmed();
    if (templateName.isEmpty()) {
        isValid = false;
        errorMsg = "错误：模板名称不能为空";
        m_templateNameEdit->setStyleSheet("border: 2px solid red;");
    }
    
    // 验证角度范围
    if (m_angleStartSpinBox->value() >= m_angleEndSpinBox->value()) {
        isValid = false;
        errorMsg = "错误：角度起始值必须小于结束值";
        m_angleStartSpinBox->setStyleSheet("background-color: #ffe6e6;");
        m_angleEndSpinBox->setStyleSheet("background-color: #ffe6e6;");
    }
    
    // 验证角度步长
    if (m_angleStepSpinBox->value() <= 0) {
        isValid = false;
        errorMsg = "错误：角度步长必须大于0";
        m_angleStepSpinBox->setStyleSheet("background-color: #ffe6e6;");
    }
    
    // 验证缩放范围
    if (m_scaleStartSpinBox->value() >= m_scaleEndSpinBox->value()) {
        isValid = false;
        errorMsg = "错误：缩放起始值必须小于结束值";
        m_scaleStartSpinBox->setStyleSheet("background-color: #ffe6e6;");
        m_scaleEndSpinBox->setStyleSheet("background-color: #ffe6e6;");
    }
    
    // 验证缩放步长
    if (m_scaleStepSpinBox->value() <= 0) {
        isValid = false;
        errorMsg = "错误：缩放步长必须大于0";
        m_scaleStepSpinBox->setStyleSheet("background-color: #ffe6e6;");
    }
    
    // 验证特征点数量
    if (m_numFeaturesSpinBox->value() <= 0) {
        isValid = false;
        errorMsg = "错误：特征点数量必须大于0";
        m_numFeaturesSpinBox->setStyleSheet("background-color: #ffe6e6;");
    }
    
    // 验证梯度阈值
    if (m_weakThreshSpinBox->value() >= m_strongThreshSpinBox->value()) {
        isValid = false;
        errorMsg = "错误：弱梯度阈值必须小于强梯度阈值";
        m_weakThreshSpinBox->setStyleSheet("background-color: #ffe6e6;");
        m_strongThreshSpinBox->setStyleSheet("background-color: #ffe6e6;");
    }
    
    // 更新错误标签显示
    if (!isValid && !errorMsg.isEmpty()) {
        m_errorLabel->setText(errorMsg);
        m_errorLabel->show();
    } else {
        m_errorLabel->hide();
    }
    
    // 更新OK按钮状态
    m_okButton->setEnabled(isValid);
}
