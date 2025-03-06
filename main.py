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
import time
from PyQt5.QtWidgets import *  # QSizePolicy 在 QtWidgets 中
from PyQt5.QtGui import QImage, QPixmap, QPainter, QPen, QColor, QIntValidator
from PyQt5.QtCore import Qt, QThread, QPoint, QRect
from mainwindow import Ui_MainWindow
from Tools.camera_thread import CameraThread
from Tools.settings_manager import SettingsManager
from Tools.log_manager import LogManager
from Tools.measurement_manager import MeasurementManager, DrawingType, DrawingObject
from Tools.drawing_manager import DrawingManager

class GridLabel(QLabel):
    """带网格功能的标签控件"""
    
    def __init__(self, parent=None):
        super().__init__(parent)
        self.grid_spacing = 0  # 网格间隔，0表示不显示网格
        self.grid_color = QColor(255, 0, 0)  # 网格颜色，默认红色
        self.grid_style = Qt.DashLine  # 网格线型，默认虚线
        self.grid_width = 1  # 网格线宽，默认2像素
        
        # 图像显示相关属性
        self.setMinimumSize(300, 300)  # 设置更大的最小尺寸
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)  # 设置尺寸策略为扩展
        self.setAlignment(Qt.AlignCenter)  # 居中对齐
        self.setScaledContents(False)  # 不自动缩放内容，我们在paintEvent中手动处理
    
    def setGridSpacing(self, spacing):
        """设置网格间隔"""
        self.grid_spacing = spacing
        self.update()  # 更新显示
    
    def clearGrid(self):
        """清除网格"""
        self.grid_spacing = 0
        self.update()  # 更新显示
    
    def paintEvent(self, event):
        """重写绘制事件"""
        # 首先绘制图像
        painter = QPainter(self)
        
        # 绘制图像
        pixmap = self.pixmap()
        if pixmap and not pixmap.isNull():
            # 获取标签和图像的尺寸
            label_width = self.width()
            label_height = self.height()
            pixmap_width = pixmap.width()
            pixmap_height = pixmap.height()
            
            # 计算缩放比例，使图像高度与标签高度完全一致
            scale = label_height / pixmap_height
            
            # 计算缩放后的宽度
            scaled_width = int(pixmap_width * scale)
            
            # 计算居中显示的位置
            x = int((label_width - scaled_width) / 2)
            
            # 绘制缩放后的图像
            target_rect = QRect(x, 0, scaled_width, label_height)
            painter.drawPixmap(target_rect, pixmap)
        
        # 如果设置了网格间隔且大于0，则绘制网格
        if self.grid_spacing > 0:
            pen = QPen(self.grid_color)
            pen.setStyle(self.grid_style)
            pen.setWidth(self.grid_width)
            painter.setPen(pen)
            
            # 获取标签的大小
            width = self.width()
            height = self.height()
            
            # 绘制水平线
            y = 0
            while y < height:
                painter.drawLine(0, y, width, y)
                y += self.grid_spacing
            
            # 绘制垂直线
            x = 0
            while x < width:
                painter.drawLine(x, 0, x, height)
                x += self.grid_spacing
        
        painter.end()

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
        
        # 添加一个标志位来控制是否应该触发网格更新
        self._is_programmatic_text_change = False

    def _init_ui(self):
        """初始化UI控件"""
        # 替换原有的QLabel为GridLabel
        self.grid_labels = []
        
        # 替换主界面的三个视图标签
        for old_label_name in ['lbVerticalView', 'lbLeftView', 'lbFrontView']:
            old_label = getattr(self, old_label_name)
            parent = old_label.parent()
            
            # 获取原标签的布局信息
            geometry = old_label.geometry()
            
            # 创建新的GridLabel
            grid_label = GridLabel(parent)
            
            # 复制原标签的属性
            grid_label.setObjectName(old_label_name)
            grid_label.setGeometry(geometry)
            grid_label.setStyleSheet(old_label.styleSheet())
            
            # 如果原标签在布局中，需要保持布局
            layout = None
            layout_position = None
            
            # 查找标签所在的布局
            for parent_layout in parent.findChildren(QLayout):
                # 处理不同类型的布局
                if isinstance(parent_layout, QGridLayout):
                    # 对于QGridLayout，需要找到行和列位置
                    for row in range(parent_layout.rowCount()):
                        for col in range(parent_layout.columnCount()):
                            item = parent_layout.itemAtPosition(row, col)
                            if item and item.widget() == old_label:
                                layout = parent_layout
                                layout_position = (row, col)
                                break
                        if layout:
                            break
                else:
                    # 对于其他布局，如QVBoxLayout, QHBoxLayout等
                    for i in range(parent_layout.count()):
                        if parent_layout.itemAt(i).widget() == old_label:
                            layout = parent_layout
                            layout_position = i
                            break
                if layout:
                    break
            
            # 保存原有的鼠标事件处理
            grid_label.mousePressEvent = lambda event, label=grid_label: self.label_mousePressEvent(event, label)
            grid_label.mouseMoveEvent = lambda event, label=grid_label: self.label_mouseMoveEvent(event, label)
            grid_label.mouseReleaseEvent = lambda event, label=grid_label: self.label_mouseReleaseEvent(event, label)
            grid_label.mouseDoubleClickEvent = lambda event, label=grid_label: self.label_mouseDoubleClickEvent(event, label)
            grid_label.contextMenuEvent = lambda event, label=grid_label: self.label_contextMenuEvent(event, label)
            
            # 替换原有标签
            if layout:
                old_label.setParent(None)
                if isinstance(layout, QGridLayout) and isinstance(layout_position, tuple):
                    # 对于QGridLayout，使用addWidget并指定行列
                    row, col = layout_position
                    layout.addWidget(grid_label, row, col)
                else:
                    # 对于其他布局，如QVBoxLayout, QHBoxLayout等
                    layout.removeItem(layout.itemAt(layout_position))
                    if isinstance(layout, QBoxLayout):  # QVBoxLayout和QHBoxLayout都是QBoxLayout的子类
                        layout.insertWidget(layout_position, grid_label)
                    else:
                        # 对于其他类型的布局，简单地添加
                        layout.addWidget(grid_label)
            else:
                old_label.setParent(None)
                grid_label.setGeometry(geometry)
            
            setattr(self, old_label_name, grid_label)
            self.grid_labels.append(grid_label)
            
            # 确保标签可见
            grid_label.show()
        
        # 替换选项卡的三个视图标签
        for old_label_name in ['lbVerticalView_2', 'lbLeftView_2', 'lbFrontView_2']:
            old_label = getattr(self, old_label_name)
            parent = old_label.parent()
            
            # 获取原标签的布局信息
            geometry = old_label.geometry()
            
            # 创建新的GridLabel
            grid_label = GridLabel(parent)
            
            # 复制原标签的属性
            grid_label.setObjectName(old_label_name)
            grid_label.setGeometry(geometry)
            grid_label.setStyleSheet(old_label.styleSheet())
            
            # 如果原标签在布局中，需要保持布局
            layout = None
            layout_position = None
            
            # 查找标签所在的布局
            for parent_layout in parent.findChildren(QLayout):
                # 处理不同类型的布局
                if isinstance(parent_layout, QGridLayout):
                    # 对于QGridLayout，需要找到行和列位置
                    for row in range(parent_layout.rowCount()):
                        for col in range(parent_layout.columnCount()):
                            item = parent_layout.itemAtPosition(row, col)
                            if item and item.widget() == old_label:
                                layout = parent_layout
                                layout_position = (row, col)
                                break
                        if layout:
                            break
                else:
                    # 对于其他布局，如QVBoxLayout, QHBoxLayout等
                    for i in range(parent_layout.count()):
                        if parent_layout.itemAt(i).widget() == old_label:
                            layout = parent_layout
                            layout_position = i
                            break
                if layout:
                    break
            
            # 保存原有的鼠标事件处理
            grid_label.mousePressEvent = lambda event, label=grid_label: self.label_mousePressEvent(event, label)
            grid_label.mouseMoveEvent = lambda event, label=grid_label: self.label_mouseMoveEvent(event, label)
            grid_label.mouseReleaseEvent = lambda event, label=grid_label: self.label_mouseReleaseEvent(event, label)
            grid_label.mouseDoubleClickEvent = lambda event, label=grid_label: self.label_mouseDoubleClickEvent(event, label)
            grid_label.contextMenuEvent = lambda event, label=grid_label: self.label_contextMenuEvent(event, label)
            
            # 替换原有标签
            if layout:
                old_label.setParent(None)
                if isinstance(layout, QGridLayout) and isinstance(layout_position, tuple):
                    # 对于QGridLayout，使用addWidget并指定行列
                    row, col = layout_position
                    layout.addWidget(grid_label, row, col)
                else:
                    # 对于其他布局，如QVBoxLayout, QHBoxLayout等
                    layout.removeItem(layout.itemAt(layout_position))
                    if isinstance(layout, QBoxLayout):  # QVBoxLayout和QHBoxLayout都是QBoxLayout的子类
                        layout.insertWidget(layout_position, grid_label)
                    else:
                        # 对于其他类型的布局，简单地添加
                        layout.addWidget(grid_label)
            else:
                old_label.setParent(None)
                grid_label.setGeometry(geometry)
            
            setattr(self, old_label_name, grid_label)
            self.grid_labels.append(grid_label)
            
            # 确保标签可见
            grid_label.show()
        
        # 设置网格密度输入框的验证器，只允许输入整数
        self.leGridDens.setValidator(QIntValidator(10, 10000, self))
        self.leGridDens_Ver.setValidator(QIntValidator(10, 10000, self))
        self.leGridDens_Left.setValidator(QIntValidator(10, 10000, self))
        self.leGridDens_Front.setValidator(QIntValidator(10, 10000, self))
        
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
        
        self.circle_line_action = QAction("线与圆测量", self) # type: ignore
        self.circle_line_action.triggered.connect(self.measure_circle_line)
        
        self.two_lines_action = QAction("线与线测量", self)  # type: ignore # 添加线与线选项
        self.two_lines_action.triggered.connect(self.measure_two_lines)
        
        # 直接添加所有菜单项
        self.context_menu.addAction(self.delete_action)
        self.context_menu.addSeparator()
        self.context_menu.addAction(self.circle_line_action)
        self.context_menu.addAction(self.two_lines_action)

        # 添加主界面保存图像按钮信号连接
        self.btnSaveImage.clicked.connect(self.save_images)

    def _connect_signals(self):
        """连接信号槽"""
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

        # 相机控制信号
        self.btnStartMeasure.clicked.connect(self.start_cameras)
        self.btnStopMeasure.clicked.connect(self.stop_cameras)

        # 主界面绘画功能按钮信号连接
        self.btnDrawPoint.clicked.connect(self.drawing_manager.start_point_measurement)  # 点按钮
        self.btnDrawStraight.clicked.connect(self.start_drawing_mode) # 直线按钮
        self.btnDrawParallel.clicked.connect(self.start_drawing_mode) # 平行线按钮
        self.btnDrawLine_Circle.clicked.connect(self.start_circle_line_measurement) # 线与圆测量按钮
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
        self.btnDrawLine_Circle_Ver.clicked.connect(self.start_circle_line_measurement) # 线与圆测量按钮
        self.btnDraw2Line_Ver.clicked.connect(self.start_two_lines_measurement) # 线与线测量按钮
        self.btnDrawSimpleCircle_Ver.clicked.connect(self.drawing_manager.start_simple_circle_measurement)  # 简单圆按钮
        self.btnDrawFineCircle_Ver.clicked.connect(self.drawing_manager.start_fine_circle_measurement)  # 精细圆按钮
        self.btnCan1StepDraw_Ver.clicked.connect(self.undo_last_drawing) # 撤销手动绘制
        self.btnCan1StepDet_Ver.clicked.connect(self.undo_last_detection) # 撤销自动检测
        self.btnClearDrawings_Ver.clicked.connect(                         # 清空按钮
            lambda: self.drawing_manager.clear_drawings(self.lbVerticalView_2))
        self.btnSaveImage_Ver.clicked.connect(lambda: self.save_images('vertical'))

        # 左侧选项卡绘画功能按钮信号连接
        self.btnLineDet_Left.clicked.connect(self.start_line_detection)  # 直线检测按钮
        self.btnCircleDet_Left.clicked.connect(self.start_circle_detection)  # 圆形检测按钮
        self.btnDrawPoint_Left.clicked.connect(self.drawing_manager.start_point_measurement)  # 点按钮
        self.btnDrawStraight_Left.clicked.connect(self.start_drawing_mode)  # 直线按钮
        self.btnDrawParallel_Left.clicked.connect(self.start_drawing_mode) # 平行线按钮
        self.btnDrawLine_Circle_Left.clicked.connect(self.start_circle_line_measurement) # 线与圆测量按钮
        self.btnDraw2Line_Left.clicked.connect(self.start_two_lines_measurement) # 线与线测量按钮
        self.btnDrawSimpleCircle_Left.clicked.connect(self.drawing_manager.start_simple_circle_measurement)  # 简单圆按钮
        self.btnDrawFineCircle_Left.clicked.connect(self.drawing_manager.start_fine_circle_measurement)  # 精细圆按钮
        self.btnCan1StepDraw_Left.clicked.connect(self.undo_last_drawing) # 撤销手动绘制
        self.btnCan1StepDet_Left.clicked.connect(self.undo_last_detection) # 撤销自动检测
        self.btnClearDrawings_Left.clicked.connect(                         # 清空按钮
            lambda: self.drawing_manager.clear_drawings(self.lbLeftView_2))
        self.btnSaveImage_Left.clicked.connect(lambda: self.save_images('left'))

        # 对向选项卡绘画功能按钮信号连接
        self.btnLineDet_Front.clicked.connect(self.start_line_detection)  # 直线检测按钮
        self.btnCircleDet_Front.clicked.connect(self.start_circle_detection)  # 圆形检测按钮
        self.btnDrawPoint_Front.clicked.connect(self.drawing_manager.start_point_measurement)  # 点按钮
        self.btnDrawStraight_Front.clicked.connect(self.start_drawing_mode)  # 直线按钮
        self.btnDrawParallel_Front.clicked.connect(self.start_drawing_mode) # 平行线按钮
        self.btnDrawLine_Circle_Front.clicked.connect(self.start_circle_line_measurement) # 线与圆测量按钮
        self.btnDraw2Line_Front.clicked.connect(self.start_two_lines_measurement) # 线与线测量按钮
        self.btnDrawSimpleCircle_Front.clicked.connect(self.drawing_manager.start_simple_circle_measurement)  # 简单圆按钮
        self.btnDrawFineCircle_Front.clicked.connect(self.drawing_manager.start_fine_circle_measurement)  # 精细圆按钮
        self.btnCan1StepDraw_Front.clicked.connect(self.undo_last_drawing) # 撤销手动绘制
        self.btnCan1StepDet_Front.clicked.connect(self.undo_last_detection) # 撤销自动检测
        self.btnClearDrawings_Front.clicked.connect(                         # 清空按钮
            lambda: self.drawing_manager.clear_drawings(self.lbFrontView_2))
        self.btnSaveImage_Front.clicked.connect(lambda: self.save_images('front'))

        # 添加选项卡切换信号连接
        self.tabWidget.currentChanged.connect(self.handle_tab_change)

        # 连接网格相关的信号
        self.leGridDens.editingFinished.connect(lambda: self.update_grid(0))  # 主界面
        self.btnCancelGrids.clicked.connect(lambda: self.clear_grid(0))

        # 连接垂直选项卡网格相关的信号
        self.leGridDens_Ver.editingFinished.connect(lambda: self.update_grid(1))  # 垂直选项卡
        self.btnCancelGrids_Ver.clicked.connect(lambda: self.clear_grid(1))

        # 连接左侧选项卡网格相关的信号
        self.leGridDens_Left.editingFinished.connect(lambda: self.update_grid(2))  # 左视图选项卡
        self.btnCancelGrids_Left.clicked.connect(lambda: self.clear_grid(2))

        # 连接前视图选项卡网格相关的信号
        self.leGridDens_Front.editingFinished.connect(lambda: self.update_grid(3))  # 前视图选项卡
        self.btnCancelGrids_Front.clicked.connect(lambda: self.clear_grid(3))

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

    def update_grid(self, tab_index):
        """更新网格显示"""
        try:
            # 根据选项卡索引获取对应的输入框和标签
            if tab_index == 0:  # 主界面
                grid_input = self.leGridDens
                labels = [self.lbVerticalView, self.lbLeftView, self.lbFrontView]
            elif tab_index == 1:  # 垂直选项卡
                grid_input = self.leGridDens_Ver
                labels = [self.lbVerticalView_2, self.lbVerticalView]  # 包括主界面对应视图
            elif tab_index == 2:  # 左视图选项卡
                grid_input = self.leGridDens_Left
                labels = [self.lbLeftView_2, self.lbLeftView]  # 包括主界面对应视图
            elif tab_index == 3:  # 前视图选项卡
                grid_input = self.leGridDens_Front
                labels = [self.lbFrontView_2, self.lbFrontView]  # 包括主界面对应视图
            else:
                return

            # 获取网格间隔
            grid_spacing_text = grid_input.text()
            if not grid_spacing_text:
                return
                
            grid_spacing = int(grid_spacing_text)
            if grid_spacing <= 0:
                return

            # 更新所有相关标签的网格
            for label in labels:
                if isinstance(label, GridLabel):
                    label.setGridSpacing(grid_spacing)
                    label.update()  # 强制重绘
            
            self.log_manager.log_ui_operation(f"设置网格间隔为 {grid_spacing} 像素")
        except ValueError:
            # 输入不是有效的整数
            grid_input.clear()
            self.log_manager.log_warning("网格间隔必须是有效的整数")

    def clear_grid(self, tab_index):
        """清除网格显示"""
        # 根据选项卡索引获取对应的输入框和标签
        if tab_index == 0:  # 主界面
            grid_input = self.leGridDens
            labels = [self.lbVerticalView, self.lbLeftView, self.lbFrontView]
        elif tab_index == 1:  # 垂直选项卡
            grid_input = self.leGridDens_Ver
            labels = [self.lbVerticalView_2, self.lbVerticalView]  # 包括主界面对应视图
        elif tab_index == 2:  # 左视图选项卡
            grid_input = self.leGridDens_Left
            labels = [self.lbLeftView_2, self.lbLeftView]  # 包括主界面对应视图
        elif tab_index == 3:  # 前视图选项卡
            grid_input = self.leGridDens_Front
            labels = [self.lbFrontView_2, self.lbFrontView]  # 包括主界面对应视图
        else:
            return

        # 清除所有相关标签的网格
        for label in labels:
            if isinstance(label, GridLabel):
                label.clearGrid()
                label.update()  # 强制重绘
        
        # 清除网格间隔输入框
        grid_input.clear()
        
        self.log_manager.log_ui_operation("清除网格显示")

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
            
            # 更新主界面的垂直视图（主界面总是需要更新）
            main_measurement = self.drawing_manager.get_measurement_manager(self.lbVerticalView)
            if main_measurement:
                main_display_frame = main_measurement.layer_manager.render_frame(frame)
                if main_display_frame is not None:
                    self.display_image(main_display_frame, self.lbVerticalView)
            
            # 更新垂直选项卡的视图（当选项卡索引为1时）
            if self.tabWidget.currentIndex() == 1:
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
            
            # 更新主界面的左侧视图（主界面总是需要更新）
            main_measurement = self.drawing_manager.get_measurement_manager(self.lbLeftView)
            if main_measurement:
                main_display_frame = main_measurement.layer_manager.render_frame(frame)
                if main_display_frame is not None:
                    self.display_image(main_display_frame, self.lbLeftView)
            
            # 更新左侧选项卡的视图（当选项卡索引为2时）
            if self.tabWidget.currentIndex() == 2:
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
            
            # 更新主界面的前视图（主界面总是需要更新）
            main_measurement = self.drawing_manager.get_measurement_manager(self.lbFrontView)
            if main_measurement:
                main_display_frame = main_measurement.layer_manager.render_frame(frame)
                if main_display_frame is not None:
                    self.display_image(main_display_frame, self.lbFrontView)
            
            # 更新对向选项卡的视图（当选项卡索引为3时）
            if self.tabWidget.currentIndex() == 3:
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
                
            # 确保保存当前的网格间隔
            grid_spacing = 0
            if isinstance(label, GridLabel):
                grid_spacing = label.grid_spacing
            
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
            
            # 设置图像
            label.setPixmap(pixmap)
            
            # 恢复网格间隔
            if isinstance(label, GridLabel) and grid_spacing > 0:
                label.setGridSpacing(grid_spacing)
            
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
        """保存图像"""
        try:
            # 获取保存路径
            save_dir = QFileDialog.getExistingDirectory(self, "选择保存目录", "")
            if not save_dir:
                return
            
            timestamp = time.strftime("%Y%m%d_%H%M%S")
            
            if view_type == 'vertical':
                # 保存垂直视图
                if self.current_frame_vertical is not None:
                    # 获取带有测量结果的帧
                    measurement = self.drawing_manager.get_measurement_manager(self.lbVerticalView_2)
                    if measurement:
                        display_frame = measurement.layer_manager.render_frame(self.current_frame_vertical)
                        if display_frame is not None:
                            # 保存图像（不包含网格）
                            save_path = os.path.join(save_dir, f"vertical_{timestamp}.png")
                            cv2.imwrite(save_path, display_frame)
                            self.log_manager.log_ui_operation(f"保存垂直视图到 {save_path}")
            elif view_type == 'left':
                # 保存左视图
                if self.current_frame_left is not None:
                    # 获取带有测量结果的帧
                    measurement = self.drawing_manager.get_measurement_manager(self.lbLeftView_2)
                    if measurement:
                        display_frame = measurement.layer_manager.render_frame(self.current_frame_left)
                        if display_frame is not None:
                            # 保存图像（不包含网格）
                            save_path = os.path.join(save_dir, f"left_{timestamp}.png")
                            cv2.imwrite(save_path, display_frame)
                            self.log_manager.log_ui_operation(f"保存左视图到 {save_path}")
            elif view_type == 'front':
                # 保存前视图
                if self.current_frame_front is not None:
                    # 获取带有测量结果的帧
                    measurement = self.drawing_manager.get_measurement_manager(self.lbFrontView_2)
                    if measurement:
                        display_frame = measurement.layer_manager.render_frame(self.current_frame_front)
                        if display_frame is not None:
                            # 保存图像（不包含网格）
                            save_path = os.path.join(save_dir, f"front_{timestamp}.png")
                            cv2.imwrite(save_path, display_frame)
                            self.log_manager.log_ui_operation(f"保存前视图到 {save_path}")
            else:
                # 保存所有视图
                self.save_all_views()
                    
        except Exception as e:
            error_msg = f"保存图像失败: {str(e)}"
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
                
                # 把LINE和LINE_SEGMENT都当作直线处理
                is_line = lambda t: t in [DrawingType.LINE, DrawingType.LINE_SEGMENT]
                
                # 添加分隔线
                self.context_menu.addSeparator()
                
                # 圆和直线（或线段）的选项
                is_circle_line = ((types[0] == DrawingType.CIRCLE and is_line(types[1])) or
                                (types[1] == DrawingType.CIRCLE and is_line(types[0])))
                
                # 两条直线（或线段）的选项
                is_two_lines = all(is_line(t) for t in types)
                if is_two_lines:
                    self.two_lines_action = QAction("线与线测量", self)
                    self.two_lines_action.triggered.connect(self.measure_two_lines)
                    self.context_menu.addAction(self.two_lines_action)
                
                # 两个点的选项
                is_two_points = all(t == DrawingType.POINT for t in types)
                if is_two_points:
                    menu_items = self.drawing_manager.handle_context_menu(label)
                    for item_text, item_handler in menu_items:
                        action = QAction(item_text, self)
                        action.triggered.connect(item_handler)
                        self.context_menu.addAction(action)
                
                # 点和线的选项
                is_point_and_line = ((types[0] == DrawingType.POINT and is_line(types[1])) or
                                   (types[1] == DrawingType.POINT and is_line(types[0])))
                if is_point_and_line:
                    menu_items = self.drawing_manager.handle_context_menu(label)
                    for item_text, item_handler in menu_items:
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

    def save_all_views(self):
        """保存所有视图"""
        try:
            # 获取保存路径
            save_dir = QFileDialog.getExistingDirectory(self, "选择保存目录", "")
            if not save_dir:
                return
                
            timestamp = time.strftime("%Y%m%d_%H%M%S")
            
            # 保存垂直视图
            if self.current_frame_vertical is not None:
                # 获取带有测量结果的帧
                measurement = self.drawing_manager.get_measurement_manager(self.lbVerticalView)
                if measurement:
                    display_frame = measurement.layer_manager.render_frame(self.current_frame_vertical)
                    if display_frame is not None:
                        # 保存图像（不包含网格）
                        save_path = os.path.join(save_dir, f"vertical_{timestamp}.png")
                        cv2.imwrite(save_path, display_frame)
                        self.log_manager.log_ui_operation(f"保存垂直视图到 {save_path}")
            
            # 保存左视图
            if self.current_frame_left is not None:
                # 获取带有测量结果的帧
                measurement = self.drawing_manager.get_measurement_manager(self.lbLeftView)
                if measurement:
                    display_frame = measurement.layer_manager.render_frame(self.current_frame_left)
                    if display_frame is not None:
                        # 保存图像（不包含网格）
                        save_path = os.path.join(save_dir, f"left_{timestamp}.png")
                        cv2.imwrite(save_path, display_frame)
                        self.log_manager.log_ui_operation(f"保存左视图到 {save_path}")
            
            # 保存前视图
            if self.current_frame_front is not None:
                # 获取带有测量结果的帧
                measurement = self.drawing_manager.get_measurement_manager(self.lbFrontView)
                if measurement:
                    display_frame = measurement.layer_manager.render_frame(self.current_frame_front)
                    if display_frame is not None:
                        # 保存图像（不包含网格）
                        save_path = os.path.join(save_dir, f"front_{timestamp}.png")
                        cv2.imwrite(save_path, display_frame)
                        self.log_manager.log_ui_operation(f"保存前视图到 {save_path}")
                        
        except Exception as e:
            error_msg = f"保存所有视图失败: {str(e)}"
            self.log_manager.log_error(error_msg)
            self.show_error(error_msg)

    def _force_garbage_collection(self):
        """强制进行垃圾回收"""
        import gc
        gc.collect()

    def label_mousePressEvent(self, event, label):
        """标签鼠标按下事件"""
        if event.button() == Qt.LeftButton:
            # 设置当前活动视图
            self.active_view = label
            
            # 获取当前帧
            current_frame = self.get_current_frame_for_view(label)
            if current_frame is None:
                return
            
            # 获取图像坐标
            image_pos = self.convert_mouse_to_image_coords(event.pos(), label)
            if image_pos is None:
                return
            
            # 获取测量管理器
            measurement = self.drawing_manager.get_measurement_manager(label)
            if measurement:
                self.active_measurement = measurement
                # 处理鼠标按下事件
                display_frame = measurement.handle_mouse_press(image_pos, current_frame)
                if display_frame is not None:
                    self.display_image(display_frame, label)
    
    def label_mouseMoveEvent(self, event, label):
        """标签鼠标移动事件"""
        if event.buttons() & Qt.LeftButton and self.active_view == label:
            # 获取当前帧
            current_frame = self.get_current_frame_for_view(label)
            if current_frame is None:
                return
            
            # 获取图像坐标
            image_pos = self.convert_mouse_to_image_coords(event.pos(), label)
            if image_pos is None:
                return
            
            # 获取测量管理器
            measurement = self.drawing_manager.get_measurement_manager(label)
            if measurement:
                # 处理鼠标移动事件
                display_frame = measurement.handle_mouse_move(image_pos, current_frame)
                if display_frame is not None:
                    self.display_image(display_frame, label)
    
    def label_mouseReleaseEvent(self, event, label):
        """标签鼠标释放事件"""
        if event.button() == Qt.LeftButton and self.active_view == label:
            # 获取当前帧
            current_frame = self.get_current_frame_for_view(label)
            if current_frame is None:
                return
            
            # 获取图像坐标
            image_pos = self.convert_mouse_to_image_coords(event.pos(), label)
            if image_pos is None:
                return
            
            # 获取测量管理器
            measurement = self.drawing_manager.get_measurement_manager(label)
            if measurement:
                # 处理鼠标释放事件
                display_frame = measurement.handle_mouse_release(image_pos, current_frame)
                if display_frame is not None:
                    self.display_image(display_frame, label)
    
    def label_contextMenuEvent(self, event, label):
        """标签上下文菜单事件"""
        self.show_context_menu(event.globalPos(), label)

def main():
    app = QApplication(sys.argv)
    main_window = MainApp()
    main_window.show()
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()