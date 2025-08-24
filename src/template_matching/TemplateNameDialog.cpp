#include "TemplateNameDialog.h"
#include <QApplication>
#include <QStyle>
#include <QShowEvent>

TemplateNameDialog::TemplateNameDialog(QWidget *parent)
    : QDialog(parent)
    , m_mainLayout(nullptr)
    , m_promptLabel(nullptr)
    , m_nameEdit(nullptr)
    , m_errorLabel(nullptr)
    , m_buttonLayout(nullptr)
    , m_acceptButton(nullptr)
    , m_cancelButton(nullptr)
    , m_validator(nullptr)
{
    setupUI();
    setupValidation();
    
    // 设置对话框属性
    setWindowTitle("创建模板");
    setModal(true);
    setFixedSize(350, 180);
    
    // 连接信号槽
    connect(m_nameEdit, &QLineEdit::textChanged, this, &TemplateNameDialog::onTextChanged);
    connect(m_acceptButton, &QPushButton::clicked, this, &TemplateNameDialog::onAccept);
    connect(m_cancelButton, &QPushButton::clicked, this, &TemplateNameDialog::onReject);
}

QString TemplateNameDialog::getTemplateName() const
{
    return m_nameEdit->text().trimmed();
}

void TemplateNameDialog::setExistingNames(const QStringList& names)
{
    m_existingNames = names;
    onTextChanged(); // 重新验证当前输入
}

void TemplateNameDialog::setDefaultName(const QString& name)
{
    m_nameEdit->setText(name);
    m_nameEdit->selectAll();
}

void TemplateNameDialog::setPromptText(const QString& text)
{
    m_promptLabel->setText(text);
}

void TemplateNameDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    
    // 对话框显示时，聚焦到输入框并选中所有文本
    m_nameEdit->setFocus();
    m_nameEdit->selectAll();
}

void TemplateNameDialog::onTextChanged()
{
    updateAcceptButton();
}

void TemplateNameDialog::onAccept()
{
    QString name = getTemplateName();
    QString errorMessage;
    
    if (!validateName(name, errorMessage)) {
        m_errorLabel->setText(errorMessage);
        m_errorLabel->setVisible(true);
        return;
    }
    
    accept();
}

void TemplateNameDialog::onReject()
{
    reject();
}

void TemplateNameDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(12);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // 提示标签
    m_promptLabel = new QLabel("请输入模板名称：", this);
    m_promptLabel->setWordWrap(true);
    m_mainLayout->addWidget(m_promptLabel);
    
    // 名称输入框
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("模板名称");
    m_nameEdit->setMaxLength(MAX_NAME_LENGTH);
    m_mainLayout->addWidget(m_nameEdit);
    
    // 错误提示标签
    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet("QLabel { color: red; }");
    m_errorLabel->setWordWrap(true);
    m_errorLabel->setVisible(false);
    m_mainLayout->addWidget(m_errorLabel);
    
    // 添加弹性空间
    m_mainLayout->addStretch();
    
    // 按钮布局
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->addStretch();
    
    m_cancelButton = new QPushButton("取消", this);
    m_cancelButton->setMinimumSize(80, 30);
    m_buttonLayout->addWidget(m_cancelButton);
    
    m_acceptButton = new QPushButton("确定", this);
    m_acceptButton->setMinimumSize(80, 30);
    m_acceptButton->setDefault(true);
    m_buttonLayout->addWidget(m_acceptButton);
    
    m_mainLayout->addLayout(m_buttonLayout);
}

void TemplateNameDialog::setupValidation()
{
    // 设置正则表达式验证器
    // 允许中文、英文、数字、下划线、连字符和空格
    QRegularExpression regex("^[\\w\\s\\u4e00-\\u9fa5-]+$");
    m_validator = new QRegularExpressionValidator(regex, this);
    m_nameEdit->setValidator(m_validator);
}

bool TemplateNameDialog::validateName(const QString& name, QString& errorMessage) const
{
    // 检查长度
    if (name.length() < MIN_NAME_LENGTH) {
        errorMessage = "模板名称不能为空";
        return false;
    }
    
    if (name.length() > MAX_NAME_LENGTH) {
        errorMessage = QString("模板名称长度不能超过 %1 个字符").arg(MAX_NAME_LENGTH);
        return false;
    }
    
    // 检查字符有效性
    QString tempName = name;
    int pos = 0;
    if (m_validator->validate(tempName, pos) != QValidator::Acceptable) {
        errorMessage = "模板名称只能包含中文、英文、数字、下划线、连字符和空格";
        return false;
    }
    
    // 检查是否以空格开头或结尾
    if (name != name.trimmed()) {
        errorMessage = "模板名称不能以空格开头或结尾";
        return false;
    }
    
    // 检查重复
    if (isNameExists(name)) {
        errorMessage = "模板名称已存在，请选择其他名称";
        return false;
    }
    
    // 检查保留名称
    QStringList reservedNames = {"default", "template", "model", "config", "system"};
    if (reservedNames.contains(name.toLower())) {
        errorMessage = "不能使用保留名称";
        return false;
    }
    
    return true;
}

bool TemplateNameDialog::isNameExists(const QString& name) const
{
    return m_existingNames.contains(name, Qt::CaseInsensitive);
}

void TemplateNameDialog::updateAcceptButton()
{
    QString name = getTemplateName();
    QString errorMessage;
    
    bool isValid = validateName(name, errorMessage);
    m_acceptButton->setEnabled(isValid);
    
    if (isValid) {
        m_errorLabel->setVisible(false);
    } else if (!name.isEmpty()) {
        m_errorLabel->setText(errorMessage);
        m_errorLabel->setVisible(true);
    } else {
        m_errorLabel->setVisible(false);
    }
}
