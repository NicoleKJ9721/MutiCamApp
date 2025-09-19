#ifndef TEMPLATESELECTIONDIALOG_H
#define TEMPLATESELECTIONDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QPixmap>
#include <QVector>
#include "PaintingOverlay.h"
#include "../config/TemplateMatchingConfig.h"

class TemplateSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TemplateSelectionDialog(QWidget *parent = nullptr);
    ~TemplateSelectionDialog();

    /**
     * @brief 设置可用的模板列表
     * @param templates 模板信息列表
     */
    void setAvailableTemplates(const QVector<TemplateInfo>& templates);

    /**
     * @brief 获取用户选中的模板列表
     * @return 选中的模板信息列表
     */
    QVector<TemplateInfo> getSelectedTemplates() const;

    /**
     * @brief 获取匹配参数设置
     * @return 匹配参数结构体
     */
    struct MatchingParams {
        double confidenceThreshold = 0.5;  // 置信度阈值
        int maxMatches = 1;                 // 最大匹配数量
        bool enableRotation = true;         // 是否启用旋转匹配
        double rotationRange = 360.0;       // 旋转范围（度）
        bool enableScaling = true;          // 是否启用缩放匹配
        double scaleRange = 0.2;            // 缩放范围（±）
    };
    
    MatchingParams getMatchingParams() const;
    
    /**
     * @brief 将对话框参数转换为Halcon模板匹配参数
     * @return Halcon兼容的模板匹配参数
     */
    TemplateMatchingConfig::TemplateMatchingParams toHalconMatchingParams() const;
    
    // 配置管理
    void loadUIDefaultsFromConfig();

private slots:
    void onSelectAllClicked();
    void onDeselectAllClicked();
    void onTemplateItemChanged(QListWidgetItem* item);

private:
    void setupUI();
    void createTemplateList();
    void createParameterPanel();
    void createButtonPanel();
    void updateTemplateItem(QListWidgetItem* item, const TemplateInfo& templateInfo);
    QPixmap createThumbnail(const cv::Mat& image, const QSize& size = QSize(64, 64));

    // UI组件
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_contentLayout;
    
    // 模板列表
    QGroupBox* m_templateGroup;
    QListWidget* m_templateList;
    QPushButton* m_selectAllBtn;
    QPushButton* m_deselectAllBtn;
    
    // 参数面板
    QGroupBox* m_parameterGroup;
    QDoubleSpinBox* m_confidenceSpinBox;
    QSpinBox* m_maxMatchesSpinBox;
    QCheckBox* m_enableRotationCheckBox;
    QDoubleSpinBox* m_rotationRangeSpinBox;
    QCheckBox* m_enableScalingCheckBox;
    QDoubleSpinBox* m_scaleRangeSpinBox;
    
    // 按钮面板
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    
    // 数据
    QVector<TemplateInfo> m_availableTemplates;
};

#endif // TEMPLATESELECTIONDIALOG_H
