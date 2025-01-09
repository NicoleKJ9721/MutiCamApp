from PyQt5.QtCore import Qt, QObject, QPoint
from PyQt5.QtWidgets import QLabel
from Tools.measurement_manager import MeasurementManager

class DrawingManager(QObject):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.active_view = None
        self.active_measurement = None
        self.measurement_managers = {}  # 存储每个视图对应的测量管理器
        
    def setup_view(self, label: QLabel, name: str):
        """为视图设置绘画功能"""
        # 创建测量管理器
        self.measurement_managers[label] = MeasurementManager()
        
        # 启用鼠标追踪
        label.setMouseTracking(True)
        
        # 设置鼠标事件处理
        label.mousePressEvent = lambda evt: self.handle_mouse_press(evt, label)
        label.mouseMoveEvent = lambda evt: self.handle_mouse_move(evt, label)
        label.mouseReleaseEvent = lambda evt: self.handle_mouse_release(evt, label)
        label.mouseDoubleClickEvent = lambda evt: self.parent().label_mouseDoubleClickEvent(evt, label)
        
    def start_line_measurement(self):
        """启动直线测量模式"""
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
        """清空绘画
        Args:
            label: 指定要清空的视图，None表示清空所有
        """
        if label:
            if label in self.measurement_managers:
                self.measurement_managers[label].clear_measurements()
        else:
            for manager in self.measurement_managers.values():
                manager.clear_measurements()
                
    def handle_mouse_press(self, event, label):
        """处理鼠标按下事件"""
        if event.button() == Qt.LeftButton:
            self.active_view = label
            self.active_measurement = self.measurement_managers.get(label)
            if self.active_measurement:
                current_frame = self.parent().get_current_frame_for_view(label)
                if current_frame is not None:
                    image_pos = self.convert_mouse_to_image_coords(event.pos(), label, current_frame)
                    self.active_measurement.handle_mouse_press(image_pos, current_frame)
                    # 立即更新显示
                    display_frame = self.active_measurement.create_display_frame(current_frame)
                    if display_frame is not None:
                        self.parent().update_view(label, display_frame)
                    
    def handle_mouse_move(self, event, label):
        """处理鼠标移动事件"""
        if label == self.active_view and self.active_measurement:
            current_frame = self.parent().get_current_frame_for_view(label)
            if current_frame is not None:
                image_pos = self.convert_mouse_to_image_coords(event.pos(), label, current_frame)
                display_frame = self.active_measurement.handle_mouse_move(image_pos, current_frame)
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