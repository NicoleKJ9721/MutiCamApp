from PyQt5.QtCore import Qt, QObject, QPoint
from PyQt5.QtWidgets import QLabel
from Tools.measurement_manager import MeasurementManager
import time

class DrawingManager(QObject):
    def __init__(self, parent=None):
        try:
            super().__init__(parent)
            self.active_view = None
            self.active_measurement = None
            self.measurement_managers = {}  # 存储每个视图对应的测量管理器
            self.log_manager = parent.log_manager if parent else None
        except Exception as e:
            if self.log_manager:
                self.log_manager.log_error("DrawingManager初始化失败", str(e))
            raise
        
    def safe_execute(self, func, *args, **kwargs):
        """安全执行方法的装饰器"""
        try:
            return func(*args, **kwargs)
        except Exception as e:
            if self.log_manager:
                self.log_manager.log_error(f"{func.__name__}执行失败", str(e))
            return None
        
    def setup_view(self, label: QLabel, name: str):
        """为视图设置绘画功能"""
        # 创建测量管理器时传入 DrawingManager 实例本身，而不是 parent
        measurement_manager = MeasurementManager(self)
        self.measurement_managers[label] = measurement_manager
        
        # 启用鼠标追踪
        label.setMouseTracking(True)
        
        # 设置鼠标事件处理
        label.mousePressEvent = lambda evt: self.handle_mouse_press(evt, label)
        label.mouseMoveEvent = lambda evt: self.handle_mouse_move(evt, label)
        label.mouseReleaseEvent = lambda evt: self.handle_mouse_release(evt, label)
        label.mouseDoubleClickEvent = lambda evt: self.parent().label_mouseDoubleClickEvent(evt, label)
        
    def start_line_measurement(self):
        """启动线段测量模式"""
        return self.safe_execute(self._start_line_measurement)

    def _start_line_measurement(self):
        for label, manager in self.measurement_managers.items():
            manager.start_line_measurement()
            label.setCursor(Qt.CrossCursor)
            
    def start_circle_measurement(self):
        """启动圆形测量模式"""
        for label, manager in self.measurement_managers.items():
            manager.start_circle_measurement()
            label.setCursor(Qt.CrossCursor)
            
    def start_line_segment_measurement(self):
        """启动线段测量模式"""
        for label, manager in self.measurement_managers.items():
            manager.start_line_segment_measurement()
            label.setCursor(Qt.CrossCursor)
            
    def clear_drawings(self, label=None):
        """清空手动绘画"""
        if self.log_manager:
            self.log_manager.log_drawing_operation("清空绘画", f"视图: {label.objectName() if label else '所有'}")
        if label:
            # 清空特定视图的手动绘画
            if label in self.measurement_managers:
                manager = self.measurement_managers[label]
                manager.layer_manager.clear_drawings()  # 只清空手动绘制
                # 重新渲染视图
                current_frame = self.parent().get_current_frame_for_view(label)
                if current_frame is not None:
                    display_frame = manager.layer_manager.render_frame(current_frame)
                    if display_frame is not None:
                        self.parent().update_view(label, display_frame)
        else:
            # 清空所有视图的手动绘画
            for label, manager in self.measurement_managers.items():
                manager.layer_manager.clear_drawings()  # 只清空手动绘制
                # 重新渲染视图
                current_frame = self.parent().get_current_frame_for_view(label)
                if current_frame is not None:
                    display_frame = manager.layer_manager.render_frame(current_frame)
                    if display_frame is not None:
                        self.parent().update_view(label, display_frame)
                
    def handle_mouse_press(self, event, label):
        """处理鼠标按下事件"""
        if label not in self.measurement_managers:
            return
            
        self.active_view = label
        self.active_measurement = self.measurement_managers[label]
        
        # 获取当前帧
        current_frame = self.parent().get_current_frame_for_view(label)
        if current_frame is None:
            return
            
        # 转换鼠标坐标
        image_pos = self.convert_mouse_to_image_coords(event.pos(), label, current_frame)
        
        # 处理鼠标事件
        display_frame = self.active_measurement.handle_mouse_press(image_pos, current_frame)
        if display_frame is not None:
            self.parent().update_view(label, display_frame)
                    
    def handle_mouse_move(self, event, label):
        """处理鼠标移动事件"""
        if label not in self.measurement_managers:
            return
        
        # 添加节流，限制更新频率
        current_time = time.time()
        if hasattr(self, '_last_update_time') and current_time - self._last_update_time < 0.03:  # 约30fps
            return
        self._last_update_time = current_time
        
        # 处理鼠标移动
        current_frame = self.parent().get_current_frame_for_view(label)
        if current_frame is not None:
            image_pos = self.convert_mouse_to_image_coords(event.pos(), label, current_frame)
            display_frame = self.measurement_managers[label].handle_mouse_move(image_pos, current_frame)
            if display_frame is not None:
                self.parent().update_view(label, display_frame)
                    
    def handle_mouse_release(self, event, label):
        """处理鼠标释放事件"""
        if event.button() == Qt.LeftButton and label == self.active_view and self.active_measurement:
            current_frame = self.parent().get_current_frame_for_view(label)
            if current_frame is not None:
                image_pos = self.convert_mouse_to_image_coords(event.pos(), label, current_frame)
                display_frame = self.active_measurement.handle_mouse_release(image_pos, current_frame)
                if display_frame is not None:
                    self.parent().update_view(label, display_frame)
                    # 记录最后操作的视图
                    self.parent().last_active_view = label
            
            self.active_view = None
            self.active_measurement = None
            
    def convert_mouse_to_image_coords(self, pos, label, current_frame):
        """将鼠标坐标转换为图像坐标"""
        img_height, img_width = current_frame.shape[:2]
        label_width = label.width()
        label_height = label.height()
        
        ratio = min(label_width / img_width, label_height / img_height)
        display_width = int(img_width * ratio)
        display_height = int(img_height * ratio)
        
        x_offset = (label_width - display_width) // 2
        y_offset = (label_height - display_height) // 2
        
        image_x = int((pos.x() - x_offset) * img_width / display_width)
        image_y = int((pos.y() - y_offset) * img_height / display_height)
        
        image_x = max(0, min(image_x, img_width - 1))
        image_y = max(0, min(image_y, img_height - 1))
        
        return QPoint(image_x, image_y)
        
    def get_measurement_manager(self, label):
        """获取视图对应的测量管理器"""
        return self.measurement_managers.get(label) 
        
    def start_parallel_measurement(self):
        """启动平行线测量模式"""
        print("启动平行线测量")
        for label, manager in self.measurement_managers.items():
            manager.start_parallel_measurement()
            label.setCursor(Qt.CrossCursor) 
        
    def start_circle_line_measurement(self):
        """启动圆线距离测量模式"""
        print("启动圆线距离测量")
        for label, manager in self.measurement_managers.items():
            manager.start_circle_line_measurement()
            label.setCursor(Qt.CrossCursor) 
        
    def start_two_lines_measurement(self):
        """启动线与线测量模式"""
        print("启动线与线测量")
        for label, manager in self.measurement_managers.items():
            manager.start_two_lines_measurement()
            label.setCursor(Qt.CrossCursor) 
        
    def start_line_detection(self):
        """启动直线检测模式"""
        print("启动直线检测")
        for label, manager in self.measurement_managers.items():
            manager.start_line_detection()
            label.setCursor(Qt.CrossCursor)
            
    def start_circle_detection(self):
        """启动圆形检测模式"""
        print("启动圆形检测")
        for label, manager in self.measurement_managers.items():
            manager.start_circle_detection()
            label.setCursor(Qt.CrossCursor) 

    def undo_last_drawing(self):
        """撤销上一步手动绘制"""
        active_view = self.parent().last_active_view
        if active_view and active_view in self.measurement_managers:
            if self.log_manager:
                self.log_manager.log_drawing_operation("撤销绘制", f"视图: {active_view.objectName()}")
            manager = self.measurement_managers[active_view]
            if manager.layer_manager.undo_last_drawing():
                current_frame = self.parent().get_current_frame_for_view(active_view)
                if current_frame is not None:
                    display_frame = manager.layer_manager.render_frame(current_frame)
                    if display_frame is not None:
                        self.parent().update_view(active_view, display_frame)
                        
    def undo_last_detection(self):
        """撤销上一步自动检测"""
        active_view = self.parent().last_active_view
        if active_view and active_view in self.measurement_managers:
            if self.log_manager:
                self.log_manager.log_drawing_operation("撤销检测", f"视图: {active_view.objectName()}")
            manager = self.measurement_managers[active_view]
            if manager.layer_manager.undo_last_detection():
                current_frame = self.parent().get_current_frame_for_view(active_view)
                if current_frame is not None:
                    display_frame = manager.layer_manager.render_frame(current_frame)
                    if display_frame is not None:
                        self.parent().update_view(active_view, display_frame)
