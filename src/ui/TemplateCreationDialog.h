#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include "../matching/MatchingController.h"

class TemplateCreationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TemplateCreationDialog(QWidget* parent = nullptr);
    
    // 参数设置和获取方法
    void setParameters(const TemplateCreationParams& params);
    TemplateCreationParams getParameters() const;
    
    // 获取模板名称
    QString getTemplateName() const;
    
private slots:
    void onTextChanged();
    void validateParameters();
    
private:
    void setupUI();
    void connectSignals();
    
    // UI组件
    QLineEdit* m_templateNameEdit;
    
    // 角度范围参数
    QDoubleSpinBox* m_angleStartSpinBox;
    QDoubleSpinBox* m_angleEndSpinBox;
    QDoubleSpinBox* m_angleStepSpinBox;
    
    // 缩放范围参数
    QDoubleSpinBox* m_scaleStartSpinBox;
    QDoubleSpinBox* m_scaleEndSpinBox;
    QDoubleSpinBox* m_scaleStepSpinBox;
    
    // 其他参数
    QSpinBox* m_numFeaturesSpinBox;
    QDoubleSpinBox* m_weakThreshSpinBox;
    QDoubleSpinBox* m_strongThreshSpinBox;
    
    // 按钮
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
};
