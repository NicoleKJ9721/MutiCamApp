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
    setFixedSize(450, 500);
    
    setupUI();
    setupValidation();
    loadDefaultValues();
    
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
    angleBeginSpin_ = new QDoubleSpinBox();
    angleBeginSpin_->setRange(-180.0, 180.0);
    angleBeginSpin_->setSuffix("°");
    angleBeginSpin_->setToolTip("模板匹配的最小旋转角度，范围：-180°到180°");
    angleLayout->addWidget(angleBeginSpin_, 0, 1);
    
    angleLayout->addWidget(new QLabel("结束角度:"), 0, 2);
    angleEndSpin_ = new QDoubleSpinBox();
    angleEndSpin_->setRange(-180.0, 180.0);
    angleEndSpin_->setSuffix("°");
    angleEndSpin_->setToolTip("模板匹配的最大旋转角度，必须大于开始角度");
    angleLayout->addWidget(angleEndSpin_, 0, 3);
    
    angleLayout->addWidget(new QLabel("角度步长:"), 1, 0);
    angleStepSpin_ = new QDoubleSpinBox();
    angleStepSpin_->setRange(0.1, 45.0);
    angleStepSpin_->setSuffix("°");
    angleStepSpin_->setToolTip("角度搜索的步长，值越小精度越高但速度越慢，建议1-10°");
    angleLayout->addWidget(angleStepSpin_, 1, 1);
    
    mainLayout->addWidget(angleGroup);
    
    // 缩放范围组
    QGroupBox* scaleGroup = new QGroupBox("缩放范围设置");
    QGridLayout* scaleLayout = new QGridLayout(scaleGroup);
    
    scaleLayout->addWidget(new QLabel("最小缩放:"), 0, 0);
    scaleBeginSpin_ = new QDoubleSpinBox();
    scaleBeginSpin_->setRange(0.1, 5.0);
    scaleBeginSpin_->setDecimals(2);
    scaleBeginSpin_->setToolTip("模板匹配的最小缩放比例，1.0为原始大小，建议0.5-1.5");
    scaleLayout->addWidget(scaleBeginSpin_, 0, 1);
    
    scaleLayout->addWidget(new QLabel("最大缩放:"), 0, 2);
    scaleEndSpin_ = new QDoubleSpinBox();
    scaleEndSpin_->setRange(0.1, 5.0);
    scaleEndSpin_->setDecimals(2);
    scaleEndSpin_->setToolTip("模板匹配的最大缩放比例，必须大于最小缩放");
    scaleLayout->addWidget(scaleEndSpin_, 0, 3);
    
    scaleLayout->addWidget(new QLabel("缩放步长:"), 1, 0);
    scaleStepSpin_ = new QDoubleSpinBox();
    scaleStepSpin_->setRange(0.01, 1.0);
    scaleStepSpin_->setDecimals(2);
    scaleStepSpin_->setToolTip("缩放搜索的步长，值越小精度越高但速度越慢，建议0.05-0.1");
    scaleLayout->addWidget(scaleStepSpin_, 1, 1);
    
    mainLayout->addWidget(scaleGroup);
    
    // 高级参数组
    QGroupBox* advancedGroup = new QGroupBox("高级参数");
    QGridLayout* advancedLayout = new QGridLayout(advancedGroup);
    
    advancedLayout->addWidget(new QLabel("特征数量:"), 0, 0);
    numFeaturesSpin_ = new QSpinBox();
    numFeaturesSpin_->setRange(0, 10000);
    numFeaturesSpin_->setSpecialValueText("自动");
    numFeaturesSpin_->setToolTip("模板特征点数量，0表示自动，手动设置建议100-1000");
    advancedLayout->addWidget(numFeaturesSpin_, 0, 1);
    
    advancedLayout->addWidget(new QLabel("弱阈值:"), 1, 0);
    weakThreshSpin_ = new QDoubleSpinBox();
    weakThreshSpin_->setRange(0.0, 255.0);
    weakThreshSpin_->setToolTip("边缘检测的弱阈值，用于初步筛选特征点，建议10-50");
    advancedLayout->addWidget(weakThreshSpin_, 1, 1);
    
    advancedLayout->addWidget(new QLabel("强阈值:"), 1, 2);
    strongThreshSpin_ = new QDoubleSpinBox();
    strongThreshSpin_->setRange(0.0, 255.0);
    strongThreshSpin_->setToolTip("边缘检测的强阈值，用于确定关键特征点，必须大于弱阈值，建议30-100");
    advancedLayout->addWidget(strongThreshSpin_, 1, 3);
    
    mainLayout->addWidget(advancedGroup);
    
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
    
    connect(angleBeginSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &TemplateCreationDialog::onAngleRangeChanged);
    connect(angleEndSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &TemplateCreationDialog::onAngleRangeChanged);
    connect(angleStepSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &TemplateCreationDialog::onAngleRangeChanged);
    
    connect(scaleBeginSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &TemplateCreationDialog::onScaleRangeChanged);
    connect(scaleEndSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &TemplateCreationDialog::onScaleRangeChanged);
    connect(scaleStepSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &TemplateCreationDialog::onScaleRangeChanged);
    
    connect(weakThreshSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &TemplateCreationDialog::onParametersChanged);
    connect(strongThreshSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &TemplateCreationDialog::onParametersChanged);
}

void TemplateCreationDialog::loadDefaultValues() {
    QFile file("../config/template_matching.json");
    if (!file.open(QIODevice::ReadOnly)) {
        // 使用常量默认值
        angleBeginSpin_->setValue(DEFAULT_ANGLE_BEGIN);
        angleEndSpin_->setValue(DEFAULT_ANGLE_END);
        angleStepSpin_->setValue(DEFAULT_ANGLE_STEP);
        scaleBeginSpin_->setValue(DEFAULT_SCALE_BEGIN);
        scaleEndSpin_->setValue(DEFAULT_SCALE_END);
        scaleStepSpin_->setValue(DEFAULT_SCALE_STEP);
        numFeaturesSpin_->setValue(DEFAULT_NUM_FEATURES);
        weakThreshSpin_->setValue(DEFAULT_WEAK_THRESH);
        strongThreshSpin_->setValue(DEFAULT_STRONG_THRESH);
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject config = doc.object();
    QJsonObject creation = config["template_creation"].toObject();
    
    if (!creation.isEmpty()) {
        QJsonObject angleRange = creation["angle_range"].toObject();
        angleBeginSpin_->setValue(angleRange["begin"].toDouble(DEFAULT_ANGLE_BEGIN));
        angleEndSpin_->setValue(angleRange["end"].toDouble(DEFAULT_ANGLE_END));
        angleStepSpin_->setValue(angleRange["step"].toDouble(DEFAULT_ANGLE_STEP));
        
        QJsonObject scaleRange = creation["scale_range"].toObject();
        scaleBeginSpin_->setValue(scaleRange["begin"].toDouble(DEFAULT_SCALE_BEGIN));
        scaleEndSpin_->setValue(scaleRange["end"].toDouble(DEFAULT_SCALE_END));
        scaleStepSpin_->setValue(scaleRange["step"].toDouble(DEFAULT_SCALE_STEP));
        
        numFeaturesSpin_->setValue(creation["num_features"].toInt(DEFAULT_NUM_FEATURES));
        weakThreshSpin_->setValue(creation["weak_thresh"].toDouble(DEFAULT_WEAK_THRESH));
        strongThreshSpin_->setValue(creation["strong_thresh"].toDouble(DEFAULT_STRONG_THRESH));
    }
}

void TemplateCreationDialog::saveCurrentValuesAsDefaults() {
    // 读取现有配置文件
    QJsonObject fullConfig;
    QFile file("../config/template_matching.json");
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        fullConfig = doc.object();
        file.close();
    }
    
    // 更新template_creation部分
    fullConfig["template_creation"] = getTemplateCreationConfig();
    
    // 保存回文件
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(fullConfig).toJson(QJsonDocument::Indented));
        file.close();
    }
}

void TemplateCreationDialog::accept() {
    // 保存当前参数为下次默认值
    saveCurrentValuesAsDefaults();
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
    double begin = angleBeginSpin_->value();
    double end = angleEndSpin_->value();
    double step = angleStepSpin_->value();
    
    angleRangeValid_ = (begin < end) && (step > 0) && (step <= (end - begin));
    
    setInputError(angleBeginSpin_, !angleRangeValid_);
    setInputError(angleEndSpin_, !angleRangeValid_);
    setInputError(angleStepSpin_, !angleRangeValid_);
    
    validateAllInputs();
}

void TemplateCreationDialog::onScaleRangeChanged() {
    double begin = scaleBeginSpin_->value();
    double end = scaleEndSpin_->value();
    double step = scaleStepSpin_->value();
    
    scaleRangeValid_ = (begin < end) && (step > 0) && (step <= (end - begin));
    
    setInputError(scaleBeginSpin_, !scaleRangeValid_);
    setInputError(scaleEndSpin_, !scaleRangeValid_);
    setInputError(scaleStepSpin_, !scaleRangeValid_);
    
    validateAllInputs();
}

void TemplateCreationDialog::onParametersChanged() {
    double weakThresh = weakThreshSpin_->value();
    double strongThresh = strongThreshSpin_->value();
    
    parametersValid_ = strongThresh >= weakThresh;
    
    setInputError(weakThreshSpin_, !parametersValid_);
    setInputError(strongThreshSpin_, !parametersValid_);
    
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
    
    if (!parametersValid_) {
        errors << "阈值设置无效：强阈值必须大于等于弱阈值";
    }
    
    if (errors.isEmpty()) {
        errorLabel_->setText("");  // 清空文本但保持标签可见以维持布局
    } else {
        errorLabel_->setText(errors.join("\n"));
    }
}

void TemplateCreationDialog::updateOkButtonState() {
    bool allValid = nameValid_ && angleRangeValid_ && scaleRangeValid_ && parametersValid_;
    okButton_->setEnabled(allValid);
}

QString TemplateCreationDialog::getTemplateName() const {
    return nameEdit_->text().trimmed();
}

QJsonObject TemplateCreationDialog::getTemplateCreationConfig() const {
    QJsonObject config;
    
    QJsonObject angleRange;
    angleRange["begin"] = angleBeginSpin_->value();
    angleRange["end"] = angleEndSpin_->value();
    angleRange["step"] = angleStepSpin_->value();
    
    QJsonObject scaleRange;
    scaleRange["begin"] = scaleBeginSpin_->value();
    scaleRange["end"] = scaleEndSpin_->value();
    scaleRange["step"] = scaleStepSpin_->value();
    
    config["angle_range"] = angleRange;
    config["scale_range"] = scaleRange;
    config["num_features"] = numFeaturesSpin_->value();
    config["weak_thresh"] = weakThreshSpin_->value();
    config["strong_thresh"] = strongThreshSpin_->value();
    
    return config;
}
