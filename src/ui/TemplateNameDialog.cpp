#include "TemplateNameDialog.h"
#include <QMessageBox>

TemplateNameDialog::TemplateNameDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("创建模板");
    setModal(true);
    setFixedSize(300, 120);
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    QLabel* label = new QLabel("请输入模板名称:");
    layout->addWidget(label);
    
    nameEdit_ = new QLineEdit();
    nameEdit_->setPlaceholderText("例如: 螺丝_型号A");
    layout->addWidget(nameEdit_);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    cancelButton_ = new QPushButton("取消");
    okButton_ = new QPushButton("确定");
    okButton_->setEnabled(false);
    
    buttonLayout->addWidget(cancelButton_);
    buttonLayout->addWidget(okButton_);
    layout->addLayout(buttonLayout);
    
    connect(okButton_, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton_, &QPushButton::clicked, this, &QDialog::reject);
    connect(nameEdit_, &QLineEdit::textChanged, this, &TemplateNameDialog::onTextChanged);
    connect(nameEdit_, &QLineEdit::returnPressed, [this]() {
        if (okButton_->isEnabled()) accept();
    });
    
    nameEdit_->setFocus();
}

QString TemplateNameDialog::getTemplateName() const {
    return nameEdit_->text().trimmed();
}

void TemplateNameDialog::onTextChanged() {
    okButton_->setEnabled(!nameEdit_->text().trimmed().isEmpty());
}
