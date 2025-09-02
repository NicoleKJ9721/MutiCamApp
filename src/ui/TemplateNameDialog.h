#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

class TemplateNameDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit TemplateNameDialog(QWidget* parent = nullptr);
    QString getTemplateName() const;
    
private:
    QLineEdit* nameEdit_;
    QPushButton* okButton_;
    QPushButton* cancelButton_;
    
private slots:
    void onTextChanged();
};
