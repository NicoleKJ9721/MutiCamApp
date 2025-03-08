import warnings
warnings.filterwarnings("ignore", category=DeprecationWarning)
import sys
import os
# 添加项目根目录到系统路径
current_dir = os.path.dirname(os.path.abspath(__file__))
if current_dir not in sys.path:
    sys.path.append(current_dir)
import numpy as np
import cv2
from typing import List
from datetime import datetime
from PyQt5.QtWidgets import *
from PyQt5.QtGui import QImage, QPixmap, QIntValidator
from PyQt5.QtCore import Qt, QThread, QPoint
from mainwindow import Ui_MainWindow
from Tools.camera_thread import CameraThread
from Tools.settings_manager import SettingsManager
from Tools.log_manager import LogManager
from Tools.measurement_manager import MeasurementManager, DrawingType, DrawingObject
from Tools.drawing_manager import DrawingManager
from Tools.grid_container import GridContainer

class MainApp(QMainWindow, Ui_MainWindow): # type: ignore
    def __init__(self):
        super().__init__()
        self.setupUi(self)
        
        # 为每个视图维护独立的帧
        self.current_frame_vertical = None
        self.current_frame_left = None
        self.current_frame_front = None
        
        # 确保log_manager最先初始化
        self.log_manager = LogManager()
        
        # 创建 DrawingManager，传入 self（MainApp实例）
        self.drawing_manager = DrawingManager(self)
        
        # 修改这里：传入settings_file参数
        self.settings_manager = SettingsManager(settings_file="./Settings/settings.json", log_manager=self.log_manager)
        
        # 先加载设置
        self.settings_manager.load_settings(self)
        
        # 设置窗口大小
        try:
            width = int(self.ledUIWidth.text())
            height = int(self.ledUIHeight.text())
            self.resize(width, height)
        except ValueError:
            self.resize(1000, 800)
        
        # 然后再初始化UI和连接信号
        self._init_ui()
        self._connect_signals()
        
        # 初始化日志管理器
        self.log_manager.log_ui_operation("程序启动")
        
        # 初始化相机线程
        self.ver_camera_thread = None
        self.left_camera_thread = None
        self.front_camera_thread = None

        # 初始化测量管理器，传入self作为父对象
        self.ver_measurement = MeasurementManager(self)
        self.left_measurement = MeasurementManager(self)
        self.front_measurement = MeasurementManager(self)
        self.ver_measurement_2 = MeasurementManager(self)
        
        # 当前活动的视图
        self.active_view = None
        self.active_measurement = None

        # 添加标志位
        self.is_undoing = False
        
    def _init_ui(self):
        """初始化UI控件"""
        
        # 初始化按钮状态
        self.btnStopMeasure.setEnabled(False)

        # 初始化绘画管理器
        self.drawing_manager = DrawingManager(self)
        
        # 先设置主界面视图的绘画功能
        self.drawing_manager.setup_view(self.lbVerticalView, "vertical")
        self.drawing_manager.setup_view(self.lbLeftView, "left")      # 主界面左视图
        self.drawing_manager.setup_view(self.lbFrontView, "front")
        
        # 设置选项卡视图的绘画功能
        self.drawing_manager.setup_view(self.lbVerticalView_2, "vertical_2")
        self.drawing_manager.setup_view(self.lbLeftView_2, "left_2")  # 选项卡左视图
        self.drawing_manager.setup_view(self.lbFrontView_2, "front_2")
        
        # 为网格密度输入框添加整数验证器
        int_validator = QIntValidator()
        int_validator.setBottom(1)  # 设置最小值为1
        self.leGridDens.setValidator(int_validator)
        self.leGridDens_Ver.setValidator(int_validator)
        self.leGridDens_Left.setValidator(int_validator)
        self.leGridDens_Front.setValidator(int_validator)
        
        # 创建网格容器并包装主界面垂直视图
        self.vertical_grid_container = GridContainer(self)
        parent_layout = self.lbVerticalView.parent().layout()
        parent_layout.removeWidget(self.lbVerticalView)
        self.vertical_grid_container.addWidget(self.lbVerticalView)
        parent_layout.addWidget(self.vertical_grid_container, 0, 0, 1, 1)
        
        # 创建网格容器并包装主界面左侧视图
        self.left_grid_container = GridContainer(self)
        parent_layout = self.lbLeftView.parent().layout()
        parent_layout.removeWidget(self.lbLeftView)
        self.left_grid_container.addWidget(self.lbLeftView)
        parent_layout.addWidget(self.left_grid_container, 0, 0, 1, 1)
        
        # 创建网格容器并包装主界面对向视图
        self.front_grid_container = GridContainer(self)
        parent_layout = self.lbFrontView.parent().layout()
        parent_layout.removeWidget(self.lbFrontView)
        self.front_grid_container.addWidget(self.lbFrontView)
        parent_layout.addWidget(self.front_grid_container, 0, 0, 1, 1)
        
        # 创建网格容器并包装垂直视图选项卡
        self.vertical_grid_container_2 = GridContainer(self)
        parent_layout_2 = self.lbVerticalView_2.parent().layout()
        parent_layout_2.removeWidget(self.lbVerticalView_2)
        self.vertical_grid_container_2.addWidget(self.lbVerticalView_2)
        parent_layout_2.addWidget(self.vertical_grid_container_2, 0, 0, 1, 1)
        
        # 创建网格容器并包装左侧视图选项卡
        self.left_grid_container_2 = GridContainer(self)
        parent_layout_2 = self.lbLeftView_2.parent().layout()
        parent_layout_2.removeWidget(self.lbLeftView_2)
        self.left_grid_container_2.addWidget(self.lbLeftView_2)
        parent_layout_2.addWidget(self.left_grid_container_2, 0, 0, 1, 1)
        
        # 创建网格容器并包装对向视图选项卡
        self.front_grid_container_2 = GridContainer(self)
        parent_layout_2 = self.lbFrontView_2.parent().layout()
        parent_layout_2.removeWidget(self.lbFrontView_2)
        self.front_grid_container_2.addWidget(self.lbFrontView_2)
        parent_layout_2.addWidget(self.front_grid_container_2, 0, 0, 1, 1)

        # 启用按键事件
        self.lbVerticalView.setFocusPolicy(Qt.StrongFocus)
        self.lbLeftView.setFocusPolicy(Qt.StrongFocus)
        self.lbFrontView.setFocusPolicy(Qt.StrongFocus)
        
        # 设置选项卡视图的按键功能
        self.lbVerticalView_2.setFocusPolicy(Qt.StrongFocus)
        self.lbLeftView_2.setFocusPolicy(Qt.StrongFocus)
        self.lbFrontView_2.setFocusPolicy(Qt.StrongFocus)

        # 创建右键菜单
        self.context_menu = QMenu(self) # type: ignore
        
        # 创建所有菜单项
        self.delete_action = QAction("删除", self) # type: ignore
        self.delete_action.triggered.connect(self.delete_selected_objects)
        
        
        self.two_lines_action = QAction("线与线测量", self)  # type: ignore # 添加线与线选项
        self.two_lines_action.triggered.connect(self.measure_two_lines)
        
        # 直接添加所有菜单项
        self.context_menu.addAction(self.delete_action)
        self.context_menu.addSeparator()
        self.context_menu.addAction(self.two_lines_action)

        # 添加主界面保存图像按钮信号连接
        self.btnSaveImage.clicked.connect(self.save_all_views)

    def _connect_signals(self):
        """连接信号和槽"""
        # 连接相机控制按钮
        # 相机控制信号
        self.btnStartMeasure.clicked.connect(self.start_cameras)
        self.btnStopMeasure.clicked.connect(self.stop_cameras)
        
        # 连接选项卡切换信号
        self.tabWidget.currentChanged.connect(self.handle_tab_change)
        
        # 连接网格控制按钮 - 主界面
        self.btnCancelGrids.clicked.connect(lambda: self.clear_grid(0))
        self.leGridDens.editingFinished.connect(lambda: self.apply_grid_spacing(0))
        
        # 连接网格控制按钮 - 垂直视图选项卡
        self.btnCancelGrids_Ver.clicked.connect(lambda: self.clear_grid(1))
        self.leGridDens_Ver.editingFinished.connect(lambda: self.apply_grid_spacing(1))
        
        # 连接网格控制按钮 - 左视图选项卡
        self.btnCancelGrids_Left.clicked.connect(lambda: self.clear_grid(2))
        self.leGridDens_Left.editingFinished.connect(lambda: self.apply_grid_spacing(2))
        
        # 连接网格控制按钮 - 前视图选项卡
        self.btnCancelGrids_Front.clicked.connect(lambda: self.clear_grid(3))
        self.leGridDens_Front.editingFinished.connect(lambda: self.apply_grid_spacing(3))

        # 相机参数 修改为 editingFinished 信号，只在用户编辑完成时触发
        self.ledVerCamSN.editingFinished.connect(lambda: self.settings_manager.save_settings(self))
        self.ledLeftCamSN.editingFinished.connect(lambda: self.settings_manager.save_settings(self))
        self.ledFrontCamSN.editingFinished.connect(lambda: self.settings_manager.save_settings(self))

        # 直线查找 在编辑完成时保存设置
        self.ledCannyLineLow.editingFinished.connect(lambda: self.settings_manager.save_settings(self))
        self.ledCannyLineHigh.editingFinished.connect(lambda: self.settings_manager.save_settings(self))
        self.ledLineDetThreshold.editingFinished.connect(lambda: self.settings_manager.save_settings(self))
        self.ledLineDetMinLength.editingFinished.connect(lambda: self.settings_manager.save_settings(self))
        self.ledLineDetMaxGap.editingFinished.connect(lambda: self.settings_manager.save_settings(self))
        
        # 圆查找参数 在编辑完成时保存设置
        self.ledCannyCircleLow.editingFinished.connect(lambda: self.settings_manager.save_settings(self))
        self.ledCannyCircleHigh.editingFinished.connect(lambda: self.settings_manager.save_settings(self))
        self.ledCircleDetParam2.editingFinished.connect(lambda: self.settings_manager.save_settings(self))
        
        # UI尺寸修改信号 - 改用 textChanged 并添加实时更新窗口大小的功能
        self.ledUIWidth.textChanged.connect(self.update_window_size)
        self.ledUIHeight.textChanged.connect(self.update_window_size)
        
        # UI尺寸修改信号 在编辑完成时保存设置
        self.ledUIWidth.editingFinished.connect(lambda: self.settings_manager.save_settings(self))
        self.ledUIHeight.editingFinished.connect(lambda: self.settings_manager.save_settings(self))

        # 主界面绘画功能按钮信号连接
        self.btnDrawPoint.clicked.connect(self.drawing_manager.start_point_measurement)  # 点按钮
        self.btnDrawStraight.clicked.connect(self.start_drawing_mode) # 直线按钮
        self.btnDrawParallel.clicked.connect(self.start_drawing_mode) # 平行线按钮
        # self.btnDrawLine_Circle.clicked.connect(self.start_circle_line_measurement) # 线与圆测量按钮
        self.btnDraw2Line.clicked.connect(self.start_two_lines_measurement) # 线与线测量按钮
        self.btnDrawSimpleCircle.clicked.connect(self.drawing_manager.start_simple_circle_measurement)  # 简单圆按钮
        self.btnDrawFineCircle.clicked.connect(self.drawing_manager.start_fine_circle_measurement)  # 精细圆按钮
        self.btnCan1StepDraw.clicked.connect(self.undo_last_drawing)  # 撤销手动绘制
        self.btnCan1StepDet.clicked.connect(self.undo_last_detection) # 撤销自动检测
        self.btnClearDrawings.clicked.connect(
            lambda: [
                self.drawing_manager.clear_drawings(self.lbVerticalView),
                self.drawing_manager.clear_drawings(self.lbLeftView),
                self.drawing_manager.clear_drawings(self.lbFrontView)
            ]
        )
        self.btnLineDet.clicked.connect(self.start_line_detection)  # 直线检测按钮
        self.btnCircleDet.clicked.connect(self.start_circle_detection)  # 圆形检测按钮

        # 垂直选项卡绘画功能按钮信号连接
        self.btnLineDet_Ver.clicked.connect(self.start_line_detection)  # 直线检测按钮
        self.btnCircleDet_Ver.clicked.connect(self.start_circle_detection)  # 圆形检测按钮
        self.btnDrawPoint_Ver.clicked.connect(self.drawing_manager.start_point_measurement)  # 点按钮
        self.btnDrawStraight_Ver.clicked.connect(self.start_drawing_mode)  # 直线按钮
        self.btnDrawParallel_Ver.clicked.connect(self.start_drawing_mode) # 平行线按钮
        # self.btnDrawLine_Circle_Ver.clicked.connect(self.start_circle_line_measurement) # 线与圆测量按钮
        self.btnDraw2Line_Ver.clicked.connect(self.start_two_lines_measurement) # 线与线测量按钮
        self.btnDrawSimpleCircle_Ver.clicked.connect(self.drawing_manager.start_simple_circle_measurement)  # 简单圆按钮
        self.btnDrawFineCircle_Ver.clicked.connect(self.drawing_manager.start_fine_circle_measurement)  # 精细圆按钮
        self.btnCan1StepDraw_Ver.clicked.connect(self.undo_last_drawing) # 撤销手动绘制
        self.btnCan1StepDet_Ver.clicked.connect(self.undo_last_detection) # 撤销自动检测
        self.btnClearDrawings_Ver.clicked.connect(                         # 清空按钮
            lambda: self.drawing_manager.clear_drawings(self.lbVerticalView_2))
        self.btnSaveImage_Ver.clicked.connect(lambda: self.save_images('vertical'))
        self.btnCalibration_Ver.clicked.connect(self.start_calibration)

        # 左侧选项卡绘画功能按钮信号连接
        self.btnLineDet_Left.clicked.connect(self.start_line_detection)  # 直线检测按钮
        self.btnCircleDet_Left.clicked.connect(self.start_circle_detection)  # 圆形检测按钮
        self.btnDrawPoint_Left.clicked.connect(self.drawing_manager.start_point_measurement)  # 点按钮
        self.btnDrawStraight_Left.clicked.connect(self.start_drawing_mode)  # 直线按钮
        self.btnDrawParallel_Left.clicked.connect(self.start_drawing_mode) # 平行线按钮
        # self.btnDrawLine_Circle_Left.clicked.connect(self.start_circle_line_measurement) # 线与圆测量按钮
        self.btnDraw2Line_Left.clicked.connect(self.start_two_lines_measurement) # 线与线测量按钮
        self.btnDrawSimpleCircle_Left.clicked.connect(self.drawing_manager.start_simple_circle_measurement)  # 简单圆按钮
        self.btnDrawFineCircle_Left.clicked.connect(self.drawing_manager.start_fine_circle_measurement)  # 精细圆按钮
        self.btnCan1StepDraw_Left.clicked.connect(self.undo_last_drawing) # 撤销手动绘制
        self.btnCan1StepDet_Left.clicked.connect(self.undo_last_detection) # 撤销自动检测
        self.btnClearDrawings_Left.clicked.connect(                         # 清空按钮
            lambda: self.drawing_manager.clear_drawings(self.lbLeftView_2))
        self.btnSaveImage_Left.clicked.connect(lambda: self.save_images('left'))
        self.btnCalibration_Left.clicked.connect(self.start_calibration)

        # 对向选项卡绘画功能按钮信号连接
        self.btnLineDet_Front.clicked.connect(self.start_line_detection)  # 直线检测按钮
        self.btnCircleDet_Front.clicked.connect(self.start_circle_detection)  # 圆形检测按钮
        self.btnDrawPoint_Front.clicked.connect(self.drawing_manager.start_point_measurement)  # 点按钮
        self.btnDrawStraight_Front.clicked.connect(self.start_drawing_mode)  # 直线按钮
        self.btnDrawParallel_Front.clicked.connect(self.start_drawing_mode) # 平行线按钮
        # self.btnDrawLine_Circle_Front.clicked.connect(self.start_circle_line_measurement) # 线与圆测量按钮
        self.btnDraw2Line_Front.clicked.connect(self.start_two_lines_measurement) # 线与线测量按钮
        self.btnDrawSimpleCircle_Front.clicked.connect(self.drawing_manager.start_simple_circle_measurement)  # 简单圆按钮
        self.btnDrawFineCircle_Front.clicked.connect(self.drawing_manager.start_fine_circle_measurement)  # 精细圆按钮
        self.btnCan1StepDraw_Front.clicked.connect(self.undo_last_drawing) # 撤销手动绘制
        self.btnCan1StepDet_Front.clicked.connect(self.undo_last_detection) # 撤销自动检测
        self.btnClearDrawings_Front.clicked.connect(                         # 清空按钮
            lambda: self.drawing_manager.clear_drawings(self.lbFrontView_2))
        self.btnSaveImage_Front.clicked.connect(lambda: self.save_images('front'))
        self.btnCalibration_Front.clicked.connect(self.start_calibration)

    def handle_tab_change(self, index):
        """处理选项卡切换事件"""
        print(f"切换到选项卡: {index}")  # 添加调试信息
        if index == 0:  # 主界面
            self.last_active_view = self.lbVerticalView  # 默认设置为垂直视图

        elif index == 1:  # 垂直选项卡
            self.last_active_view = self.lbVerticalView_2
            # 立即更新垂直选项卡视图
            if self.current_frame_vertical is not None:
                tab_measurement = self.drawing_manager.get_measurement_manager(self.lbVerticalView_2)
                if tab_measurement:
                    display_frame = tab_measurement.layer_manager.render_frame(self.current_frame_vertical)
                    if display_frame is not None:
                        self.display_image(display_frame, self.lbVerticalView_2)

        elif index == 2:  # 左视图选项卡
            self.last_active_view = self.lbLeftView_2
            # 立即更新左视图选项卡
            if self.current_frame_left is not None:
                tab_measurement = self.drawing_manager.get_measurement_manager(self.lbLeftView_2)
                if tab_measurement:
                    display_frame = tab_measurement.layer_manager.render_frame(self.current_frame_left)
                    if display_frame is not None:
                        self.display_image(display_frame, self.lbLeftView_2)

        elif index == 3:  # 前视图选项卡
            self.last_active_view = self.lbFrontView_2
            # 立即更新前视图选项卡
            if self.current_frame_front is not None:
                tab_measurement = self.drawing_manager.get_measurement_manager(self.lbFrontView_2)
                if tab_measurement:
                    display_frame = tab_measurement.layer_manager.render_frame(self.current_frame_front)
                    if display_frame is not None:
                        self.display_image(display_frame, self.lbFrontView_2)

    def start_drawing_mode(self):
        """启动绘画模式"""
        try:
            sender = self.sender()
            if sender in [self.btnDrawParallel, self.btnDrawParallel_Ver, self.btnDrawParallel_Left, self.btnDrawParallel_Front]:
                self.drawing_manager.start_parallel_measurement()
            elif sender in [self.btnDrawStraight, self.btnDrawStraight_Ver, self.btnDrawStraight_Left, self.btnDrawStraight_Front]:
                self.drawing_manager.start_line_measurement()
            elif sender in [self.btnDrawCircle, self.btnDrawCircle_Ver, self.btnDrawCircle_Left, self.btnDrawCircle_Front]:
                self.drawing_manager.start_circle_measurement()
            elif sender in [self.btnDrawLine, self.btnDrawLine_Ver, self.btnDrawLine_Left, self.btnDrawLine_Front]:
                self.drawing_manager.start_line_segment_measurement()
        except Exception as e:
            raise Exception(f"启动绘画模式失败: {str(e)}")
  
    def start_cameras(self):
        """启动所有相机"""
        try:
            self.log_manager.log_ui_operation("开始测量")
            
            # 创建并启动所有相机线程
            camera_configs = [
                {
                    'thread_attr': 'ver_camera_thread',
                    'sn': self.ledVerCamSN.text(),
                    'update_func': self.update_ver_camera_view,
                    'label': '垂直相机'
                },
                {
                    'thread_attr': 'left_camera_thread',
                    'sn': self.ledLeftCamSN.text(),
                    'update_func': self.update_left_camera_view,
                    'label': '左相机'
                },
                {
                    'thread_attr': 'front_camera_thread',
                    'sn': self.ledFrontCamSN.text(),
                    'update_func': self.update_front_camera_view,
                    'label': '前相机'
                }
            ]
            
            # 第一步：先创建所有相机线程但不启动
            for config in camera_configs:
                thread_attr = config['thread_attr']
                thread = getattr(self, thread_attr)
                
                # 如果线程不存在，创建新线程
                if thread is None:
                    thread = CameraThread(
                        camera_sn=config['sn']
                    )
                    # 连接信号（只连接一次）
                    thread.frame_ready.connect(config['update_func'])
                    thread.error_occurred.connect(self.show_error)
                    setattr(self, thread_attr, thread)
            
            # 第二步：预初始化相机（创建相机实例但不开始取流）
            threads = [self.ver_camera_thread, self.left_camera_thread, self.front_camera_thread]
            for thread in threads:
                if thread and not thread.isRunning():
                    thread.pre_initialize()
            
            # 短暂延迟，确保所有相机都完成预初始化
            QThread.msleep(100)
            
            # 第三步：同时启动所有相机线程
            for i, (thread, config) in enumerate(zip(threads, camera_configs)):
                if thread and not thread.isRunning():
                    thread.start()
                    # 获取相机参数
                    exposure_time = thread.camera.get_exposure_time()
                    pixel_format = thread.camera.get_pixel_format()
                    self.log_manager.log_camera_operation(
                        "启动", 
                        config['sn'], 
                        f"曝光时间: {exposure_time}, 格式: {pixel_format}"
                    )

            # 更新运行按钮状态
            self.btnStartMeasure.setEnabled(False)
            self.btnStopMeasure.setEnabled(True)

        except Exception as e:
            error_msg = f"启动相机失败: {str(e)}"
            self.log_manager.log_error(error_msg)
            self.show_error(error_msg)

    def stop_cameras(self):
        """停止所有相机"""
        try:
            self.log_manager.log_ui_operation("停止测量")
            
            # 停止所有相机线程
            for thread, name in [(self.ver_camera_thread, "垂直相机"), 
                               (self.left_camera_thread, "左相机"),
                               (self.front_camera_thread, "前相机")]:
                if thread:
                    if thread.running:
                        thread.stop()
                        self.log_manager.log_camera_operation("停止", details=name)
                    thread.wait()  # 等待线程完全停止
            
            # 清空线程引用
            self.ver_camera_thread = None
            self.left_camera_thread = None
            self.front_camera_thread = None

            # 更新按钮状态
            self.btnStartMeasure.setEnabled(True)
            self.btnStopMeasure.setEnabled(False)
            
        except Exception as e:
            error_msg = f"停止相机失败: {str(e)}"
            self.log_manager.log_error(error_msg)
            self.show_error(error_msg)

    def update_ver_camera_view(self, frame):
        """更新垂直相机视图"""
        try:
            # 保存当前帧的引用而不是拷贝
            self.current_frame_vertical = frame
            
            # 更新主界面的垂直视图（当选项卡索引为0时）
            if self.tabWidget.currentIndex() == 0 and self.lbVerticalView.isVisible():
                # 更新主界面的垂直视图
                main_measurement = self.drawing_manager.get_measurement_manager(self.lbVerticalView)
                if main_measurement:
                    main_display_frame = main_measurement.layer_manager.render_frame(frame)
                    if main_display_frame is not None:
                        self.display_image(main_display_frame, self.lbVerticalView)
            
            # 更新垂直选项卡的视图（当选项卡索引为1时）
            if self.tabWidget.currentIndex() == 1 and self.lbVerticalView_2.isVisible():
                tab_measurement = self.drawing_manager.get_measurement_manager(self.lbVerticalView_2)
                if tab_measurement:
                    tab_display_frame = tab_measurement.layer_manager.render_frame(frame)
                    if tab_display_frame is not None:
                        self.display_image(tab_display_frame, self.lbVerticalView_2)
                    
        except Exception as e:
            self.log_manager.log_error("更新垂直相机视图失败", str(e))

    def update_left_camera_view(self, frame):
        """更新左侧相机视图"""
        try:
            # 保存当前帧的引用而不是拷贝
            self.current_frame_left = frame
            
            # 更新主界面的左侧视图（当选项卡索引为0时）
            if self.tabWidget.currentIndex() == 0 and self.lbLeftView.isVisible():
                main_measurement = self.drawing_manager.get_measurement_manager(self.lbLeftView)
                if main_measurement:
                    main_display_frame = main_measurement.layer_manager.render_frame(frame)
                    if main_display_frame is not None:
                        self.display_image(main_display_frame, self.lbLeftView)
            
            # 更新左侧选项卡的视图（当选项卡索引为2时）
            if self.tabWidget.currentIndex() == 2 and self.lbLeftView_2.isVisible():
                tab_measurement = self.drawing_manager.get_measurement_manager(self.lbLeftView_2)
                if tab_measurement:
                    tab_display_frame = tab_measurement.layer_manager.render_frame(frame)
                    if tab_display_frame is not None:
                        self.display_image(tab_display_frame, self.lbLeftView_2)
                    
        except Exception as e:
            self.log_manager.log_error("更新左侧相机视图失败", str(e))

    def update_front_camera_view(self, frame):
        """更新对向相机视图"""
        try:
            # 保存当前帧的引用而不是拷贝
            self.current_frame_front = frame
            
            # 更新主界面的前视图（当选项卡索引为0时）
            if self.tabWidget.currentIndex() == 0 and self.lbFrontView.isVisible():
                main_measurement = self.drawing_manager.get_measurement_manager(self.lbFrontView)
                if main_measurement:
                    main_display_frame = main_measurement.layer_manager.render_frame(frame)
                if main_display_frame is not None:
                    self.display_image(main_display_frame, self.lbFrontView)
            
            # 更新对向选项卡的视图（当选项卡索引为3时）
            if self.tabWidget.currentIndex() == 3 and self.lbFrontView_2.isVisible():
                tab_measurement = self.drawing_manager.get_measurement_manager(self.lbFrontView_2)
                if tab_measurement:
                    tab_display_frame = tab_measurement.layer_manager.render_frame(frame)
                    if tab_display_frame is not None:
                        self.display_image(tab_display_frame, self.lbFrontView_2)
                    
        except Exception as e:
            self.log_manager.log_error("更新对向相机视图失败", str(e))

    def display_image(self, frame, label):
        """优化的图像显示函数"""
        try:
            if frame is None or label is None:
                return
                
            height, width = frame.shape[:2]
            label_size = label.size()
            
            # 计算缩放比例
            scale = min(label_size.width() / width, label_size.height() / height)
            
            # 只有当需要缩放时才进行缩放，并使用更快的INTER_NEAREST方法
            if abs(scale - 1.0) > 0.01:  # 添加一个小的阈值
                new_width = int(width * scale)
                new_height = int(height * scale)
                # 使用INTER_NEAREST进行快速缩放
                frame = cv2.resize(frame, (new_width, new_height), interpolation=cv2.INTER_NEAREST)
                
            # 转换为QImage，避免复制数据
            if len(frame.shape) == 2:  # Mono8
                # 对于灰度图像，直接使用Format_Grayscale8
                q_img = QImage(frame.data, frame.shape[1], frame.shape[0], frame.shape[1], QImage.Format_Grayscale8)
            else:  # RGB
                # 确保步长正确
                bytes_per_line = frame.shape[1] * 3
                q_img = QImage(frame.data, frame.shape[1], frame.shape[0], bytes_per_line, QImage.Format_RGB888)
            
            # 创建QPixmap并设置
            pixmap = QPixmap.fromImage(q_img)
            label.setPixmap(pixmap)
            
            # 强制更新显示
            label.update()

        except Exception as e:
            self.log_manager.log_error("显示图像失败", str(e))

    def show_error(self, message):
        QMessageBox.critical(self, "错误", message)

    def closeEvent(self, event):
        """关闭窗口事件处理"""
        try:
            self.log_manager.log_ui_operation("关闭程序")
            
            # 保存当前窗口大小到设置
            self.settings_manager.save_settings(self)
            
            # 停止所有相机线程
            if hasattr(self, 'ver_camera_thread') and self.ver_camera_thread:
                self.ver_camera_thread.stop()
                self.ver_camera_thread.wait()
                self.ver_camera_thread = None
                
            if hasattr(self, 'left_camera_thread') and self.left_camera_thread:
                self.left_camera_thread.stop()
                self.left_camera_thread.wait()
                self.left_camera_thread = None
                
            if hasattr(self, 'front_camera_thread') and self.front_camera_thread:
                self.front_camera_thread.stop()
                self.front_camera_thread.wait()
                self.front_camera_thread = None
            
            # 清理图像缓存
            self.current_frame_vertical = None
            self.current_frame_left = None
            self.current_frame_front = None
            
            # 清理所有绘画资源
            if hasattr(self, 'drawing_manager'):
                self.drawing_manager.clear_drawings()
                self.drawing_manager = None
            
            # 清理测量管理器
            if hasattr(self, 'ver_measurement'):
                self.ver_measurement = None
            if hasattr(self, 'left_measurement'):
                self.left_measurement = None
            if hasattr(self, 'front_measurement'):
                self.front_measurement = None
            if hasattr(self, 'ver_measurement_2'):
                self.ver_measurement_2 = None
            
            # 清理UI资源
            for label in [self.lbVerticalView, self.lbLeftView, self.lbFrontView,
                         self.lbVerticalView_2, self.lbLeftView_2, self.lbFrontView_2]:
                if label:
                    label.clear()
            
            # 等待资源释放
            QThread.msleep(200)
            
            # 手动触发垃圾回收
            self._force_garbage_collection()
            
            event.accept()
            
        except Exception as e:
            error_msg = f"关闭窗口时出错: {str(e)}"
            self.log_manager.log_error(error_msg)
            event.accept()

    def convert_mouse_to_image_coords(self, pos, label):
        """将鼠标坐标转换为图像坐标"""
        current_frame = self.get_current_frame_for_view(label)
        if current_frame is None:
            print("无法获取当前帧")  # 调试信息
            return QPoint(0, 0)
        
        # 获取图像和标签尺寸
        img_height, img_width = current_frame.shape[:2]
        label_width = label.width()
        label_height = label.height()
        
        # 计算缩放比例
        ratio = min(label_width / img_width, label_height / img_height)
        display_width = int(img_width * ratio)
        display_height = int(img_height * ratio)
        
        # 计算偏移量
        x_offset = (label_width - display_width) // 2
        y_offset = (label_height - display_height) // 2
        
        # 转换坐标
        image_x = int((pos.x() - x_offset) * img_width / display_width)
        image_y = int((pos.y() - y_offset) * img_height / display_height)
        
        # 确保坐标在图像范围内
        image_x = max(0, min(image_x, img_width - 1))
        image_y = max(0, min(image_y, img_height - 1))
        
        print(f"鼠标坐标转换: 屏幕({pos.x()}, {pos.y()}) -> 图像({image_x}, {image_y})")  # 调试信息
        return QPoint(image_x, image_y)

    def mouseReleaseEvent(self, event):
        """鼠标释放事件"""
        if event.button() == Qt.LeftButton and self.active_view and self.active_measurement:
            pos = event.pos()
            view_pos = self.active_view.mapFrom(self, pos)
            image_pos = self.convert_mouse_to_image_coords(view_pos, self.active_view)
            
            current_frame = self.get_current_frame_for_view(self.active_view)
            if current_frame is not None:
                # 处理鼠标释放事件
                display_frame = self.active_measurement.handle_mouse_release(image_pos, current_frame)
                if display_frame is not None:
                    self.update_view(self.active_view, display_frame)
                    # 记录最后操作的视图
                    self.last_active_view = self.active_view
            
            self.active_view = None
            self.active_measurement = None

    def get_current_frame_for_view(self, label):
        """根据视图标签获取对应的当前帧"""
        # 根据标签返回对应相机的帧
        if label in [self.lbVerticalView, self.lbVerticalView_2]:
            return self.current_frame_vertical
        elif label in [self.lbLeftView, self.lbLeftView_2]:
            return self.current_frame_left
        elif label in [self.lbFrontView, self.lbFrontView_2]:
            return self.current_frame_front
        return None

    def update_view(self, label, frame):
        """更新指定视图的显示"""
        if frame is not None:
            self.display_image(frame, label)

    def label_mouseDoubleClickEvent(self, event, label):
        """处理标签的双击事件"""
        if event.button() == Qt.LeftButton:
            # 只处理主界面视图的双击事件，跳转到对应的选项卡
            if label == self.lbVerticalView:
                self.tabWidget.setCurrentIndex(1)
                # 更新最后操作的视图为对应的选项卡视图
                self.last_active_view = self.lbVerticalView_2
            elif label == self.lbLeftView:
                self.tabWidget.setCurrentIndex(2)
                # 更新最后操作的视图为对应的选项卡视图
                self.last_active_view = self.lbLeftView_2
            elif label == self.lbFrontView:
                self.tabWidget.setCurrentIndex(3)
                # 更新最后操作的视图为对应的选项卡视图
                self.last_active_view = self.lbFrontView_2
            else:
                # 对于其他视图，记录最后操作的视图
                self.last_active_view = label

    def undo_last_drawing(self):
        """撤销上一步手动绘制"""
        self.drawing_manager.undo_last_drawing()
        
    def undo_last_detection(self):
        """撤销上一步自动检测"""
        self.drawing_manager.undo_last_detection()

    def start_circle_line_measurement(self):
        """启动圆线距离测量模式"""
        self.drawing_manager.start_circle_line_measurement()

    def start_two_lines_measurement(self):
        """启动线与线测量模式"""
        self.drawing_manager.start_two_lines_measurement()

    def save_images(self, view_type=None):
        """保存原始图像和可视化图像"""
        try:
            # 根据当前标签页或传入的view_type确定要保存的视图
            if view_type is None:
                current_tab = self.tabWidget.currentIndex()
                view_type = {
                    1: 'vertical',
                    2: 'left',
                    3: 'front'
                }.get(current_tab, 'vertical')
            
            # 获取对应视图的标签和测量管理器
            view_map = {
                'vertical': (self.lbVerticalView_2, self.ver_camera_thread, "垂直"),
                'left': (self.lbLeftView_2, self.left_camera_thread, "左侧"),
                'front': (self.lbFrontView_2, self.front_camera_thread, "对向")
            }
            
            view_label, camera_thread, view_name = view_map.get(view_type)
            
            # 检查相机帧
            if camera_thread is None or camera_thread.current_frame is None:
                print(f"{view_name}视图没有可用的相机图像")
                return
            
            current_frame = camera_thread.current_frame.copy()  # 创建副本
            
            # 检查图像数据
            if current_frame is None or current_frame.size == 0:
                print(f"{view_name}视图的图像帧无效")
                return
                
            # 打印图像信息用于调试
            print(f"图像信息: shape={current_frame.shape}, dtype={current_frame.dtype}")
            
            # 修改路径创建部分
            import os
            from datetime import datetime
            
            # 基础路径 - 使用正斜杠
            base_path = "D:/CamImage"
            today = datetime.now().strftime("%Y.%m.%d")
            
            # 使用 os.path.join 并替换反斜杠为正斜杠
            date_dir = os.path.join(base_path, today).replace('\\', '/')
            view_dir = os.path.join(date_dir, view_name).replace('\\', '/')
            
            # 确保目录存在
            os.makedirs(view_dir, exist_ok=True)
            
            current_time = datetime.now().strftime("%H-%M-%S")
            
            try:
                # 保存原始图像
                origin_filename = f"{current_time}_origin.jpg"
                origin_path = os.path.join(view_dir, origin_filename).replace('\\', '/')
                
                # 检查图像格式并进行必要的转换
                if len(current_frame.shape) == 2:  # 如果是灰度图
                    save_frame = current_frame
                else:  # 如果是RGB图像，需要转换为BGR格式
                    save_frame = cv2.cvtColor(current_frame, cv2.COLOR_RGB2BGR)
                
                # 确保图像数据类型正确
                if save_frame.dtype != np.uint8:
                    save_frame = (save_frame * 255).astype(np.uint8)
                
                # 打印保存路径和图像信息
                print(f"尝试保存原始图像到: {origin_path}")
                print(f"图像信息: shape={save_frame.shape}, dtype={save_frame.dtype}")
                
                # 使用 imencode 和 文件写入的方式保存图像
                is_success, buffer = cv2.imencode(".jpg", save_frame, [cv2.IMWRITE_JPEG_QUALITY, 100])
                if is_success:
                    with open(origin_path, "wb") as f:
                        f.write(buffer.tobytes())
                    print(f"原始图像已保存: {origin_filename}")
                    self.log_manager.log_ui_operation(
                        "保存原始图像成功",
                        f"文件路径: {origin_path}"
                    )
                else:
                    raise Exception("图像编码失败")
                
                # 保存可视化图像
                measurement_manager = self.drawing_manager.get_measurement_manager(view_label)
                if measurement_manager:
                    # 获取可视化帧
                    visual_frame = measurement_manager.layer_manager.render_frame(current_frame.copy())
                    
                    if visual_frame is not None:
                        # 如果是RGB图像,需要转换为BGR格式
                        if len(visual_frame.shape) == 3:
                            visual_frame = cv2.cvtColor(visual_frame, cv2.COLOR_RGB2BGR)
                            
                        visual_filename = f"{current_time}_visual.jpg"
                        visual_path = os.path.join(view_dir, visual_filename).replace('\\', '/')
                        
                        # 保存可视化图像
                        is_success, buffer = cv2.imencode(".jpg", visual_frame, [cv2.IMWRITE_JPEG_QUALITY, 100])
                        if is_success:
                            with open(visual_path, "wb") as f:
                                f.write(buffer.tobytes())
                            print(f"可视化图像已保存: {visual_filename}")
                            self.log_manager.log_ui_operation(
                                "保存可视化图像成功",
                                f"文件路径: {visual_path}"
                            )
                    
                    # 重新渲染视图
                    self.update_view_with_current_frame(view_label)
                    
            except Exception as e:
                error_msg = f"保存图像过程中出错: {str(e)}"
                print(error_msg)
                self.log_manager.log_error(error_msg)
                self.show_error(error_msg)
                
        except Exception as e:
            error_msg = f"保存图像时出错: {str(e)}"
            print(error_msg)
            self.log_manager.log_error(error_msg)
            self.show_error(error_msg)

    def start_line_detection(self):
        """启动直线检测模式"""
        print("开始直线检测")
        self.drawing_manager.start_line_detection()

    def start_circle_detection(self):
        """启动圆形检测模式"""
        print("开始圆形检测")
        self.drawing_manager.start_circle_detection()

    def start_calibration(self):
        """启动标定模式"""
        print("开始标定")
        self.drawing_manager.start_calibration()
    
    def update_window_size(self):
        """更新窗口大小"""
        try:
            # 获取输入框的文本
            width_text = self.ledUIWidth.text()
            height_text = self.ledUIHeight.text()
            
            # 检查文本是否为空
            if width_text and height_text:
                width = int(width_text)
                height = int(height_text)
                
                # 设置合理的最小值
                width = max(800, width)
                height = max(600, height)
                
                # 调整窗口大小
                self.resize(width, height)
                
        except ValueError:
            # 如果转换失败，忽略错误
            pass

    def update_view_with_current_frame(self, label):
        """使用当前帧更新视图"""
        current_frame = self.get_current_frame_for_view(label)
        if current_frame is not None:
            manager = self.drawing_manager.measurement_managers[label]
            display_frame = manager.layer_manager.render_frame(current_frame.copy())
            if display_frame is not None:
                self.update_view(label, display_frame)

    def resizeEvent(self, event):
        """窗口大小改变事件处理"""
        super().resizeEvent(event)
        
        # 获取当前窗口大小并更新到UI输入框
        current_width = self.width()
        current_height = self.height()
        
        # 更新UI输入框，但避免触发textChanged信号导致循环
        # 暂时断开信号连接
        self.ledUIWidth.blockSignals(True)
        self.ledUIHeight.blockSignals(True)
        
        # 更新输入框
        self.ledUIWidth.setText(str(current_width))
        self.ledUIHeight.setText(str(current_height))
        
        # 恢复信号连接
        self.ledUIWidth.blockSignals(False)
        self.ledUIHeight.blockSignals(False)
        
        # 获取所有需要更新的视图标签
        all_views = [
            self.lbVerticalView, self.lbLeftView, self.lbFrontView,  # 主界面视图
            self.lbVerticalView_2, self.lbLeftView_2, self.lbFrontView_2  # 选项卡视图
        ]
        
        # 更新每个视图
        for label in all_views:
            if label in self.drawing_manager.measurement_managers:
                self.update_view_with_current_frame(label)

    def keyPressEvent(self, event):
        """处理按键事件"""
        # 获取当前活动的视图
        focused_widget = QApplication.focusWidget()
        if focused_widget in self.drawing_manager.measurement_managers:
            manager = self.drawing_manager.measurement_managers[focused_widget]
            current_frame = self.get_current_frame_for_view(focused_widget)
            if current_frame is not None:
                display_frame = manager.handle_key_press(event, current_frame)
                if display_frame is not None:
                    self.update_view(focused_widget, display_frame)

    def show_context_menu(self, pos, label):
        """显示右键菜单"""
        if label in self.drawing_manager.measurement_managers:
            manager = self.drawing_manager.measurement_managers[label]
            selected = manager.layer_manager.selected_objects
            
            if not selected:
                return
                
            # 清理之前的菜单项
            self.context_menu.clear()
            
            # 添加删除选项
            self.delete_action = QAction("删除", self)
            self.delete_action.triggered.connect(self.delete_selected_objects)
            self.context_menu.addAction(self.delete_action)
            
            # 如果选中了两个图元，添加相应的测量选项
            if len(selected) == 2:
                # 获取选中图元的类型
                types = [obj.type for obj in selected]
                
                # 区分直线和线段
                is_line = lambda t: t == DrawingType.LINE  # 只匹配直线
                is_line_segment = lambda t: t == DrawingType.LINE_SEGMENT  # 只匹配线段
                is_circle = lambda t: t in [DrawingType.CIRCLE, DrawingType.SIMPLE_CIRCLE, DrawingType.FINE_CIRCLE]
                
                # 添加分隔线
                self.context_menu.addSeparator()
                
                # 圆和直线的选项
                is_circle_and_line = ((types[0] == DrawingType.CIRCLE and is_line(types[1])) or
                                    (types[1] == DrawingType.CIRCLE and is_line(types[0])))
                
                if is_circle_and_line:
                    # 使用DrawingManager的handle_context_menu方法获取菜单项
                    menu_items = self.drawing_manager.handle_context_menu(label)
                    for item_text, item_handler in menu_items:
                        if item_text == "直线与圆":  # 只添加直线与圆选项
                            action = QAction(item_text, self)
                            action.triggered.connect(item_handler)
                            self.context_menu.addAction(action)
                
                # 圆和线段的选项
                is_circle_and_line_segment = ((types[0] == DrawingType.CIRCLE and is_line_segment(types[1])) or
                                           (types[1] == DrawingType.CIRCLE and is_line_segment(types[0])))
                
                if is_circle_and_line_segment:
                    menu_items = self.drawing_manager.handle_context_menu(label)
                    for item_text, item_handler in menu_items:
                        if item_text == "线段与圆":  # 只添加线段与圆选项
                            action = QAction(item_text, self)
                            action.triggered.connect(item_handler)
                            self.context_menu.addAction(action)
                
                # 两条直线的选项
                is_two_lines = all(is_line(t) for t in types)
                if is_two_lines:
                    self.two_lines_action = QAction("线与线测量", self)
                    self.two_lines_action.triggered.connect(self.measure_two_lines)
                    self.context_menu.addAction(self.two_lines_action)
                
                # 两条线段的选项
                is_two_line_segments = all(is_line_segment(t) for t in types)
                if is_two_line_segments:
                    # 添加线段角度测量选项
                    self.line_segment_angle_action = QAction("线段角度测量", self)
                    self.line_segment_angle_action.triggered.connect(self.measure_line_segment_angle)
                    self.context_menu.addAction(self.line_segment_angle_action)
                
                # 两个点的选项
                is_two_points = all(t == DrawingType.POINT for t in types)
                if is_two_points:
                    menu_items = self.drawing_manager.handle_context_menu(label)
                    for item_text, item_handler in menu_items:
                        action = QAction(item_text, self)
                        action.triggered.connect(item_handler)
                        self.context_menu.addAction(action)
                
                # 点和直线的选项
                is_point_and_line = ((types[0] == DrawingType.POINT and is_line(types[1])) or
                                   (types[1] == DrawingType.POINT and is_line(types[0])))
                
                if is_point_and_line:
                    menu_items = self.drawing_manager.handle_context_menu(label)
                    for item_text, item_handler in menu_items:
                        if item_text == "点与线":
                            action = QAction(item_text, self)
                            action.triggered.connect(item_handler)
                            self.context_menu.addAction(action)
                
                # 点和线段的选项
                is_point_and_line_segment = ((types[0] == DrawingType.POINT and is_line_segment(types[1])) or
                                          (types[1] == DrawingType.POINT and is_line_segment(types[0])))
                
                if is_point_and_line_segment:
                    menu_items = self.drawing_manager.handle_context_menu(label)
                    for item_text, item_handler in menu_items:
                        if item_text == "点与线":
                            action = QAction("点与线段", self)
                            action.triggered.connect(item_handler)
                            self.context_menu.addAction(action)
                
                # 点和圆的选项
                is_point_and_circle = ((types[0] == DrawingType.POINT and is_circle(types[1])) or
                                     (types[1] == DrawingType.POINT and is_circle(types[0])))
                
                if is_point_and_circle:
                    menu_items = self.drawing_manager.handle_context_menu(label)
                    for item_text, item_handler in menu_items:
                        if item_text == "点与圆":
                            action = QAction(item_text, self)
                            action.triggered.connect(item_handler)
                            self.context_menu.addAction(action)
            
            # 显示菜单
            self.context_menu.exec_(label.mapToGlobal(pos))

    def measure_circle_line(self):
        """线与圆测量"""
        active_label = QApplication.focusWidget()
        if active_label in self.drawing_manager.measurement_managers:
            manager = self.drawing_manager.measurement_managers[active_label]
            selected = manager.layer_manager.selected_objects
            if len(selected) == 2:
                # 找到圆和线（包括线段）
                circle = next((obj for obj in selected if obj.type == DrawingType.CIRCLE), None)
                line = next((obj for obj in selected 
                            if obj.type in [DrawingType.LINE, DrawingType.LINE_SEGMENT]), None)
                if circle and line:
                    # 创建新的圆线测量对象
                    properties = {
                        'color': (0, 255, 0),  # RGB格式：绿色
                        'thickness': 2
                    }
                    new_obj = DrawingObject(
                        type=DrawingType.CIRCLE_LINE,
                        points=[*circle.points, *line.points],  # 合并圆和线的点
                        properties=properties
                    )
                    # 添加到绘制列表
                    manager.layer_manager.drawing_objects.append(new_obj)
                    # 清除选择
                    manager.layer_manager.clear_selection()
                    # 更新视图
                    current_frame = self.get_current_frame_for_view(active_label)
                    if current_frame is not None:
                        display_frame = manager.layer_manager.render_frame(current_frame)
                        if display_frame is not None:
                            self.update_view(active_label, display_frame)

    def delete_selected_objects(self):
        """删除选中的对象"""
        active_view = self.last_active_view
        if active_view:
            self.drawing_manager.delete_selected_objects(active_view)

    def measure_two_lines(self):
        """线与线测量"""
        active_label = QApplication.focusWidget()
        if active_label in self.drawing_manager.measurement_managers:
            manager = self.drawing_manager.measurement_managers[active_label]
            selected = manager.layer_manager.selected_objects
            if len(selected) == 2 and all(obj.type == DrawingType.LINE for obj in selected):
                # 计算交点
                line1_p1, line1_p2 = selected[0].points
                line2_p1, line2_p2 = selected[1].points
                
                # 计算直线参数
                a1 = line1_p2.y() - line1_p1.y()
                b1 = line1_p1.x() - line1_p2.x()
                c1 = line1_p2.x() * line1_p1.y() - line1_p1.x() * line1_p2.y()
                
                a2 = line2_p2.y() - line2_p1.y()
                b2 = line2_p1.x() - line2_p2.x()
                c2 = line2_p2.x() * line2_p1.y() - line2_p1.x() * line2_p2.y()
                
                # 计算交点
                det = a1 * b2 - a2 * b1
                if abs(det) < 1e-6:  # 平行或重合
                    intersection_point = None
                else:
                    x = (b1 * c2 - b2 * c1) / det
                    y = (c1 * a2 - c2 * a1) / det
                    intersection_point = QPoint(int(x), int(y))
                
                # 创建新的两线测量对象
                properties = {
                    'color': (0, 255, 0),  # RGB格式：绿色
                    'thickness': 2,
                    'intersection_point': intersection_point  # 添加交点信息
                }
                new_obj = DrawingObject(
                    type=DrawingType.TWO_LINES,
                    points=[*selected[0].points, *selected[1].points],  # 合并两条线的点
                    properties=properties
                )
                # 添加到绘制列表
                manager.layer_manager.drawing_objects.append(new_obj)
                # 清除选择
                manager.layer_manager.clear_selection()
                # 更新视图
                current_frame = self.get_current_frame_for_view(active_label)
                if current_frame is not None:
                    display_frame = manager.layer_manager.render_frame(current_frame)
                    if display_frame is not None:
                        self.update_view(active_label, display_frame)

    def measure_line_segment_angle(self):
        """线段角度测量"""
        active_label = QApplication.focusWidget()
        if active_label in self.drawing_manager.measurement_managers:
            manager = self.drawing_manager.measurement_managers[active_label]
            selected = manager.layer_manager.selected_objects
            if len(selected) == 2 and all(obj.type == DrawingType.LINE_SEGMENT for obj in selected):
                # 获取两条线段的点
                line1_p1, line1_p2 = selected[0].points
                line2_p1, line2_p2 = selected[1].points
                
                # 创建新的线段角度测量对象
                properties = {
                    'color': (0, 255, 0),  # RGB格式：绿色
                    'thickness': 2
                }
                new_obj = DrawingObject(
                    type=DrawingType.LINE_SEGMENT_ANGLE,
                    points=[line1_p1, line1_p2, line2_p1, line2_p2],  # 合并两条线段的点
                    properties=properties
                )
                # 添加到绘制列表
                manager.layer_manager.drawing_objects.append(new_obj)
                # 清除选择但保持原始线段可见
                for obj in selected:
                    obj.selected = False
                manager.layer_manager.selected_objects = []
                # 更新视图
                current_frame = self.get_current_frame_for_view(active_label)
                if current_frame is not None:
                    display_frame = manager.layer_manager.render_frame(current_frame)
                    if display_frame is not None:
                        self.update_view(active_label, display_frame)

    def save_all_views(self):
        """保存主界面所有可用视图的图像"""
        try:
            # 获取主界面的视图和对应的相机线程
            views = [
                (self.lbVerticalView, self.ver_camera_thread, "垂直"),  # 主界面的垂直视图
                (self.lbLeftView, self.left_camera_thread, "左侧"),     # 主界面的左侧视图
                (self.lbFrontView, self.front_camera_thread, "对向")    # 主界面的前视图
            ]
            
            saved_count = 0
            for view_label, camera_thread, view_name in views:
                try:
                    # 检查相机是否可用
                    if camera_thread is None or camera_thread.current_frame is None:
                        print(f"{view_name}视图没有可用的相机图像")
                        continue
                    
                    current_frame = camera_thread.current_frame.copy()
                    if current_frame is None or current_frame.size == 0:
                        print(f"{view_name}视图的图像帧无效")
                        continue
                    
                    # 创建保存路径
                    base_path = "D:/CamImage"
                    today = datetime.now().strftime("%Y.%m.%d")
                    date_dir = os.path.join(base_path, today).replace('\\', '/')
                    view_dir = os.path.join(date_dir, view_name).replace('\\', '/')
                    os.makedirs(view_dir, exist_ok=True)
                    
                    current_time = datetime.now().strftime("%H-%M-%S")
                    
                    # 保存原始图像
                    origin_filename = f"{current_time}_origin.jpg"
                    origin_path = os.path.join(view_dir, origin_filename).replace('\\', '/')
                    
                    # 将BGR转换为RGB后保存原始图像
                    rgb_frame = cv2.cvtColor(current_frame, cv2.COLOR_BGR2RGB)
                    is_success, buffer = cv2.imencode(".jpg", rgb_frame, [cv2.IMWRITE_JPEG_QUALITY, 100])
                    if is_success:
                        with open(origin_path, "wb") as f:
                            f.write(buffer.tobytes())
                        print(f"原始图像已保存: {origin_filename}")
                        
                        # 保存可视化图像
                        measurement_manager = self.drawing_manager.get_measurement_manager(view_label)
                        if measurement_manager:
                            
                            # 获取可视化帧
                            visual_frame = measurement_manager.layer_manager.render_frame(current_frame.copy())
                            
                            if visual_frame is not None:
                                visual_filename = f"{current_time}_visual.jpg"
                                visual_path = os.path.join(view_dir, visual_filename).replace('\\', '/')
                                
                                # 将BGR转换为RGB后保存可视化图像
                                rgb_visual = cv2.cvtColor(visual_frame, cv2.COLOR_BGR2RGB)
                                is_success, buffer = cv2.imencode(".jpg", rgb_visual, [cv2.IMWRITE_JPEG_QUALITY, 100])
                                if is_success:
                                    with open(visual_path, "wb") as f:
                                        f.write(buffer.tobytes())
                                    print(f"可视化图像已保存: {visual_filename}")
                    
                    saved_count += 1
                    self.log_manager.log_ui_operation(
                        f"保存{view_name}视图成功",
                        f"原始图像: {origin_path}\n可视化图像: {visual_path if 'visual_path' in locals() else '无'}"
                    )
                
                except Exception as e:
                    print(f"{view_name}视图保存失败: {str(e)}")
                    continue
            
            # 显示保存结果
            if saved_count > 0:
                self.log_manager.log_ui_operation(
                    "保存完成",
                    f"成功保存了 {saved_count} 个视图的图像"
                )
            else:
                self.show_error("没有可用的相机视图可以保存")
                
        except Exception as e:
            error_msg = f"保存图像时出错: {str(e)}"
            print(error_msg)
            self.log_manager.log_error(error_msg)
            self.show_error(error_msg)

    def _force_garbage_collection(self):
        """强制进行垃圾回收"""
        import gc
        gc.collect()

    def apply_grid_spacing(self, current_tab):
        """应用网格间距"""
        # 根据当前选项卡获取对应的网格密度输入框
        if current_tab == 0:  # 主界面
            grid_input = self.leGridDens
            containers = [self.vertical_grid_container, self.left_grid_container, self.front_grid_container]
        elif current_tab == 1:  # 垂直视图选项卡
            grid_input = self.leGridDens_Ver
            containers = [self.vertical_grid_container_2, self.vertical_grid_container]  # 包括主界面对应视图
        elif current_tab == 2:  # 左视图选项卡
            grid_input = self.leGridDens_Left
            containers = [self.left_grid_container_2, self.left_grid_container]  # 包括主界面对应视图
        elif current_tab == 3:  # 前视图选项卡
            grid_input = self.leGridDens_Front
            containers = [self.front_grid_container_2, self.front_grid_container]  # 包括主界面对应视图
        else:
            return
        
        grid_spacing_text = grid_input.text()
        if not grid_spacing_text:
            return
        
        grid_spacing = int(grid_spacing_text)
        if grid_spacing <= 0:
                return
        
        # 确保网格间距至少为10
        if grid_spacing < 10:
            grid_spacing = 10
            
            # 更新输入框显示
            if current_tab == 0:  # 主界面
                self.leGridDens.setText(str(grid_spacing))
            elif current_tab == 1:  # 垂直视图选项卡
                self.leGridDens_Ver.setText(str(grid_spacing))
            elif current_tab == 2:  # 左视图选项卡
                self.leGridDens_Left.setText(str(grid_spacing))
            elif current_tab == 3:  # 前视图选项卡
                self.leGridDens_Front.setText(str(grid_spacing))
        

        for container in containers:
            if isinstance(container, GridContainer):
                container.setGridSpacing(grid_spacing)

        # 记录操作
        if grid_spacing > 0:
            self.log_manager.log_ui_operation(f"设置网格间距为 {grid_spacing} 像素")
        else:
            self.log_manager.log_ui_operation("取消网格显示")
            
    def clear_grid(self, tab_index):
        """清除网格显示"""
        # 根据选项卡索引获取对应的输入框和标签
        if tab_index == 0:  # 主界面
            grid_input = self.leGridDens
            containers = [self.vertical_grid_container, self.left_grid_container, self.front_grid_container]
        elif tab_index == 1:  # 垂直选项卡
            grid_input = self.leGridDens_Ver
            containers = [self.vertical_grid_container_2, self.vertical_grid_container]  # 包括主界面对应视图
        elif tab_index == 2:  # 左视图选项卡
            grid_input = self.leGridDens_Left
            containers = [self.left_grid_container_2, self.left_grid_container]  # 包括主界面对应视图
        elif tab_index == 3:  # 前视图选项卡
            grid_input = self.leGridDens_Front
            containers = [self.front_grid_container_2, self.front_grid_container]  # 包括主界面对应视图
        else:
            return

        # 清除所有相关标签的网格
        for container in containers:
            if isinstance(container, GridContainer):
                container.setGridSpacing(0)
        
        # 清除网格间隔输入框
        grid_input.clear()
        
        self.log_manager.log_ui_operation("清除网格显示")

def main():
    app = QApplication(sys.argv)
    main_window = MainApp()
    main_window.show()
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()