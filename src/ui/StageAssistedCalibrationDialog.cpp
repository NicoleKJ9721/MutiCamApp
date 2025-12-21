#include "StageAssistedCalibrationDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

StageAssistedCalibrationDialog::StageAssistedCalibrationDialog(QWidget* parent)
    : QDialog(parent)
{
    initializeUI();
}

void StageAssistedCalibrationDialog::setViewName(const QString& viewName)
{
    if (m_viewLabel) {
        m_viewLabel->setText(viewName);
    }
}

StageAssistedCalibrationDialog::Params StageAssistedCalibrationDialog::params() const
{
    Params p;
    p.mode = static_cast<CalibrationMode>(m_modeCombo ? m_modeCombo->currentData().toInt()
                                                      : static_cast<int>(CalibrationMode::Point));
    p.axis = static_cast<AxisControl::AxisIndex>(m_axisCombo ? m_axisCombo->currentData().toInt()
                                                             : static_cast<int>(AxisControl::AxisIndex::Y_AXIS));
    p.direction = (m_directionCombo && m_directionCombo->currentData().toInt() < 0) ? -1 : 1;
    p.distanceUm = m_distanceSpin ? m_distanceSpin->value() : 1000.0;
    p.roiSizePx = m_roiSizeSpin ? m_roiSizeSpin->value() : 80;
    p.searchRadiusPx = m_searchRadiusSpin ? m_searchRadiusSpin->value() : 500;
    return p;
}

void StageAssistedCalibrationDialog::initializeUI()
{
    setWindowTitle("载物台辅助标定");
    setModal(true);
    setFixedSize(460, 340);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(10);

    m_titleLabel = new QLabel("载物台辅助标定（μm/像素）", this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    mainLayout->addWidget(m_titleLabel);

    auto* form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);
    form->setFormAlignment(Qt::AlignLeft);

    m_viewLabel = new QLabel("-", this);
    form->addRow("视图：", m_viewLabel);

    m_modeCombo = new QComboBox(this);
    m_modeCombo->addItem("圆标定", static_cast<int>(CalibrationMode::Circle));
    m_modeCombo->addItem("平行线标定", static_cast<int>(CalibrationMode::ParallelLine));
    m_modeCombo->addItem("特征点（自动识别，可能有误差）", static_cast<int>(CalibrationMode::Point));
    m_modeCombo->setCurrentIndex(0);
    form->addRow("标定方式：", m_modeCombo);

    m_axisCombo = new QComboBox(this);
    m_axisCombo->addItem("X轴", static_cast<int>(AxisControl::AxisIndex::Z_AXIS));
    m_axisCombo->addItem("Y轴", static_cast<int>(AxisControl::AxisIndex::Y_AXIS));
    m_axisCombo->addItem("Z轴", static_cast<int>(AxisControl::AxisIndex::X_AXIS));
    m_axisCombo->setCurrentIndex(1);
    form->addRow("选择轴：", m_axisCombo);

    m_directionCombo = new QComboBox(this);
    m_directionCombo->addItem("正方向 (+)", 1);
    m_directionCombo->addItem("负方向 (-)", -1);
    form->addRow("方向：", m_directionCombo);

    m_distanceSpin = new QDoubleSpinBox(this);
    m_distanceSpin->setDecimals(2);
    m_distanceSpin->setRange(0.1, 200000.0);
    m_distanceSpin->setValue(1000.0);
    m_distanceSpin->setSuffix(" μm");
    form->addRow("移动距离：", m_distanceSpin);

    m_roiSizeSpin = new QSpinBox(this);
    m_roiSizeSpin->setRange(20, 300);
    m_roiSizeSpin->setValue(80);
    m_roiSizeSpin->setSuffix(" px");
    form->addRow("特征ROI：", m_roiSizeSpin);

    m_searchRadiusSpin = new QSpinBox(this);
    m_searchRadiusSpin->setRange(50, 2000);
    m_searchRadiusSpin->setValue(500);
    m_searchRadiusSpin->setSuffix(" px");
    form->addRow("搜索半径：", m_searchRadiusSpin);

    mainLayout->addLayout(form);

    auto* hint = new QLabel("提示：特征点模式先点选特征点，软件会移动载物台并自动计算比例。\n"
                            "圆/平行线模式需在移动前后分别绘制圆或直线。", this);
    hint->setStyleSheet("color: #666; font-size: 10px;");
    hint->setWordWrap(true);
    mainLayout->addWidget(hint);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setText("开始");
    buttons->button(QDialogButtonBox::Cancel)->setText("取消");
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttons);
}
