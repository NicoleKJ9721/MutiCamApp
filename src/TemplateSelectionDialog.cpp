#include "TemplateSelectionDialog.h"
#include <QApplication>
#include <QMessageBox>
#include <QFileInfo>
#include <QPixmap>
#include <QIcon>
#include <opencv2/opencv.hpp>

TemplateSelectionDialog::TemplateSelectionDialog(QWidget *parent)
    : QDialog(parent)
    , m_mainLayout(nullptr)
    , m_contentLayout(nullptr)
    , m_templateGroup(nullptr)
    , m_templateList(nullptr)
    , m_selectAllBtn(nullptr)
    , m_deselectAllBtn(nullptr)
    , m_parameterGroup(nullptr)
    , m_confidenceSpinBox(nullptr)
    , m_maxMatchesSpinBox(nullptr)
    , m_enableRotationCheckBox(nullptr)
    , m_rotationRangeSpinBox(nullptr)
    , m_enableScalingCheckBox(nullptr)
    , m_scaleRangeSpinBox(nullptr)
    , m_buttonLayout(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
{
    setupUI();
    setWindowTitle("模板选择和匹配参数设置");
    setModal(true);
    resize(600, 400);
}

TemplateSelectionDialog::~TemplateSelectionDialog()
{
}

void TemplateSelectionDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_contentLayout = new QHBoxLayout();
    
    createTemplateList();
    createParameterPanel();
    createButtonPanel();
    
    m_mainLayout->addLayout(m_contentLayout);
    m_mainLayout->addLayout(m_buttonLayout);
}

void TemplateSelectionDialog::createTemplateList()
{
    m_templateGroup = new QGroupBox("可用模板", this);
    QVBoxLayout* templateLayout = new QVBoxLayout(m_templateGroup);
    
    // 模板列表
    m_templateList = new QListWidget(this);
    m_templateList->setSelectionMode(QAbstractItemView::NoSelection);
    templateLayout->addWidget(m_templateList);
    
    // 选择按钮
    QHBoxLayout* selectLayout = new QHBoxLayout();
    m_selectAllBtn = new QPushButton("全选", this);
    m_deselectAllBtn = new QPushButton("全不选", this);
    selectLayout->addWidget(m_selectAllBtn);
    selectLayout->addWidget(m_deselectAllBtn);
    selectLayout->addStretch();
    templateLayout->addLayout(selectLayout);
    
    // 连接信号
    connect(m_selectAllBtn, &QPushButton::clicked, this, &TemplateSelectionDialog::onSelectAllClicked);
    connect(m_deselectAllBtn, &QPushButton::clicked, this, &TemplateSelectionDialog::onDeselectAllClicked);
    connect(m_templateList, &QListWidget::itemChanged, this, &TemplateSelectionDialog::onTemplateItemChanged);
    
    m_contentLayout->addWidget(m_templateGroup, 2);
}

void TemplateSelectionDialog::createParameterPanel()
{
    m_parameterGroup = new QGroupBox("匹配参数", this);
    QVBoxLayout* paramLayout = new QVBoxLayout(m_parameterGroup);
    
    // 置信度阈值
    QHBoxLayout* confidenceLayout = new QHBoxLayout();
    confidenceLayout->addWidget(new QLabel("置信度阈值:", this));
    m_confidenceSpinBox = new QDoubleSpinBox(this);
    m_confidenceSpinBox->setRange(0.1, 1.0);
    m_confidenceSpinBox->setSingleStep(0.1);
    m_confidenceSpinBox->setValue(0.7);
    m_confidenceSpinBox->setDecimals(2);
    confidenceLayout->addWidget(m_confidenceSpinBox);
    paramLayout->addLayout(confidenceLayout);
    
    // 最大匹配数量
    QHBoxLayout* maxMatchesLayout = new QHBoxLayout();
    maxMatchesLayout->addWidget(new QLabel("最大匹配数:", this));
    m_maxMatchesSpinBox = new QSpinBox(this);
    m_maxMatchesSpinBox->setRange(1, 100);
    m_maxMatchesSpinBox->setValue(10);
    maxMatchesLayout->addWidget(m_maxMatchesSpinBox);
    paramLayout->addLayout(maxMatchesLayout);
    
    // 旋转匹配
    m_enableRotationCheckBox = new QCheckBox("启用旋转匹配", this);
    m_enableRotationCheckBox->setChecked(true);
    paramLayout->addWidget(m_enableRotationCheckBox);
    
    QHBoxLayout* rotationLayout = new QHBoxLayout();
    rotationLayout->addWidget(new QLabel("旋转范围(度):", this));
    m_rotationRangeSpinBox = new QDoubleSpinBox(this);
    m_rotationRangeSpinBox->setRange(0.0, 360.0);
    m_rotationRangeSpinBox->setValue(360.0);
    m_rotationRangeSpinBox->setDecimals(1);
    rotationLayout->addWidget(m_rotationRangeSpinBox);
    paramLayout->addLayout(rotationLayout);
    
    // 缩放匹配
    m_enableScalingCheckBox = new QCheckBox("启用缩放匹配", this);
    m_enableScalingCheckBox->setChecked(false);
    paramLayout->addWidget(m_enableScalingCheckBox);
    
    QHBoxLayout* scaleLayout = new QHBoxLayout();
    scaleLayout->addWidget(new QLabel("缩放范围(±):", this));
    m_scaleRangeSpinBox = new QDoubleSpinBox(this);
    m_scaleRangeSpinBox->setRange(0.0, 1.0);
    m_scaleRangeSpinBox->setValue(0.2);
    m_scaleRangeSpinBox->setDecimals(2);
    scaleLayout->addWidget(m_scaleRangeSpinBox);
    paramLayout->addLayout(scaleLayout);
    
    paramLayout->addStretch();
    m_contentLayout->addWidget(m_parameterGroup, 1);
}

void TemplateSelectionDialog::createButtonPanel()
{
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->addStretch();
    
    m_okButton = new QPushButton("确定", this);
    m_cancelButton = new QPushButton("取消", this);
    
    m_buttonLayout->addWidget(m_okButton);
    m_buttonLayout->addWidget(m_cancelButton);
    
    connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void TemplateSelectionDialog::setAvailableTemplates(const QVector<TemplateInfo>& templates)
{
    m_availableTemplates = templates;
    m_templateList->clear();
    
    for (const TemplateInfo& templateInfo : templates) {
        QListWidgetItem* item = new QListWidgetItem(m_templateList);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked); // 默认不选中
        updateTemplateItem(item, templateInfo);
        m_templateList->addItem(item);
    }
}

QVector<TemplateInfo> TemplateSelectionDialog::getSelectedTemplates() const
{
    QVector<TemplateInfo> selectedTemplates;
    
    for (int i = 0; i < m_templateList->count(); ++i) {
        QListWidgetItem* item = m_templateList->item(i);
        if (item && item->checkState() == Qt::Checked) {
            TemplateInfo templateInfo = m_availableTemplates[i];
            templateInfo.isSelected = true;
            selectedTemplates.append(templateInfo);
        }
    }
    
    return selectedTemplates;
}

TemplateSelectionDialog::MatchingParams TemplateSelectionDialog::getMatchingParams() const
{
    MatchingParams params;
    params.confidenceThreshold = m_confidenceSpinBox->value();
    params.maxMatches = m_maxMatchesSpinBox->value();
    params.enableRotation = m_enableRotationCheckBox->isChecked();
    params.rotationRange = m_rotationRangeSpinBox->value();
    params.enableScaling = m_enableScalingCheckBox->isChecked();
    params.scaleRange = m_scaleRangeSpinBox->value();
    return params;
}

void TemplateSelectionDialog::onSelectAllClicked()
{
    for (int i = 0; i < m_templateList->count(); ++i) {
        QListWidgetItem* item = m_templateList->item(i);
        if (item) {
            item->setCheckState(Qt::Checked);
        }
    }
}

void TemplateSelectionDialog::onDeselectAllClicked()
{
    for (int i = 0; i < m_templateList->count(); ++i) {
        QListWidgetItem* item = m_templateList->item(i);
        if (item) {
            item->setCheckState(Qt::Unchecked);
        }
    }
}

void TemplateSelectionDialog::onTemplateItemChanged(QListWidgetItem* item)
{
    // 可以在这里添加项目状态变化的处理逻辑
    Q_UNUSED(item)
}

void TemplateSelectionDialog::updateTemplateItem(QListWidgetItem* item, const TemplateInfo& templateInfo)
{
    // 检查 templateImage 是否有效
    if (!templateInfo.templateImage.empty()) {
        // 如果有图像 (旧逻辑，可能仍需兼容)，创建缩略图
        QPixmap thumbnail = createThumbnail(templateInfo.templateImage);
        item->setIcon(QIcon(thumbnail));
        
        // 设置文本信息 (从 templateImage 获取尺寸)
        QString itemText = QString("%1\n尺寸: %2x%3\n创建时间: %4")
                           .arg(templateInfo.name)
                           .arg(templateInfo.templateImage.cols)
                           .arg(templateInfo.templateImage.rows)
                           .arg(templateInfo.createdTime.toString("yyyy-MM-dd hh:mm"));
        item->setText(itemText);
    } else {
        // 如果没有图像 (新逻辑，从.yaml加载)，不设置图标
        item->setIcon(QIcon()); // 清除图标
        
        // 设置文本信息 (现在从 originalROI 获取尺寸)
        QString itemText = QString("%1\n尺寸: %2x%3\n创建时间: %4")
                           .arg(templateInfo.name)
                           .arg(static_cast<int>(templateInfo.originalROI.width()))
                           .arg(static_cast<int>(templateInfo.originalROI.height()))
                           .arg(templateInfo.createdTime.toString("yyyy-MM-dd hh:mm"));
        item->setText(itemText);
    }
}

QPixmap TemplateSelectionDialog::createThumbnail(const cv::Mat& image, const QSize& size)
{
    if (image.empty()) {
        return QPixmap(size);
    }
    
    try {
        cv::Mat thumbnail;
        cv::resize(image, thumbnail, cv::Size(size.width(), size.height()));
        
        // 转换为QImage
        QImage qimg;
        if (thumbnail.channels() == 3) {
            cv::cvtColor(thumbnail, thumbnail, cv::COLOR_BGR2RGB);
            qimg = QImage(thumbnail.data, thumbnail.cols, thumbnail.rows, thumbnail.step, QImage::Format_RGB888);
        } else if (thumbnail.channels() == 1) {
            qimg = QImage(thumbnail.data, thumbnail.cols, thumbnail.rows, thumbnail.step, QImage::Format_Grayscale8);
        } else {
            return QPixmap(size);
        }
        
        return QPixmap::fromImage(qimg);
    } catch (const cv::Exception& e) {
        qWarning() << "创建缩略图失败：" << e.what();
        return QPixmap(size);
    }
}
