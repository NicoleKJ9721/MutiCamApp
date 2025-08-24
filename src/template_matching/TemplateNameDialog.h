#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

/**
 * @brief 模板命名对话框
 * 
 * 用于输入新模板的名称，支持名称验证和重复检查
 */
class TemplateNameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TemplateNameDialog(QWidget *parent = nullptr);
    ~TemplateNameDialog() = default;

    // 获取输入的模板名称
    QString getTemplateName() const;
    
    // 设置已存在的模板名称列表（用于重复检查）
    void setExistingNames(const QStringList& names);
    
    // 设置默认名称
    void setDefaultName(const QString& name);
    
    // 设置提示信息
    void setPromptText(const QString& text);

protected:
    void showEvent(QShowEvent* event) override;

private slots:
    void onTextChanged();
    void onAccept();
    void onReject();

private:
    void setupUI();
    void setupValidation();
    bool validateName(const QString& name, QString& errorMessage) const;
    bool isNameExists(const QString& name) const;
    void updateAcceptButton();

private:
    QVBoxLayout* m_mainLayout;
    QLabel* m_promptLabel;
    QLineEdit* m_nameEdit;
    QLabel* m_errorLabel;
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_acceptButton;
    QPushButton* m_cancelButton;
    
    QStringList m_existingNames;
    QRegularExpressionValidator* m_validator;
    
    // 验证规则
    static constexpr int MIN_NAME_LENGTH = 1;
    static constexpr int MAX_NAME_LENGTH = 50;
};
