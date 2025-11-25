#include "TemplateCreationDialog.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QMessageBox>
#include <QApplication>

TemplateCreationDialog::TemplateCreationDialog(QWidget* parent) 
    : QDialog(parent)
    , nameValid_(false)
    , angleRangeValid_(true)
    , scaleRangeValid_(true)
    , parametersValid_(true) {
    
    setWindowTitle("创建模板");
    setModal(true);
    setFixedSize(450, 600);  // 增加高度以容纳新参数
    
    setupUI();
    setupValidation();
    loadConfigFromFile();  // 从配置文件加载参数
    
    // 初始化时设置名称输入框为错误状态（因为为空）
    setInputError(nameEdit_, true);
    validateAllInputs();
}

void TemplateCreationDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 模板名称组
    QGroupBox* nameGroup = new QGroupBox("模板信息");
    QHBoxLayout* nameLayout = new QHBoxLayout(nameGroup);
    
    QLabel* nameLabel = new QLabel("模板名称:");
    nameEdit_ = new QLineEdit();
    nameEdit_->setPlaceholderText("例如: 螺丝_型号A");
    nameEdit_->setToolTip("输入模板的唯一标识名称，用于后续识别和管理");
    
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(nameEdit_);
    mainLayout->addWidget(nameGroup);
    
    // 角度范围组
    QGroupBox* angleGroup = new QGroupBox("角度范围设置");
    QGridLayout* angleLayout = new QGridLayout(angleGroup);
    
    angleLayout->addWidget(new QLabel("开始角度:"), 0, 0);
    angleStartSpin_ = new QDoubleSpinBox();
    angleStartSpin_->setRange(-180.0, 180.0);
    angleStartSpin_->setSingleStep(1.0);  // 设置单步调整幅度为1度
    angleStartSpin_->setSuffix("°");
    angleStartSpin_->setToolTip("模板匹配的起始旋转角度，对应Halcon的AngleStart参数");
    angleLayout->addWidget(angleStartSpin_, 0, 1);
    
    angleLayout->addWidget(new QLabel("角度范围:"), 0, 2);
    angleExtentSpin_ = new QDoubleSpinBox();
    angleExtentSpin_->setRange(0.0, 360.0);
    angleExtentSpin_->setSingleStep(5.0);  // 设置单步调整幅度为5度
    angleExtentSpin_->setSuffix("°");
    angleExtentSpin_->setToolTip("模板匹配的角度搜索范围，对应Halcon的AngleExtent参数");
    angleLayout->addWidget(angleExtentSpin_, 0, 3);
    
    angleLayout->addWidget(new QLabel("角度步长:"), 1, 0);
    angleStepSpin_ = new QDoubleSpinBox();
    angleStepSpin_->setRange(0.1, 45.0);
    angleStepSpin_->setSingleStep(0.5);  // 设置单步调整幅度为0.5度
    angleStepSpin_->setSuffix("°");
    angleStepSpin_->setToolTip("角度搜索的步长，值越小精度越高但速度越慢，建议1-10°");
    angleLayout->addWidget(angleStepSpin_, 1, 1);
    
    mainLayout->addWidget(angleGroup);
    
    // 缩放范围组
    QGroupBox* scaleGroup = new QGroupBox("缩放范围设置");
    QGridLayout* scaleLayout = new QGridLayout(scaleGroup);
    
    scaleLayout->addWidget(new QLabel("最小缩放:"), 0, 0);
    scaleMinSpin_ = new QDoubleSpinBox();
    scaleMinSpin_->setRange(0.1, 5.0);
    scaleMinSpin_->setDecimals(2);
    scaleMinSpin_->setSingleStep(0.1);  // 设置单步调整幅度为0.1
    scaleMinSpin_->setToolTip("模板匹配的最小缩放比例，对应Halcon的ScaleMin参数");
    scaleLayout->addWidget(scaleMinSpin_, 0, 1);
    
    scaleLayout->addWidget(new QLabel("最大缩放:"), 0, 2);
    scaleMaxSpin_ = new QDoubleSpinBox();
    scaleMaxSpin_->setRange(0.1, 5.0);
    scaleMaxSpin_->setDecimals(2);
    scaleMaxSpin_->setSingleStep(0.1);  // 设置单步调整幅度为0.1
    scaleMaxSpin_->setToolTip("模板匹配的最大缩放比例，对应Halcon的ScaleMax参数");
    scaleLayout->addWidget(scaleMaxSpin_, 0, 3);
    
    scaleLayout->addWidget(new QLabel("缩放步长:"), 1, 0);
    scaleStepSpin_ = new QDoubleSpinBox();
    scaleStepSpin_->setRange(0.01, 1.0);
    scaleStepSpin_->setDecimals(2);
    scaleStepSpin_->setSingleStep(0.01);  // 设置单步调整幅度为0.01
    scaleStepSpin_->setToolTip("缩放搜索的步长，值越小精度越高但速度越慢，建议0.05-0.1");
    scaleLayout->addWidget(scaleStepSpin_, 1, 1);
    
    mainLayout->addWidget(scaleGroup);
    
    // Halcon参数组
    QGroupBox* halconGroup = new QGroupBox("Halcon参数");
    QGridLayout* halconLayout = new QGridLayout(halconGroup);
    
    halconLayout->addWidget(new QLabel("金字塔层数:"), 0, 0);
    numLevelsSpin_ = new QSpinBox();
    numLevelsSpin_->setRange(1, 10);
    numLevelsSpin_->setToolTip("形状模型的金字塔层数，对应Halcon的NumLevels参数");
    halconLayout->addWidget(numLevelsSpin_, 0, 1);
    
    halconLayout->addWidget(new QLabel("优化方式:"), 0, 2);
    optimizationCombo_ = new QComboBox();
    optimizationCombo_->setToolTip("形状模型优化方式，auto为自动选择");
    halconLayout->addWidget(optimizationCombo_, 0, 3);
    
    halconLayout->addWidget(new QLabel("极性度量:"), 1, 0);
    metricCombo_ = new QComboBox();
    metricCombo_->setToolTip("极性度量方式，影响匹配的敏感度");
    halconLayout->addWidget(metricCombo_, 1, 1);
    
    halconLayout->addWidget(new QLabel("对比度:"), 1, 2);
    contrastCombo_ = new QComboBox();
    contrastCombo_->setToolTip("对比度参数，auto为自动选择");
    halconLayout->addWidget(contrastCombo_, 1, 3);
    
    halconLayout->addWidget(new QLabel("最小对比度:"), 2, 0);
    minContrastCombo_ = new QComboBox();
    minContrastCombo_->setToolTip("最小对比度阈值，auto为自动选择");
    halconLayout->addWidget(minContrastCombo_, 2, 1);
    
    // 设置Halcon参数组合框选项
    setupHalconParameterCombos();
    
    mainLayout->addWidget(halconGroup);
    
    // 错误信息标签 - 设置固定高度避免布局跳动
    errorLabel_ = new QLabel();
    errorLabel_->setStyleSheet("QLabel { color: red; font-weight: bold; }");
    errorLabel_->setWordWrap(true);
    errorLabel_->setMinimumHeight(60);  // 设置最小高度
    errorLabel_->setMaximumHeight(60);  // 设置最大高度
    errorLabel_->setAlignment(Qt::AlignTop);  // 顶部对齐
    mainLayout->addWidget(errorLabel_);
    
    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    cancelButton_ = new QPushButton("取消");
    okButton_ = new QPushButton("确定");
    okButton_->setEnabled(false);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton_);
    buttonLayout->addWidget(okButton_);
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号
    connect(okButton_, &QPushButton::clicked, this, &TemplateCreationDialog::accept);
    connect(cancelButton_, &QPushButton::clicked, this, &QDialog::reject);
    
    nameEdit_->setFocus();
}

void TemplateCreationDialog::setupValidation() {
    connect(nameEdit_, &QLineEdit::textChanged, this, &TemplateCreationDialog::onNameChanged);
    
    connect(angleStartSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &TemplateCreationDialog::onAngleRangeChanged);
    connect(angleExtentSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &TemplateCreationDialog::onAngleRangeChanged);
    connect(angleStepSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &TemplateCreationDialog::onAngleRangeChanged);
    
    connect(scaleMinSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &TemplateCreationDialog::onScaleRangeChanged);
    connect(scaleMaxSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &TemplateCreationDialog::onScaleRangeChanged);
    connect(scaleStepSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &TemplateCreationDialog::onScaleRangeChanged);
    
    // Halcon参数变更时不需要特别的验证逻辑，都是下拉框选择
}


void TemplateCreationDialog::accept() {
    // 保存当前参数到配置文件
    saveConfigToFile();
    QDialog::accept();
}

void TemplateCreationDialog::setInputError(QWidget* widget, bool hasError) {
    if (hasError) {
        // 使用淡红色背景替代红色边框
        if (qobject_cast<QLineEdit*>(widget)) {
            widget->setStyleSheet("QLineEdit { background-color: #ffe6e6; }");
        } else if (qobject_cast<QDoubleSpinBox*>(widget)) {
            widget->setStyleSheet("QDoubleSpinBox { background-color: #ffe6e6; }");
        } else if (qobject_cast<QSpinBox*>(widget)) {
            widget->setStyleSheet("QSpinBox { background-color: #ffe6e6; }");
        } else {
            widget->setStyleSheet("background-color: #ffe6e6;");
        }
    } else {
        widget->setStyleSheet("");
    }
}

void TemplateCreationDialog::onNameChanged() {
    nameValid_ = !nameEdit_->text().trimmed().isEmpty();
    setInputError(nameEdit_, !nameValid_);
    validateAllInputs();
}

void TemplateCreationDialog::onAngleRangeChanged() {
    double extent = angleExtentSpin_->value();
    double step = angleStepSpin_->value();
    
    angleRangeValid_ = (extent > 0) && (step > 0) && (step <= extent);
    
    setInputError(angleStartSpin_, !angleRangeValid_);
    setInputError(angleExtentSpin_, !angleRangeValid_);
    setInputError(angleStepSpin_, !angleRangeValid_);
    
    validateAllInputs();
}

void TemplateCreationDialog::onScaleRangeChanged() {
    double min = scaleMinSpin_->value();
    double max = scaleMaxSpin_->value();
    double step = scaleStepSpin_->value();
    
    scaleRangeValid_ = (min < max) && (step > 0) && (step <= (max - min));
    
    setInputError(scaleMinSpin_, !scaleRangeValid_);
    setInputError(scaleMaxSpin_, !scaleRangeValid_);
    setInputError(scaleStepSpin_, !scaleRangeValid_);
    
    validateAllInputs();
}

void TemplateCreationDialog::onParametersChanged() {
    // Halcon参数都是通过下拉框选择，无需特殊验证
    validateAllInputs();
}

void TemplateCreationDialog::validateAllInputs() {
    updateErrorMessage();
    updateOkButtonState();
}

void TemplateCreationDialog::updateErrorMessage() {
    QStringList errors;
    
    if (!nameValid_) {
        errors << "模板名称不能为空";
    }
    
    if (!angleRangeValid_) {
        errors << "角度范围无效：结束角度必须大于开始角度，步长必须大于0且不超过角度范围";
    }
    
    if (!scaleRangeValid_) {
        errors << "缩放范围无效：最大缩放必须大于最小缩放，步长必须大于0且不超过缩放范围";
    }
    
    if (errors.isEmpty()) {
        errorLabel_->setText("");  // 清空文本但保持标签可见以维持布局
    } else {
        errorLabel_->setText(errors.join("\n"));
    }
}

void TemplateCreationDialog::updateOkButtonState() {
    bool allValid = nameValid_ && angleRangeValid_ && scaleRangeValid_;
    okButton_->setEnabled(allValid);
}

QString TemplateCreationDialog::getTemplateName() const {
    return nameEdit_->text().trimmed();
}

QJsonObject TemplateCreationDialog::getTemplateCreationConfig() const {
    QJsonObject config;
    
    // 使用Halcon官方参数名称
    config["angle_start"] = angleStartSpin_->value();
    config["angle_extent"] = angleExtentSpin_->value();
    config["angle_step"] = angleStepSpin_->value();
    config["scale_min"] = scaleMinSpin_->value();
    config["scale_max"] = scaleMaxSpin_->value();
    config["scale_step"] = scaleStepSpin_->value();
    config["num_levels"] = numLevelsSpin_->value();
    config["optimization"] = optimizationCombo_->currentText();
    config["metric"] = metricCombo_->currentText();
    config["contrast"] = contrastCombo_->currentText();
    config["min_contrast"] = minContrastCombo_->currentText();
    
    return config;
}

void TemplateCreationDialog::loadConfigFromFile() {
    TemplateMatchingConfig* config = TemplateMatchingConfig::instance();
    config->loadConfig();
    loadParametersFromConfig();
}

void TemplateCreationDialog::saveConfigToFile() {
    saveParametersToConfig();
    TemplateMatchingConfig* config = TemplateMatchingConfig::instance();
    config->saveConfig();
}

void TemplateCreationDialog::loadParametersFromConfig() {
    TemplateMatchingConfig* config = TemplateMatchingConfig::instance();
    TemplateMatchingConfig::TemplateCreationParams params = config->getTemplateCreationParams();
    
    // 设置角度范围
    angleStartSpin_->setValue(params.angleStart);
    angleExtentSpin_->setValue(params.angleExtent);
    angleStepSpin_->setValue(params.angleStep);
    
    // 设置缩放范围
    scaleMinSpin_->setValue(params.scaleMin);
    scaleMaxSpin_->setValue(params.scaleMax);
    scaleStepSpin_->setValue(params.scaleStep);
    
    // 设置Halcon参数
    numLevelsSpin_->setValue(params.numLevels);
    
    // 设置组合框值
    optimizationCombo_->setCurrentText(params.optimization);
    metricCombo_->setCurrentText(params.metric);
    contrastCombo_->setCurrentText(params.contrast);
    minContrastCombo_->setCurrentText(params.minContrast);
}

void TemplateCreationDialog::saveParametersToConfig() {
    TemplateMatchingConfig::TemplateCreationParams params;
    
    // 获取角度范围
    params.angleStart = angleStartSpin_->value();
    params.angleExtent = angleExtentSpin_->value();
    params.angleStep = angleStepSpin_->value();
    
    // 获取缩放范围
    params.scaleMin = scaleMinSpin_->value();
    params.scaleMax = scaleMaxSpin_->value();
    params.scaleStep = scaleStepSpin_->value();
    
    // 获取Halcon参数
    params.numLevels = numLevelsSpin_->value();
    params.optimization = optimizationCombo_->currentText();
    params.metric = metricCombo_->currentText();
    params.contrast = contrastCombo_->currentText();
    params.minContrast = minContrastCombo_->currentText();
    
    TemplateMatchingConfig* config = TemplateMatchingConfig::instance();
    config->setTemplateCreationParams(params);
}

void TemplateCreationDialog::setupHalconParameterCombos() {
    // 优化方式选项
    optimizationCombo_->addItems({"auto", "none", "precompilation"});
    optimizationCombo_->setCurrentText("auto");
    
    // 极性度量选项
    metricCombo_->addItems({"use_polarity", "ignore_global_polarity", 
                           "ignore_local_polarity", "ignore_part_polarity"});
    metricCombo_->setCurrentText("use_polarity");
    
    // 对比度选项
    contrastCombo_->addItems({"auto", "high", "low"});
    contrastCombo_->setCurrentText("auto");
    
    // 最小对比度选项
    minContrastCombo_->addItems({"auto", "5", "10", "15", "20", "30"});
    minContrastCombo_->setCurrentText("auto");
}
