#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QComboBox>

/**
 * @brief 棋盘格标定参数输入对话框
 * 
 * 用于输入棋盘格标定所需的参数：
 * - 棋盘格内角点数量（行数和列数）
 * - 棋盘格方格的实际尺寸
 * - 单位选择
 */
class CheckerboardCalibrationDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 棋盘格标定参数结构体
     */
    struct CheckerboardParams {
        int cornersX;           // 棋盘格内角点X方向数量
        int cornersY;           // 棋盘格内角点Y方向数量
        double squareSize;      // 方格实际尺寸
        QString unit;           // 单位
        
        CheckerboardParams() : cornersX(9), cornersY(7), squareSize(1.0), unit("mm") {}
    };

    /**
     * @brief 构造函数
     * @param parent 父窗口
     */
    explicit CheckerboardCalibrationDialog(QWidget *parent = nullptr);

    /**
     * @brief 获取棋盘格标定参数
     * @return 标定参数
     */
    CheckerboardParams getParams() const;

private slots:
    /**
     * @brief 确定按钮点击事件
     */
    void onOkClicked();

    /**
     * @brief 取消按钮点击事件
     */
    void onCancelClicked();

    /**
     * @brief 预设参数选择改变事件
     */
    void onPresetChanged();

private:
    /**
     * @brief 初始化UI
     */
    void initializeUI();

    /**
     * @brief 创建参数输入组
     * @return 参数输入组控件
     */
    QGroupBox* createParameterGroup();

    /**
     * @brief 创建预设选择组
     * @return 预设选择组控件
     */
    QGroupBox* createPresetGroup();

    /**
     * @brief 创建按钮组
     * @return 按钮组控件
     */
    QWidget* createButtonGroup();

    /**
     * @brief 更新参数显示
     */
    void updateParameters(int cornersX, int cornersY, double squareSize, const QString& unit);

private:
    QComboBox* m_presetCombo;           // 预设选择
    QSpinBox* m_cornersXSpinBox;        // X方向角点数
    QSpinBox* m_cornersYSpinBox;        // Y方向角点数
    QDoubleSpinBox* m_squareSizeSpinBox; // 方格尺寸
    QComboBox* m_unitCombo;             // 单位选择
    
    QPushButton* m_okButton;            // 确定按钮
    QPushButton* m_cancelButton;        // 取消按钮
    
    CheckerboardParams m_params;        // 当前参数
};
