#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QLabel>
#include <QPushButton>
#include <QButtonGroup>
#include <QGroupBox>
#include <QFont>

/**
 * @brief 标定方式选择对话框
 * 
 * 提供三种标定方式供用户选择：
 * 1. 单点标定 - 快速标定，精度一般
 * 2. 多点标定 - 高精度测量，需要多个已知距离
 * 3. 棋盘格标定 - 自动标定，需要标准棋盘格图案
 */
class CalibrationDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 标定方式枚举
     */
    enum CalibrationMethod {
        SinglePoint,    // 单点标定
        MultiPoint,     // 多点标定
        Checkerboard    // 棋盘格标定
    };

    /**
     * @brief 构造函数
     * @param parent 父窗口
     */
    explicit CalibrationDialog(QWidget *parent = nullptr);

    /**
     * @brief 获取选择的标定方式
     * @return 标定方式
     */
    CalibrationMethod getSelectedMethod() const;

private slots:
    /**
     * @brief 确定按钮点击事件
     */
    void onOkClicked();

    /**
     * @brief 取消按钮点击事件
     */
    void onCancelClicked();

private:
    /**
     * @brief 初始化UI
     */
    void initializeUI();

    /**
     * @brief 创建标定方式选项组
     * @return 选项组控件
     */
    QGroupBox* createMethodGroup();

    /**
     * @brief 创建按钮组
     * @return 按钮组控件
     */
    QWidget* createButtonGroup();

private:
    QRadioButton* m_singlePointRadio;    // 单点标定选项
    QRadioButton* m_multiPointRadio;     // 多点标定选项
    QRadioButton* m_checkerboardRadio;   // 棋盘格标定选项
    QButtonGroup* m_methodButtonGroup;   // 方式按钮组
    
    QPushButton* m_okButton;             // 确定按钮
    QPushButton* m_cancelButton;         // 取消按钮
    
    CalibrationMethod m_selectedMethod;  // 选择的标定方式
};
