from PyQt5.QtCore import Qt, QObject, QPoint
from PyQt5.QtWidgets import QLabel
from Tools.measurement_manager import MeasurementManager, DrawingType, DrawingObject
import time
import weakref
import numpy as np

class DrawingManager(QObject):
    def __init__(self, parent=None):
        try:
            super().__init__(parent)
            self.active_view = None
            self.active_measurement = None
            self.measurement_managers = {}  # 存储每个视图对应的测量管理器
            self.log_manager = parent.log_manager if parent else None
            # 添加视图映射关系
            self.view_pairs = {}  # 存储主界面视图和选项卡视图的对应关系
            self.view_pairs_reverse = {}  # 存储反向映射关系
            self._last_update_time = 0  # 用于节流
            self._update_interval = 1.0 / 60  # 降低绘画更新率到60fps，减少CPU负担
            self._frame_cache = {}  # 缓存渲染后的帧
            self._cache_timeout = 0.5  # 增加缓存超时时间到0.5秒，减少渲染频率
            self._render_buffer = {}  # 渲染缓冲区
            self._max_cached_frames = 10  # 最大缓存帧数
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
        # 创建测量管理器
        self.measurement_managers[label] = MeasurementManager(self.parent())

        # 设置是否为主界面视图
        self.measurement_managers[label].set_main_view(name in ['vertical', 'left', 'front'])
        
        # 设置视图对应关系
        if name == "vertical":
            paired_view = self.parent().lbVerticalView_2
            self.view_pairs[label] = paired_view
            self.view_pairs_reverse[paired_view] = label
        elif name == "left":
            paired_view = self.parent().lbLeftView_2
            self.view_pairs[label] = paired_view
            self.view_pairs_reverse[paired_view] = label
        elif name == "front":
            paired_view = self.parent().lbFrontView_2
            self.view_pairs[label] = paired_view
            self.view_pairs_reverse[paired_view] = label
        elif name == "vertical_2":
            paired_view = self.parent().lbVerticalView
            self.view_pairs[label] = paired_view
            self.view_pairs_reverse[paired_view] = label
        elif name == "left_2":
            paired_view = self.parent().lbLeftView
            self.view_pairs[label] = paired_view
            self.view_pairs_reverse[paired_view] = label
        elif name == "front_2":
            paired_view = self.parent().lbFrontView
            self.view_pairs[label] = paired_view
            self.view_pairs_reverse[paired_view] = label
        
        # 启用鼠标追踪
        label.setMouseTracking(True)
        
        # 设置鼠标事件处理
        label.mousePressEvent = lambda evt: self.handle_mouse_press(evt, label)
        label.mouseMoveEvent = lambda evt: self.handle_mouse_move(evt, label)
        label.mouseReleaseEvent = lambda evt: self.handle_mouse_release(evt, label)
        label.mouseDoubleClickEvent = lambda evt: self.parent().label_mouseDoubleClickEvent(evt, label)
        
        # 添加右键菜单事件
        label.setContextMenuPolicy(Qt.CustomContextMenu)
        label.customContextMenuRequested.connect(
            lambda pos: self.parent().show_context_menu(pos, label)
        )
        
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
                manager.layer_manager.clear_drawings()
                
                # 使当前视图的缓存无效
                self._invalidate_cache(label)
                
                # 同步清空对应视图
                paired_view = self.get_paired_view(label)
                if paired_view:
                    self.measurement_managers[paired_view].layer_manager.clear_drawings()
                    # 使配对视图的缓存无效
                    self._invalidate_cache(paired_view)
                
                # 重新渲染两个视图
                for view in [label, paired_view]:
                    if view:
                        current_frame = self.parent().get_current_frame_for_view(view)
                        if current_frame is not None:
                            display_frame = self._get_cached_frame(view, current_frame)
                            if display_frame is not None:
                                self.parent().update_view(view, display_frame)
        else:
            # 清空所有视图的手动绘画
            self._invalidate_cache()  # 清空所有缓存
            for label, manager in self.measurement_managers.items():
                manager.layer_manager.clear_drawings()
                # 重新渲染视图
                current_frame = self.parent().get_current_frame_for_view(label)
                if current_frame is not None:
                    display_frame = self._get_cached_frame(label, current_frame)
                    if display_frame is not None:
                        self.parent().update_view(label, display_frame)
                
    def handle_mouse_press(self, event, label):
        """处理鼠标按下事件"""
        if label not in self.measurement_managers:
            return
        
        # 更新活动视图和测量管理器
        self.active_view = label
        self.active_measurement = self.measurement_managers[label]
        
        # 同时更新父窗口的last_active_view，确保右键菜单能正确工作
        if hasattr(self.parent(), 'last_active_view'):
            self.parent().last_active_view = label
        
        # 获取当前帧
        current_frame = self.parent().get_current_frame_for_view(label)
        if current_frame is None:
            return
        
        # 转换鼠标坐标
        image_pos = self.convert_mouse_to_image_coords(event.pos(), label, current_frame)
        
        # 检查是否是右键点击
        if event.button() == Qt.RightButton:
            
            # 如果正在绘制，退出绘制状态
            if self.active_measurement.drawing or self.active_measurement.draw_mode:
                self.active_measurement.exit_drawing_mode()
                display_frame = self.active_measurement.layer_manager.render_frame(current_frame)
                if display_frame is not None:
                    self.parent().update_view(label, display_frame)
                    
                    # 如果是平行线测量，同步退出状态
                    if self.active_measurement.draw_mode == DrawingType.PARALLEL:
                        paired_view = self.get_paired_view(label)
                        if paired_view:
                            paired_manager = self.measurement_managers.get(paired_view)
                            if paired_manager:
                                paired_manager.exit_drawing_mode()
                                paired_frame = self.parent().get_current_frame_for_view(paired_view)
                                if paired_frame is not None:
                                    paired_display_frame = paired_manager.layer_manager.render_frame(paired_frame)
                                    if paired_display_frame is not None:
                                        self.parent().update_view(paired_view, paired_display_frame)
                return
        
        # 只处理左键点击的绘制操作
        if event.button() == Qt.LeftButton:
            # 对于点测量模式，在鼠标按下时不处理
            if self.active_measurement.draw_mode == DrawingType.POINT:
                return
                
            # 处理其他模式的鼠标事件
            display_frame = self.active_measurement.handle_mouse_press(image_pos, current_frame)
            if display_frame is not None:
                self.parent().update_view(label, display_frame)
                
                # 如果是平行线测量，同步到对应的视图
                if self.active_measurement.draw_mode == DrawingType.PARALLEL:
                    paired_view = self.get_paired_view(label)
                    if paired_view:
                        paired_manager = self.measurement_managers.get(paired_view)
                        if paired_manager:
                            # 同步绘制状态
                            paired_manager.draw_mode = self.active_measurement.draw_mode
                            paired_manager.drawing = self.active_measurement.drawing
                            paired_manager.layer_manager.drawing_objects = self.active_measurement.layer_manager.drawing_objects.copy()
                            # 更新对应视图
                            paired_frame = self.parent().get_current_frame_for_view(paired_view)
                            if paired_frame is not None:
                                paired_display_frame = paired_manager.layer_manager.render_frame(paired_frame)
                                if paired_display_frame is not None:
                                    self.parent().update_view(paired_view, paired_display_frame)
                    
    def handle_mouse_move(self, event, label):
        """处理鼠标移动事件"""
        # 只在按住左键时处理
        if not (event.buttons() & Qt.LeftButton):
            return
        
        if label not in self.measurement_managers:
            return
        
        # 使用节流控制更新频率
        current_time = time.time()
        if current_time - self._last_update_time < self._update_interval:
            return
        self._last_update_time = current_time
        
        # 处理鼠标移动
        current_frame = self.parent().get_current_frame_for_view(label)
        if current_frame is not None:
            image_pos = self.convert_mouse_to_image_coords(event.pos(), label, current_frame)
            manager = self.measurement_managers[label]
            
            # 使当前视图的缓存无效
            self._invalidate_cache(label)
            
            display_frame = manager.handle_mouse_move(image_pos, current_frame)
            if display_frame is not None:
                self.parent().update_view(label, display_frame)
                
                # 如果是平行线测量，同步到对应的视图
                if manager.draw_mode == DrawingType.PARALLEL:
                    paired_view = self.get_paired_view(label)
                    if paired_view:
                        paired_manager = self.measurement_managers.get(paired_view)
                        if paired_manager:
                            # 同步绘制状态
                            paired_manager.draw_mode = manager.draw_mode
                            paired_manager.drawing = manager.drawing
                            paired_manager.layer_manager.drawing_objects = manager.layer_manager.drawing_objects.copy()
                            
                            # 使配对视图的缓存无效
                            self._invalidate_cache(paired_view)
                            
                            # 更新对应视图
                            paired_frame = self.parent().get_current_frame_for_view(paired_view)
                            if paired_frame is not None:
                                paired_display_frame = paired_manager.layer_manager.render_frame(paired_frame)
                                if paired_display_frame is not None:
                                    self.parent().update_view(paired_view, paired_display_frame)
                    
    def handle_mouse_release(self, event, label):
        """处理鼠标释放事件"""
        # 只处理左键释放
        if event.button() != Qt.LeftButton:
            return
        
        if event.button() == Qt.LeftButton and label == self.active_view and self.active_measurement:
            current_frame = self.parent().get_current_frame_for_view(label)
            if current_frame is not None:
                image_pos = self.convert_mouse_to_image_coords(event.pos(), label, current_frame)
                
                # 对于点测量模式，在鼠标释放时处理
                if self.active_measurement.draw_mode == DrawingType.POINT:
                    display_frame = self.active_measurement.handle_mouse_press(image_pos, current_frame)
                else:
                    display_frame = self.active_measurement.handle_mouse_release(image_pos, current_frame)
                    
                if display_frame is not None:
                    self.parent().update_view(label, display_frame)
                    # 记录最后操作的视图
                    self.parent().last_active_view = label
                    
                    # 同步到对应的视图
                    paired_view = self.get_paired_view(label)
                    if paired_view:
                        self.sync_drawings(label, paired_view)
                        
                        # 如果是平行线测量，确保同步状态
                        if self.active_measurement.draw_mode == DrawingType.PARALLEL:
                            paired_manager = self.measurement_managers.get(paired_view)
                            if paired_manager:
                                paired_manager.draw_mode = self.active_measurement.draw_mode
                                paired_manager.drawing = self.active_measurement.drawing
            
            self.active_view = None
            self.active_measurement = None
            
    def convert_mouse_to_image_coords(self, pos, label, current_frame):
        """将鼠标坐标转换为图像坐标"""
        img_height, img_width = current_frame.shape[:2]
        label_width = label.width()
        label_height = label.height()
        
        # 获取缩放因子和偏移量
        zoom_factor = 1.0
        view_offset_x = 0
        view_offset_y = 0
        
        # 查找父容器是否为GridContainer
        from Tools.grid_container import GridContainer
        parent = label.parent()
        while parent:
            if isinstance(parent, GridContainer):
                zoom_factor = parent.get_zoom_factor()
                # 获取视图偏移量
                if zoom_factor > 1.0:
                    view_offset_x = parent.view_offset_x
                    view_offset_y = parent.view_offset_y
                break
            parent = parent.parent()
        
        # 计算缩放比例
        base_ratio = min(label_width / img_width, label_height / img_height)
        ratio = base_ratio * zoom_factor
        display_width = int(img_width * ratio)
        display_height = int(img_height * ratio)
        
        # 计算中心偏移
        x_offset = (label_width - display_width) // 2
        y_offset = (label_height - display_height) // 2
        
        # 考虑缩放偏移量进行坐标转换
        if zoom_factor > 1.0:
            # 调整鼠标位置，考虑视图偏移
            adjusted_x = pos.x() - view_offset_x
            adjusted_y = pos.y() - view_offset_y
            
            # 转换调整后的坐标到图像坐标
            image_x = int(adjusted_x * img_width / display_width)
            image_y = int(adjusted_y * img_height / display_height)
        else:
            # 未缩放时的标准转换
            image_x = int((pos.x() - x_offset) * img_width / display_width)
            image_y = int((pos.y() - y_offset) * img_height / display_height)
        
        # 确保坐标在图像范围内
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
            
            # 同步到对应的视图
            paired_view = self.get_paired_view(label)
            if paired_view and paired_view in self.measurement_managers:
                paired_manager = self.measurement_managers[paired_view]
                paired_manager.start_parallel_measurement()
                paired_view.setCursor(Qt.CrossCursor)
        
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

    def start_point_measurement(self):
        """启动点测量模式"""
        print("启动点测量")
        for label, manager in self.measurement_managers.items():
            manager.start_point_measurement()
            label.setCursor(Qt.CrossCursor)

    def start_circle_detection(self):
        """启动圆形检测模式"""
        print("启动圆形检测")
        for label, manager in self.measurement_managers.items():
            manager.start_circle_detection()
            label.setCursor(Qt.CrossCursor)

    def start_calibration(self):
        """启动标定模式"""
        print("启动标定")
        
        # 获取当前活动视图
        active_view = None
        
        # 如果self.active_view已设置，则使用它
        if hasattr(self, 'active_view') and self.active_view:
            active_view = self.active_view
        # 否则，尝试从父窗口获取当前活动视图
        elif hasattr(self.parent(), 'last_active_view') and self.parent().last_active_view:
            active_view = self.parent().last_active_view
        # 如果仍然没有找到活动视图，使用第一个可用的视图
        elif self.measurement_managers:
            active_view = list(self.measurement_managers.keys())[0]
            
        # 如果找到了活动视图，启动标定模式
        if active_view:
            manager = self.measurement_managers.get(active_view)
            if manager:
                # 设置为当前活动视图
                self.active_view = active_view
                
                # 启动标定模式
                manager.start_calibration()
                active_view.setCursor(Qt.CrossCursor)
                
                # 记录最后操作的视图
                if hasattr(self.parent(), 'last_active_view'):
                    self.parent().last_active_view = active_view
        else:
            print("无法找到活动视图，标定模式启动失败")

    def handle_context_menu(self, label):
        """处理右键菜单"""
        manager = self.measurement_managers.get(label)
        if not manager:
            return []

        selected = manager.layer_manager.selected_objects
        if len(selected) != 2:
            return []

        # 获取选中图元的类型
        types = [obj.type for obj in selected]
        
        menu_items = []
        
        # 如果选中了两个点
        if len(types) == 2 and all(t == DrawingType.POINT for t in types):
            menu_items.append(("点与点", lambda: self._create_point_to_point(label, selected[0], selected[1])))
            
        # 区分直线和线段
        is_line = lambda t: t == DrawingType.LINE  # 只匹配直线
        is_line_segment = lambda t: t == DrawingType.LINE_SEGMENT  # 只匹配线段
        is_circle = lambda t: t in [DrawingType.CIRCLE, DrawingType.SIMPLE_CIRCLE, DrawingType.FINE_CIRCLE]
        
        # 检查是否有中线对象
        has_midline = False
        midline_obj = None
        for obj in selected:
            if obj.properties.get('is_midline', False):
                has_midline = True
                midline_obj = obj
                break
            
        # 如果选中了一个点和一条直线
        if len(types) == 2 and ((types[0] == DrawingType.POINT and is_line(types[1])) or
                               (types[1] == DrawingType.POINT and is_line(types[0]))):
            point = selected[0] if types[0] == DrawingType.POINT else selected[1]
            line = selected[1] if types[0] == DrawingType.POINT else selected[0]
            menu_items.append(("点与线", lambda: self._create_point_to_line(label, point, line)))
            
        # 如果选中了一个点和一条线段
        if len(types) == 2 and ((types[0] == DrawingType.POINT and is_line_segment(types[1])) or
                               (types[1] == DrawingType.POINT and is_line_segment(types[0]))):
            point = selected[0] if types[0] == DrawingType.POINT else selected[1]
            line = selected[1] if types[0] == DrawingType.POINT else selected[0]
            menu_items.append(("点与线", lambda: self._create_point_to_line(label, point, line)))
            
        # 如果选中了一个点和一个圆
        if len(types) == 2 and ((types[0] == DrawingType.POINT and is_circle(types[1])) or
                               (types[1] == DrawingType.POINT and is_circle(types[0]))):
            point = selected[0] if types[0] == DrawingType.POINT else selected[1]
            circle = selected[1] if types[0] == DrawingType.POINT else selected[0]
            menu_items.append(("点与圆", lambda: self._create_point_to_circle(label, point, circle)))
            
        # 如果选中了一条线段和一个圆
        if len(types) == 2 and ((is_line_segment(types[0]) and is_circle(types[1])) or
                               (is_circle(types[0]) and is_line_segment(types[1]))):
            line = selected[0] if is_line_segment(types[0]) else selected[1]
            circle = selected[1] if is_line_segment(types[0]) else selected[0]
            menu_items.append(("线段与圆", lambda: self._create_line_segment_to_circle(label, line, circle)))
            
        # 如果选中了一条直线和一个圆
        if len(types) == 2 and ((is_line(types[0]) and is_circle(types[1])) or
                               (is_circle(types[0]) and is_line(types[1]))):
            line = selected[0] if is_line(types[0]) else selected[1]
            circle = selected[1] if is_line(types[0]) else selected[0]
            menu_items.append(("直线与圆", lambda: self._create_line_to_circle(label, line, circle)))
            
        # 如果选中了一条中线和一个圆
        if has_midline and len(types) == 2:
            # 找出另一个对象
            other_obj = selected[0] if selected[1] == midline_obj else selected[1]
            if is_circle(other_obj.type):
                menu_items.append(("中线与圆", lambda: self._create_line_to_circle(label, midline_obj, other_obj)))
            
        return menu_items

    def delete_selected_objects(self, label):
        """删除选中的对象"""
        if label in self.measurement_managers:
            manager = self.measurement_managers[label]
            # 删除选中的对象
            manager.layer_manager.delete_selected_objects()
            
            # 使当前视图的缓存无效
            self._invalidate_cache(label)
            
            # 更新当前视图
            current_frame = self.parent().get_current_frame_for_view(label)
            if current_frame is not None:
                display_frame = self._get_cached_frame(label, current_frame)
                if display_frame is not None:
                    self.parent().update_view(label, display_frame)
            
            # 同步到对应的视图
            paired_view = self.get_paired_view(label)
            if paired_view:
                # 同步删除操作
                paired_manager = self.measurement_managers.get(paired_view)
                if paired_manager:
                    # 同步绘制对象
                    paired_manager.layer_manager.drawing_objects = manager.layer_manager.drawing_objects.copy()
                    # 清除选择状态
                    paired_manager.layer_manager.clear_selection()
                    
                    # 使配对视图的缓存无效
                    self._invalidate_cache(paired_view)
                    
                    # 更新配对视图
                    paired_frame = self.parent().get_current_frame_for_view(paired_view)
                    if paired_frame is not None:
                        paired_display_frame = self._get_cached_frame(paired_view, paired_frame)
                        if paired_display_frame is not None:
                            self.parent().update_view(paired_view, paired_display_frame)
            
            # 记录操作日志
            if self.log_manager:
                self.log_manager.log_drawing_operation(
                    "删除绘制对象",
                    f"视图: {label.objectName()}"
                )

    def undo_last_drawing(self):
        """撤销上一步手动绘制"""
        active_view = self.parent().last_active_view
        if active_view and active_view in self.measurement_managers:
            if self.log_manager:
                self.log_manager.log_drawing_operation("撤销绘制", f"视图: {active_view.objectName()}")
            manager = self.measurement_managers[active_view]
            if manager.layer_manager.undo_last_drawing():
                # 更新当前视图
                current_frame = self.parent().get_current_frame_for_view(active_view)
                if current_frame is not None:
                    display_frame = manager.layer_manager.render_frame(current_frame)
                    if display_frame is not None:
                        self.parent().update_view(active_view, display_frame)
                        
                        # 同步到对应的视图
                        paired_view = self.get_paired_view(active_view)
                        if paired_view:
                            self.sync_drawings(active_view, paired_view)
            
    def undo_last_detection(self):
        """撤销上一步自动检测"""
        active_view = self.parent().last_active_view
        if active_view and active_view in self.measurement_managers:
            if self.log_manager:
                self.log_manager.log_drawing_operation("撤销检测", f"视图: {active_view.objectName()}")
            manager = self.measurement_managers[active_view]
            if manager.layer_manager.undo_last_detection():
                # 更新当前视图
                current_frame = self.parent().get_current_frame_for_view(active_view)
                if current_frame is not None:
                    display_frame = manager.layer_manager.render_frame(current_frame)
                    if display_frame is not None:
                        self.parent().update_view(active_view, display_frame)
                        
                        # 同步到对应的视图
                        paired_view = self.get_paired_view(active_view)
                        if paired_view:
                            self.sync_drawings(active_view, paired_view)

    def sync_drawings(self, source_label, target_label):
        """同步两个视图之间的绘制内容"""
        if source_label in self.measurement_managers and target_label in self.measurement_managers:
            source_manager = self.measurement_managers[source_label]
            target_manager = self.measurement_managers[target_label]
            
            # 同步绘制对象
            target_manager.layer_manager.drawing_objects = source_manager.layer_manager.drawing_objects.copy()
            
            # 更新目标视图
            current_frame = self.parent().get_current_frame_for_view(target_label)
            if current_frame is not None:
                display_frame = target_manager.layer_manager.render_frame(current_frame.copy())
                if display_frame is not None:
                    self.parent().update_view(target_label, display_frame)
                    
    def get_main_view(self, tab_view):
        """获取选项卡视图对应的主界面视图"""
        for main_view, paired_view in self.view_pairs.items():
            if paired_view == tab_view:
                return main_view
        return None

    def _create_point_to_point(self, label, point1, point2):
        """创建点到点的测量"""
        if label in self.measurement_managers:
            manager = self.measurement_managers[label]
            # 创建新的线段对象
            properties = {
                'color': (0, 255, 0),  # RGB格式：绿色
                'thickness': 2
            }
            new_obj = DrawingObject(
                type=DrawingType.LINE_SEGMENT,
                points=[point1.points[0], point2.points[0]],  # 使用两个点的坐标
                properties=properties
            )
            # 添加到绘制列表
            manager.layer_manager.drawing_objects.append(new_obj)
            # 清除选择
            manager.layer_manager.clear_selection()
            # 更新视图
            current_frame = self.parent().get_current_frame_for_view(label)
            if current_frame is not None:
                display_frame = manager.layer_manager.render_frame(current_frame)
                if display_frame is not None:
                    self.parent().update_view(label, display_frame)
                    
                    # 同步到对应的视图
                    paired_view = self.get_paired_view(label)
                    if paired_view:
                        self.sync_drawings(label, paired_view)

    def _create_point_to_line(self, label, point, line):
        """创建点到线的测量"""
        if label in self.measurement_managers:
            manager = self.measurement_managers[label]
            # 创建新的点到线测量对象
            properties = {
                'color': (0, 255, 0),  # RGB格式：绿色
                'thickness': 2,
                'is_dashed': True,  # 使用虚线
                'show_distance': True,  # 显示距离
                'show_text': True  # 显示文本
            }
            # 使用线段的两个点和选中的点
            line_p1, line_p2 = line.points[0], line.points[1]
            point_p = point.points[0]
            
            new_obj = DrawingObject(
                type=DrawingType.POINT_TO_LINE,
                points=[point_p, line_p1, line_p2],  # 点和线的坐标
                properties=properties
            )
            # 添加到绘制列表
            manager.layer_manager.drawing_objects.append(new_obj)
            # 清除选择
            manager.layer_manager.clear_selection()
            # 更新视图
            current_frame = self.parent().get_current_frame_for_view(label)
            if current_frame is not None:
                display_frame = manager.layer_manager.render_frame(current_frame)
                if display_frame is not None:
                    self.parent().update_view(label, display_frame)
                    
                    # 同步到对应的视图
                    paired_view = self.get_paired_view(label)
                    if paired_view:
                        self.sync_drawings(label, paired_view)

    def _create_point_to_circle(self, label, point, circle):
        """创建点到圆的测量"""
        if label in self.measurement_managers:
            manager = self.measurement_managers[label]
            # 创建新的点到圆测量对象
            properties = {
                'color': (0, 255, 0),  # RGB格式：绿色
                'thickness': 2,
                'is_dashed': True,  # 使用虚线
                'show_distance': True,  # 显示距离
                'radius': 5  # 点的半径
            }
            
            # 获取点和圆的坐标
            point_p = point.points[0]
            circle_center = circle.points[0]
            # 对于圆，我们需要一个圆上的点来计算半径
            circle_radius_point = circle.points[1] if len(circle.points) > 1 else None
            
            if circle_radius_point is None:
                # 如果没有圆上的点，可能是使用了其他方式定义的圆
                # 尝试从属性中获取半径和中心点
                if 'radius' in circle.properties and 'center' in circle.properties:
                    # 创建一个圆上的点
                    radius = circle.properties['radius']
                    circle_radius_point = QPoint(circle_center.x() + radius, circle_center.y())
                else:
                    # 无法确定圆的信息，返回
                    return
            
            new_obj = DrawingObject(
                type=DrawingType.POINT_TO_CIRCLE,
                points=[point_p, circle_center, circle_radius_point],  # 点和圆的坐标
                properties=properties
            )
            
            # 添加到绘制列表
            manager.layer_manager.drawing_objects.append(new_obj)
            # 清除选择
            manager.layer_manager.clear_selection()
            # 更新视图
            current_frame = self.parent().get_current_frame_for_view(label)
            if current_frame is not None:
                # 使缓存无效
                self._invalidate_cache(label)
                # 获取新的渲染帧
                display_frame = self._get_cached_frame(label, current_frame)
                if display_frame is not None:
                    self.parent().update_view(label, display_frame)
                    
                    # 同步到对应的视图
                    paired_view = self.get_paired_view(label)
                    if paired_view:
                        self.sync_drawings(label, paired_view)

    def _create_line_segment_to_circle(self, label, line, circle):
        """创建线段到圆的测量"""
        if label in self.measurement_managers:
            manager = self.measurement_managers[label]
            # 创建新的线段到圆测量对象
            properties = {
                'color': (0, 255, 0),  # RGB格式：绿色
                'thickness': 2,
                'is_dashed': True,  # 使用虚线
                'show_distance': True,  # 显示距离
                'radius': 5  # 点的半径
            }
            
            # 获取线段和圆的坐标
            line_p1, line_p2 = line.points[0], line.points[1]
            circle_center = circle.points[0]
            # 对于圆，我们需要一个圆上的点来计算半径
            circle_radius_point = circle.points[1] if len(circle.points) > 1 else None
            
            if circle_radius_point is None:
                # 如果没有圆上的点，可能是使用了其他方式定义的圆
                # 尝试从属性中获取半径和中心点
                if 'radius' in circle.properties and 'center' in circle.properties:
                    # 创建一个圆上的点
                    radius = circle.properties['radius']
                    circle_radius_point = QPoint(circle_center.x() + radius, circle_center.y())
                else:
                    # 无法确定圆的信息，返回
                    return
            
            new_obj = DrawingObject(
                type=DrawingType.LINE_SEGMENT_TO_CIRCLE,
                points=[line_p1, line_p2, circle_center, circle_radius_point],  # 线段和圆的坐标
                properties=properties
            )
            
            # 添加到绘制列表
            manager.layer_manager.drawing_objects.append(new_obj)
            # 清除选择
            manager.layer_manager.clear_selection()
            # 更新视图
            current_frame = self.parent().get_current_frame_for_view(label)
            if current_frame is not None:
                # 使缓存无效
                self._invalidate_cache(label)
                # 获取新的渲染帧
                display_frame = self._get_cached_frame(label, current_frame)
                if display_frame is not None:
                    self.parent().update_view(label, display_frame)
                    
                    # 同步到对应的视图
                    paired_view = self.get_paired_view(label)
                    if paired_view:
                        self.sync_drawings(label, paired_view)

    def _get_cached_frame(self, label, current_frame):
        """获取缓存的帧，如果缓存无效则重新渲染"""
        current_time = time.time()
        cache_key = id(label)
        
        # 检查缓存大小，如果超过限制则清理最旧的
        if len(self._frame_cache) > self._max_cached_frames:
            oldest_key = min(self._frame_cache.keys(), 
                           key=lambda k: self._frame_cache[k]['timestamp'])
            self._frame_cache.pop(oldest_key)
        
        # 检查是否有有效缓存
        if cache_key in self._frame_cache:
            cache_entry = self._frame_cache[cache_key]
            # 增加缓存超时时间，减少重新渲染的频率
            if current_time - cache_entry['timestamp'] < self._cache_timeout:
                return cache_entry['frame']
        
        # 如果没有缓存或缓存已过期，重新渲染
        manager = self.measurement_managers.get(label)
        if manager:
            # 使用渲染缓冲区
            if cache_key not in self._render_buffer:
                self._render_buffer[cache_key] = np.empty_like(current_frame)
            
            # 先复制当前帧到渲染缓冲区
            np.copyto(self._render_buffer[cache_key], current_frame)
            # 在缓冲区上进行渲染
            display_frame = manager.layer_manager.render_frame(self._render_buffer[cache_key])
            
            if display_frame is not None:
                # 更新缓存
                self._frame_cache[cache_key] = {
                    'frame': display_frame,
                    'timestamp': current_time
                }
                
                return display_frame
        return None

    def _invalidate_cache(self, label=None):
        """使缓存无效"""
        if label:
            cache_key = id(label)
            self._frame_cache.pop(cache_key, None)
        else:
            self._frame_cache.clear()

    def get_paired_view(self, label):
        """获取配对的视图"""
        # 先尝试从正向映射获取
        paired_view = self.view_pairs.get(label)
        if paired_view is not None:
            return paired_view
        # 如果没有找到，尝试从反向映射获取
        return self.view_pairs_reverse.get(label)

    def _cleanup_resources(self):
        """清理资源"""
        self._frame_cache.clear()
        self._render_buffer.clear()
        
    def __del__(self):
        """析构函数"""
        self._cleanup_resources()

    def start_simple_circle_measurement(self):
        """启动简单圆测量模式"""
        print("启动简单圆测量")
        try:
            # 获取当前活动视图
            active_view = self.parent().active_view
            if active_view and active_view in self.measurement_managers:
                # 设置当前活动视图的测量模式
                manager = self.measurement_managers[active_view]
                manager.start_simple_circle_measurement()
                active_view.setCursor(Qt.CrossCursor)
                
                # 同步设置对应视图的测量模式
                paired_view = self.get_paired_view(active_view)
                if paired_view and paired_view in self.measurement_managers:
                    paired_manager = self.measurement_managers[paired_view]
                    paired_manager.start_simple_circle_measurement()
                    paired_view.setCursor(Qt.CrossCursor)
            else:
                # 如果没有活动视图，设置所有视图的测量模式
                for label, manager in self.measurement_managers.items():
                    manager.start_simple_circle_measurement()
                    label.setCursor(Qt.CrossCursor)
        except Exception as e:
            if self.log_manager:
                self.log_manager.log_error("启动简单圆测量模式失败", str(e))
                
    def start_fine_circle_measurement(self):
        """启动精细圆测量模式"""
        try:
            print("启动精细圆测量")  # 添加调试信息
            # 获取当前活动视图
            active_view = self.parent().active_view
            if active_view and active_view in self.measurement_managers:
                # 设置当前活动视图的测量模式
                manager = self.measurement_managers[active_view]
                manager.start_fine_circle_measurement()
                active_view.setCursor(Qt.CrossCursor)
                
                # 同步设置对应视图的测量模式
                paired_view = self.get_paired_view(active_view)
                if paired_view and paired_view in self.measurement_managers:
                    paired_manager = self.measurement_managers[paired_view]
                    paired_manager.start_fine_circle_measurement()
                    paired_view.setCursor(Qt.CrossCursor)
            else:
                # 如果没有活动视图，设置所有视图的测量模式
                for label, manager in self.measurement_managers.items():
                    manager.start_fine_circle_measurement()
                    label.setCursor(Qt.CrossCursor)
        except Exception as e:
            if self.log_manager:
                self.log_manager.log_error("启动精细圆测量模式失败", str(e))

    def _create_line_to_circle(self, label, line, circle):
        """创建直线到圆的测量"""
        if label in self.measurement_managers:
            manager = self.measurement_managers[label]
            # 创建新的直线到圆测量对象
            properties = {
                'color': (0, 255, 0),  # RGB格式：绿色
                'thickness': 2,
                'is_dashed': True,  # 使用虚线
                'show_distance': True,  # 显示距离
                'radius': 5  # 点的半径
            }
            
            # 获取直线和圆的坐标
            line_p1, line_p2 = line.points[0], line.points[1]
            circle_center = circle.points[0]
            # 对于圆，我们需要一个圆上的点来计算半径
            circle_radius_point = circle.points[1] if len(circle.points) > 1 else None
            
            if circle_radius_point is None:
                # 如果没有圆上的点，可能是使用了其他方式定义的圆
                # 尝试从属性中获取半径和中心点
                if 'radius' in circle.properties and 'center' in circle.properties:
                    # 创建一个圆上的点
                    radius = circle.properties['radius']
                    circle_radius_point = QPoint(circle_center.x() + radius, circle_center.y())
                else:
                    # 无法确定圆的信息，返回
                    return
            
            new_obj = DrawingObject(
                type=DrawingType.LINE_TO_CIRCLE,
                points=[line_p1, line_p2, circle_center, circle_radius_point],  # 直线和圆的坐标
                properties=properties
            )
            
            # 添加到绘制列表
            manager.layer_manager.drawing_objects.append(new_obj)
            # 清除选择
            manager.layer_manager.clear_selection()
            # 更新视图
            current_frame = self.parent().get_current_frame_for_view(label)
            if current_frame is not None:
                # 使缓存无效
                self._invalidate_cache(label)
                # 获取新的渲染帧
                display_frame = self._get_cached_frame(label, current_frame)
                if display_frame is not None:
                    self.parent().update_view(label, display_frame)
                    
                    # 同步到对应的视图
                    paired_view = self.get_paired_view(label)
                    if paired_view:
                        self.sync_drawings(label, paired_view)
