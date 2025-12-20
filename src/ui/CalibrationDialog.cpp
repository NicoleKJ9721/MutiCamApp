#include "CalibrationDialog.h"
#include <QApplication>

CalibrationDialog::CalibrationDialog(QWidget *parent)
    : QDialog(parent)
    , m_singlePointRadio(nullptr)
    , m_multiPointRadio(nullptr)
    , m_circleRadio(nullptr)
    , m_stageAssistedRadio(nullptr)
    , m_methodButtonGroup(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
    , m_selectedMethod(None)
{
    initializeUI();
}

CalibrationDialog::CalibrationMethod CalibrationDialog::getSelectedMethod() const
{
    return m_selectedMethod;
}

void CalibrationDialog::initializeUI()
{
    setWindowTitle("选择标定方式");
    setModal(true);
    // 增大固定尺寸，以容纳更多说明文本（包括圆标定）
    setFixedSize(440, 440);
    
    // 创建主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // 添加标题
    QLabel* titleLabel = new QLabel("请选择像素标定方式：", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);
    
    // 添加标定方式选项组
    QGroupBox* methodGroup = createMethodGroup();
    mainLayout->addWidget(methodGroup);
    
    // 添加弹簧，将按钮推到底部
    mainLayout->addStretch();
    
    // 添加按钮组
    QWidget* buttonWidget = createButtonGroup();
    mainLayout->addWidget(buttonWidget);
    
    // 连接信号
    connect(m_okButton, &QPushButton::clicked, this, &CalibrationDialog::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &CalibrationDialog::onCancelClicked);

    connect(m_singlePointRadio, &QRadioButton::toggled, this, &CalibrationDialog::updateOkButtonState);
    connect(m_multiPointRadio, &QRadioButton::toggled, this, &CalibrationDialog::updateOkButtonState);
    connect(m_circleRadio, &QRadioButton::toggled, this, &CalibrationDialog::updateOkButtonState);
    connect(m_stageAssistedRadio, &QRadioButton::toggled, this, &CalibrationDialog::updateOkButtonState);
    updateOkButtonState();
}

QGroupBox* CalibrationDialog::createMethodGroup()
{
    QGroupBox* groupBox = new QGroupBox("标定方式", this);
    QVBoxLayout* layout = new QVBoxLayout(groupBox);
    layout->setSpacing(10);
    
    // 创建按钮组
    m_methodButtonGroup = new QButtonGroup(this);
    
    // 单点标定选项
    m_singlePointRadio = new QRadioButton("单点标定", this);
    QLabel* singlePointDesc = new QLabel("  适用于快速标定，绘制一条已知长度的线段", this);
    singlePointDesc->setStyleSheet("color: #666; font-size: 10px;");
    m_methodButtonGroup->addButton(m_singlePointRadio, static_cast<int>(SinglePoint));
    
    // 多点标定选项
    m_multiPointRadio = new QRadioButton("多点标定", this);
    QLabel* multiPointDesc = new QLabel("  适用于高精度测量，使用多个已知距离进行标定", this);
    multiPointDesc->setStyleSheet("color: #666; font-size: 10px;");
    m_methodButtonGroup->addButton(m_multiPointRadio, static_cast<int>(MultiPoint));
    
    // 圆标定选项
    m_circleRadio = new QRadioButton("圆标定", this);
    QLabel* circleDesc = new QLabel("  适用于使用已知半径的标准圆，通过 ROI 自动检测圆进行标定", this);
    circleDesc->setStyleSheet("color: #666; font-size: 10px;");
    m_methodButtonGroup->addButton(m_circleRadio, static_cast<int>(Circle));

    // 载物台辅助标定选项
    m_stageAssistedRadio = new QRadioButton("载物台辅助标定", this);
    QLabel* stageAssistedDesc = new QLabel("  适用于操作员不清楚实际距离：点选一次特征点，载物台移动后自动计算 μm/像素", this);
    stageAssistedDesc->setStyleSheet("color: #666; font-size: 10px;");
    stageAssistedDesc->setWordWrap(true);
    m_methodButtonGroup->addButton(m_stageAssistedRadio, static_cast<int>(StageAssisted));
    
    // 添加到布局
    layout->addWidget(m_stageAssistedRadio);
    layout->addWidget(stageAssistedDesc);
    layout->addSpacing(5);

    layout->addWidget(m_circleRadio);
    layout->addWidget(circleDesc);
    layout->addSpacing(5);

    layout->addWidget(m_singlePointRadio);
    layout->addWidget(singlePointDesc);
    layout->addSpacing(5);
    
    layout->addWidget(m_multiPointRadio);
    layout->addWidget(multiPointDesc);
    layout->addSpacing(5);
    
    return groupBox;
}

QWidget* CalibrationDialog::createButtonGroup()
{
    QWidget* buttonWidget = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(buttonWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // 添加弹簧，将按钮推到右侧
    layout->addStretch();
    
    // 取消按钮
    m_cancelButton = new QPushButton("取消", this);
    m_cancelButton->setMinimumSize(80, 30);
    layout->addWidget(m_cancelButton);
    
    // 确定按钮
    m_okButton = new QPushButton("确定", this);
    m_okButton->setMinimumSize(80, 30);
    m_okButton->setDefault(true);
    m_okButton->setEnabled(false);
    layout->addWidget(m_okButton);
    
    return buttonWidget;
}

void CalibrationDialog::updateOkButtonState()
{
    if (!m_okButton || !m_methodButtonGroup) {
        return;
    }
    m_okButton->setEnabled(m_methodButtonGroup->checkedId() >= 0);
}

void CalibrationDialog::onOkClicked()
{
    // 获取选择的标定方式
    int selectedId = m_methodButtonGroup->checkedId();
    if (selectedId >= 0) {
        m_selectedMethod = static_cast<CalibrationMethod>(selectedId);
    }
    
    accept();
}

void CalibrationDialog::onCancelClicked()
{
    reject();
}
