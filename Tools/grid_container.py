from PyQt5.QtWidgets import QWidget, QLabel, QVBoxLayout, QStackedLayout, QGridLayout
from PyQt5.QtGui import QPainter, QPen, QColor
from PyQt5.QtCore import Qt, pyqtSignal, QEvent, QPoint

class GridOverlay(QWidget):
    """
    一个透明的覆盖层，用于在QLabel上方绘制网格
    """
    
    def __init__(self, parent=None):
        super().__init__(parent)
        
        # 网格属性
        self.grid_spacing = 0  # 默认不显示网格
        self.grid_color = QColor(255, 0, 0)  # 红色
        self.grid_style = Qt.DashLine  # 虚线
        self.grid_width = 1  # 网格线宽，默认2像素
        
        # 设置为透明背景
        self.setAttribute(Qt.WA_TransparentForMouseEvents)
        self.setAttribute(Qt.WA_NoSystemBackground)
        self.setAttribute(Qt.WA_TranslucentBackground)
        
    def setGridSpacing(self, spacing):
        """设置网格间距"""
        self.grid_spacing = spacing
        self.update()  # 触发重绘
        
    def setGridColor(self, color):
        """设置网格颜色"""
        self.grid_color = color
        self.update()  # 触发重绘
        
    def setGridStyle(self, style):
        """设置网格线样式"""
        self.grid_style = style
        self.update()  # 触发重绘
        
    def paintEvent(self, event):
        """重写绘制事件，绘制网格"""
        super().paintEvent(event)
        
        # 如果网格间距为0，则不绘制网格
        if self.grid_spacing <= 0:
            return
            
        painter = QPainter(self)
        # 关闭抗锯齿以获得清晰的像素级线条
        painter.setRenderHint(QPainter.Antialiasing, False)
        
        # 设置网格线的画笔
        pen = QPen(self.grid_color)
        pen.setWidth(self.grid_width)
        pen.setStyle(self.grid_style)
        # 确保像素对齐
        pen.setCosmetic(True)
        painter.setPen(pen)
        
        # 绘制水平线
        for y in range(0, self.height(), self.grid_spacing):
            painter.drawLine(0, y, self.width(), y)
            
        # 绘制垂直线
        for x in range(0, self.width(), self.grid_spacing):
            painter.drawLine(x, 0, x, self.height())

class GridContainer(QWidget):
    """
    一个容器，包含QLabel和GridOverlay，使网格显示在QLabel上方
    """
    
    def __init__(self, parent=None):
        super().__init__(parent)
        
        # 创建网格布局
        self.grid_layout = QGridLayout(self)
        self.grid_layout.setContentsMargins(0, 0, 0, 0)
        self.grid_layout.setSpacing(0)
        
        # 内部QLabel引用
        self.label = None
        
        # 创建网格覆盖层
        self.grid_overlay = GridOverlay(self)
        
        # 设置覆盖层的堆叠顺序
        self.grid_overlay.raise_()
        self.grid_overlay.setGeometry(0, 0, self.width(), self.height())
        
        # 缩放相关属性
        self.zoom_factor = 1.0  # 初始缩放因子
        self.min_zoom_factor = 1.0  # 最小缩放因子（不能小于原始大小）
        self.max_zoom_factor = 5.0  # 最大缩放因子
        self.zoom_step = 0.1  # 每次滚轮滚动的缩放步长
        self.zoom_center = QPoint(0, 0)  # 缩放中心点
        self.view_offset_x = 0  # 当前视图的X偏移量
        self.view_offset_y = 0  # 当前视图的Y偏移量
        self.last_zoom_factor = 1.0  # 上一次的缩放因子
        
        # 设置可以接收键盘焦点
        self.setFocusPolicy(Qt.StrongFocus)
        
    def resizeEvent(self, event):
        """重写调整大小事件，确保覆盖层大小与容器一致"""
        super().resizeEvent(event)
        if self.grid_overlay:
            self.grid_overlay.setGeometry(0, 0, self.width(), self.height())
        
    def setGridSpacing(self, spacing):
        """设置网格间距"""
        self.grid_overlay.setGridSpacing(spacing)
        
    def setGridColor(self, color):
        """设置网格颜色"""
        self.grid_overlay.setGridColor(color)
        
    def addWidget(self, widget):
        """添加子部件到容器中"""
        if isinstance(widget, QLabel):
            self.label = widget
            self.grid_layout.addWidget(widget, 0, 0)
            
            # 确保网格覆盖层在最上层
            self.grid_overlay.setParent(None)  # 临时移除父对象
            self.grid_overlay.setParent(self)  # 重新设置父对象
            self.grid_overlay.raise_()  # 提升到最上层
            self.grid_overlay.setGeometry(0, 0, self.width(), self.height())
            self.grid_overlay.show()
            
    # 转发鼠标事件到内部的QLabel
    def mousePressEvent(self, event):
        # 获取焦点，以便接收键盘事件
        self.setFocus()
        
        if self.label and hasattr(self.label, 'mousePressEvent'):
            self.label.mousePressEvent(event)
        else:
            super().mousePressEvent(event)
            
    def mouseMoveEvent(self, event):
        if self.label and hasattr(self.label, 'mouseMoveEvent'):
            self.label.mouseMoveEvent(event)
        else:
            super().mouseMoveEvent(event)
            
    def mouseReleaseEvent(self, event):
        if self.label and hasattr(self.label, 'mouseReleaseEvent'):
            self.label.mouseReleaseEvent(event)
        else:
            super().mouseReleaseEvent(event)
            
    def mouseDoubleClickEvent(self, event):
        # 获取焦点，以便接收键盘事件
        self.setFocus()
        
        if self.label and hasattr(self.label, 'mouseDoubleClickEvent'):
            self.label.mouseDoubleClickEvent(event)
        else:
            super().mouseDoubleClickEvent(event)
            
    def wheelEvent(self, event):
        """处理滚轮事件，实现放大缩小功能"""
        # 获取焦点，以便接收键盘事件
        self.setFocus()
        
        # 记录鼠标位置作为缩放中心
        self.zoom_center = event.pos()
        
        # 计算缩放因子变化
        delta = event.angleDelta().y()
        zoom_delta = self.zoom_step if delta > 0 else -self.zoom_step
        
        # 更新缩放因子
        new_zoom_factor = self.zoom_factor + zoom_delta
        
        # 限制缩放范围
        new_zoom_factor = max(self.min_zoom_factor, min(self.max_zoom_factor, new_zoom_factor))
        
        # 如果缩放因子没有变化，不进行处理
        if abs(new_zoom_factor - self.zoom_factor) < 0.001:
            return
            
        # 保存上一次的缩放因子（在更新zoom_factor之前）
        self.last_zoom_factor = self.zoom_factor
            
        # 更新缩放因子
        self.zoom_factor = new_zoom_factor
        
        # 更新视图
        self.update_view()
        
        # 阻止事件继续传播
        event.accept()
        
    def update_view(self):
        """更新视图显示"""
        if self.label and self.label.pixmap():
            # 获取当前帧
            current_frame = self.window().get_current_frame_for_view(self.label)
            if current_frame is not None:
                # 获取测量管理器
                drawing_manager = self.window().drawing_manager
                if drawing_manager:
                    measurement_manager = drawing_manager.get_measurement_manager(self.label)
                    if measurement_manager:
                        # 重新渲染帧
                        display_frame = measurement_manager.layer_manager.render_frame(current_frame.copy())
                        if display_frame is not None:
                            # 更新视图
                            self.window().update_view_with_zoom(self.label, display_frame, self.zoom_factor, self.zoom_center)
                            
    def get_zoom_factor(self):
        """获取当前缩放因子"""
        return self.zoom_factor
        
    def reset_zoom(self):
        """重置缩放"""
        self.zoom_factor = 1.0
        self.view_offset_x = 0
        self.view_offset_y = 0
        self.last_zoom_factor = 1.0
        self.update_view()
        
    def keyPressEvent(self, event):
        """处理键盘按键事件，实现方向键移动功能"""
        # 只有在放大状态下才处理方向键移动
        if self.zoom_factor > 1.0:
            # 根据缩放因子调整移动步长，缩放越大，移动步长越大
            base_step = 10
            move_step = int(base_step * self.zoom_factor)
            
            # 处理方向键
            if event.key() == Qt.Key_Left:
                self.view_offset_x += move_step
                self.update_view()
                event.accept()
            elif event.key() == Qt.Key_Right:
                self.view_offset_x -= move_step
                self.update_view()
                event.accept()
            elif event.key() == Qt.Key_Up:
                self.view_offset_y += move_step
                self.update_view()
                event.accept()
            elif event.key() == Qt.Key_Down:
                self.view_offset_y -= move_step
                self.update_view()
                event.accept()
            else:
                # 其他按键交给父类处理
                super().keyPressEvent(event)
        else:
            # 未放大状态下，交给父类处理
            super().keyPressEvent(event) 