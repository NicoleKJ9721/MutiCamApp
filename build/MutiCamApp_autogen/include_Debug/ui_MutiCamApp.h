/********************************************************************************
** Form generated from reading UI file 'MutiCamApp.ui'
**
** Created by: Qt User Interface Compiler version 6.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MUTICAMAPP_H
#define UI_MUTICAMAPP_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MutiCamApp
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QTabWidget *tabWidget;
    QWidget *tabMain;
    QVBoxLayout *verticalLayout_main;
    QHBoxLayout *horizontalLayout_top;
    QGroupBox *groupBoxVertical;
    QGridLayout *gridLayout_vertical;
    QLabel *lbVerticalView;
    QGroupBox *groupBoxLeft;
    QGridLayout *gridLayout_left;
    QLabel *lbLeftView;
    QHBoxLayout *horizontalLayout_bottom;
    QGroupBox *groupBoxFront;
    QGridLayout *gridLayout_front;
    QLabel *lbFrontView;
    QGroupBox *groupBoxControl;
    QVBoxLayout *verticalLayout_control;
    QHBoxLayout *horizontalLayout_gridAndDetection;
    QGroupBox *groupBoxDrawTools;
    QVBoxLayout *verticalLayout_drawTools;
    QHBoxLayout *horizontalLayout_drawRow1;
    QPushButton *btnDrawPoint;
    QPushButton *btnDrawStraight;
    QPushButton *btnDrawSimpleCircle;
    QHBoxLayout *horizontalLayout_drawRow2;
    QPushButton *btnDrawParallel;
    QPushButton *btnDraw2Line;
    QPushButton *btnDrawFineCircle;
    QPushButton *btnCan1StepDraw;
    QPushButton *btnClearDrawings;
    QPushButton *btnSaveImage;
    QGroupBox *groupBoxGrid;
    QVBoxLayout *verticalLayout_gridControl;
    QHBoxLayout *horizontalLayout_gridDensity;
    QLabel *labelGridDensity;
    QLineEdit *leGridDensity;
    QLabel *labelPixel;
    QPushButton *btnCancelGrids;
    QGroupBox *groupBoxAutoDetection;
    QHBoxLayout *horizontalLayout;
    QPushButton *btnCan1StepDet;
    QPushButton *btnCircleDet;
    QPushButton *btnLineDet;
    QGroupBox *groupBoxMeasurement;
    QHBoxLayout *horizontalLayout_measureButtons;
    QPushButton *btnStartMeasure;
    QPushButton *btnStopMeasure;
    QWidget *tabVertical;
    QGridLayout *gridLayout_tabVertical;
    QGroupBox *groupBoxVerticalMain;
    QVBoxLayout *verticalLayout_verticalMain;
    QGroupBox *groupBoxVerticalGrid;
    QGridLayout *gridLayout_verticalGrid;
    QLabel *labelVerticalGridDensity;
    QLineEdit *leGridDensVertical;
    QPushButton *btnCancelGridsVertical;
    QLabel *labelVerticalPixel;
    QGroupBox *groupBoxVerticalAutoMeasure;
    QVBoxLayout *verticalLayout_verticalAutoMeasure;
    QPushButton *btnLineDetVertical;
    QPushButton *btnCircleDetVertical;
    QPushButton *btnCan1StepDetVertical;
    QGroupBox *groupBoxVerticalDrawTools;
    QVBoxLayout *verticalLayout_verticalDrawTools;
    QPushButton *btnDrawPointVertical;
    QPushButton *btnDrawStraightVertical;
    QPushButton *btnDrawSimpleCircleVertical;
    QPushButton *btnDrawFineCircleVertical;
    QPushButton *btnDrawParallelVertical;
    QPushButton *btnDraw2LineVertical;
    QPushButton *btnCan1StepDrawVertical;
    QPushButton *btnClearDrawingsVertical;
    QPushButton *btnCalibrationVertical;
    QGroupBox *groupBoxVerticalDisplay;
    QVBoxLayout *verticalLayout_verticalDisplay;
    QLabel *lbVerticalView2;
    QPushButton *btnSaveImageVertical;
    QWidget *tabLeft;
    QHBoxLayout *horizontalLayout_tabLeft;
    QGroupBox *groupBoxLeftMain;
    QVBoxLayout *verticalLayout_leftMain;
    QGroupBox *groupBoxLeftGrid;
    QGridLayout *gridLayout_leftGrid;
    QLineEdit *leGridDensLeft;
    QLabel *labelLeftGridDensity;
    QLabel *labelLeftPixel;
    QPushButton *btnCancelGridsLeft;
    QGroupBox *groupBoxLeftAutoMeasure;
    QVBoxLayout *verticalLayout_leftAutoMeasure;
    QPushButton *btnLineDetLeft;
    QPushButton *btnCircleDetLeft;
    QPushButton *btnCan1StepDetLeft;
    QGroupBox *groupBoxLeftDrawTools;
    QVBoxLayout *verticalLayout_leftDrawTools;
    QPushButton *btnDrawPointLeft;
    QPushButton *btnDrawStraightLeft;
    QPushButton *btnDrawSimpleCircleLeft;
    QPushButton *btnDrawFineCircleLeft;
    QPushButton *btnDrawParallelLeft;
    QPushButton *btnDraw2LineLeft;
    QPushButton *btnCan1StepDrawLeft;
    QPushButton *btnClearDrawingsLeft;
    QPushButton *btnCalibrationLeft;
    QGroupBox *groupBoxLeftDisplay;
    QVBoxLayout *verticalLayout_leftDisplay;
    QLabel *lbLeftView2;
    QPushButton *btnSaveImageLeft;
    QWidget *tabFront;
    QHBoxLayout *horizontalLayout_tabFront;
    QGroupBox *groupBoxFrontMain;
    QVBoxLayout *verticalLayout_frontMain;
    QGroupBox *groupBoxFrontGrid;
    QGridLayout *gridLayout_frontGrid;
    QPushButton *btnCancelGridsFront;
    QLabel *labelFrontPixel;
    QLabel *labelFrontGridDensity;
    QLineEdit *leGridDensFront;
    QGroupBox *groupBoxFrontAutoMeasure;
    QVBoxLayout *verticalLayout_frontAutoMeasure;
    QPushButton *btnLineDetFront;
    QPushButton *btnCircleDetFront;
    QPushButton *btnCan1StepDetFront;
    QGroupBox *groupBoxFrontDrawTools;
    QVBoxLayout *verticalLayout_frontDrawTools;
    QPushButton *btnDrawPointFront;
    QPushButton *btnDrawStraightFront;
    QPushButton *btnDrawSimpleCircleFront;
    QPushButton *btnDrawFineCircleFront;
    QPushButton *btnDrawParallelFront;
    QPushButton *btnDraw2LineFront;
    QPushButton *btnCan1StepDrawFront;
    QPushButton *btnClearDrawingsFront;
    QPushButton *btnCalibrationFront;
    QGroupBox *groupBoxFrontDisplay;
    QVBoxLayout *verticalLayout_frontDisplay;
    QLabel *lbFrontView2;
    QPushButton *btnSaveImageFront;
    QStatusBar *statusbar;
    QMenuBar *menubar;

    void setupUi(QMainWindow *MutiCamApp)
    {
        if (MutiCamApp->objectName().isEmpty())
            MutiCamApp->setObjectName("MutiCamApp");
        MutiCamApp->resize(1014, 739);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MutiCamApp->sizePolicy().hasHeightForWidth());
        MutiCamApp->setSizePolicy(sizePolicy);
        MutiCamApp->setMinimumSize(QSize(512, 400));
        MutiCamApp->setMaximumSize(QSize(16777215, 16777215));
        centralwidget = new QWidget(MutiCamApp);
        centralwidget->setObjectName("centralwidget");
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setObjectName("gridLayout");
        tabWidget = new QTabWidget(centralwidget);
        tabWidget->setObjectName("tabWidget");
        QFont font;
        font.setPointSize(12);
        font.setBold(true);
        tabWidget->setFont(font);
        tabMain = new QWidget();
        tabMain->setObjectName("tabMain");
        verticalLayout_main = new QVBoxLayout(tabMain);
        verticalLayout_main->setObjectName("verticalLayout_main");
        horizontalLayout_top = new QHBoxLayout();
        horizontalLayout_top->setObjectName("horizontalLayout_top");
        groupBoxVertical = new QGroupBox(tabMain);
        groupBoxVertical->setObjectName("groupBoxVertical");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Ignored);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(groupBoxVertical->sizePolicy().hasHeightForWidth());
        groupBoxVertical->setSizePolicy(sizePolicy1);
        groupBoxVertical->setFont(font);
        gridLayout_vertical = new QGridLayout(groupBoxVertical);
        gridLayout_vertical->setSpacing(0);
        gridLayout_vertical->setObjectName("gridLayout_vertical");
        gridLayout_vertical->setSizeConstraint(QLayout::SizeConstraint::SetDefaultConstraint);
        gridLayout_vertical->setContentsMargins(0, 0, 0, 0);
        lbVerticalView = new QLabel(groupBoxVertical);
        lbVerticalView->setObjectName("lbVerticalView");
        lbVerticalView->setMinimumSize(QSize(0, 0));
        lbVerticalView->setMaximumSize(QSize(16777215, 16777215));
        lbVerticalView->setAlignment(Qt::AlignmentFlag::AlignCenter);

        gridLayout_vertical->addWidget(lbVerticalView, 0, 0, 1, 1);


        horizontalLayout_top->addWidget(groupBoxVertical);

        groupBoxLeft = new QGroupBox(tabMain);
        groupBoxLeft->setObjectName("groupBoxLeft");
        sizePolicy1.setHeightForWidth(groupBoxLeft->sizePolicy().hasHeightForWidth());
        groupBoxLeft->setSizePolicy(sizePolicy1);
        groupBoxLeft->setFont(font);
        gridLayout_left = new QGridLayout(groupBoxLeft);
        gridLayout_left->setSpacing(0);
        gridLayout_left->setObjectName("gridLayout_left");
        gridLayout_left->setSizeConstraint(QLayout::SizeConstraint::SetDefaultConstraint);
        gridLayout_left->setContentsMargins(0, 0, 0, 0);
        lbLeftView = new QLabel(groupBoxLeft);
        lbLeftView->setObjectName("lbLeftView");
        lbLeftView->setMinimumSize(QSize(0, 0));
        lbLeftView->setMaximumSize(QSize(16777215, 16777215));
        lbLeftView->setAlignment(Qt::AlignmentFlag::AlignCenter);

        gridLayout_left->addWidget(lbLeftView, 0, 0, 1, 1);


        horizontalLayout_top->addWidget(groupBoxLeft);

        horizontalLayout_top->setStretch(0, 1);
        horizontalLayout_top->setStretch(1, 1);

        verticalLayout_main->addLayout(horizontalLayout_top);

        horizontalLayout_bottom = new QHBoxLayout();
        horizontalLayout_bottom->setObjectName("horizontalLayout_bottom");
        groupBoxFront = new QGroupBox(tabMain);
        groupBoxFront->setObjectName("groupBoxFront");
        sizePolicy1.setHeightForWidth(groupBoxFront->sizePolicy().hasHeightForWidth());
        groupBoxFront->setSizePolicy(sizePolicy1);
        groupBoxFront->setFont(font);
        gridLayout_front = new QGridLayout(groupBoxFront);
        gridLayout_front->setSpacing(0);
        gridLayout_front->setObjectName("gridLayout_front");
        gridLayout_front->setSizeConstraint(QLayout::SizeConstraint::SetDefaultConstraint);
        gridLayout_front->setContentsMargins(0, 0, 0, 0);
        lbFrontView = new QLabel(groupBoxFront);
        lbFrontView->setObjectName("lbFrontView");
        lbFrontView->setMinimumSize(QSize(0, 0));
        lbFrontView->setMaximumSize(QSize(16777215, 16777215));
        lbFrontView->setAlignment(Qt::AlignmentFlag::AlignCenter);

        gridLayout_front->addWidget(lbFrontView, 0, 0, 1, 1);


        horizontalLayout_bottom->addWidget(groupBoxFront);

        groupBoxControl = new QGroupBox(tabMain);
        groupBoxControl->setObjectName("groupBoxControl");
        sizePolicy1.setHeightForWidth(groupBoxControl->sizePolicy().hasHeightForWidth());
        groupBoxControl->setSizePolicy(sizePolicy1);
        groupBoxControl->setFont(font);
        groupBoxControl->setAcceptDrops(false);
        groupBoxControl->setFlat(false);
        verticalLayout_control = new QVBoxLayout(groupBoxControl);
        verticalLayout_control->setObjectName("verticalLayout_control");
        horizontalLayout_gridAndDetection = new QHBoxLayout();
        horizontalLayout_gridAndDetection->setObjectName("horizontalLayout_gridAndDetection");
        groupBoxDrawTools = new QGroupBox(groupBoxControl);
        groupBoxDrawTools->setObjectName("groupBoxDrawTools");
        QFont font1;
        font1.setPointSize(9);
        font1.setBold(true);
        groupBoxDrawTools->setFont(font1);
        groupBoxDrawTools->setAlignment(Qt::AlignmentFlag::AlignCenter);
        verticalLayout_drawTools = new QVBoxLayout(groupBoxDrawTools);
        verticalLayout_drawTools->setObjectName("verticalLayout_drawTools");
        horizontalLayout_drawRow1 = new QHBoxLayout();
        horizontalLayout_drawRow1->setObjectName("horizontalLayout_drawRow1");
        btnDrawPoint = new QPushButton(groupBoxDrawTools);
        btnDrawPoint->setObjectName("btnDrawPoint");
        btnDrawPoint->setFont(font1);

        horizontalLayout_drawRow1->addWidget(btnDrawPoint);

        btnDrawStraight = new QPushButton(groupBoxDrawTools);
        btnDrawStraight->setObjectName("btnDrawStraight");
        btnDrawStraight->setFont(font1);

        horizontalLayout_drawRow1->addWidget(btnDrawStraight);

        btnDrawSimpleCircle = new QPushButton(groupBoxDrawTools);
        btnDrawSimpleCircle->setObjectName("btnDrawSimpleCircle");
        btnDrawSimpleCircle->setFont(font1);

        horizontalLayout_drawRow1->addWidget(btnDrawSimpleCircle);


        verticalLayout_drawTools->addLayout(horizontalLayout_drawRow1);

        horizontalLayout_drawRow2 = new QHBoxLayout();
        horizontalLayout_drawRow2->setObjectName("horizontalLayout_drawRow2");
        btnDrawParallel = new QPushButton(groupBoxDrawTools);
        btnDrawParallel->setObjectName("btnDrawParallel");
        btnDrawParallel->setFont(font1);

        horizontalLayout_drawRow2->addWidget(btnDrawParallel);

        btnDraw2Line = new QPushButton(groupBoxDrawTools);
        btnDraw2Line->setObjectName("btnDraw2Line");
        btnDraw2Line->setFont(font1);

        horizontalLayout_drawRow2->addWidget(btnDraw2Line);

        btnDrawFineCircle = new QPushButton(groupBoxDrawTools);
        btnDrawFineCircle->setObjectName("btnDrawFineCircle");
        btnDrawFineCircle->setFont(font1);

        horizontalLayout_drawRow2->addWidget(btnDrawFineCircle);


        verticalLayout_drawTools->addLayout(horizontalLayout_drawRow2);

        btnCan1StepDraw = new QPushButton(groupBoxDrawTools);
        btnCan1StepDraw->setObjectName("btnCan1StepDraw");
        QFont font2;
        font2.setPointSize(10);
        font2.setBold(true);
        btnCan1StepDraw->setFont(font2);

        verticalLayout_drawTools->addWidget(btnCan1StepDraw);

        btnClearDrawings = new QPushButton(groupBoxDrawTools);
        btnClearDrawings->setObjectName("btnClearDrawings");
        btnClearDrawings->setFont(font2);

        verticalLayout_drawTools->addWidget(btnClearDrawings);

        btnSaveImage = new QPushButton(groupBoxDrawTools);
        btnSaveImage->setObjectName("btnSaveImage");
        btnSaveImage->setFont(font2);

        verticalLayout_drawTools->addWidget(btnSaveImage);


        horizontalLayout_gridAndDetection->addWidget(groupBoxDrawTools);

        groupBoxGrid = new QGroupBox(groupBoxControl);
        groupBoxGrid->setObjectName("groupBoxGrid");
        groupBoxGrid->setFont(font1);
        verticalLayout_gridControl = new QVBoxLayout(groupBoxGrid);
        verticalLayout_gridControl->setObjectName("verticalLayout_gridControl");
        horizontalLayout_gridDensity = new QHBoxLayout();
        horizontalLayout_gridDensity->setObjectName("horizontalLayout_gridDensity");
        labelGridDensity = new QLabel(groupBoxGrid);
        labelGridDensity->setObjectName("labelGridDensity");
        labelGridDensity->setFont(font1);

        horizontalLayout_gridDensity->addWidget(labelGridDensity);

        leGridDensity = new QLineEdit(groupBoxGrid);
        leGridDensity->setObjectName("leGridDensity");
        leGridDensity->setMinimumSize(QSize(0, 0));
        leGridDensity->setMaximumSize(QSize(60, 16777215));
        leGridDensity->setAlignment(Qt::AlignmentFlag::AlignCenter);

        horizontalLayout_gridDensity->addWidget(leGridDensity);

        labelPixel = new QLabel(groupBoxGrid);
        labelPixel->setObjectName("labelPixel");

        horizontalLayout_gridDensity->addWidget(labelPixel);

        btnCancelGrids = new QPushButton(groupBoxGrid);
        btnCancelGrids->setObjectName("btnCancelGrids");
        btnCancelGrids->setFont(font1);

        horizontalLayout_gridDensity->addWidget(btnCancelGrids);


        verticalLayout_gridControl->addLayout(horizontalLayout_gridDensity);

        groupBoxAutoDetection = new QGroupBox(groupBoxGrid);
        groupBoxAutoDetection->setObjectName("groupBoxAutoDetection");
        groupBoxAutoDetection->setFont(font1);
        horizontalLayout = new QHBoxLayout(groupBoxAutoDetection);
        horizontalLayout->setObjectName("horizontalLayout");
        btnCan1StepDet = new QPushButton(groupBoxAutoDetection);
        btnCan1StepDet->setObjectName("btnCan1StepDet");
        btnCan1StepDet->setFont(font1);

        horizontalLayout->addWidget(btnCan1StepDet);

        btnCircleDet = new QPushButton(groupBoxAutoDetection);
        btnCircleDet->setObjectName("btnCircleDet");
        btnCircleDet->setFont(font1);

        horizontalLayout->addWidget(btnCircleDet);

        btnLineDet = new QPushButton(groupBoxAutoDetection);
        btnLineDet->setObjectName("btnLineDet");
        btnLineDet->setFont(font1);

        horizontalLayout->addWidget(btnLineDet);


        verticalLayout_gridControl->addWidget(groupBoxAutoDetection);

        groupBoxMeasurement = new QGroupBox(groupBoxGrid);
        groupBoxMeasurement->setObjectName("groupBoxMeasurement");
        groupBoxMeasurement->setFont(font1);
        horizontalLayout_measureButtons = new QHBoxLayout(groupBoxMeasurement);
        horizontalLayout_measureButtons->setObjectName("horizontalLayout_measureButtons");
        btnStartMeasure = new QPushButton(groupBoxMeasurement);
        btnStartMeasure->setObjectName("btnStartMeasure");
        btnStartMeasure->setEnabled(true);
        QSizePolicy sizePolicy2(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(btnStartMeasure->sizePolicy().hasHeightForWidth());
        btnStartMeasure->setSizePolicy(sizePolicy2);
        btnStartMeasure->setMinimumSize(QSize(0, 0));
        QFont font3;
        font3.setPointSize(11);
        font3.setBold(true);
        btnStartMeasure->setFont(font3);
        btnStartMeasure->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);

        horizontalLayout_measureButtons->addWidget(btnStartMeasure);

        btnStopMeasure = new QPushButton(groupBoxMeasurement);
        btnStopMeasure->setObjectName("btnStopMeasure");
        sizePolicy2.setHeightForWidth(btnStopMeasure->sizePolicy().hasHeightForWidth());
        btnStopMeasure->setSizePolicy(sizePolicy2);
        btnStopMeasure->setMinimumSize(QSize(0, 0));
        btnStopMeasure->setFont(font3);

        horizontalLayout_measureButtons->addWidget(btnStopMeasure);


        verticalLayout_gridControl->addWidget(groupBoxMeasurement);


        horizontalLayout_gridAndDetection->addWidget(groupBoxGrid);


        verticalLayout_control->addLayout(horizontalLayout_gridAndDetection);


        horizontalLayout_bottom->addWidget(groupBoxControl);

        horizontalLayout_bottom->setStretch(0, 1);
        horizontalLayout_bottom->setStretch(1, 1);

        verticalLayout_main->addLayout(horizontalLayout_bottom);

        verticalLayout_main->setStretch(0, 1);
        verticalLayout_main->setStretch(1, 1);
        tabWidget->addTab(tabMain, QString());
        tabVertical = new QWidget();
        tabVertical->setObjectName("tabVertical");
        gridLayout_tabVertical = new QGridLayout(tabVertical);
        gridLayout_tabVertical->setObjectName("gridLayout_tabVertical");
        groupBoxVerticalMain = new QGroupBox(tabVertical);
        groupBoxVerticalMain->setObjectName("groupBoxVerticalMain");
        groupBoxVerticalMain->setFont(font);
        groupBoxVerticalMain->setAcceptDrops(false);
        groupBoxVerticalMain->setFlat(false);
        verticalLayout_verticalMain = new QVBoxLayout(groupBoxVerticalMain);
        verticalLayout_verticalMain->setObjectName("verticalLayout_verticalMain");
        groupBoxVerticalGrid = new QGroupBox(groupBoxVerticalMain);
        groupBoxVerticalGrid->setObjectName("groupBoxVerticalGrid");
        groupBoxVerticalGrid->setFont(font1);
        groupBoxVerticalGrid->setAlignment(Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignVCenter);
        gridLayout_verticalGrid = new QGridLayout(groupBoxVerticalGrid);
        gridLayout_verticalGrid->setObjectName("gridLayout_verticalGrid");
        labelVerticalGridDensity = new QLabel(groupBoxVerticalGrid);
        labelVerticalGridDensity->setObjectName("labelVerticalGridDensity");
        QSizePolicy sizePolicy3(QSizePolicy::Policy::Maximum, QSizePolicy::Policy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(labelVerticalGridDensity->sizePolicy().hasHeightForWidth());
        labelVerticalGridDensity->setSizePolicy(sizePolicy3);
        labelVerticalGridDensity->setFont(font1);
        labelVerticalGridDensity->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        gridLayout_verticalGrid->addWidget(labelVerticalGridDensity, 0, 0, 1, 2);

        leGridDensVertical = new QLineEdit(groupBoxVerticalGrid);
        leGridDensVertical->setObjectName("leGridDensVertical");
        QSizePolicy sizePolicy4(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(leGridDensVertical->sizePolicy().hasHeightForWidth());
        leGridDensVertical->setSizePolicy(sizePolicy4);
        leGridDensVertical->setMaximumSize(QSize(70, 16777215));
        leGridDensVertical->setAlignment(Qt::AlignmentFlag::AlignCenter);

        gridLayout_verticalGrid->addWidget(leGridDensVertical, 1, 0, 1, 1);

        btnCancelGridsVertical = new QPushButton(groupBoxVerticalGrid);
        btnCancelGridsVertical->setObjectName("btnCancelGridsVertical");
        btnCancelGridsVertical->setFont(font1);

        gridLayout_verticalGrid->addWidget(btnCancelGridsVertical, 2, 0, 1, 2);

        labelVerticalPixel = new QLabel(groupBoxVerticalGrid);
        labelVerticalPixel->setObjectName("labelVerticalPixel");
        sizePolicy.setHeightForWidth(labelVerticalPixel->sizePolicy().hasHeightForWidth());
        labelVerticalPixel->setSizePolicy(sizePolicy);

        gridLayout_verticalGrid->addWidget(labelVerticalPixel, 1, 1, 1, 1);


        verticalLayout_verticalMain->addWidget(groupBoxVerticalGrid);

        groupBoxVerticalAutoMeasure = new QGroupBox(groupBoxVerticalMain);
        groupBoxVerticalAutoMeasure->setObjectName("groupBoxVerticalAutoMeasure");
        groupBoxVerticalAutoMeasure->setFont(font1);
        groupBoxVerticalAutoMeasure->setAlignment(Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignVCenter);
        verticalLayout_verticalAutoMeasure = new QVBoxLayout(groupBoxVerticalAutoMeasure);
        verticalLayout_verticalAutoMeasure->setObjectName("verticalLayout_verticalAutoMeasure");
        btnLineDetVertical = new QPushButton(groupBoxVerticalAutoMeasure);
        btnLineDetVertical->setObjectName("btnLineDetVertical");
        btnLineDetVertical->setFont(font1);

        verticalLayout_verticalAutoMeasure->addWidget(btnLineDetVertical);

        btnCircleDetVertical = new QPushButton(groupBoxVerticalAutoMeasure);
        btnCircleDetVertical->setObjectName("btnCircleDetVertical");
        btnCircleDetVertical->setFont(font1);

        verticalLayout_verticalAutoMeasure->addWidget(btnCircleDetVertical);

        btnCan1StepDetVertical = new QPushButton(groupBoxVerticalAutoMeasure);
        btnCan1StepDetVertical->setObjectName("btnCan1StepDetVertical");
        btnCan1StepDetVertical->setFont(font1);

        verticalLayout_verticalAutoMeasure->addWidget(btnCan1StepDetVertical);


        verticalLayout_verticalMain->addWidget(groupBoxVerticalAutoMeasure);

        groupBoxVerticalDrawTools = new QGroupBox(groupBoxVerticalMain);
        groupBoxVerticalDrawTools->setObjectName("groupBoxVerticalDrawTools");
        groupBoxVerticalDrawTools->setFont(font1);
        groupBoxVerticalDrawTools->setAlignment(Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignVCenter);
        verticalLayout_verticalDrawTools = new QVBoxLayout(groupBoxVerticalDrawTools);
        verticalLayout_verticalDrawTools->setObjectName("verticalLayout_verticalDrawTools");
        btnDrawPointVertical = new QPushButton(groupBoxVerticalDrawTools);
        btnDrawPointVertical->setObjectName("btnDrawPointVertical");
        btnDrawPointVertical->setFont(font1);

        verticalLayout_verticalDrawTools->addWidget(btnDrawPointVertical);

        btnDrawStraightVertical = new QPushButton(groupBoxVerticalDrawTools);
        btnDrawStraightVertical->setObjectName("btnDrawStraightVertical");
        btnDrawStraightVertical->setFont(font1);

        verticalLayout_verticalDrawTools->addWidget(btnDrawStraightVertical);

        btnDrawSimpleCircleVertical = new QPushButton(groupBoxVerticalDrawTools);
        btnDrawSimpleCircleVertical->setObjectName("btnDrawSimpleCircleVertical");
        btnDrawSimpleCircleVertical->setFont(font1);

        verticalLayout_verticalDrawTools->addWidget(btnDrawSimpleCircleVertical);

        btnDrawFineCircleVertical = new QPushButton(groupBoxVerticalDrawTools);
        btnDrawFineCircleVertical->setObjectName("btnDrawFineCircleVertical");
        btnDrawFineCircleVertical->setFont(font1);

        verticalLayout_verticalDrawTools->addWidget(btnDrawFineCircleVertical);

        btnDrawParallelVertical = new QPushButton(groupBoxVerticalDrawTools);
        btnDrawParallelVertical->setObjectName("btnDrawParallelVertical");
        btnDrawParallelVertical->setFont(font1);

        verticalLayout_verticalDrawTools->addWidget(btnDrawParallelVertical);

        btnDraw2LineVertical = new QPushButton(groupBoxVerticalDrawTools);
        btnDraw2LineVertical->setObjectName("btnDraw2LineVertical");
        btnDraw2LineVertical->setFont(font1);

        verticalLayout_verticalDrawTools->addWidget(btnDraw2LineVertical);

        btnCan1StepDrawVertical = new QPushButton(groupBoxVerticalDrawTools);
        btnCan1StepDrawVertical->setObjectName("btnCan1StepDrawVertical");
        btnCan1StepDrawVertical->setFont(font1);

        verticalLayout_verticalDrawTools->addWidget(btnCan1StepDrawVertical);

        btnClearDrawingsVertical = new QPushButton(groupBoxVerticalDrawTools);
        btnClearDrawingsVertical->setObjectName("btnClearDrawingsVertical");
        btnClearDrawingsVertical->setFont(font1);

        verticalLayout_verticalDrawTools->addWidget(btnClearDrawingsVertical);

        btnCalibrationVertical = new QPushButton(groupBoxVerticalDrawTools);
        btnCalibrationVertical->setObjectName("btnCalibrationVertical");
        btnCalibrationVertical->setFont(font1);

        verticalLayout_verticalDrawTools->addWidget(btnCalibrationVertical);


        verticalLayout_verticalMain->addWidget(groupBoxVerticalDrawTools);

        verticalLayout_verticalMain->setStretch(1, 1);
        verticalLayout_verticalMain->setStretch(2, 3);

        gridLayout_tabVertical->addWidget(groupBoxVerticalMain, 0, 0, 1, 1);

        groupBoxVerticalDisplay = new QGroupBox(tabVertical);
        groupBoxVerticalDisplay->setObjectName("groupBoxVerticalDisplay");
        groupBoxVerticalDisplay->setFont(font);
        verticalLayout_verticalDisplay = new QVBoxLayout(groupBoxVerticalDisplay);
        verticalLayout_verticalDisplay->setObjectName("verticalLayout_verticalDisplay");
        lbVerticalView2 = new QLabel(groupBoxVerticalDisplay);
        lbVerticalView2->setObjectName("lbVerticalView2");
        QSizePolicy sizePolicy5(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(lbVerticalView2->sizePolicy().hasHeightForWidth());
        lbVerticalView2->setSizePolicy(sizePolicy5);
        lbVerticalView2->setMinimumSize(QSize(0, 0));
        lbVerticalView2->setMaximumSize(QSize(16777215, 16777215));
        lbVerticalView2->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout_verticalDisplay->addWidget(lbVerticalView2);

        btnSaveImageVertical = new QPushButton(groupBoxVerticalDisplay);
        btnSaveImageVertical->setObjectName("btnSaveImageVertical");
        btnSaveImageVertical->setFont(font1);

        verticalLayout_verticalDisplay->addWidget(btnSaveImageVertical);


        gridLayout_tabVertical->addWidget(groupBoxVerticalDisplay, 0, 1, 1, 1);

        tabWidget->addTab(tabVertical, QString());
        tabLeft = new QWidget();
        tabLeft->setObjectName("tabLeft");
        horizontalLayout_tabLeft = new QHBoxLayout(tabLeft);
        horizontalLayout_tabLeft->setObjectName("horizontalLayout_tabLeft");
        groupBoxLeftMain = new QGroupBox(tabLeft);
        groupBoxLeftMain->setObjectName("groupBoxLeftMain");
        groupBoxLeftMain->setFont(font);
        verticalLayout_leftMain = new QVBoxLayout(groupBoxLeftMain);
        verticalLayout_leftMain->setObjectName("verticalLayout_leftMain");
        groupBoxLeftGrid = new QGroupBox(groupBoxLeftMain);
        groupBoxLeftGrid->setObjectName("groupBoxLeftGrid");
        groupBoxLeftGrid->setFont(font1);
        gridLayout_leftGrid = new QGridLayout(groupBoxLeftGrid);
        gridLayout_leftGrid->setObjectName("gridLayout_leftGrid");
        leGridDensLeft = new QLineEdit(groupBoxLeftGrid);
        leGridDensLeft->setObjectName("leGridDensLeft");
        sizePolicy4.setHeightForWidth(leGridDensLeft->sizePolicy().hasHeightForWidth());
        leGridDensLeft->setSizePolicy(sizePolicy4);
        leGridDensLeft->setMaximumSize(QSize(70, 16777215));
        leGridDensLeft->setAlignment(Qt::AlignmentFlag::AlignCenter);

        gridLayout_leftGrid->addWidget(leGridDensLeft, 1, 0, 1, 1);

        labelLeftGridDensity = new QLabel(groupBoxLeftGrid);
        labelLeftGridDensity->setObjectName("labelLeftGridDensity");
        sizePolicy3.setHeightForWidth(labelLeftGridDensity->sizePolicy().hasHeightForWidth());
        labelLeftGridDensity->setSizePolicy(sizePolicy3);
        labelLeftGridDensity->setFont(font1);
        labelLeftGridDensity->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        gridLayout_leftGrid->addWidget(labelLeftGridDensity, 0, 0, 1, 2);

        labelLeftPixel = new QLabel(groupBoxLeftGrid);
        labelLeftPixel->setObjectName("labelLeftPixel");

        gridLayout_leftGrid->addWidget(labelLeftPixel, 1, 1, 1, 1);

        btnCancelGridsLeft = new QPushButton(groupBoxLeftGrid);
        btnCancelGridsLeft->setObjectName("btnCancelGridsLeft");
        QSizePolicy sizePolicy6(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);
        sizePolicy6.setHorizontalStretch(0);
        sizePolicy6.setVerticalStretch(0);
        sizePolicy6.setHeightForWidth(btnCancelGridsLeft->sizePolicy().hasHeightForWidth());
        btnCancelGridsLeft->setSizePolicy(sizePolicy6);
        btnCancelGridsLeft->setFont(font1);

        gridLayout_leftGrid->addWidget(btnCancelGridsLeft, 2, 0, 1, 2);


        verticalLayout_leftMain->addWidget(groupBoxLeftGrid);

        groupBoxLeftAutoMeasure = new QGroupBox(groupBoxLeftMain);
        groupBoxLeftAutoMeasure->setObjectName("groupBoxLeftAutoMeasure");
        groupBoxLeftAutoMeasure->setFont(font1);
        verticalLayout_leftAutoMeasure = new QVBoxLayout(groupBoxLeftAutoMeasure);
        verticalLayout_leftAutoMeasure->setObjectName("verticalLayout_leftAutoMeasure");
        btnLineDetLeft = new QPushButton(groupBoxLeftAutoMeasure);
        btnLineDetLeft->setObjectName("btnLineDetLeft");
        btnLineDetLeft->setFont(font1);

        verticalLayout_leftAutoMeasure->addWidget(btnLineDetLeft);

        btnCircleDetLeft = new QPushButton(groupBoxLeftAutoMeasure);
        btnCircleDetLeft->setObjectName("btnCircleDetLeft");
        btnCircleDetLeft->setFont(font1);

        verticalLayout_leftAutoMeasure->addWidget(btnCircleDetLeft);

        btnCan1StepDetLeft = new QPushButton(groupBoxLeftAutoMeasure);
        btnCan1StepDetLeft->setObjectName("btnCan1StepDetLeft");
        btnCan1StepDetLeft->setFont(font1);

        verticalLayout_leftAutoMeasure->addWidget(btnCan1StepDetLeft);


        verticalLayout_leftMain->addWidget(groupBoxLeftAutoMeasure);

        groupBoxLeftDrawTools = new QGroupBox(groupBoxLeftMain);
        groupBoxLeftDrawTools->setObjectName("groupBoxLeftDrawTools");
        groupBoxLeftDrawTools->setFont(font1);
        verticalLayout_leftDrawTools = new QVBoxLayout(groupBoxLeftDrawTools);
        verticalLayout_leftDrawTools->setObjectName("verticalLayout_leftDrawTools");
        btnDrawPointLeft = new QPushButton(groupBoxLeftDrawTools);
        btnDrawPointLeft->setObjectName("btnDrawPointLeft");
        btnDrawPointLeft->setFont(font1);

        verticalLayout_leftDrawTools->addWidget(btnDrawPointLeft);

        btnDrawStraightLeft = new QPushButton(groupBoxLeftDrawTools);
        btnDrawStraightLeft->setObjectName("btnDrawStraightLeft");
        btnDrawStraightLeft->setFont(font1);

        verticalLayout_leftDrawTools->addWidget(btnDrawStraightLeft);

        btnDrawSimpleCircleLeft = new QPushButton(groupBoxLeftDrawTools);
        btnDrawSimpleCircleLeft->setObjectName("btnDrawSimpleCircleLeft");
        btnDrawSimpleCircleLeft->setFont(font1);

        verticalLayout_leftDrawTools->addWidget(btnDrawSimpleCircleLeft);

        btnDrawFineCircleLeft = new QPushButton(groupBoxLeftDrawTools);
        btnDrawFineCircleLeft->setObjectName("btnDrawFineCircleLeft");
        btnDrawFineCircleLeft->setFont(font1);

        verticalLayout_leftDrawTools->addWidget(btnDrawFineCircleLeft);

        btnDrawParallelLeft = new QPushButton(groupBoxLeftDrawTools);
        btnDrawParallelLeft->setObjectName("btnDrawParallelLeft");
        btnDrawParallelLeft->setFont(font1);

        verticalLayout_leftDrawTools->addWidget(btnDrawParallelLeft);

        btnDraw2LineLeft = new QPushButton(groupBoxLeftDrawTools);
        btnDraw2LineLeft->setObjectName("btnDraw2LineLeft");
        btnDraw2LineLeft->setFont(font1);

        verticalLayout_leftDrawTools->addWidget(btnDraw2LineLeft);

        btnCan1StepDrawLeft = new QPushButton(groupBoxLeftDrawTools);
        btnCan1StepDrawLeft->setObjectName("btnCan1StepDrawLeft");
        btnCan1StepDrawLeft->setFont(font1);

        verticalLayout_leftDrawTools->addWidget(btnCan1StepDrawLeft);

        btnClearDrawingsLeft = new QPushButton(groupBoxLeftDrawTools);
        btnClearDrawingsLeft->setObjectName("btnClearDrawingsLeft");
        btnClearDrawingsLeft->setFont(font1);

        verticalLayout_leftDrawTools->addWidget(btnClearDrawingsLeft);

        btnCalibrationLeft = new QPushButton(groupBoxLeftDrawTools);
        btnCalibrationLeft->setObjectName("btnCalibrationLeft");
        btnCalibrationLeft->setFont(font1);

        verticalLayout_leftDrawTools->addWidget(btnCalibrationLeft);


        verticalLayout_leftMain->addWidget(groupBoxLeftDrawTools);

        verticalLayout_leftMain->setStretch(1, 1);
        verticalLayout_leftMain->setStretch(2, 3);

        horizontalLayout_tabLeft->addWidget(groupBoxLeftMain);

        groupBoxLeftDisplay = new QGroupBox(tabLeft);
        groupBoxLeftDisplay->setObjectName("groupBoxLeftDisplay");
        groupBoxLeftDisplay->setFont(font);
        verticalLayout_leftDisplay = new QVBoxLayout(groupBoxLeftDisplay);
        verticalLayout_leftDisplay->setObjectName("verticalLayout_leftDisplay");
        lbLeftView2 = new QLabel(groupBoxLeftDisplay);
        lbLeftView2->setObjectName("lbLeftView2");
        sizePolicy5.setHeightForWidth(lbLeftView2->sizePolicy().hasHeightForWidth());
        lbLeftView2->setSizePolicy(sizePolicy5);
        lbLeftView2->setMinimumSize(QSize(0, 0));
        lbLeftView2->setMaximumSize(QSize(16777215, 16777215));
        lbLeftView2->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout_leftDisplay->addWidget(lbLeftView2);

        btnSaveImageLeft = new QPushButton(groupBoxLeftDisplay);
        btnSaveImageLeft->setObjectName("btnSaveImageLeft");
        btnSaveImageLeft->setFont(font1);

        verticalLayout_leftDisplay->addWidget(btnSaveImageLeft);


        horizontalLayout_tabLeft->addWidget(groupBoxLeftDisplay);

        tabWidget->addTab(tabLeft, QString());
        tabFront = new QWidget();
        tabFront->setObjectName("tabFront");
        horizontalLayout_tabFront = new QHBoxLayout(tabFront);
        horizontalLayout_tabFront->setObjectName("horizontalLayout_tabFront");
        groupBoxFrontMain = new QGroupBox(tabFront);
        groupBoxFrontMain->setObjectName("groupBoxFrontMain");
        groupBoxFrontMain->setFont(font);
        verticalLayout_frontMain = new QVBoxLayout(groupBoxFrontMain);
        verticalLayout_frontMain->setObjectName("verticalLayout_frontMain");
        groupBoxFrontGrid = new QGroupBox(groupBoxFrontMain);
        groupBoxFrontGrid->setObjectName("groupBoxFrontGrid");
        groupBoxFrontGrid->setFont(font1);
        gridLayout_frontGrid = new QGridLayout(groupBoxFrontGrid);
        gridLayout_frontGrid->setObjectName("gridLayout_frontGrid");
        btnCancelGridsFront = new QPushButton(groupBoxFrontGrid);
        btnCancelGridsFront->setObjectName("btnCancelGridsFront");
        btnCancelGridsFront->setFont(font1);

        gridLayout_frontGrid->addWidget(btnCancelGridsFront, 2, 0, 1, 2);

        labelFrontPixel = new QLabel(groupBoxFrontGrid);
        labelFrontPixel->setObjectName("labelFrontPixel");
        sizePolicy6.setHeightForWidth(labelFrontPixel->sizePolicy().hasHeightForWidth());
        labelFrontPixel->setSizePolicy(sizePolicy6);

        gridLayout_frontGrid->addWidget(labelFrontPixel, 1, 1, 1, 1);

        labelFrontGridDensity = new QLabel(groupBoxFrontGrid);
        labelFrontGridDensity->setObjectName("labelFrontGridDensity");
        sizePolicy3.setHeightForWidth(labelFrontGridDensity->sizePolicy().hasHeightForWidth());
        labelFrontGridDensity->setSizePolicy(sizePolicy3);
        labelFrontGridDensity->setFont(font1);
        labelFrontGridDensity->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        gridLayout_frontGrid->addWidget(labelFrontGridDensity, 0, 0, 1, 2);

        leGridDensFront = new QLineEdit(groupBoxFrontGrid);
        leGridDensFront->setObjectName("leGridDensFront");
        sizePolicy4.setHeightForWidth(leGridDensFront->sizePolicy().hasHeightForWidth());
        leGridDensFront->setSizePolicy(sizePolicy4);
        leGridDensFront->setMaximumSize(QSize(70, 16777215));
        leGridDensFront->setAlignment(Qt::AlignmentFlag::AlignCenter);

        gridLayout_frontGrid->addWidget(leGridDensFront, 1, 0, 1, 1);


        verticalLayout_frontMain->addWidget(groupBoxFrontGrid);

        groupBoxFrontAutoMeasure = new QGroupBox(groupBoxFrontMain);
        groupBoxFrontAutoMeasure->setObjectName("groupBoxFrontAutoMeasure");
        groupBoxFrontAutoMeasure->setFont(font1);
        verticalLayout_frontAutoMeasure = new QVBoxLayout(groupBoxFrontAutoMeasure);
        verticalLayout_frontAutoMeasure->setObjectName("verticalLayout_frontAutoMeasure");
        btnLineDetFront = new QPushButton(groupBoxFrontAutoMeasure);
        btnLineDetFront->setObjectName("btnLineDetFront");
        btnLineDetFront->setFont(font1);

        verticalLayout_frontAutoMeasure->addWidget(btnLineDetFront);

        btnCircleDetFront = new QPushButton(groupBoxFrontAutoMeasure);
        btnCircleDetFront->setObjectName("btnCircleDetFront");
        btnCircleDetFront->setFont(font1);

        verticalLayout_frontAutoMeasure->addWidget(btnCircleDetFront);

        btnCan1StepDetFront = new QPushButton(groupBoxFrontAutoMeasure);
        btnCan1StepDetFront->setObjectName("btnCan1StepDetFront");
        btnCan1StepDetFront->setFont(font1);

        verticalLayout_frontAutoMeasure->addWidget(btnCan1StepDetFront);


        verticalLayout_frontMain->addWidget(groupBoxFrontAutoMeasure);

        groupBoxFrontDrawTools = new QGroupBox(groupBoxFrontMain);
        groupBoxFrontDrawTools->setObjectName("groupBoxFrontDrawTools");
        groupBoxFrontDrawTools->setFont(font1);
        verticalLayout_frontDrawTools = new QVBoxLayout(groupBoxFrontDrawTools);
        verticalLayout_frontDrawTools->setObjectName("verticalLayout_frontDrawTools");
        btnDrawPointFront = new QPushButton(groupBoxFrontDrawTools);
        btnDrawPointFront->setObjectName("btnDrawPointFront");
        btnDrawPointFront->setFont(font1);

        verticalLayout_frontDrawTools->addWidget(btnDrawPointFront);

        btnDrawStraightFront = new QPushButton(groupBoxFrontDrawTools);
        btnDrawStraightFront->setObjectName("btnDrawStraightFront");
        btnDrawStraightFront->setFont(font1);

        verticalLayout_frontDrawTools->addWidget(btnDrawStraightFront);

        btnDrawSimpleCircleFront = new QPushButton(groupBoxFrontDrawTools);
        btnDrawSimpleCircleFront->setObjectName("btnDrawSimpleCircleFront");
        btnDrawSimpleCircleFront->setFont(font1);

        verticalLayout_frontDrawTools->addWidget(btnDrawSimpleCircleFront);

        btnDrawFineCircleFront = new QPushButton(groupBoxFrontDrawTools);
        btnDrawFineCircleFront->setObjectName("btnDrawFineCircleFront");
        btnDrawFineCircleFront->setFont(font1);

        verticalLayout_frontDrawTools->addWidget(btnDrawFineCircleFront);

        btnDrawParallelFront = new QPushButton(groupBoxFrontDrawTools);
        btnDrawParallelFront->setObjectName("btnDrawParallelFront");
        btnDrawParallelFront->setFont(font1);

        verticalLayout_frontDrawTools->addWidget(btnDrawParallelFront);

        btnDraw2LineFront = new QPushButton(groupBoxFrontDrawTools);
        btnDraw2LineFront->setObjectName("btnDraw2LineFront");
        btnDraw2LineFront->setFont(font1);

        verticalLayout_frontDrawTools->addWidget(btnDraw2LineFront);

        btnCan1StepDrawFront = new QPushButton(groupBoxFrontDrawTools);
        btnCan1StepDrawFront->setObjectName("btnCan1StepDrawFront");
        btnCan1StepDrawFront->setFont(font1);

        verticalLayout_frontDrawTools->addWidget(btnCan1StepDrawFront);

        btnClearDrawingsFront = new QPushButton(groupBoxFrontDrawTools);
        btnClearDrawingsFront->setObjectName("btnClearDrawingsFront");
        btnClearDrawingsFront->setFont(font1);

        verticalLayout_frontDrawTools->addWidget(btnClearDrawingsFront);

        btnCalibrationFront = new QPushButton(groupBoxFrontDrawTools);
        btnCalibrationFront->setObjectName("btnCalibrationFront");
        btnCalibrationFront->setFont(font1);

        verticalLayout_frontDrawTools->addWidget(btnCalibrationFront);


        verticalLayout_frontMain->addWidget(groupBoxFrontDrawTools);

        verticalLayout_frontMain->setStretch(1, 1);
        verticalLayout_frontMain->setStretch(2, 3);

        horizontalLayout_tabFront->addWidget(groupBoxFrontMain);

        groupBoxFrontDisplay = new QGroupBox(tabFront);
        groupBoxFrontDisplay->setObjectName("groupBoxFrontDisplay");
        groupBoxFrontDisplay->setFont(font);
        verticalLayout_frontDisplay = new QVBoxLayout(groupBoxFrontDisplay);
        verticalLayout_frontDisplay->setObjectName("verticalLayout_frontDisplay");
        lbFrontView2 = new QLabel(groupBoxFrontDisplay);
        lbFrontView2->setObjectName("lbFrontView2");
        sizePolicy5.setHeightForWidth(lbFrontView2->sizePolicy().hasHeightForWidth());
        lbFrontView2->setSizePolicy(sizePolicy5);
        lbFrontView2->setMinimumSize(QSize(0, 0));
        lbFrontView2->setMaximumSize(QSize(16777215, 16777215));
        lbFrontView2->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout_frontDisplay->addWidget(lbFrontView2);

        btnSaveImageFront = new QPushButton(groupBoxFrontDisplay);
        btnSaveImageFront->setObjectName("btnSaveImageFront");
        btnSaveImageFront->setFont(font1);

        verticalLayout_frontDisplay->addWidget(btnSaveImageFront);


        horizontalLayout_tabFront->addWidget(groupBoxFrontDisplay);

        tabWidget->addTab(tabFront, QString());

        gridLayout->addWidget(tabWidget, 0, 0, 1, 1);

        MutiCamApp->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(MutiCamApp);
        statusbar->setObjectName("statusbar");
        MutiCamApp->setStatusBar(statusbar);
        menubar = new QMenuBar(MutiCamApp);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1014, 33));
        MutiCamApp->setMenuBar(menubar);

        retranslateUi(MutiCamApp);

        tabWidget->setCurrentIndex(3);


        QMetaObject::connectSlotsByName(MutiCamApp);
    } // setupUi

    void retranslateUi(QMainWindow *MutiCamApp)
    {
        MutiCamApp->setWindowTitle(QCoreApplication::translate("MutiCamApp", "\351\235\266\350\243\205\351\205\215\345\257\271\346\216\245\346\265\213\351\207\217\350\275\257\344\273\266", nullptr));
        groupBoxVertical->setTitle(QCoreApplication::translate("MutiCamApp", "\345\236\202\347\233\264\350\247\206\345\233\276", nullptr));
        lbVerticalView->setText(QString());
        groupBoxLeft->setTitle(QCoreApplication::translate("MutiCamApp", "\345\267\246\344\276\247\350\247\206\345\233\276", nullptr));
        lbLeftView->setText(QString());
        groupBoxFront->setTitle(QCoreApplication::translate("MutiCamApp", "\345\257\271\345\220\221\350\247\206\345\233\276", nullptr));
        lbFrontView->setText(QString());
        groupBoxControl->setTitle(QCoreApplication::translate("MutiCamApp", "\346\216\247\345\210\266\351\235\242\346\235\277", nullptr));
        groupBoxDrawTools->setTitle(QCoreApplication::translate("MutiCamApp", "\347\273\230\347\224\273", nullptr));
        btnDrawPoint->setText(QCoreApplication::translate("MutiCamApp", "\347\202\271", nullptr));
        btnDrawStraight->setText(QCoreApplication::translate("MutiCamApp", "\347\233\264\347\272\277", nullptr));
        btnDrawSimpleCircle->setText(QCoreApplication::translate("MutiCamApp", "\347\256\200\345\215\225\345\234\206", nullptr));
        btnDrawParallel->setText(QCoreApplication::translate("MutiCamApp", "\345\271\263\350\241\214\347\272\277", nullptr));
        btnDraw2Line->setText(QCoreApplication::translate("MutiCamApp", "\347\272\277\344\270\216\347\272\277", nullptr));
        btnDrawFineCircle->setText(QCoreApplication::translate("MutiCamApp", "\347\262\276\347\273\206\345\234\206", nullptr));
        btnCan1StepDraw->setText(QCoreApplication::translate("MutiCamApp", "\346\222\244\351\224\200\344\270\212\346\255\245\347\273\230\347\224\273", nullptr));
        btnClearDrawings->setText(QCoreApplication::translate("MutiCamApp", "\346\270\205\347\251\272\347\273\230\347\224\273", nullptr));
        btnSaveImage->setText(QCoreApplication::translate("MutiCamApp", "\344\277\235\345\255\230\345\233\276\345\203\217\357\274\210\345\216\237\345\247\213+\345\217\257\350\247\206\345\214\226\357\274\211", nullptr));
        groupBoxGrid->setTitle(QCoreApplication::translate("MutiCamApp", "\347\275\221\346\240\274", nullptr));
        labelGridDensity->setText(QCoreApplication::translate("MutiCamApp", "\347\275\221\346\240\274\345\257\206\345\272\246", nullptr));
        labelPixel->setText(QCoreApplication::translate("MutiCamApp", "\345\203\217\347\264\240", nullptr));
        btnCancelGrids->setText(QCoreApplication::translate("MutiCamApp", "\345\217\226\346\266\210\347\275\221\346\240\274", nullptr));
        groupBoxAutoDetection->setTitle(QCoreApplication::translate("MutiCamApp", "\350\207\252\345\212\250\346\265\213\351\207\217", nullptr));
        btnCan1StepDet->setText(QCoreApplication::translate("MutiCamApp", "\346\222\244\351\224\200\344\270\212\346\255\245", nullptr));
        btnCircleDet->setText(QCoreApplication::translate("MutiCamApp", "\345\234\206\346\237\245\346\211\276", nullptr));
        btnLineDet->setText(QCoreApplication::translate("MutiCamApp", "\347\233\264\347\272\277\346\237\245\346\211\276", nullptr));
        groupBoxMeasurement->setTitle(QCoreApplication::translate("MutiCamApp", "\350\277\220\350\241\214", nullptr));
        btnStartMeasure->setStyleSheet(QCoreApplication::translate("MutiCamApp", "QPushButton { background-color: #4CAF50; color: white; border: none; border-radius: 5px; }\n"
"QPushButton:hover { background-color: #45a049; }\n"
"QPushButton:pressed { background-color: #3d8b40; }", nullptr));
        btnStartMeasure->setText(QCoreApplication::translate("MutiCamApp", "\345\274\200\345\247\213\346\265\213\351\207\217", nullptr));
        btnStopMeasure->setStyleSheet(QCoreApplication::translate("MutiCamApp", "QPushButton { background-color: #f44336; color: white; border: none; border-radius: 5px; }\n"
"QPushButton:hover { background-color: #da190b; }\n"
"QPushButton:pressed { background-color: #b71c1c; }", nullptr));
        btnStopMeasure->setText(QCoreApplication::translate("MutiCamApp", "\345\201\234\346\255\242\346\265\213\351\207\217", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabMain), QCoreApplication::translate("MutiCamApp", "\344\270\273\347\225\214\351\235\242", nullptr));
        groupBoxVerticalMain->setTitle(QString());
        groupBoxVerticalGrid->setTitle(QCoreApplication::translate("MutiCamApp", "\347\275\221\346\240\274", nullptr));
        labelVerticalGridDensity->setText(QCoreApplication::translate("MutiCamApp", "\350\276\223\345\205\245\347\275\221\346\240\274\345\257\206\345\272\246", nullptr));
        btnCancelGridsVertical->setText(QCoreApplication::translate("MutiCamApp", "\345\217\226\346\266\210\347\275\221\346\240\274", nullptr));
        labelVerticalPixel->setText(QCoreApplication::translate("MutiCamApp", "\345\203\217\347\264\240", nullptr));
        groupBoxVerticalAutoMeasure->setTitle(QCoreApplication::translate("MutiCamApp", "\350\207\252\345\212\250\346\265\213\351\207\217", nullptr));
        btnLineDetVertical->setText(QCoreApplication::translate("MutiCamApp", "\347\233\264\347\272\277\346\237\245\346\211\276", nullptr));
        btnCircleDetVertical->setText(QCoreApplication::translate("MutiCamApp", "\345\234\206\346\237\245\346\211\276", nullptr));
        btnCan1StepDetVertical->setText(QCoreApplication::translate("MutiCamApp", "\346\222\244\351\224\200\350\257\206\345\210\253", nullptr));
        groupBoxVerticalDrawTools->setTitle(QCoreApplication::translate("MutiCamApp", "\347\273\230\345\233\276\345\267\245\345\205\267", nullptr));
        btnDrawPointVertical->setText(QCoreApplication::translate("MutiCamApp", "\347\202\271", nullptr));
        btnDrawStraightVertical->setText(QCoreApplication::translate("MutiCamApp", "\347\233\264\347\272\277", nullptr));
        btnDrawSimpleCircleVertical->setText(QCoreApplication::translate("MutiCamApp", "\347\256\200\345\215\225\345\234\206", nullptr));
        btnDrawFineCircleVertical->setText(QCoreApplication::translate("MutiCamApp", "\347\262\276\347\241\256\345\234\206", nullptr));
        btnDrawParallelVertical->setText(QCoreApplication::translate("MutiCamApp", "\345\271\263\350\241\214\347\272\277", nullptr));
        btnDraw2LineVertical->setText(QCoreApplication::translate("MutiCamApp", "\350\247\222\345\272\246\347\272\277", nullptr));
        btnCan1StepDrawVertical->setText(QCoreApplication::translate("MutiCamApp", "\346\222\244\351\224\200", nullptr));
        btnClearDrawingsVertical->setText(QCoreApplication::translate("MutiCamApp", "\346\270\205\351\231\244", nullptr));
        btnCalibrationVertical->setText(QCoreApplication::translate("MutiCamApp", "\346\240\207\345\256\232", nullptr));
        groupBoxVerticalDisplay->setTitle(QCoreApplication::translate("MutiCamApp", "\345\233\276\345\203\217\346\230\276\347\244\272", nullptr));
        lbVerticalView2->setText(QString());
        btnSaveImageVertical->setText(QCoreApplication::translate("MutiCamApp", "\344\277\235\345\255\230\345\233\276\345\203\217", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabVertical), QCoreApplication::translate("MutiCamApp", "\345\236\202\347\233\264\350\247\206\345\233\276", nullptr));
        groupBoxLeftMain->setTitle(QString());
        groupBoxLeftGrid->setTitle(QCoreApplication::translate("MutiCamApp", "\347\275\221\346\240\274", nullptr));
        labelLeftGridDensity->setText(QCoreApplication::translate("MutiCamApp", "\350\276\223\345\205\245\347\275\221\346\240\274\345\257\206\345\272\246", nullptr));
        labelLeftPixel->setText(QCoreApplication::translate("MutiCamApp", "\345\203\217\347\264\240", nullptr));
        btnCancelGridsLeft->setText(QCoreApplication::translate("MutiCamApp", "\345\217\226\346\266\210\347\275\221\346\240\274", nullptr));
        groupBoxLeftAutoMeasure->setTitle(QCoreApplication::translate("MutiCamApp", "\350\207\252\345\212\250\346\265\213\351\207\217", nullptr));
        btnLineDetLeft->setText(QCoreApplication::translate("MutiCamApp", "\347\233\264\347\272\277\346\237\245\346\211\276", nullptr));
        btnCircleDetLeft->setText(QCoreApplication::translate("MutiCamApp", "\345\234\206\346\237\245\346\211\276", nullptr));
        btnCan1StepDetLeft->setText(QCoreApplication::translate("MutiCamApp", "\346\222\244\351\224\200\350\257\206\345\210\253", nullptr));
        groupBoxLeftDrawTools->setTitle(QCoreApplication::translate("MutiCamApp", "\347\273\230\345\233\276\345\267\245\345\205\267", nullptr));
        btnDrawPointLeft->setText(QCoreApplication::translate("MutiCamApp", "\347\202\271", nullptr));
        btnDrawStraightLeft->setText(QCoreApplication::translate("MutiCamApp", "\347\233\264\347\272\277", nullptr));
        btnDrawSimpleCircleLeft->setText(QCoreApplication::translate("MutiCamApp", "\347\256\200\345\215\225\345\234\206", nullptr));
        btnDrawFineCircleLeft->setText(QCoreApplication::translate("MutiCamApp", "\347\262\276\347\241\256\345\234\206", nullptr));
        btnDrawParallelLeft->setText(QCoreApplication::translate("MutiCamApp", "\345\271\263\350\241\214\347\272\277", nullptr));
        btnDraw2LineLeft->setText(QCoreApplication::translate("MutiCamApp", "\350\247\222\345\272\246\347\272\277", nullptr));
        btnCan1StepDrawLeft->setText(QCoreApplication::translate("MutiCamApp", "\346\222\244\351\224\200", nullptr));
        btnClearDrawingsLeft->setText(QCoreApplication::translate("MutiCamApp", "\346\270\205\351\231\244", nullptr));
        btnCalibrationLeft->setText(QCoreApplication::translate("MutiCamApp", "\346\240\207\345\256\232", nullptr));
        groupBoxLeftDisplay->setTitle(QCoreApplication::translate("MutiCamApp", "\345\233\276\345\203\217\346\230\276\347\244\272", nullptr));
        lbLeftView2->setText(QString());
        btnSaveImageLeft->setText(QCoreApplication::translate("MutiCamApp", "\344\277\235\345\255\230\345\233\276\345\203\217", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabLeft), QCoreApplication::translate("MutiCamApp", "\345\267\246\344\276\247\350\247\206\345\233\276", nullptr));
        groupBoxFrontMain->setTitle(QString());
        groupBoxFrontGrid->setTitle(QCoreApplication::translate("MutiCamApp", "\347\275\221\346\240\274", nullptr));
        btnCancelGridsFront->setText(QCoreApplication::translate("MutiCamApp", "\345\217\226\346\266\210\347\275\221\346\240\274", nullptr));
        labelFrontPixel->setText(QCoreApplication::translate("MutiCamApp", "\345\203\217\347\264\240", nullptr));
        labelFrontGridDensity->setText(QCoreApplication::translate("MutiCamApp", "\350\276\223\345\205\245\347\275\221\346\240\274\345\257\206\345\272\246", nullptr));
        groupBoxFrontAutoMeasure->setTitle(QCoreApplication::translate("MutiCamApp", "\350\207\252\345\212\250\346\265\213\351\207\217", nullptr));
        btnLineDetFront->setText(QCoreApplication::translate("MutiCamApp", "\347\233\264\347\272\277\346\237\245\346\211\276", nullptr));
        btnCircleDetFront->setText(QCoreApplication::translate("MutiCamApp", "\345\234\206\346\237\245\346\211\276", nullptr));
        btnCan1StepDetFront->setText(QCoreApplication::translate("MutiCamApp", "\346\222\244\351\224\200\350\257\206\345\210\253", nullptr));
        groupBoxFrontDrawTools->setTitle(QCoreApplication::translate("MutiCamApp", "\347\273\230\345\233\276\345\267\245\345\205\267", nullptr));
        btnDrawPointFront->setText(QCoreApplication::translate("MutiCamApp", "\347\202\271", nullptr));
        btnDrawStraightFront->setText(QCoreApplication::translate("MutiCamApp", "\347\233\264\347\272\277", nullptr));
        btnDrawSimpleCircleFront->setText(QCoreApplication::translate("MutiCamApp", "\347\256\200\345\215\225\345\234\206", nullptr));
        btnDrawFineCircleFront->setText(QCoreApplication::translate("MutiCamApp", "\347\262\276\347\241\256\345\234\206", nullptr));
        btnDrawParallelFront->setText(QCoreApplication::translate("MutiCamApp", "\345\271\263\350\241\214\347\272\277", nullptr));
        btnDraw2LineFront->setText(QCoreApplication::translate("MutiCamApp", "\350\247\222\345\272\246\347\272\277", nullptr));
        btnCan1StepDrawFront->setText(QCoreApplication::translate("MutiCamApp", "\346\222\244\351\224\200", nullptr));
        btnClearDrawingsFront->setText(QCoreApplication::translate("MutiCamApp", "\346\270\205\351\231\244", nullptr));
        btnCalibrationFront->setText(QCoreApplication::translate("MutiCamApp", "\346\240\207\345\256\232", nullptr));
        groupBoxFrontDisplay->setTitle(QCoreApplication::translate("MutiCamApp", "\345\233\276\345\203\217\346\230\276\347\244\272", nullptr));
        lbFrontView2->setText(QString());
        btnSaveImageFront->setText(QCoreApplication::translate("MutiCamApp", "\344\277\235\345\255\230\345\233\276\345\203\217", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabFront), QCoreApplication::translate("MutiCamApp", "\345\257\271\345\220\221\350\247\206\345\233\276", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MutiCamApp: public Ui_MutiCamApp {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MUTICAMAPP_H
