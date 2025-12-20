#pragma once

#include <QDialog>

#include "../controllers/AxisControllerEnums.h"

class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QSpinBox;

class StageAssistedCalibrationDialog : public QDialog
{
    Q_OBJECT

public:
    enum class CalibrationMode {
        Point = 0,
        Circle,
        ParallelLine
    };

    struct Params {
        CalibrationMode mode = CalibrationMode::Point;
        AxisControl::AxisIndex axis = AxisControl::AxisIndex::Y_AXIS;
        int direction = 1;          // +1 / -1
        double distanceUm = 1000.0; // 载物台移动距离（μm）
        int roiSizePx = 80;         // 追踪ROI边长（px）
        int searchRadiusPx = 500;   // 搜索半径（px）
    };

    explicit StageAssistedCalibrationDialog(QWidget* parent = nullptr);

    void setViewName(const QString& viewName);
    Params params() const;

private:
    void initializeUI();

    QLabel* m_titleLabel = nullptr;
    QLabel* m_viewLabel = nullptr;
    QComboBox* m_modeCombo = nullptr;
    QComboBox* m_axisCombo = nullptr;
    QComboBox* m_directionCombo = nullptr;
    QDoubleSpinBox* m_distanceSpin = nullptr;
    QSpinBox* m_roiSizeSpin = nullptr;
    QSpinBox* m_searchRadiusSpin = nullptr;
};
