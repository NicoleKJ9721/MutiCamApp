#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>
#include <QJsonObject>
#include <QComboBox>
#include "../config/TemplateMatchingConfig.h"

class TemplateCreationDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit TemplateCreationDialog(QWidget* parent = nullptr);
    
    // 获取用户输入的所有参数
    QString getTemplateName() const;
    QJsonObject getTemplateCreationConfig() const;
    
    // 重写accept方法以保存参数
    void accept() override;
    
    // 配置管理
    void loadConfigFromFile();
    void saveConfigToFile();
    
private:
    // 默认值常量
    static constexpr double DEFAULT_ANGLE_BEGIN = -90.0;
    static constexpr double DEFAULT_ANGLE_END = 90.0;
    static constexpr double DEFAULT_ANGLE_STEP = 4.0;
    static constexpr double DEFAULT_SCALE_BEGIN = 0.9;
    static constexpr double DEFAULT_SCALE_END = 1.1;
    static constexpr double DEFAULT_SCALE_STEP = 0.05;
    static constexpr int DEFAULT_NUM_FEATURES = 0;
    static constexpr double DEFAULT_WEAK_THRESH = 30.0;
    static constexpr double DEFAULT_STRONG_THRESH = 60.0;
    
    // UI组件
    QLineEdit* nameEdit_;
    
    // 角度范围参数 - Halcon官方命名
    QDoubleSpinBox* angleStartSpin_;
    QDoubleSpinBox* angleExtentSpin_;
    QDoubleSpinBox* angleStepSpin_;
    
    // 缩放范围参数 - Halcon官方命名
    QDoubleSpinBox* scaleMinSpin_;
    QDoubleSpinBox* scaleMaxSpin_;
    QDoubleSpinBox* scaleStepSpin_;
    
    // Halcon特定参数
    QSpinBox* numLevelsSpin_;
    QComboBox* optimizationCombo_;
    QComboBox* metricCombo_;
    QComboBox* contrastCombo_;
    QComboBox* minContrastCombo_;
    
    // 按钮和状态
    QPushButton* okButton_;
    QPushButton* cancelButton_;
    QLabel* errorLabel_;
    
    // 验证状态
    bool nameValid_;
    bool angleRangeValid_;
    bool scaleRangeValid_;
    bool parametersValid_;
    
    // 私有方法
    void setupUI();
    void setupValidation();
    void setInputError(QWidget* widget, bool hasError);
    
    // 配置相关私有方法
    void loadParametersFromConfig();
    void saveParametersToConfig();
    void setupHalconParameterCombos();
    void updateErrorMessage();
    void updateOkButtonState();
    
private slots:
    void onNameChanged();
    void onAngleRangeChanged();
    void onScaleRangeChanged();
    void onParametersChanged();
    void validateAllInputs();
};
