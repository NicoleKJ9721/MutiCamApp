import warnings
warnings.filterwarnings("ignore", category=DeprecationWarning)
import sys
import os
import numpy as np
import cv2

# 添加项目根目录到系统路径
current_dir = os.path.dirname(os.path.abspath(__file__))
if current_dir not in sys.path:
    sys.path.append(current_dir)

from PyQt5.QtWidgets import *
from PyQt5.QtGui import QImage, QPixmap
from PyQt5.QtCore import Qt, QThread, QPoint
from mainwindow import Ui_MainWindow
# import cv2
from Tools.camera_thread import CameraThread
from Tools.settings_manager import SettingsManager
from Tools.log_manager import LogManager
from Tools.measurement_manager import MeasurementManager
from Tools.drawing_manager import DrawingManager

class MainApp(QMainWindow, Ui_MainWindow):
    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.current_frame = None
        
        # 初始化管理器
        self.log_manager = LogManager()
        self.settings_manager = SettingsManager()
        
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

        # 初始化测量管理器
        self.ver_measurement = MeasurementManager()
        self.left_measurement = MeasurementManager()
        self.front_measurement = MeasurementManager()
        self.ver_measurement_2 = MeasurementManager()
        
        # 当前活动的视图
        self.active_view = None
        self.active_measurement = None

        # 添加标志位
        self.is_undoing = False

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
        self.drawing_manager.setup_view(self.lbLeftView, "left")
        self.drawing_manager.setup_view(self.lbFrontView, "front")
        self.drawing_manager.setup_view(self.lbVerticalView_2, "vertical_2")

    def _connect_signals(self):
        """连接信号槽"""
        # 修改为 editingFinished 信号，只在用户编辑完成时触发
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

        # 其他参数也改用 editingFinished
        self.ledCannyLineLow.editingFinished.connect(lambda: self.settings_manager.save_settings(self))
        self.ledCannyLineHigh.editingFinished.connect(lambda: self.settings_manager.save_settings(self))
        self.ledLineDetThreshold.editingFinished.connect(lambda: self.settings_manager.save_settings(self))
        self.ledLineDetMinLength.editingFinished.connect(lambda: self.settings_manager.save_settings(self))
        self.ledLineDetMaxGap.editingFinished.connect(lambda: self.settings_manager.save_settings(self))

        # UI尺寸修改信号 - 改用 textChanged 并添加实时更新窗口大小的功能
        self.ledUIWidth.textChanged.connect(self.update_window_size)
        self.ledUIHeight.textChanged.connect(self.update_window_size)
        
        # 在编辑完成时保存设置
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

        # 绘画功能按钮信号连接
        self.btnDrawStraight.clicked.connect(self.start_drawing_mode)
        self.btnDrawCircle.clicked.connect(self.start_drawing_mode)
        self.btnDrawLine.clicked.connect(self.start_drawing_mode)
        self.btnDrawParallel.clicked.connect(self.start_drawing_mode)
        
        # 添加直线检测按钮信号连接
        self.btnLineDet.clicked.connect(self.start_line_detection)  # 主界面按钮
        self.btnLineDet_Ver.clicked.connect(self.start_line_detection)  # 垂直选项卡按钮
        
        # 清空绘画按钮信号连接
        self.btnClearDrawings.clicked.connect(lambda: self.drawing_manager.clear_drawings())
        self.btnClearDrawings_Left.clicked.connect(
            lambda: self.drawing_manager.clear_drawings(self.lbLeftView))
        self.btnClearDrawings_Front.clicked.connect(
            lambda: self.drawing_manager.clear_drawings(self.lbFrontView))

        # 垂直选项卡的绘画功能按钮信号连接
        self.btnDrawStraight_Ver.clicked.connect(self.start_drawing_mode)  # 直线按钮
        self.btnDrawCircle_Ver.clicked.connect(self.start_drawing_mode)    # 圆形按钮
        self.btnDrawLine_Ver.clicked.connect(self.start_drawing_mode)      # 线段按钮
        self.btnClearDrawings_Ver.clicked.connect(                         # 清空按钮
            lambda: self.drawing_manager.clear_drawings(self.lbVerticalView_2))

        # 添加平行线按钮信号连接
        self.btnDrawParallel.clicked.connect(self.start_drawing_mode)
        self.btnDrawParallel_Ver.clicked.connect(self.start_drawing_mode)

        # 先断开所有旧的连接（安全的方式）
        for btn in [self.btnCan1StepDraw, self.btnCan1StepDraw_Ver, 
                    self.btnCan1StepDraw_Left, self.btnCan1StepDraw_Front]:
            try:
                btn.clicked.disconnect()
            except TypeError:
                # 如果没有连接，会抛出 TypeError，我们可以安全地忽略它
                pass
        
        # 使用普通的 clicked 信号
        print("连接撤销按钮信号")
        self.btnCan1StepDraw.clicked.connect(self.undo_last_drawing)
        self.btnCan1StepDraw_Ver.clicked.connect(self.undo_last_drawing)
        self.btnCan1StepDraw_Left.clicked.connect(self.undo_last_drawing)
        self.btnCan1StepDraw_Front.clicked.connect(self.undo_last_drawing)

        self.btnDraw2Line.clicked.connect(self.start_two_lines_measurement)
        self.btnDrawLine_Circle.clicked.connect(self.start_circle_line_measurement)

        # 主界面和垂直选项卡的线与线测量按钮
        self.btnDraw2Line.clicked.connect(self.start_two_lines_measurement)
        self.btnDraw2Line_Ver.clicked.connect(self.start_two_lines_measurement)

        # 主界面和垂直选项卡的线与圆测量按钮
        self.btnDrawLine_Circle.clicked.connect(self.start_circle_line_measurement)
        self.btnDrawLine_Circle_Ver.clicked.connect(self.start_circle_line_measurement)

        # 垂直选项卡的保存图像按钮
        self.btnSaveImage_Ver.clicked.connect(self.save_images_vertical)

        # 不要在这里连接相机信号，移到 initCamera 中

    def initCamera(self):
        """初始化相机"""
        # 创建相机线程
        self.ver_camera_thread = CameraThread()
        
        # 连接相机信号
        self.ver_camera_thread.frame_signal.connect(self.update_frame)
        
        # 设置相机参数
        self.ver_camera_thread.running = False
        self.capturing = False

    def start_drawing_mode(self):
        """启动绘画模式"""
        sender = self.sender()
        if sender in [self.btnDrawParallel, self.btnDrawParallel_Ver]:
            self.drawing_manager.start_parallel_measurement()
        elif sender in [self.btnDrawStraight, self.btnDrawStraight_Ver]:
            self.drawing_manager.start_line_measurement()
        elif sender in [self.btnDrawCircle, self.btnDrawCircle_Ver]:
            self.drawing_manager.start_circle_measurement()
        elif sender in [self.btnDrawLine, self.btnDrawLine_Ver]:
            self.drawing_manager.start_line_segment_measurement()

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
                self.ver_camera_thread = CameraThread(
                    camera_sn=self.ledVerCamSN.text(),
                    exposure_time=int(self.ledVerCamExposureTime.text()),
                    image_format='Mono8' if self.cbVerCamImageFormat.currentText() == 'Mono8' else 'RGB8'
                )
                # 只连接到一个更新函数
                self.ver_camera_thread.frame_ready.connect(self.update_ver_camera_view)
                self.ver_camera_thread.error_occurred.connect(self.show_error)
                self.ver_camera_thread.start()
                
            self.ver_camera_thread.frame_ready.connect(self.update_ver_camera_view)
            self.ver_camera_thread.error_occurred.connect(self.show_error)
            self.ver_camera_thread.start()
            self.log_manager.log_camera_operation("启动", self.ledVerCamSN.text(), 
                                                f"曝光时间: {self.ledVerCamExposureTime.text()}, 格式: {self.cbVerCamImageFormat.currentText()}")

            # 左相机
            self.left_camera_thread = CameraThread(
                camera_sn=self.ledLeftCamSN.text(),
                exposure_time=int(self.ledLeftCamExposureTime.text()),
                image_format='Mono8' if self.cbLeftCamImageFormat.currentText() == 'Mono8' else 'RGB8'
            )
            self.left_camera_thread.frame_ready.connect(self.update_left_camera_view)
            self.left_camera_thread.error_occurred.connect(self.show_error)
            self.left_camera_thread.start()
            self.log_manager.log_camera_operation("启动", self.ledLeftCamSN.text(), 
                                                f"曝光时间: {self.ledLeftCamExposureTime.text()}, 格式: {self.cbLeftCamImageFormat.currentText()}")

            # 前相机
            self.front_camera_thread = CameraThread(
                camera_sn=self.ledFrontCamSN.text(),
                exposure_time=int(self.ledFrontCamExposureTime.text()),
                image_format='Mono8' if self.cbFrontCamImageFormat.currentText() == 'Mono8' else 'RGB8'
            )
            self.front_camera_thread.frame_ready.connect(self.update_front_camera_view)
            self.front_camera_thread.error_occurred.connect(self.show_error)
            self.front_camera_thread.start()
            self.log_manager.log_camera_operation("启动", self.ledFrontCamSN.text(), 
                                                f"曝光时间: {self.ledFrontCamExposureTime.text()}, 格式: {self.cbFrontCamImageFormat.currentText()}")

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
        self.current_frame = frame.copy()
        
        # 更新主界面的垂直视图
        main_measurement = self.drawing_manager.get_measurement_manager(self.lbVerticalView)
        if main_measurement:
            # 使用 LayerManager 渲染帧
            main_display_frame = main_measurement.layer_manager.render_frame(frame.copy())
            self.display_image(main_display_frame, self.lbVerticalView)
        
        # 更新垂直选项卡的视图
        tab_measurement = self.drawing_manager.get_measurement_manager(self.lbVerticalView_2)
        if tab_measurement:
            # 使用 LayerManager 渲染帧
            tab_display_frame = tab_measurement.layer_manager.render_frame(frame.copy())
            self.display_image(tab_display_frame, self.lbVerticalView_2)

    def update_left_camera_view(self, frame):
        """更新左侧相机视图"""
        measurement = self.drawing_manager.get_measurement_manager(self.lbLeftView)
        if measurement:
            # 使用 LayerManager 渲染帧
            display_frame = measurement.layer_manager.render_frame(frame.copy())
            self.display_image(display_frame, self.lbLeftView)
        else:
            self.display_image(frame, self.lbLeftView)

    def update_front_camera_view(self, frame):
        """更新前视相机视图"""
        measurement = self.drawing_manager.get_measurement_manager(self.lbFrontView)
        if measurement:
            # 使用 LayerManager 渲染帧
            display_frame = measurement.layer_manager.render_frame(frame.copy())
            self.display_image(display_frame, self.lbFrontView)
        else:
            self.display_image(frame, self.lbFrontView)

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

    def label_mousePressEvent(self, event, label):
        """处理标签的鼠标按下事件"""
        print(f"鼠标按下事件: {label.objectName()}")  # 调试信息
        if event.button() == Qt.LeftButton:
            # 确定当前活动的视图和测量管理器
            if label == self.lbVerticalView:
                self.active_view = self.lbVerticalView
                self.active_measurement = self.ver_measurement
                print("激活垂直视图")  # 调试信息
            elif label == self.lbLeftView:
                self.active_view = self.lbLeftView
                self.active_measurement = self.left_measurement
                print("激活左侧视图")  # 调试信息
            elif label == self.lbFrontView:
                self.active_view = self.lbFrontView
                self.active_measurement = self.front_measurement
                print("激活前视图")  # 调试信息
            
            if self.active_measurement:
                image_pos = self.convert_mouse_to_image_coords(event.pos(), label)
                current_frame = self.get_current_frame_for_view(label)
                print(f"图像坐标: {image_pos.x()}, {image_pos.y()}")  # 调试信息
                if current_frame is not None:
                    self.active_measurement.handle_mouse_press(image_pos, current_frame)
                else:
                    print("没有获取到当前帧")  # 调试信息

    def label_mouseMoveEvent(self, event, label):
        """处理标签的鼠标移动事件"""
        if label == self.active_view and self.active_measurement:
            image_pos = self.convert_mouse_to_image_coords(event.pos(), label)
            current_frame = self.get_current_frame_for_view(label)
            display_frame = self.active_measurement.handle_mouse_move(image_pos, current_frame)
            
            if display_frame is not None:
                self.update_view(label, display_frame)

    def label_mouseReleaseEvent(self, event, label):
        """处理标签的鼠标释放事件"""
        if event.button() == Qt.LeftButton and label == self.active_view and self.active_measurement:
            image_pos = self.convert_mouse_to_image_coords(event.pos(), label)
            current_frame = self.get_current_frame_for_view(label)
            display_frame = self.active_measurement.handle_mouse_release(image_pos, current_frame)
            
            if display_frame is not None:
                self.update_view(label, display_frame)
            
            self.active_view = None
            self.active_measurement = None

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

    def get_current_frame_for_view(self, label):
        """获取对应视图的当前帧"""
        frame = None
        if (label == self.lbVerticalView or label == self.lbVerticalView_2) and self.ver_camera_thread:
            frame = self.ver_camera_thread.current_frame
        elif label == self.lbLeftView and self.left_camera_thread:
            frame = self.left_camera_thread.current_frame
        elif label == self.lbFrontView and self.front_camera_thread:
            frame = self.front_camera_thread.current_frame
        
        if frame is None and not hasattr(self, '_frame_warning_shown'):
            print(f"警告: {label.objectName()} 没有可用的图像帧")  # 调试信息
            self._frame_warning_shown = True
        return frame

    def update_view(self, label, frame):
        """更新视图显示"""
        if frame is not None:
            height, width, channel = frame.shape
            bytes_per_line = 3 * width
            q_image = QImage(frame.data, width, height, bytes_per_line, QImage.Format_RGB888)
            pixmap = QPixmap.fromImage(q_image)
            label.setPixmap(pixmap.scaled(label.size(), Qt.KeepAspectRatio))

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
            elif label in [self.lbVerticalView_2, self.lbleftView_2, self.lbFrontView_2]:
                self.tabWidget.setCurrentIndex(0)
            
            # 记录最后操作的视图
            self.last_active_view = label

    def start_single_camera(self, camera_type):
        """启动单个相机"""
        try:
            if camera_type == 'vertical':
                self.ver_camera_thread = CameraThread(
                    camera_sn=self.ledVerCamSN.text(),
                    exposure_time=int(self.ledVerCamExposureTime.text()),
                    image_format='Mono8' if self.cbVerCamImageFormat.currentText() == 'Mono8' else 'RGB8'
                )
                # 只连接到一个更新函数
                self.ver_camera_thread.frame_ready.connect(self.update_ver_camera_view)
                self.ver_camera_thread.error_occurred.connect(self.show_error)
                self.ver_camera_thread.start()
                self.log_manager.log_camera_operation("启动", self.ledVerCamSN.text(), 
                                                    f"曝光时间: {self.ledVerCamExposureTime.text()}, 格式: {self.cbVerCamImageFormat.currentText()}")

            elif camera_type == 'left':
                self.left_camera_thread = CameraThread(
                    camera_sn=self.ledLeftCamSN.text(),
                    exposure_time=int(self.ledLeftCamExposureTime.text()),
                    image_format='Mono8' if self.cbLeftCamImageFormat.currentText() == 'Mono8' else 'RGB8'
                )
                # 只连接到一个更新函数
                self.left_camera_thread.frame_ready.connect(self.update_left_camera_view)
                self.left_camera_thread.error_occurred.connect(self.show_error)
                self.left_camera_thread.start()
                self.log_manager.log_camera_operation("启动", self.ledLeftCamSN.text(), 
                                                    f"曝光时间: {self.ledLeftCamExposureTime.text()}, 格式: {self.cbLeftCamImageFormat.currentText()}")

            elif camera_type == 'front':
                self.front_camera_thread = CameraThread(
                    camera_sn=self.ledFrontCamSN.text(),
                    exposure_time=int(self.ledFrontCamExposureTime.text()),
                    image_format='Mono8' if self.cbFrontCamImageFormat.currentText() == 'Mono8' else 'RGB8'
                )
                # 只连接到一个更新函数
                self.front_camera_thread.frame_ready.connect(self.update_front_camera_view)
                self.front_camera_thread.error_occurred.connect(self.show_error)
                self.front_camera_thread.start()
                self.log_manager.log_camera_operation("启动", self.ledFrontCamSN.text(), 
                                                    f"曝光时间: {self.ledFrontCamExposureTime.text()}, 格式: {self.cbFrontCamImageFormat.currentText()}")
                
            # 更新按钮状态
            self.btnStartMeasure.setEnabled(False)
            self.btnStopMeasure.setEnabled(True)
            
        except Exception as e:
            error_msg = f"启动相机失败: {str(e)}"
            self.log_manager.log_error(error_msg)
            self.show_error(error_msg)

    def undo_last_drawing(self):
        """撤销上一步绘画"""
        # 防止重复触发
        if self.is_undoing:
            print("正在撤销中，忽略重复触发")
            return
        
        self.is_undoing = True
        try:
            print("\n=== 开始撤销操作 ===")
            print(f"当前标签页索引: {self.tabWidget.currentIndex()}")
            
            # 获取当前活动的视图
            current_tab = self.tabWidget.currentIndex()
            current_view = None
            
            if current_tab == 0:  # 主界面
                if hasattr(self, 'last_active_view'):
                    current_view = self.last_active_view
                    print(f"主界面，最后活动视图: {current_view.objectName()}")
            else:
                # 选项卡视图
                view_map = {
                    1: self.lbVerticalView_2,
                    2: self.lbleftView_2,
                    3: self.lbFrontView_2
                }
                current_view = view_map.get(current_tab)
                if current_view:
                    print(f"选项卡视图: {current_view.objectName()}")
            
            if current_view:
                measurement = self.drawing_manager.get_measurement_manager(current_view)
                if measurement:
                    # 获取当前帧
                    current_frame = self.get_current_frame_for_view(current_view)
                    if current_frame is not None:
                        # 执行撤销操作
                        if measurement.undo_last_drawing():
                            # 重新创建显示帧
                            display_frame = measurement.create_display_frame(current_frame)
                            if display_frame is not None:
                                self.update_view(current_view, display_frame)
            print("=== 撤销操作结束 ===\n")
        finally:
            self.is_undoing = False

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

    def start_circle_line_measurement(self):
        """启动圆线距离测量模式"""
        self.drawing_manager.start_circle_line_measurement()

    def start_two_lines_measurement(self):
        """启动线与线测量模式"""
        self.drawing_manager.start_two_lines_measurement()

    def save_images_vertical(self):
        """垂直选项卡保存原始图像和可视化图像"""
        try:
            # 获取当前相机帧
            if self.current_frame is None:
                print("没有可用的相机图像")
                return
            
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
            cv2.imwrite(origin_path, cv2.cvtColor(self.current_frame, cv2.COLOR_RGB2BGR))  # 转换颜色空间
            print(f"原始图像已保存: {origin_filename}")
            
            # 获取可视化图像（原始图像 + 绘画）
            measurement_manager = self.drawing_manager.get_measurement_manager(self.lbVerticalView_2)
            if measurement_manager:
                # 获取当前显示的帧
                visual_frame = measurement_manager.layer_manager.render_frame(self.current_frame.copy())
                if visual_frame is not None:
                    # 保存可视化图像
                    visual_filename = f"visual_{current_time}.jpg"
                    visual_path = os.path.join(save_dir, visual_filename)
                    cv2.imwrite(visual_path, cv2.cvtColor(visual_frame, cv2.COLOR_RGB2BGR))  # 转换颜色空间
                    
                    print(f"图像已保存到: {save_dir}")
                    print(f"可视化图像: {visual_filename}")
                else:
                    print("保存原始图像成功，没有可视化内容需要保存")
                
        except Exception as e:
            print(f"保存图像时出错: {str(e)}")

    def update_frame(self, frame):
        """更新相机帧显示"""
        # 保存当前帧
        self.current_frame = frame.copy()
        
        # 调用原有的更新显示逻辑
        if self.verticalCameraLabel:
            height, width, channel = frame.shape
            bytes_per_line = 3 * width
            q_image = QImage(frame.data, width, height, bytes_per_line, QImage.Format_BGR888)
            scaled_pixmap = QPixmap.fromImage(q_image).scaled(
                self.verticalCameraLabel.size(),
                Qt.KeepAspectRatio,
                Qt.SmoothTransformation
            )
            self.verticalCameraLabel.setPixmap(scaled_pixmap)

    def start_line_detection(self):
        """启动直线检测模式"""
        print("开始直线检测")
        self.drawing_manager.start_line_detection()

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

def main():
    app = QApplication(sys.argv)
    main_window = MainApp()
    main_window.show()
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()