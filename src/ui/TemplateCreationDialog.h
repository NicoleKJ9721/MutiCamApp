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

class TemplateCreationDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit TemplateCreationDialog(QWidget* parent = nullptr);
    
    // 获取用户输入的所有参数
    QString getTemplateName() const;
    QJsonObject getTemplateCreationConfig() const;
    
    // 重写accept方法以保存参数
    void accept() override;
    
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
    
    // 角度范围参数
    QDoubleSpinBox* angleBeginSpin_;
    QDoubleSpinBox* angleEndSpin_;
    QDoubleSpinBox* angleStepSpin_;
    
    // 缩放范围参数
    QDoubleSpinBox* scaleBeginSpin_;
    QDoubleSpinBox* scaleEndSpin_;
    QDoubleSpinBox* scaleStepSpin_;
    
    // 其他参数
    QSpinBox* numFeaturesSpin_;
    QDoubleSpinBox* weakThreshSpin_;
    QDoubleSpinBox* strongThreshSpin_;
    
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
    void loadDefaultValues();
    void saveCurrentValuesAsDefaults();
    void setInputError(QWidget* widget, bool hasError);
    void updateErrorMessage();
    void updateOkButtonState();
    
private slots:
    void onNameChanged();
    void onAngleRangeChanged();
    void onScaleRangeChanged();
    void onParametersChanged();
    void validateAllInputs();
};
