import warnings
warnings.filterwarnings("ignore", category=DeprecationWarning)
import sys
import os
import numpy as np
import cv2
from typing import List

# 添加项目根目录到系统路径
current_dir = os.path.dirname(os.path.abspath(__file__))
if current_dir not in sys.path:
    sys.path.append(current_dir)

from PyQt5.QtWidgets import *
from PyQt5.QtGui import QImage, QPixmap
from PyQt5.QtCore import Qt, QThread, QPoint
from mainwindow import Ui_MainWindow
from Tools.camera_thread import CameraThread
from Tools.settings_manager import SettingsManager
from Tools.log_manager import LogManager
from Tools.measurement_manager import MeasurementManager
from Tools.drawing_manager import DrawingManager

class MainApp(QMainWindow, Ui_MainWindow):
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

        # 当前活动的视图
        self.active_view = None
        self.active_measurement = None

        # 添加标志位
        self.is_undoing = False

        # 初始化网格密度下拉框
        self.init_grid_density_combos()
        
        # 连接网格控制信号
        self.connect_grid_controls()

    def _init_ui(self):
        """初始化UI控件"""
        # 初始化ComboBox选项
        self.settings_manager.init_combo_boxes(self)
        
        # 初始化按钮状态
        self.btnStopMeasure.setEnabled(False)

        # 初始化绘画管理器
        self.drawing_manager = DrawingManager(self)
        
        # 设置主界面视图的绘画功能
        self.drawing_manager.setup_view(self.lbVerticalView, "vertical")
        self.drawing_manager.setup_view(self.lbLeftView, "left")      # 主界面左视图
        self.drawing_manager.setup_view(self.lbFrontView, "front")
        
        # 设置选项卡视图的绘画功能
        self.drawing_manager.setup_view(self.lbVerticalView_2, "vertical_2")
        self.drawing_manager.setup_view(self.lbLeftView_2, "left_2")  # 选项卡左视图
        self.drawing_manager.setup_view(self.lbFrontView_2, "front_2")

    def _connect_signals(self):
        """连接信号槽"""
        # 相机参数 修改为 editingFinished 信号，只在用户编辑完成时触发
        self.ledVerCamExposureTime.editingFinished.connect(lambda: self.settings_manager.save_settings(self))
        self.ledLeftCamExposureTime.editingFinished.connect(lambda: self.settings_manager.save_settings(self))
        self.ledFrontCamExposureTime.editingFinished.connect(lambda: self.settings_manager.save_settings(self))
        self.ledVerCamSN.editingFinished.connect(lambda: self.settings_manager.save_settings(self))
        self.ledLeftCamSN.editingFinished.connect(lambda: self.settings_manager.save_settings(self))
        self.ledFrontCamSN.editingFinished.connect(lambda: self.settings_manager.save_settings(self))
        
        # 图像格式修改信号保持不变
        self.cbVerCamImageFormat.currentTextChanged.connect(lambda: self.settings_manager.save_settings(self))
        self.cbLeftCamImageFormat.currentTextChanged.connect(lambda: self.settings_manager.save_settings(self))
        self.cbFrontCamImageFormat.currentTextChanged.connect(lambda: self.settings_manager.save_settings(self))

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

        # 相机控制信号
        self.btnStartMeasure.clicked.connect(self.start_cameras)
        self.btnStopMeasure.clicked.connect(self.stop_cameras)

        # 参数更新信号
        self.ledVerCamExposureTime.editingFinished.connect(self.update_camera_params)
        self.ledLeftCamExposureTime.editingFinished.connect(self.update_camera_params)
        self.ledFrontCamExposureTime.editingFinished.connect(self.update_camera_params)
        self.cbVerCamImageFormat.currentTextChanged.connect(self.update_camera_params)
        self.cbLeftCamImageFormat.currentTextChanged.connect(self.update_camera_params)
        self.cbFrontCamImageFormat.currentTextChanged.connect(self.update_camera_params)

        # 主界面绘画功能按钮信号连接
        self.btnDrawLine.clicked.connect(self.start_drawing_mode) # 线段按钮
        self.btnDrawStraight.clicked.connect(self.start_drawing_mode) # 直线按钮
        self.btnDrawCircle.clicked.connect(self.start_drawing_mode) # 圆形按钮
        self.btnDrawParallel.clicked.connect(self.start_drawing_mode) # 平行线按钮
        self.btnDrawLine_Circle.clicked.connect(self.start_circle_line_measurement) # 线与圆测量按钮
        self.btnDraw2Line.clicked.connect(self.start_two_lines_measurement) # 线与线测量按钮
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
        self.btnDrawLine_Ver.clicked.connect(self.start_drawing_mode)      # 线段按钮
        self.btnDrawStraight_Ver.clicked.connect(self.start_drawing_mode)  # 直线按钮
        self.btnDrawCircle_Ver.clicked.connect(self.start_drawing_mode)    # 圆形按钮
        self.btnDrawParallel_Ver.clicked.connect(self.start_drawing_mode) # 平行线按钮
        self.btnDrawLine_Circle_Ver.clicked.connect(self.start_circle_line_measurement) # 线与圆测量按钮
        self.btnDraw2Line_Ver.clicked.connect(self.start_two_lines_measurement) # 线与线测量按钮
        self.btnCan1StepDraw_Ver.clicked.connect(self.undo_last_drawing) # 撤销手动绘制
        self.btnCan1StepDet_Ver.clicked.connect(self.undo_last_detection) # 撤销自动检测
        self.btnClearDrawings_Ver.clicked.connect(                         # 清空按钮
            lambda: self.drawing_manager.clear_drawings(self.lbVerticalView_2))
        self.btnSaveImage_Ver.clicked.connect(lambda: self.save_images('vertical'))

        # 左侧选项卡绘画功能按钮信号连接
        self.btnLineDet_Left.clicked.connect(self.start_line_detection)  # 直线检测按钮
        self.btnCircleDet_Left.clicked.connect(self.start_circle_detection)  # 圆形检测按钮
        self.btnDrawLine_Left.clicked.connect(self.start_drawing_mode)      # 线段按钮
        self.btnDrawStraight_Left.clicked.connect(self.start_drawing_mode)  # 直线按钮
        self.btnDrawCircle_Left.clicked.connect(self.start_drawing_mode)    # 圆形按钮
        self.btnDrawParallel_Left.clicked.connect(self.start_drawing_mode) # 平行线按钮
        self.btnDrawLine_Circle_Left.clicked.connect(self.start_circle_line_measurement) # 线与圆测量按钮
        self.btnDraw2Line_Left.clicked.connect(self.start_two_lines_measurement) # 线与线测量按钮
        self.btnCan1StepDraw_Left.clicked.connect(self.undo_last_drawing) # 撤销手动绘制
        self.btnCan1StepDet_Left.clicked.connect(self.undo_last_detection) # 撤销自动检测
        self.btnClearDrawings_Left.clicked.connect(                         # 清空按钮
            lambda: self.drawing_manager.clear_drawings(self.lbLeftView_2))
        self.btnSaveImage_Left.clicked.connect(lambda: self.save_images('left'))

        # 对向选项卡绘画功能按钮信号连接
        self.btnLineDet_Front.clicked.connect(self.start_line_detection)  # 直线检测按钮
        self.btnCircleDet_Front.clicked.connect(self.start_circle_detection)  # 圆形检测按钮
        self.btnDrawLine_Front.clicked.connect(self.start_drawing_mode)      # 线段按钮
        self.btnDrawStraight_Front.clicked.connect(self.start_drawing_mode)  # 直线按钮
        self.btnDrawCircle_Front.clicked.connect(self.start_drawing_mode)    # 圆形按钮
        self.btnDrawParallel_Front.clicked.connect(self.start_drawing_mode) # 平行线按钮
        self.btnDrawLine_Circle_Front.clicked.connect(self.start_circle_line_measurement) # 线与圆测量按钮
        self.btnDraw2Line_Front.clicked.connect(self.start_two_lines_measurement) # 线与线测量按钮
        self.btnCan1StepDraw_Front.clicked.connect(self.undo_last_drawing) # 撤销手动绘制
        self.btnCan1StepDet_Front.clicked.connect(self.undo_last_detection) # 撤销自动检测
        self.btnClearDrawings_Front.clicked.connect(                         # 清空按钮
            lambda: self.drawing_manager.clear_drawings(self.lbFrontView_2))
        self.btnSaveImage_Front.clicked.connect(lambda: self.save_images('front'))

        # 不要在这里连接相机信号，移到 initCamera 中

        # 添加选项卡切换信号连接
        self.tabWidget.currentChanged.connect(self.handle_tab_change)

    def handle_tab_change(self, index):
        """处理选项卡切换事件"""
        print(f"切换到选项卡: {index}")  # 添加调试信息
        if index == 0:  # 主界面
            self.last_active_view = self.lbVerticalView  # 默认设置为垂直视图
        elif index == 1:  # 垂直选项卡
            self.last_active_view = self.lbVerticalView_2
        elif index == 2:  # 左视图选项卡
            self.last_active_view = self.lbLeftView_2
        elif index == 3:  # 前视图选项卡
            self.last_active_view = self.lbFrontView_2

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

    def update_camera_params(self):
        """更新相机参数"""
        try:
            # 记录参数修改
            if self.ver_camera_thread and self.ver_camera_thread.running:
                old_exposure = self.ver_camera_thread.exposure_time
                new_exposure = int(self.ledVerCamExposureTime.text())
                if old_exposure != new_exposure:
                    self.log_manager.log_parameter_change("垂直相机曝光时间", old_exposure, new_exposure, 
                                                        self.ledVerCamSN.text())
            
            # 如果相机正在运行，先停止
            is_running = False
            if self.ver_camera_thread and self.ver_camera_thread.running:
                is_running = True
                self.stop_cameras()
                
            # 保存新的设置
            self.settings_manager.save_settings(self)
            
            # 如果之前在运行，则重新启动相机
            if is_running:
                self.start_cameras()
                
        except Exception as e:
            error_msg = f"更新相机参数失败: {str(e)}"
            self.log_manager.log_error(error_msg)
            self.show_error(error_msg)

    def start_cameras(self):
        """启动所有相机"""
        try:
            self.log_manager.log_ui_operation("开始测量")
            
            # 垂直相机
            if self.ver_camera_thread is None:
                # 创建相机线程
                self.ver_camera_thread = CameraThread(
                    camera_sn=self.ledVerCamSN.text(),
                    exposure_time=int(self.ledVerCamExposureTime.text()),
                    image_format='Mono8' if self.cbVerCamImageFormat.currentText() == 'Mono8' else 'RGB8'
                )
                # 连接信号（只连接一次）
                self.ver_camera_thread.frame_ready.connect(self.update_ver_camera_view)
                self.ver_camera_thread.error_occurred.connect(self.show_error)
                
            # 启动相机线程
            if not self.ver_camera_thread.isRunning():
                self.ver_camera_thread.start()
                self.log_manager.log_camera_operation(
                    "启动", 
                    self.ledVerCamSN.text(), 
                    f"曝光时间: {self.ledVerCamExposureTime.text()}, 格式: {self.cbVerCamImageFormat.currentText()}"
                )

            # 左相机
            if self.left_camera_thread is None:
                # 创建相机线程
                self.left_camera_thread = CameraThread(
                    camera_sn=self.ledLeftCamSN.text(),
                    exposure_time=int(self.ledLeftCamExposureTime.text()),
                    image_format='Mono8' if self.cbLeftCamImageFormat.currentText() == 'Mono8' else 'RGB8'
                )
                # 连接信号（只连接一次）
                self.left_camera_thread.frame_ready.connect(self.update_left_camera_view)
                self.left_camera_thread.error_occurred.connect(self.show_error)
                
            # 启动相机线程
            if not self.left_camera_thread.isRunning():
                self.left_camera_thread.start()
                self.log_manager.log_camera_operation(
                    "启动", 
                    self.ledLeftCamSN.text(), 
                    f"曝光时间: {self.ledLeftCamExposureTime.text()}, 格式: {self.cbLeftCamImageFormat.currentText()}"
                )

            # 前相机
            if self.front_camera_thread is None:
                # 创建相机线程
                self.front_camera_thread = CameraThread(
                    camera_sn=self.ledFrontCamSN.text(),
                    exposure_time=int(self.ledFrontCamExposureTime.text()),
                    image_format='Mono8' if self.cbFrontCamImageFormat.currentText() == 'Mono8' else 'RGB8'
                )
                # 连接信号（只连接一次）
                self.front_camera_thread.frame_ready.connect(self.update_front_camera_view)
                self.front_camera_thread.error_occurred.connect(self.show_error)
                
            # 启动相机线程
            if not self.front_camera_thread.isRunning():
                self.front_camera_thread.start()
                self.log_manager.log_camera_operation(
                    "启动", 
                    self.ledFrontCamSN.text(), 
                    f"曝光时间: {self.ledFrontCamExposureTime.text()}, 格式: {self.cbFrontCamImageFormat.currentText()}"
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
        # 保存当前帧
        self.current_frame_vertical = frame.copy()
        
        # 更新主界面的垂直视图
        main_measurement = self.drawing_manager.get_measurement_manager(self.lbVerticalView)
        if main_measurement:
            main_display_frame = main_measurement.layer_manager.render_frame(frame.copy())
            self.display_image(main_display_frame, self.lbVerticalView)
        
        # 更新垂直选项卡的视图
        tab_measurement = self.drawing_manager.get_measurement_manager(self.lbVerticalView_2)
        if tab_measurement:
            tab_display_frame = tab_measurement.layer_manager.render_frame(frame.copy())
            self.display_image(tab_display_frame, self.lbVerticalView_2)

    def update_left_camera_view(self, frame):
        """更新左侧相机视图"""
        # 保存当前帧
        self.current_frame_left = frame.copy()
        
        # 更新主界面的左侧视图
        main_measurement = self.drawing_manager.get_measurement_manager(self.lbLeftView)
        if main_measurement:
            main_display_frame = main_measurement.layer_manager.render_frame(frame.copy())
            self.display_image(main_display_frame, self.lbLeftView)
        
        # 更新左侧选项卡的视图
        tab_measurement = self.drawing_manager.get_measurement_manager(self.lbLeftView_2)
        if tab_measurement:
            tab_display_frame = tab_measurement.layer_manager.render_frame(frame.copy())
            self.display_image(tab_display_frame, self.lbLeftView_2)

    def update_front_camera_view(self, frame):
        """更新对向相机视图"""
        # 保存当前帧
        self.current_frame_front = frame.copy()
        
        # 更新主界面的前视图
        main_measurement = self.drawing_manager.get_measurement_manager(self.lbFrontView)
        if main_measurement:
            main_display_frame = main_measurement.layer_manager.render_frame(frame.copy())
            self.display_image(main_display_frame, self.lbFrontView)
        
        # 更新对向选项卡的视图
        tab_measurement = self.drawing_manager.get_measurement_manager(self.lbFrontView_2)
        if tab_measurement:
            tab_display_frame = tab_measurement.layer_manager.render_frame(frame.copy())
            self.display_image(tab_display_frame, self.lbFrontView_2)

    def display_image(self, frame, label):
        try:
            height, width = frame.shape[:2]
            if len(frame.shape) == 2:  # Mono8
                bytes_per_line = width
                q_img = QImage(frame.data, width, height, bytes_per_line, QImage.Format_Grayscale8)
            else:  # RGB
                bytes_per_line = 3 * width
                q_img = QImage(frame.data, width, height, bytes_per_line, QImage.Format_RGB888)

            # 缩放图像以适应标签大小
            pixmap = QPixmap.fromImage(q_img)
            label.setPixmap(pixmap.scaled(label.size(), Qt.KeepAspectRatio))

        except Exception as e:
            self.show_error(f"显示图像失败: {str(e)}")

    def show_error(self, message):
        QMessageBox.critical(self, "错误", message)

    def closeEvent(self, event):
        """关闭窗口事件处理"""
        try:
            self.log_manager.log_ui_operation("关闭程序")
            # 停止所有相机
            self.stop_cameras()
            # 清理所有绘画资源
            self.drawing_manager.clear_drawings()
            # 等待资源释放
            QThread.msleep(100)
            # 手动触发垃圾回收
            import gc
            gc.collect()
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
            # 获取当前标签页索引
            current_tab = self.tabWidget.currentIndex()
            
            # 根据当前视图设置目标标签页
            if label == self.lbVerticalView:
                self.tabWidget.setCurrentIndex(1)
            elif label == self.lbLeftView:
                self.tabWidget.setCurrentIndex(2)
            elif label == self.lbFrontView:
                self.tabWidget.setCurrentIndex(3)
            elif label in [self.lbVerticalView_2, self.lbLeftView_2, self.lbFrontView_2]:
                self.tabWidget.setCurrentIndex(0)
            
            # 记录最后操作的视图
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
        """保存原始图像和可视化图像
        Args:
            view_type: 视图类型，可以是 'vertical', 'left', 'front'
        """
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
                'vertical': (self.lbVerticalView_2, self.ver_camera_thread),
                'left': (self.lbLeftView_2, self.left_camera_thread),
                'front': (self.lbFrontView_2, self.front_camera_thread)
            }
            
            view_label, camera_thread = view_map.get(view_type)
            
            # 检查相机帧是否可用
            if not camera_thread or not camera_thread.current_frame is not None:
                print(f"{view_type}视图没有可用的相机图像")
                return
            
            current_frame = camera_thread.current_frame
            
            # 创建保存路径
            import os
            from datetime import datetime
            
            # 基础路径
            base_path = "D:/CamImage"
            
            # 获取当前日期作为子文件夹名
            today = datetime.now().strftime("%Y-%m-%d")
            save_dir = os.path.join(base_path, today)
            
            # 确保文件夹存在
            os.makedirs(save_dir, exist_ok=True)
            
            # 获取当前时间作为文件名
            current_time = datetime.now().strftime("%H_%M_%S")
            
            # 保存原始图像
            origin_filename = f"origin_{current_time}.jpg"
            origin_path = os.path.join(save_dir, origin_filename)
            cv2.imwrite(origin_path, cv2.cvtColor(current_frame, cv2.COLOR_RGB2BGR))
            print(f"原始图像已保存: {origin_filename}")
            
            # 获取可视化图像（原始图像 + 绘画）
            measurement_manager = self.drawing_manager.get_measurement_manager(view_label)
            if measurement_manager:
                # 保存当前网格状态
                original_grid_state = measurement_manager.layer_manager.show_grid
                original_grid_density = measurement_manager.layer_manager.grid_density
                
                # 临时关闭网格
                measurement_manager.layer_manager.set_grid(False)
                
                # 获取当前显示的帧（不含网格）
                visual_frame = measurement_manager.layer_manager.render_frame(current_frame.copy())
                
                # 恢复原来的网格状态
                measurement_manager.layer_manager.set_grid(original_grid_state, original_grid_density)
                
                if visual_frame is not None:
                    # 保存可视化图像
                    visual_filename = f"visual_{current_time}.jpg"
                    visual_path = os.path.join(save_dir, visual_filename)
                    cv2.imwrite(visual_path, cv2.cvtColor(visual_frame, cv2.COLOR_RGB2BGR))
                    
                    print(f"图像已保存到: {save_dir}")
                    print(f"可视化图像: {visual_filename}")
                else:
                    print("保存原始图像成功，没有可视化内容需要保存")
                
                # 重新渲染视图（恢复网格显示）
                self.update_view_with_current_frame(view_label)
                
        except Exception as e:
            print(f"保存图像时出错: {str(e)}")

    def start_line_detection(self):
        """启动直线检测模式"""
        print("开始直线检测")
        self.drawing_manager.start_line_detection()

    def start_circle_detection(self):
        """启动圆形检测模式"""
        print("开始圆形检测")
        self.drawing_manager.start_circle_detection()
    
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

    def init_grid_density_combos(self):
        """初始化网格密度下拉框"""
        densities = [
            ("10% (密)", 0.1),
            ("20%", 0.2),
            ("30%", 0.3),
            ("40%", 0.4),
            ("50%", 0.5),
            ("60%", 0.6),
            ("70%", 0.7),
            ("80% (疏)", 0.8)
        ]
        
        # 为所有网格密度下拉框添加选项
        for combo in [self.cbGridDens, self.cbGridDens_Ver, self.cbGridDens_Left, self.cbGridDens_Front]:
            for text, _ in densities:
                combo.addItem(text)
            
    def connect_grid_controls(self):
        """连接网格控制信号"""
        # 主界面网格控制
        self.cbGridDens.activated.connect(
            lambda: self.update_grid_density(self.cbGridDens.currentIndex(), [self.lbVerticalView, self.lbLeftView, self.lbFrontView]))
        self.btnCancelGrids.clicked.connect(
            lambda: self.toggle_grid(False, [self.lbVerticalView, self.lbLeftView, self.lbFrontView]))
        
        # 垂直视图网格控制
        self.cbGridDens_Ver.activated.connect(
            lambda: self.update_grid_density(self.cbGridDens_Ver.currentIndex(), [self.lbVerticalView_2]))
        self.btnCancelGrids_Ver.clicked.connect(
            lambda: self.toggle_grid(False, [self.lbVerticalView_2]))
        
        # 左视图网格控制
        self.cbGridDens_Left.activated.connect(
            lambda: self.update_grid_density(self.cbGridDens_Left.currentIndex(), [self.lbLeftView_2]))
        self.btnCancelGrids_Left.clicked.connect(
            lambda: self.toggle_grid(False, [self.lbLeftView_2]))
        
        # 对向视图网格控制
        self.cbGridDens_Front.activated.connect(
            lambda: self.update_grid_density(self.cbGridDens_Front.currentIndex(), [self.lbFrontView_2]))
        self.btnCancelGrids_Front.clicked.connect(
            lambda: self.toggle_grid(False, [self.lbFrontView_2]))

    def update_grid_density(self, index: int, labels: List[QLabel]):
        """更新网格密度"""
        density = (index + 1) * 0.1  # 将索引转换为密度值
        for label in labels:
            if label in self.drawing_manager.measurement_managers:
                if self.log_manager:
                    self.log_manager.log_ui_operation("更新网格密度", f"密度: {density}, 视图: {label.objectName()}")
                manager = self.drawing_manager.measurement_managers[label]
                manager.set_grid(True, density)
                # 重新渲染视图
                self.update_view_with_current_frame(label)

    def toggle_grid(self, show: bool, labels: List[QLabel]):
        """切换网格显示状态"""
        for label in labels:
            if label in self.drawing_manager.measurement_managers:
                if self.log_manager:
                    self.log_manager.log_ui_operation("切换网格显示", f"显示: {show}, 视图: {label.objectName()}")
                manager = self.drawing_manager.measurement_managers[label]
                manager.set_grid(show)
                # 重新渲染视图
                self.update_view_with_current_frame(label)

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
        
        # 获取所有需要更新的视图标签
        all_views = [
            self.lbVerticalView, self.lbLeftView, self.lbFrontView,  # 主界面视图
            self.lbVerticalView_2, self.lbLeftView_2, self.lbFrontView_2  # 选项卡视图
        ]
        
        # 更新每个视图
        for label in all_views:
            if label in self.drawing_manager.measurement_managers:
                self.update_view_with_current_frame(label)

def main():
    app = QApplication(sys.argv)
    main_window = MainApp()
    main_window.show()
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()