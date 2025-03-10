import warnings
warnings.filterwarnings("ignore", category=DeprecationWarning)

from PyQt5.QtCore import QObject, QPoint, Qt
from PyQt5.QtWidgets import QApplication  # 从QtWidgets导入QApplication
import cv2
import numpy as np
from dataclasses import dataclass
from enum import Enum
from typing import List, Optional

class DrawingType(Enum):
    LINE = "直线"
    CIRCLE = "圆形"
    LINE_SEGMENT = "线段"
    PARALLEL = "平行线"
    CIRCLE_LINE = "圆线测量"
    TWO_LINES = "两线夹角"
    LINE_DETECT = "直线检测"
    CIRCLE_DETECT = "圆检测"
    POINT = "点"
    POINT_TO_POINT = "点与点"  # 添加点与点类型
    POINT_TO_LINE = "点与线"   # 添加点与线类型
    POINT_TO_CIRCLE = "点与圆"  # 添加点与圆类型
    LINE_SEGMENT_TO_CIRCLE = "线段与圆"  # 添加线段与圆类型
    LINE_TO_CIRCLE = "直线与圆"  # 添加直线与圆类型
    SIMPLE_CIRCLE = "简单圆"  # 添加新类型
    FINE_CIRCLE = "精细圆"  # 添加新类型
    LINE_SEGMENT_ANGLE = "线段角度"  # 添加线段角度测量类型

@dataclass
class DrawingObject:
    type: DrawingType
    points: List[QPoint]
    properties: dict  # 存储颜色、线宽等属性
    visible: bool = True
    selected: bool = False  # 添加选中状态标记

class LayerManager:
    def __init__(self):
        self.drawing_objects: List[DrawingObject] = []  # 手动绘制对象
        self.detection_objects: List[DrawingObject] = []  # 自动检测对象
        self.current_object: Optional[DrawingObject] = None
        self.base_frame = None
        self.selected_objects = []  # 存储选中的图元
        self.pixel_scale = 1.0  # 默认像素比例（1像素 = 1微米）
        
    def set_pixel_scale(self, scale):
        """设置像素比例"""
        self.pixel_scale = scale
        
    def get_pixel_scale(self):
        """获取像素比例"""
        return self.pixel_scale
    
    def clear(self):
        """清空所有绘制对象"""
        self.drawing_objects = []
        self.detection_objects = []
        self.current_object = None

    def clear_drawings(self):
        """清除所有手动绘制"""
        # 保留检测对象
        self.drawing_objects = [obj for obj in self.drawing_objects if obj.properties.get('is_detection', False)]
        self.current_object = None
        
    def clear_detections(self):
        """清空自动检测"""
        self.detection_objects = []
        
    def commit_drawing(self):
        """提交当前绘制"""
        if self.current_object:
            # 通过properties中的is_detection标记判断是否为检测结果
            if self.current_object.properties.get('is_detection', False):
                # 将检测对象添加到检测列表
                print(f"将检测对象添加到检测列表: {self.current_object.type}")
                self.detection_objects.append(self.current_object)
            else:
                # 非检测对象直接添加到绘制列表
                self.drawing_objects.append(self.current_object)
            self.current_object = None
            
    def undo_last_drawing(self):
        """撤销最后一次手动绘制"""
        if self.drawing_objects:
            # 从后向前遍历,找到第一个非检测对象
            for i in range(len(self.drawing_objects) - 1, -1, -1):
                obj = self.drawing_objects[i]
                if not obj.properties.get('is_detection', False):
                    self.drawing_objects.pop(i)
                    return True
        return False
        
    def undo_last_detection(self):
        """撤销最后一次检测结果"""
        if self.drawing_objects:
            # 从后向前遍历,找到第一个检测对象
            for i in range(len(self.drawing_objects) - 1, -1, -1):
                obj = self.drawing_objects[i]
                if obj.properties.get('is_detection', False):
                    self.drawing_objects.pop(i)
                    return True
        return False
        
    def render_object(self, frame, obj: DrawingObject):
        """渲染单个绘制对象"""
        if obj is None or not obj.visible:
            return
            
        # 根据对象类型调用相应的绘制方法
        draw_methods = {
            DrawingType.CIRCLE: self._draw_circle,  # 圆，简单圆、精细圆的基础
            DrawingType.LINE_SEGMENT: self._draw_line_segment,  # 线段，右键点到点
            DrawingType.POINT: self._draw_point,  # 点
            DrawingType.LINE: self._draw_line,  # 直线
            DrawingType.SIMPLE_CIRCLE: self._draw_simple_circle,  # 简单圆
            DrawingType.FINE_CIRCLE: self._draw_fine_circle,  # 精细圆
            DrawingType.PARALLEL: self._draw_parallel,  # 平行线
            DrawingType.CIRCLE_LINE: self._draw_circle_line,  # 圆线
            DrawingType.TWO_LINES: self._draw_two_lines,  # 两线
            DrawingType.LINE_DETECT: self._draw_line_detect,  # 直线检测
            DrawingType.CIRCLE_DETECT: self._draw_circle_detect,  # 圆检测
            DrawingType.POINT_TO_LINE: self._draw_point_to_line,  # 右键点到线，生成垂直虚线
            DrawingType.LINE_SEGMENT_TO_CIRCLE: self._draw_line_segment_to_circle,  # 右键线段到圆，生成垂直虚线
            DrawingType.POINT_TO_CIRCLE: self._draw_point_to_circle,  # 右键点到圆，生成垂直虚线
            DrawingType.LINE_TO_CIRCLE: self._draw_line_to_circle,  # 右键直线到圆，生成垂直虚线
            DrawingType.LINE_SEGMENT_ANGLE: self._draw_line_segment_angle,  # 线段角度测量
        }
        
        try:
            draw_method = draw_methods.get(obj.type)
            if draw_method:
                draw_method(frame, obj)
        except Exception as e:
            print(f"渲染对象时出错: {str(e)}")

    def render_frame(self, frame):
        """渲染帧"""
        if frame is None:
            return None
            
        # 检查图像是否为灰度图
        is_grayscale = len(frame.shape) == 2
        
        # 如果是灰度图，转换为RGB以便绘制彩色图形
        if is_grayscale:
            # 创建RGB副本以便绘制彩色图形
            display_frame = cv2.cvtColor(frame, cv2.COLOR_GRAY2RGB)
        else:
            # 创建副本以避免修改原始帧
            display_frame = frame.copy()
        
        
        # 渲染所有已完成的绘制对象
        for obj in self.drawing_objects:
            self.render_object(display_frame, obj)
            
        # 渲染所有检测对象
        for obj in self.detection_objects:
            self.render_object(display_frame, obj)
            
        # 渲染当前正在绘制的对象
        if self.current_object:
            self.render_object(display_frame, self.current_object)
            
        return display_frame

    def start_drawing(self, drawing_type: DrawingType, properties: dict):
        """开始新的绘制"""
        self.current_object = DrawingObject(
            type=drawing_type,
            points=[],
            properties=properties
        )
    
    def update_current_object(self, point: QPoint):
        """更新当前绘制对象"""
        if self.current_object:
            if self.current_object.type == DrawingType.TWO_LINES:
                # 两线夹角测量模式处理
                if len(self.current_object.points) < 2:
                    # 第一阶段：绘制第一条线
                    if len(self.current_object.points) == 0:
                        self.current_object.points.append(point)
                    else:
                        self.current_object.points[1] = point
                elif len(self.current_object.points) >= 2 and len(self.current_object.points) < 4:
                    # 第二阶段：绘制第二条线
                    if len(self.current_object.points) == 2:
                        self.current_object.points.append(point)
                    elif len(self.current_object.points) == 3:
                        self.current_object.points[3] = point
            elif self.current_object.type == DrawingType.LINE_SEGMENT_ANGLE:
                # 线段角度测量模式处理
                if len(self.current_object.points) < 2:
                    # 第一阶段：绘制第一条线段
                    if len(self.current_object.points) == 0:
                        self.current_object.points.append(point)
                    else:
                        self.current_object.points[1] = point
                elif len(self.current_object.points) >= 2 and len(self.current_object.points) < 4:
                    # 第二阶段：绘制第二条线段
                    if len(self.current_object.points) == 2:
                        self.current_object.points.append(point)
                    elif len(self.current_object.points) == 3:
                        self.current_object.points[3] = point
            elif self.current_object.type == DrawingType.CIRCLE_LINE:
                # 圆线距离测量模式处理
                if len(self.current_object.points) < 2:
                    # 第一阶段：绘制圆
                    if len(self.current_object.points) == 0:
                        self.current_object.points.append(point)
                    else:
                        self.current_object.points[1] = point
                elif len(self.current_object.points) == 2:
                    # 第二阶段：开始绘制直线
                    self.current_object.points.append(point)
                elif len(self.current_object.points) == 3:
                    # 第二阶段：更新直线终点
                    self.current_object.points.append(point)
                elif len(self.current_object.points) == 4:
                    # 更新直线终点位置
                    self.current_object.points[3] = point
            elif self.current_object.type == DrawingType.PARALLEL:
                # 保持平行线模式处理不变
                if len(self.current_object.points) < 2:
                    if len(self.current_object.points) == 0:
                        self.current_object.points.append(point)
                    elif len(self.current_object.points) == 1:
                        self.current_object.points.append(point)
                    else:
                        self.current_object.points[1] = point
                else:
                    if len(self.current_object.points) == 2:
                        self.current_object.points.append(point)
                    elif len(self.current_object.points) > 2:
                        self.current_object.points[2] = point
            elif self.current_object.type == DrawingType.LINE_DETECT:
                # 直线检测模式处理
                if len(self.current_object.points) < 2:
                    if len(self.current_object.points) == 0:
                        self.current_object.points.append(point)
                    else:
                        self.current_object.points[1] = point
            elif self.current_object.type == DrawingType.CIRCLE_DETECT:
                # 圆形检测模式处理
                if len(self.current_object.points) < 2:
                    if len(self.current_object.points) == 0:
                        self.current_object.points.append(point)
                    else:
                        self.current_object.points[1] = point
            elif self.current_object.type == DrawingType.SIMPLE_CIRCLE:
                # 简单圆模式处理
                pass
            elif self.current_object.type == DrawingType.FINE_CIRCLE:
                # 精细圆模式处理
                pass
            else:
                # 其他模式的原有处理逻辑保持不变
                if len(self.current_object.points) == 0:
                    self.current_object.points.append(point)
                elif len(self.current_object.points) == 1:
                    self.current_object.points.append(point)
                else:
                    self.current_object.points[1] = point

    def _calculate_circle_from_three_points(self, points):
        """根据三个点计算圆心和半径"""
        try:
            # 提取三个点的坐标
            x1, y1 = points[0].x(), points[0].y()
            x2, y2 = points[1].x(), points[1].y()
            x3, y3 = points[2].x(), points[2].y()
            
            # 计算三点确定的圆的参数
            # 使用行列式计算
            D = 2 * (x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2))
            if abs(D) < 1e-10:  # 三点共线
                return None
                
            # 计算圆心
            center_x = ((x1*x1 + y1*y1) * (y2 - y3) + (x2*x2 + y2*y2) * (y3 - y1) + (x3*x3 + y3*y3) * (y1 - y2)) / D
            center_y = ((x1*x1 + y1*y1) * (x3 - x2) + (x2*x2 + y2*y2) * (x1 - x3) + (x3*x3 + y3*y3) * (x2 - x1)) / D
            
            # 计算半径
            radius = np.sqrt((center_x - x1)**2 + (center_y - y1)**2)
            
            return (center_x, center_y), radius
        except Exception as e:
            print(f"计算圆参数时出错: {str(e)}")
            return None

    def _draw_line(self, frame, obj: DrawingObject):
        """绘制直线"""
        if len(obj.points) >= 1:
            height, width = frame.shape[:2]
            max_length = int(np.sqrt(width**2 + height**2))
            
            p1 = obj.points[0]  # 起始点
            p2 = obj.points[1] if len(obj.points) >= 2 else obj.points[-1]
            
            dx = p2.x() - p1.x()
            dy = p2.y() - p1.y()
            
            if dx != 0 or dy != 0:  # 避免除零错误
                # 计算直线方向向量
                length = np.sqrt(dx*dx + dy*dy)
                dx, dy = dx/length, dy/length
                
                # 计算延长线的起点和终点
                start_x = int(p1.x() - dx * max_length)
                start_y = int(p1.y() - dy * max_length)
                end_x = int(p1.x() + dx * max_length)
                end_y = int(p1.y() + dy * max_length)
                
                # 计算角度（相对于水平线）
                angle = np.degrees(np.arctan2(-dy, dx))  # 使用-dy是因为y轴向下为正
                if angle < 0:
                    angle += 180
                
                # 如果图元被选中，先绘制高亮轮廓
                if obj.selected:
                    cv2.line(frame,
                            (start_x, start_y),
                            (end_x, end_y),
                            (0, 0, 255),  # 蓝色高亮
                            obj.properties['thickness'] + 8,
                            cv2.LINE_AA)
                
                # 绘制虚线效果
                if obj == self.current_object:
                    # 当前正在绘制时显示虚线
                    dash_length = 10
                    gap_length = 10
                    segment_length = dash_length + gap_length
                    total_length = max_length * 2
                    num_segments = int(total_length / segment_length)
                    
                    for i in range(num_segments):
                        start_dist = i * segment_length - max_length
                        end_dist = start_dist + dash_length
                        
                        seg_start_x = int(p1.x() + dx * start_dist)
                        seg_start_y = int(p1.y() + dy * start_dist)
                        seg_end_x = int(p1.x() + dx * end_dist)
                        seg_end_y = int(p1.y() + dy * end_dist)
                        
                        # 确保点在图像范围内
                        if (0 <= seg_start_x < width and 0 <= seg_start_y < height and
                            0 <= seg_end_x < width and 0 <= seg_end_y < height):
                            cv2.line(frame,
                                    (seg_start_x, seg_start_y),
                                    (seg_end_x, seg_end_y),
                                    obj.properties['color'],
                                    obj.properties['thickness'],
                                    cv2.LINE_AA)
                else:
                    # 完成绘制后显示实线
                    cv2.line(frame,
                            (start_x, start_y),
                            (end_x, end_y),
                            obj.properties['color'],
                            obj.properties['thickness'],
                            cv2.LINE_AA)
                
                # 绘制起始点标记
                cv2.circle(frame,
                          (p1.x(), p1.y()),
                          3,
                          obj.properties['color'],
                          -1)
                
                # 显示角度信息
                angle_text = f"{angle:.1f}"  # 移除度数符号，后面用圆圈代替
                
                # 计算文本位置（在起点附近）
                text_x = p1.x() + 20
                text_y = p1.y() - 20
                
                # 绘制文本（带背景）
                font = cv2.FONT_HERSHEY_SIMPLEX
                font_scale = self._calculate_font_scale(frame.shape[0])  # 使用基于图像高度的字体大小
                thickness = 2     # 增加字体粗细
                padding = 10      # 增大背景padding
                (text_width, text_height), baseline = cv2.getTextSize(angle_text, font, font_scale, thickness)
                
                # 绘制文本背景
                cv2.rectangle(frame,
                            (int(text_x - padding), int(text_y - text_height - padding)),
                            (int(text_x + text_width + padding + 15), int(text_y + padding)),  # 增加空间给度数符号
                            (0, 0, 0),
                            -1)
                
                # 绘制文本
                cv2.putText(frame,
                          angle_text,
                          (text_x, text_y),
                          font,
                          font_scale,
                          obj.properties['color'],
                          thickness,
                          cv2.LINE_AA)
                
                # 绘制度数符号（空心圆）
                dot_x = text_x + text_width + 8
                dot_y = text_y - text_height + text_height// 6
                cv2.circle(frame,
                          (int(dot_x), int(dot_y)),
                          int(6 * max(0.9, int(font_scale))),  # 圆的半径
                          obj.properties['color'],
                          2,  # 1表示空心圆
                          cv2.LINE_AA)
                
                # 添加调试信息
                if obj.selected:
                    print(f"Drawing selected {obj.type} at points: {obj.points}")

    def _draw_circle(self, frame, obj: DrawingObject):
        """绘制圆形"""
        if len(obj.points) >= 2:
            center = obj.points[0]
            radius_point = obj.points[1]
            radius_pixels = int(np.sqrt((radius_point.x() - center.x())**2 + 
                               (radius_point.y() - center.y())**2))
            
            # 保存像素半径到属性中
            obj.properties['pixel_radius'] = radius_pixels
            
            # 获取像素比例并计算实际半径和直径
            pixel_scale = self.get_pixel_scale()
            real_radius = radius_pixels * pixel_scale
            obj.properties['radius'] = real_radius
            
            # 如果图元被选中，先绘制蓝色高亮轮廓
            if obj.selected:
                cv2.circle(frame,
                          (center.x(), center.y()),
                          radius_pixels,
                          (0, 0, 255),  # 蓝色高亮
                          obj.properties['thickness'] + 8,
                          cv2.LINE_AA)
            
            # 绘制正常圆形
            cv2.circle(frame,
                      (center.x(), center.y()),
                      radius_pixels,
                      obj.properties['color'],
                      obj.properties['thickness'],
                      cv2.LINE_AA)
            
            # 绘制圆心
            cv2.circle(frame,
                      (center.x(), center.y()),
                      3,
                      obj.properties['color'],
                      -1)
            
            # 准备显示文本
            coord_text = f"({center.x()}, {center.y()})"
            
            # 如果有像素比例，显示实际尺寸，否则显示像素尺寸
            if 'display_text' in obj.properties:
                radius_text = obj.properties['display_text']
            else:
                if pixel_scale != 1.0:
                    radius_text = f"R={radius_pixels:.1f}um"
                    obj.properties['display_text'] = radius_text
                else:
                    radius_text = f"R={radius_pixels:.1f}px"
            
            # 设置文本参数
            font = cv2.FONT_HERSHEY_SIMPLEX
            font_scale = self._calculate_font_scale(frame.shape[0])  # 使用基于图像高度的字体大小
            thickness = 2     # 增加字体粗细
            padding = 10      # 增大背景padding
            
            # 计算两个文本的尺寸
            (coord_width, text_height), baseline = cv2.getTextSize(coord_text, font, font_scale, thickness)
            (radius_width, _), _ = cv2.getTextSize(radius_text, font, font_scale, thickness)
            # 显示圆心坐标和半径
            coord_text = f"({center.x()}, {center.y()})"
            
            # 计算文本位置（在圆心右侧）
            text_x = center.x() + 20
            coord_y = center.y() - 10
            radius_y = coord_y + text_height + 10  # 半径文本在坐标文本下方
            
            # 计算背景矩形的尺寸
            max_width = max(coord_width, radius_width)
            total_height = (text_height + 10) * 2  # 两行文本的总高度
            
            # 绘制文本背景
            padding = 6
            cv2.rectangle(frame,
                         (int(text_x - padding), 
                          int(coord_y - text_height - padding)),
                         (int(text_x + max_width + padding), 
                          int(radius_y + padding)),
                         (0, 0, 0),
                         -1)
            
            # 绘制坐标文本
            cv2.putText(frame,
                       coord_text,
                       (text_x, coord_y),
                       font,
                       font_scale,
                       obj.properties['color'],
                       thickness,
                       cv2.LINE_AA)
            
            # 绘制半径文本
            cv2.putText(frame,
                       radius_text,
                       (text_x, radius_y),
                       font,
                       font_scale,
                       obj.properties['color'],
                       thickness,
                       cv2.LINE_AA)

    def _draw_line_segment(self, frame, obj: DrawingObject):
        """绘制线段"""
        if len(obj.points) >= 2:
            p1, p2 = obj.points[0], obj.points[1]
            
            # 如果图元被选中，先绘制高亮轮廓
            if obj.selected:
                cv2.line(frame,
                        (p1.x(), p1.y()),
                        (p2.x(), p2.y()),
                        (0, 0, 255),  # 蓝色高亮
                        obj.properties['thickness'] + 8,  # 加粗
                        cv2.LINE_AA)
            
            # 绘制正常线段
            cv2.line(frame,
                    (p1.x(), p1.y()),
                    (p2.x(), p2.y()),
                    obj.properties['color'],
                    obj.properties['thickness'],
                    cv2.LINE_AA)
            
            # 绘制端点
            cv2.circle(frame,
                      (p1.x(), p1.y()),
                      3,
                      obj.properties['color'],
                      -1)
            cv2.circle(frame,
                      (p2.x(), p2.y()),
                      3,
                      obj.properties['color'],
                      -1)
            
            # 计算长度和角度
            length_pixels = np.sqrt((p2.x() - p1.x())**2 + (p2.y() - p1.y())**2)
            dx = p2.x() - p1.x()
            dy = p2.y() - p1.y()
            angle = np.degrees(np.arctan2(-dy, dx))
            if angle < 0:
                angle += 180
                
            # 保存像素长度到属性中
            obj.properties['pixel_length'] = length_pixels
            
            # 获取像素比例并计算实际长度
            pixel_scale = self.get_pixel_scale()
            real_length = length_pixels * pixel_scale
            obj.properties['length'] = real_length

            # 准备显示文本（分两行显示）
            if 'display_text' in obj.properties:
                dist_text = obj.properties['display_text']
            else:
                # 如果有像素比例，显示实际长度，否则显示像素长度
                if pixel_scale != 1.0:
                    dist_text = f"{real_length:.1f}um"
                    obj.properties['display_text'] = dist_text
                else:
                    dist_text = f"{length_pixels:.1f}px"
            
            angle_text = f"{angle:.1f}"  # 移除度数符号，后面用圆圈代替

            # 计算文本位置（在线段中点）
            text_x = (p1.x() + p2.x()) // 2
            text_y = (p1.y() + p2.y()) // 2

            # 绘制文本（带背景）
            font = cv2.FONT_HERSHEY_SIMPLEX
            font_scale = self._calculate_font_scale(frame.shape[0])  # 使用基于图像高度的字体大小
            thickness = 2     # 增加字体粗细
            padding = 10      # 增大背景padding
            line_spacing = 15 # 增加行间距

            # 计算文本大小
            (dist_width, text_height), baseline = cv2.getTextSize(dist_text, font, font_scale, thickness)
            (angle_width, _), _ = cv2.getTextSize(angle_text, font, font_scale, thickness)

            # 计算度数符号的额外空间
            dot_space = 15  # 为度数符号预留的空间

            # 计算最大宽度（考虑度数符号的空间）
            max_width = max(dist_width, angle_width + dot_space)

            # 计算两行文本的总高度
            total_height = text_height * 2 + line_spacing + padding * 2

            # 绘制背景矩形
            cv2.rectangle(frame,
                        (int(text_x - max_width//2 - padding), 
                         int(text_y - total_height//2)),
                        (int(text_x + max_width//2 + padding), 
                         int(text_y + total_height//2)),
                        (0, 0, 0),
                        -1)

            # 绘制距离文本（第一行）
            dist_x = text_x - dist_width//2
            dist_y = text_y - line_spacing//2
            cv2.putText(frame,
                      dist_text,
                      (int(dist_x), int(dist_y)),
                      font,
                      font_scale,
                      obj.properties['color'],
                      thickness,
                      cv2.LINE_AA)

            # 绘制角度文本（第二行）
            angle_x = text_x - (angle_width + dot_space)//2
            angle_y = text_y + text_height + line_spacing//2
            cv2.putText(frame,
                      angle_text,
                      (int(angle_x), int(angle_y)),
                      font,
                      font_scale,
                      obj.properties['color'],
                      thickness,
                      cv2.LINE_AA)

            # 绘制度数符号（空心圆）
            dot_x = angle_x + angle_width + 8
            dot_y = angle_y - text_height + text_height//6
            cv2.circle(frame,
                      (int(dot_x), int(dot_y)),
                      int(6 * max(0.9, int(font_scale))),  # 圆的半径
                      obj.properties['color'],
                      2,  # 1表示空心圆
                      cv2.LINE_AA)

    def _draw_current_line_segment(self, frame, obj: DrawingObject):
        """绘制当前正在绘制的线段"""
        if len(obj.points) >= 1:
            p1 = obj.points[0]
            p2 = obj.points[-1]
            
            # 改进的虚线绘制
            dist = np.sqrt((p2.x() - p1.x())**2 + (p2.y() - p1.y())**2)
            if dist > 0:
                # 计算单位向量
                vx = (p2.x() - p1.x()) / dist
                vy = (p2.y() - p1.y()) / dist
                
                # 虚线参数
                dash_length = 10
                gap_length = 10
                segment_length = dash_length + gap_length
                
                # 计算完整的虚线段数量
                num_segments = int(dist / segment_length)
                
                # 绘制完整的虚线段
                for i in range(num_segments):
                    start_dist = i * segment_length
                    end_dist = start_dist + dash_length
                    
                    start_x = int(p1.x() + vx * start_dist)
                    start_y = int(p1.y() + vy * start_dist)
                    end_x = int(p1.x() + vx * end_dist)
                    end_y = int(p1.y() + vy * end_dist)
                    
                    cv2.line(frame,
                            (start_x, start_y),
                            (end_x, end_y),
                            obj.properties['color'],
                            obj.properties['thickness'],
                            cv2.LINE_AA)
                
                # 处理最后一段可能的不完整虚线
                remaining_dist = dist - num_segments * segment_length
                if remaining_dist > 0:
                    start_dist = num_segments * segment_length
                    end_dist = min(start_dist + dash_length, dist)
                    
                    start_x = int(p1.x() + vx * start_dist)
                    start_y = int(p1.y() + vy * start_dist)
                    end_x = int(p1.x() + vx * end_dist)
                    end_y = int(p1.y() + vy * end_dist)
                    
                    cv2.line(frame,
                            (start_x, start_y),
                            (end_x, end_y),
                            obj.properties['color'],
                            obj.properties['thickness'],
                            cv2.LINE_AA)
            
            # 绘制端点标记
            cv2.circle(frame, (p1.x(), p1.y()), 3, obj.properties['color'], -1)
            cv2.circle(frame, (p2.x(), p2.y()), 3, obj.properties['color'], -1)
            
            # 显示实时长度
            text_x = (p1.x() + p2.x()) // 2
            text_y = (p1.y() + p2.y()) // 2
            text = f"{dist:.1f}px"
            
            # 绘制文本（带背景）
            font = cv2.FONT_HERSHEY_SIMPLEX
            font_scale = self._calculate_font_scale(frame.shape[0])  # 使用基于图像高度的字体大小
            thickness = 2     # 增加字体粗细
            padding = 10      # 增大背景padding
            
            # 获取文本大小
            (text_width, text_height), baseline = cv2.getTextSize(text, font, font_scale, thickness)
            
            # 绘制文本背景
            cv2.rectangle(frame,
                         (int(text_x - text_width//2 - padding), 
                          int(text_y - text_height//2 - padding)),
                         (int(text_x + text_width//2 + padding), 
                          int(text_y + text_height//2 + padding)),
                         (0, 0, 0),
                         -1)
            
            # 绘制文本
            cv2.putText(frame,
                       text,
                       (int(text_x - text_width//2), int(text_y + text_height//2)),
                       font,
                       font_scale,
                       obj.properties['color'],
                       thickness,
                       cv2.LINE_AA)

    def _draw_parallel(self, frame, obj: DrawingObject):
        """绘制平行线"""
        if len(obj.points) >= 2:
            height, width = frame.shape[:2]
            max_length = int(np.sqrt(width**2 + height**2))
            
            # 第一条线的起点和方向点
            p1 = obj.points[0]
            p2 = obj.points[1]
            
            # 计算方向向量
            dx = p2.x() - p1.x()
            dy = p2.y() - p1.y()
            
            if dx != 0 or dy != 0:  # 避免除零错误
                # 计算直线方向向量
                length = np.sqrt(dx*dx + dy*dy)
                dx, dy = dx/length, dy/length
                
                # 计算角度（相对于水平线）
                angle = np.degrees(np.arctan2(-dy, dx))  # 使用-dy是因为y轴向下为正
                if angle < 0:
                    angle += 180

                # 计算第一条线的延长线起终点
                start_x1 = int(p1.x() - dx * max_length)
                start_y1 = int(p1.y() - dy * max_length)
                end_x1 = int(p1.x() + dx * max_length)
                end_y1 = int(p1.y() + dy * max_length)
                
                # 如果图元被选中，先绘制蓝色高亮轮廓
                if obj.selected:
                    cv2.line(frame,
                            (start_x1, start_y1),
                            (end_x1, end_y1),
                            (0, 0, 255),  # 蓝色高亮
                            obj.properties['thickness'] + 8,
                            cv2.LINE_AA)
                
                # 绘制第一条线
                if obj == self.current_object and len(obj.points) == 2:
                    # 正在绘制第一条线时显示虚线
                    dash_length = 10
                    gap_length = 10
                    segment_length = dash_length + gap_length
                    total_length = max_length * 2
                    num_segments = int(total_length / segment_length)
                    
                    for i in range(num_segments):
                        start_dist = i * segment_length - max_length
                        end_dist = start_dist + dash_length
                        
                        seg_start_x = int(p1.x() + dx * start_dist)
                        seg_start_y = int(p1.y() + dy * start_dist)
                        seg_end_x = int(p1.x() + dx * end_dist)
                        seg_end_y = int(p1.y() + dy * end_dist)
                        
                        if (0 <= seg_start_x < width and 0 <= seg_start_y < height and
                            0 <= seg_end_x < width and 0 <= seg_end_y < height):
                            cv2.line(frame,
                                    (seg_start_x, seg_start_y),
                                    (seg_end_x, seg_end_y),
                                    obj.properties['color'],
                                    obj.properties['thickness'],
                                    cv2.LINE_AA)
                else:
                    # 第一条线已确定，显示实线
                    cv2.line(frame,
                            (start_x1, start_y1),
                            (end_x1, end_y1),
                            obj.properties['color'],
                            obj.properties['thickness'],
                            cv2.LINE_AA)
                
                # 如果有第三个点，绘制第二条平行线和中线
                if len(obj.points) >= 3:
                    p3 = obj.points[2]
                    
                    # 计算第二条线的起终点
                    start_x2 = int(p3.x() - dx * max_length)
                    start_y2 = int(p3.y() - dy * max_length)
                    end_x2 = int(p3.x() + dx * max_length)
                    end_y2 = int(p3.y() + dy * max_length)
                    
                    # 如果图元被选中，先绘制蓝色高亮轮廓
                    if obj.selected:
                        cv2.line(frame,
                                (start_x2, start_y2),
                                (end_x2, end_y2),
                                (0, 0, 255),  # 蓝色高亮
                                obj.properties['thickness'] + 8,
                                cv2.LINE_AA)
                    
                    # 绘制第二条平行线
                    cv2.line(frame,
                            (start_x2, start_y2),
                            (end_x2, end_y2),
                            obj.properties['color'],
                            obj.properties['thickness'],
                            cv2.LINE_AA)
                    
                    # 计算两线之间的垂直距离
                    dist = abs((dy * (p3.x() - p1.x()) - dx * (p3.y() - p1.y())))
                    
                    # 计算中点的坐标
                    mid_x = (p1.x() + p3.x()) // 2
                    mid_y = (p1.y() + p3.y()) // 2
                    
                    # 计算中线的延长线起终点
                    start_x_mid = int(mid_x - dx * max_length)
                    start_y_mid = int(mid_y - dy * max_length)
                    end_x_mid = int(mid_x + dx * max_length)
                    end_y_mid = int(mid_y + dy * max_length)
                    
                    # 检查中线是否被选中
                    midline_selected = False
                    if 'midline_object' in obj.properties:
                        midline_selected = obj.properties['midline_object'].selected
                    
                    # 绘制中线（虚线）
                    # 如果中线被选中，使用蓝色高亮显示
                    midline_color = (0, 0, 255) if midline_selected else (255, 0, 0)
                    midline_thickness = 4 if midline_selected else 4
                    
                    dash_length = 10
                    gap_length = 10
                    segment_length = dash_length + gap_length
                    total_length = max_length * 2  # 修改为两倍最大长度
                    num_segments = int(total_length / segment_length)
                    
                    # 从中点向两边绘制虚线
                    for i in range(num_segments):
                        start_dist = i * segment_length - max_length  # 从负方向开始
                        end_dist = start_dist + dash_length
                        
                        dash_start_x = int(mid_x + dx * start_dist)
                        dash_start_y = int(mid_y + dy * start_dist)
                        dash_end_x = int(mid_x + dx * end_dist)
                        dash_end_y = int(mid_y + dy * end_dist)
                        
                        if (0 <= dash_start_x < width and 0 <= dash_start_y < height and
                            0 <= dash_end_x < width and 0 <= dash_end_y < height):
                            cv2.line(frame,
                                    (dash_start_x, dash_start_y),
                                    (dash_end_x, dash_end_y),
                                    midline_color,
                                    midline_thickness,
                                    cv2.LINE_AA)
                    
                    # 显示距离和角度信息（分两行显示）
                    dist_text = f"{dist:.1f}px"
                    angle_text = f"{angle:.1f}"  # 移除度数符号，后面用圆圈代替

                    # 计算文本位置（在两条线中间，但要错开）
                    text_x = mid_x + 40  # 向右偏移，避免与中线重叠
                    text_y = mid_y

                    # 绘制文本（带背景）
                    font = cv2.FONT_HERSHEY_SIMPLEX
                    font_scale = self._calculate_font_scale(frame.shape[0])  # 使用基于图像高度的字体大小
                    thickness = 2     # 增加字体粗细
                    padding = 10      # 增大背景padding
                    line_spacing = 15 # 增加行间距

                    # 计算文本大小
                    (dist_width, text_height), baseline = cv2.getTextSize(dist_text, font, font_scale, thickness)
                    (angle_width, _), _ = cv2.getTextSize(angle_text, font, font_scale, thickness)

                    # 计算度数符号的额外空间
                    dot_space = 15  # 为度数符号预留的空间

                    # 计算最大宽度（考虑度数符号的空间）
                    max_width = max(dist_width, angle_width + dot_space)

                    # 计算两行文本的垂直间距
                    line_spacing = 15

                    # 绘制文本背景（覆盖两行文本）
                    padding = 2
                    total_height = text_height * 2 + line_spacing + padding  # 增加额外的高度

                    # 绘制背景矩形
                    cv2.rectangle(frame,
                                (int(text_x - max_width//2 - padding), 
                                 int(text_y - total_height)),  # 上边界增加padding
                                (int(text_x + max_width//2 + padding), 
                                 int(text_y + text_height + padding)),  # 下边界延伸到文本底部以下
                                (0, 0, 0),
                                -1)

                    # 绘制距离文本（第一行）
                    dist_x = text_x - dist_width//2
                    dist_y = text_y - text_height - line_spacing
                    cv2.putText(frame,
                              dist_text,
                              (int(dist_x), int(dist_y + text_height//2)),
                              font,
                              font_scale,
                              obj.properties['color'],
                              thickness,
                              cv2.LINE_AA)

                    # 绘制角度文本（第二行）
                    angle_x = text_x - (angle_width + dot_space)//2
                    angle_y = text_y
                    cv2.putText(frame,
                              angle_text,
                              (int(angle_x), int(angle_y + text_height//2)),
                              font,
                              font_scale,
                              obj.properties['color'],
                              thickness,
                              cv2.LINE_AA)

                    # 绘制度数符号（空心圆）
                    dot_x = angle_x + angle_width + 8
                    dot_y = angle_y - text_height//3
                    cv2.circle(frame,
                              (int(dot_x), int(dot_y)),
                              int(6 * max(0.9, int(font_scale))),  # 圆的半径
                              obj.properties['color'],
                              2,  # 线宽为1表示空心圆
                              cv2.LINE_AA)

    def _draw_circle_line(self, frame, obj: DrawingObject):
        """绘制圆与线的距离测量"""
        # 如果是圆绘制阶段，绘制临时点
        if obj.properties.get('drawing_stage') == 'circle':
            temp_points = obj.properties.get('temp_points', [])
            # 绘制所有临时点
            for point in temp_points:
                cv2.circle(frame, 
                          (point.x(), point.y()), 
                          5,  # 点的半径
                          obj.properties.get('color', (0, 255, 0)), 
                          -1)  # 填充圆
                
            # 如果有两个或更多点，绘制连接线
            if len(temp_points) >= 2:
                for i in range(len(temp_points) - 1):
                    cv2.line(frame,
                            (temp_points[i].x(), temp_points[i].y()),
                            (temp_points[i+1].x(), temp_points[i+1].y()),
                            obj.properties.get('color', (0, 255, 0)),
                            obj.properties.get('thickness', 2),
                            cv2.LINE_AA)
                
                # 如果有三个点，连接第三个点和第一个点，形成闭合三角形
                if len(temp_points) == 3:
                    cv2.line(frame,
                            (temp_points[2].x(), temp_points[2].y()),
                            (temp_points[0].x(), temp_points[0].y()),
                            obj.properties.get('color', (0, 255, 0)),
                            obj.properties.get('thickness', 2),
                            cv2.LINE_AA)
            
            return
            
        # 正常绘制阶段（圆已确定）
        if len(obj.points) >= 1:
            # 绘制圆心
            center = obj.points[0]
            cv2.circle(frame, 
                      (center.x(), center.y()), 
                      3, 
                      obj.properties.get('color', (0, 255, 0)), 
                      -1)
            
            # 如果有第二个点，绘制圆
            if len(obj.points) >= 2:
                p2 = obj.points[1]
                radius = int(np.sqrt(
                    (p2.x() - center.x())**2 + 
                    (p2.y() - center.y())**2
                ))
                
                # 绘制圆
                cv2.circle(frame, 
                          (center.x(), center.y()), 
                          radius, 
                          obj.properties.get('color', (0, 255, 0)), 
                          obj.properties.get('thickness', 2),
                          cv2.LINE_AA)
                
                # 如果有第三个点和第四个点，绘制直线和垂线
                if len(obj.points) >= 4:
                    p3, p4 = obj.points[2], obj.points[3]
                    height, width = frame.shape[:2]
                    max_length = int(np.sqrt(width**2 + height**2))
                    
                    # 计算直线方向向量
                    dx = p4.x() - p3.x()
                    dy = p4.y() - p3.y()
                    if dx != 0 or dy != 0:
                        length = np.sqrt(dx*dx + dy*dy)
                        dx, dy = dx/length, dy/length
                        
                        # 计算延长线的起点和终点
                        start_x = int(p3.x() - dx * max_length)
                        start_y = int(p3.y() - dy * max_length)
                        end_x = int(p3.x() + dx * max_length)
                        end_y = int(p3.y() + dy * max_length)
                        
                        # 绘制直线
                        cv2.line(frame,
                                (start_x, start_y),
                                (end_x, end_y),
                                obj.properties.get('color', (0, 255, 0)),
                                obj.properties.get('thickness', 2),
                                cv2.LINE_AA)
                        
                        # 计算圆心到直线的垂直距离
                        signed_dist = (dy * center.x() - dx * center.y() + dx * p3.y() - dy * p3.x()) / np.sqrt(dx*dx + dy*dy)
                        dist = abs(signed_dist)  # 这里不再减去半径
                        
                        # 根据圆心在直线哪一侧来确定垂线方向
                        if signed_dist > 0:
                            perp_dx = -dy
                            perp_dy = dx
                        else:
                            perp_dx = dy
                            perp_dy = -dx
                        
                        # 计算垂线的起点和终点（从圆心出发）
                        foot_x = center.x() + dist * perp_dx
                        foot_y = center.y() + dist * perp_dy
                        
                        # 绘制垂线（虚线）
                        dash_length = 5
                        gap_length = 5
                        total_length = dist
                        num_segments = int(total_length / (dash_length + gap_length))
                        
                        for i in range(num_segments):
                            start_dist = i * (dash_length + gap_length)
                            end_dist = min(start_dist + dash_length, total_length)
                            
                            dash_start_x = int(center.x() + perp_dx * start_dist)
                            dash_start_y = int(center.y() + perp_dy * start_dist)
                            dash_end_x = int(center.x() + perp_dx * end_dist)
                            dash_end_y = int(center.y() + perp_dy * end_dist)
                            
                            cv2.line(frame,
                                    (dash_start_x, dash_start_y),
                                    (dash_end_x, dash_end_y),
                                    obj.properties.get('color', (0, 255, 0)),
                                    obj.properties.get('thickness', 2),
                                    cv2.LINE_AA)
                        
                        # 显示距离信息
                        text = f"{dist:.1f}px"  # 直接显示垂直距离，不减去半径
                        font = cv2.FONT_HERSHEY_SIMPLEX
                        font_scale = self._calculate_font_scale(frame.shape[0])  # 使用基于图像高度的字体大小
                        thickness = 2     # 增加字体粗细
                        padding = 10      # 增大背景padding
                        
                        # 计算文本位置
                        text_x = int((center.x() + foot_x) / 2)
                        text_y = int((center.y() + foot_y) / 2)
                        
                        # 获取文本大小
                        (text_width, text_height), baseline = cv2.getTextSize(text, font, font_scale, thickness)
                        
                        # 绘制文本背景
                        padding = 6
                        cv2.rectangle(frame,
                                    (int(text_x - text_width//2 - padding), 
                                     int(text_y - text_height//2 - padding)),
                                    (int(text_x + text_width//2 + padding), 
                                     int(text_y + text_height//2 + padding)),
                                    (0, 0, 0),
                                    -1)
                        
                        # 绘制文本
                        cv2.putText(frame,
                                  text,
                                  (int(text_x - text_width//2), int(text_y + text_height//2)),
                                  font,
                                  font_scale,
                                  obj.properties.get('color', (0, 255, 0)),
                                  thickness,
                                  cv2.LINE_AA)

    def _draw_object(self, frame, obj: DrawingObject):
        """绘制单个对象"""
        draw_methods = {
            DrawingType.LINE: self._draw_line,
            DrawingType.CIRCLE: self._draw_circle,
            DrawingType.LINE_SEGMENT: self._draw_line_segment,
            DrawingType.PARALLEL: self._draw_parallel,
            DrawingType.CIRCLE_LINE: self._draw_circle_line,
            DrawingType.TWO_LINES: self._draw_two_lines,
            DrawingType.LINE_DETECT: self._draw_line_detect,  # 添加直线检测
            DrawingType.CIRCLE_DETECT: self._draw_circle_detect,  # 添加圆形检测
            DrawingType.POINT: self._draw_point,  # 添加点的绘制方法
        }
        
        if obj.type in draw_methods:
            draw_methods[obj.type](frame, obj)
        
    def _draw_two_lines(self, frame, obj: DrawingObject):
        """绘制两条线的夹角测量"""
        # 首先检查是否有足够的点
        if len(obj.points) < 2:
            return  # 如果点数不足，直接返回
        
        height, width = frame.shape[:2]
        max_length = int(10 * np.sqrt(width**2 + height**2))  # 使用10倍对角线长度
        
        # 绘制第一条线
        p1 = obj.points[0]
        p2 = obj.points[1]
        
        # 计算第一条线的方向向量
        dx1 = p2.x() - p1.x()
        dy1 = p2.y() - p1.y()
        
        # 绘制第一条线
        if dx1 != 0 or dy1 != 0:
            # 计算单位向量
            length1 = np.sqrt(dx1*dx1 + dy1*dy1)
            dx1, dy1 = dx1/length1, dy1/length1
            
            # 绘制第一条线
            start_x1 = int(p1.x() - dx1 * max_length)
            start_y1 = int(p1.y() - dy1 * max_length)
            end_x1 = int(p1.x() + dx1 * max_length)
            end_y1 = int(p1.y() + dy1 * max_length)
            
            cv2.line(frame,
                    (start_x1, start_y1),
                    (end_x1, end_y1),
                    obj.properties['color'],
                    obj.properties['thickness'],
                    cv2.LINE_AA)
        
        # 如果没有第二条线的点，到这里就返回
        if len(obj.points) < 4:
            return
        
        # 绘制第二条线
        p3 = obj.points[2]
        p4 = obj.points[3]
        
        # 计算第二条线的方向向量
        dx2 = p4.x() - p3.x()
        dy2 = p4.y() - p3.y()
        
        if dx2 != 0 or dy2 != 0:
            # 计算单位向量
            length2 = np.sqrt(dx2*dx2 + dy2*dy2)
            dx2, dy2 = dx2/length2, dy2/length2
            
            # 绘制第二条线
            start_x2 = int(p3.x() - dx2 * max_length)
            start_y2 = int(p3.y() - dy2 * max_length)
            end_x2 = int(p3.x() + dx2 * max_length)
            end_y2 = int(p3.y() + dy2 * max_length)
            
            cv2.line(frame,
                    (start_x2, start_y2),
                    (end_x2, end_y2),
                    obj.properties['color'],
                    obj.properties['thickness'],
                    cv2.LINE_AA)
            
            # 计算夹角
            dot_product = dx1*dx2 + dy1*dy2
            angle = np.degrees(np.arccos(np.clip(dot_product, -1.0, 1.0)))
            
            # 计算两直线的交点
            denominator = dx1*dy2 - dy1*dx2
            if abs(denominator) > 1e-10:  # 非平行线情况
                t1 = ((p3.x() - p1.x())*dy2 - (p3.y() - p1.y())*dx2) / denominator
                intersection_x = int(p1.x() + dx1 * t1)
                intersection_y = int(p1.y() + dy1 * t1)
                
                # 检查交点是否在视图内
                if (0 <= intersection_x < width and 0 <= intersection_y < height):
                    display_x = intersection_x
                    display_y = intersection_y
                else:
                    # 如果交点在视图外，计算两直线的中点
                    mid1_x = (p1.x() + p2.x()) // 2
                    mid1_y = (p1.y() + p2.y()) // 2
                    mid2_x = (p3.x() + p4.x()) // 2
                    mid2_y = (p3.y() + p4.y()) // 2
                    display_x = (mid1_x + mid2_x) // 2
                    display_y = (mid1_y + mid2_y) // 2
                
                # 计算角平分线的方向向量（中线）
                mid_dx = dx1 + dx2
                mid_dy = dy1 + dy2
                mid_length = np.sqrt(mid_dx*mid_dx + mid_dy*mid_dy)
                if mid_length > 1e-10:
                    mid_dx /= mid_length
                    mid_dy /= mid_length
                    
                    # 绘制中线（虚线）
                    dash_length = 10
                    gap_length = 10
                    segment_length = dash_length + gap_length
                    total_length = max_length * 2
                    num_segments = int(total_length / segment_length)
                    
                    # 从显示位置向两边绘制虚线
                    for i in range(num_segments):
                        start_dist = i * segment_length - max_length
                        end_dist = start_dist + dash_length
                        
                        dash_start_x = int(display_x + mid_dx * start_dist)
                        dash_start_y = int(display_y + mid_dy * start_dist)
                        dash_end_x = int(display_x + mid_dx * end_dist)
                        dash_end_y = int(display_y + mid_dy * end_dist)
                        
                        if (0 <= dash_start_x < width and 0 <= dash_start_y < height and
                            0 <= dash_end_x < width and 0 <= dash_end_y < height):
                            cv2.line(frame,
                                    (dash_start_x, dash_start_y),
                                    (dash_end_x, dash_end_y),
                                    obj.properties['color'],
                                    obj.properties['thickness'],
                                    cv2.LINE_AA)
            
            # 准备显示的文本
            angle_text = f"{angle:.1f}"  # 移除度数符号，后面用圆圈代替
            coord_text = f"({intersection_x}, {intersection_y})"
            
            # 设置文本参数
            font = cv2.FONT_HERSHEY_SIMPLEX
            font_scale = self._calculate_font_scale(frame.shape[0])  # 使用基于图像高度的字体大小
            thickness = 2
            padding = 10
            
            # 获取文本尺寸
            (angle_width, text_height), _ = cv2.getTextSize(angle_text, font, font_scale, thickness)
            (coord_width, _), _ = cv2.getTextSize(coord_text, font, font_scale, thickness)
            
            # 计算文本位置
            text_x = display_x + 20
            angle_y = display_y - text_height - 5
            coord_y = display_y + text_height + 5
            
            # 计算背景矩形的尺寸
            max_width = max(angle_width + 15, coord_width)  # 增加空间给度数符号
            total_height = text_height * 2 + 20  # 两行文本的总高度加上间距

            # 在显示位置画一个小圆点(仅当点在视图内时)
            if 0 <= intersection_x < width and 0 <= intersection_y < height:
                cv2.circle(frame, (display_x, display_y), 6, (255, 0, 0), -1)
            
            # 绘制文本背景
            cv2.rectangle(frame,
                        (int(text_x - padding), 
                         int(angle_y - padding)),
                        (int(text_x + max_width + padding), 
                         int(coord_y + padding)),
                        (0, 0, 0),
                        -1)
            
            # 绘制角度文本
            cv2.putText(frame,
                      angle_text,
                      (text_x, angle_y + text_height),
                      font,
                      font_scale,
                      (255, 0, 0),
                      thickness,
                      cv2.LINE_AA)
                      
            # 绘制度数符号（空心圆）
            dot_x = text_x + angle_width + 8
            dot_y = angle_y + text_height//6
            cv2.circle(frame,
                      (int(dot_x), int(dot_y)),
                      int(6 * max(0.9, int(font_scale))),  # 圆的半径
                      (255, 0, 0),
                      2,  # 1表示空心圆
                      cv2.LINE_AA)
            
            # 绘制坐标文本
            cv2.putText(frame,
                      coord_text,
                      (text_x, coord_y),
                      font,
                      font_scale,
                      (255, 0, 0),
                      thickness,
                      cv2.LINE_AA)

    def _draw_line_segment_angle(self, frame, obj: DrawingObject):
        """绘制两线段的角度测量"""
        # 首先检查是否有足够的点
        if len(obj.points) < 4:
            return  # 如果点数不足，直接返回
        
        # 获取两条线段的四个点
        p1 = obj.points[0]  # 第一条线段的起点
        p2 = obj.points[1]  # 第一条线段的终点
        p3 = obj.points[2]  # 第二条线段的起点
        p4 = obj.points[3]  # 第二条线段的终点
        
        # 不再重新绘制线段，只计算和显示角度
        
        # 计算第一条线段的方向向量
        dx1 = p2.x() - p1.x()
        dy1 = p2.y() - p1.y()
        
        # 计算第二条线段的方向向量
        dx2 = p4.x() - p3.x()
        dy2 = p4.y() - p3.y()
        
        # 计算向量长度
        length1 = np.sqrt(dx1*dx1 + dy1*dy1)
        length2 = np.sqrt(dx2*dx2 + dy2*dy2)
        
        # 检查线段长度是否为零
        if length1 < 1e-10 or length2 < 1e-10:
            return
        
        # 计算单位向量
        dx1, dy1 = dx1/length1, dy1/length1
        dx2, dy2 = dx2/length2, dy2/length2
        
        # 计算夹角（点积公式）
        dot_product = dx1*dx2 + dy1*dy2
        angle = np.degrees(np.arccos(np.clip(dot_product, -1.0, 1.0)))
        
        # 计算两线段的交点作为显示角度的位置
        # 使用线性代数求解两直线交点
        # 线段1: p1 -> p2, 线段2: p3 -> p4
        
        # 计算直线方程 Ax + By = C
        A1 = p2.y() - p1.y()
        B1 = p1.x() - p2.x()
        C1 = A1 * p1.x() + B1 * p1.y()
        
        A2 = p4.y() - p3.y()
        B2 = p3.x() - p4.x()
        C2 = A2 * p3.x() + B2 * p3.y()
        
        # 计算行列式
        det = A1 * B2 - A2 * B1
        
        # 如果行列式接近0，表示两线段平行或共线，使用中点
        if abs(det) < 1e-10:
            # 使用两线段中点的中点
            mid1_x = (p1.x() + p2.x()) // 2
            mid1_y = (p1.y() + p2.y()) // 2
            mid2_x = (p3.x() + p4.x()) // 2
            mid2_y = (p3.y() + p4.y()) // 2
            display_x = (mid1_x + mid2_x) // 2
            display_y = (mid1_y + mid2_y) // 2
        else:
            # 计算交点
            display_x = int((B2 * C1 - B1 * C2) / det)
            display_y = int((A1 * C2 - A2 * C1) / det)
        
        # 在显示位置画一个小圆点
        cv2.circle(frame, (display_x, display_y), 6, (255, 0, 0), -1)
        
        # 准备显示的文本
        angle_text = f"{angle:.1f}"  # 角度值，保留一位小数
        
        # 设置文本参数
        font = cv2.FONT_HERSHEY_SIMPLEX
        font_scale = self._calculate_font_scale(frame.shape[0])  # 使用基于图像高度的字体大小
        thickness = 2
        padding = 10
        
        # 获取文本尺寸
        (angle_width, text_height), _ = cv2.getTextSize(angle_text, font, font_scale, thickness)
        
        # 计算文本位置
        text_x = display_x + 20
        text_y = display_y + text_height
        
        # 计算背景矩形的尺寸
        rect_width = angle_width + 15  # 增加空间给度数符号
        
        # 绘制文本背景
        cv2.rectangle(frame,
                    (int(text_x - padding), 
                     int(text_y - text_height - padding)),
                    (int(text_x + rect_width + padding), 
                     int(text_y + padding)),
                    (0, 0, 0),
                    -1)
        
        # 绘制角度文本
        cv2.putText(frame,
                  angle_text,
                  (text_x, text_y),
                  font,
                  font_scale,
                  (255, 0, 0),
                  thickness,
                  cv2.LINE_AA)
                  
        # 绘制度数符号（空心圆）
        dot_x = text_x + angle_width + 8
        dot_y = text_y - text_height // 3 * 2
        cv2.circle(frame,
                  (int(dot_x), int(dot_y)),
                  int(6 * max(0.9, int(font_scale))),  # 圆的半径
                  (255, 0, 0),
                  2,  # 2表示空心圆
                  cv2.LINE_AA)

    def _detect_line_in_roi(self, frame, roi_points):
        """在ROI区域内检测直线"""
        try:
            # 从 settings.json 读取参数
            from Tools.settings_manager import SettingsManager
            settings = SettingsManager()
            settings_dict = settings.load_settings_from_file()
            
            # 获取参数，如果没有则使用默认值
            canny_low = int(settings_dict.get('CannyLineLow', 50))
            canny_high = int(settings_dict.get('CannyLineHigh', 150))
            line_threshold = int(settings_dict.get('LineDetThreshold', 50))
            min_line_length = int(settings_dict.get('LineDetMinLength', 50))
            max_line_gap = int(settings_dict.get('LineDetMaxGap', 10))
            
            print(f"直线检测参数: canny_low={canny_low}, canny_high={canny_high}, line_threshold={line_threshold}, min_line_length={min_line_length}, max_line_gap={max_line_gap}")
            
            # 获取ROI区域的坐标
            x1, y1 = roi_points[0].x(), roi_points[0].y()
            x2, y2 = roi_points[1].x(), roi_points[1].y()
            
            # 确保坐标正确排序
            x_min, x_max = min(x1, x2), max(x1, x2)
            y_min, y_max = min(y1, y2), max(y1, y2)
            
            print(f"ROI区域: x_min={x_min}, x_max={x_max}, y_min={y_min}, y_max={y_max}")
            
            # 提取ROI区域
            roi = frame[y_min:y_max, x_min:x_max]
            if roi.size == 0:
                print("ROI区域为空")
                return None
            
            print(f"ROI形状: {roi.shape}")
            
            # 检查图像通道数
            gray = None
            if len(roi.shape) == 2:
                # 已经是灰度图
                gray = roi
                print("输入图像已经是灰度图")
            elif len(roi.shape) == 3 and roi.shape[2] == 3:
                # 彩色图转灰度图
                gray = cv2.cvtColor(roi, cv2.COLOR_BGR2GRAY)
                print("已将彩色图转换为灰度图")
            else:
                print(f"不支持的图像格式: shape={roi.shape}")
                return None
            
            # 边缘检测 - 使用设置的参数
            edges = cv2.Canny(gray, canny_low, canny_high, apertureSize=3)
            print(f"边缘检测完成，edges形状: {edges.shape}")
            
            # 霍夫变换检测直线 - 使用设置的参数
            try:
                lines = cv2.HoughLinesP(edges, 1, np.pi/180, 
                                       threshold=line_threshold,
                                       minLineLength=min_line_length,
                                       maxLineGap=max_line_gap)
                
                if lines is not None:
                    print(f"检测到 {len(lines)} 条线")
                else:
                    print("未检测到直线")
            except Exception as e:
                print(f"直线检测失败: {str(e)}")
                return None
            
            if lines is not None and len(lines) > 0:
                # 找到最长的线段
                max_length = 0
                best_line = None
                
                for line in lines:
                    x1, y1, x2, y2 = line[0]
                    length = np.sqrt((x2-x1)**2 + (y2-y1)**2)
                    if length > max_length:
                        max_length = length
                        best_line = line[0]
                
                if best_line is not None:
                    # 将坐标转换回原图坐标系
                    x1, y1, x2, y2 = best_line
                    return [
                        QPoint(x1 + x_min, y1 + y_min),
                        QPoint(x2 + x_min, y2 + y_min)
                    ]
                
            print("未检测到直线")
            return None
        
        except Exception as e:
            print(f"加载参数失败，使用默认值: {str(e)}")
            # 使用默认值
            canny_low = 50
            canny_high = 150
            line_threshold = 50
            min_line_length = 50
            max_line_gap = 10

    def _draw_line_detect(self, frame, obj: DrawingObject):
        """绘制直线检测"""
        if len(obj.points) >= 2:
            p1, p2 = obj.points[:2]
            
            if obj.properties.get('is_roi', True):
                # 绘制ROI框
                cv2.rectangle(frame,
                             (p1.x(), p1.y()),
                             (p2.x(), p2.y()),
                             obj.properties['color'],
                             2,
                             cv2.LINE_AA)
            else:
                if obj.properties.get('line_detected', False):
                    # 绘制检测到的直线（延伸到图像边界）
                    height, width = frame.shape[:2]
                    max_length = int(np.sqrt(width**2 + height**2))
                    
                    # 计算直线方向向量
                    dx = p2.x() - p1.x()
                    dy = p2.y() - p1.y()
                    
                    if dx != 0 or dy != 0:
                        # 计算单位向量
                        length = np.sqrt(dx*dx + dy*dy)
                        dx, dy = dx/length, dy/length
                        
                        # 计算延长线的起点和终点
                        start_x = int(p1.x() - dx * max_length)
                        start_y = int(p1.y() - dy * max_length)
                        end_x = int(p1.x() + dx * max_length)
                        end_y = int(p1.y() + dy * max_length)
                        
                        # 绘制延长线
                        cv2.line(frame,
                                (start_x, start_y),
                                (end_x, end_y),
                                obj.properties['color'],
                                obj.properties['thickness'],
                                cv2.LINE_AA)
                        
                        # 计算角度
                        dx = p2.x() - p1.x()
                        dy = p2.y() - p1.y()
                        angle = np.degrees(np.arctan2(-dy, dx))  # 使用-dy是因为y轴向下为正
                        if angle < 0:
                            angle += 180
                        
                        # 绘制文本（带背景）
                        text = f"{angle:.1f}"
                        font = cv2.FONT_HERSHEY_SIMPLEX
                        font_scale = self._calculate_font_scale(frame.shape[0])  # 使用基于图像高度的字体大小
                        thickness = 2     # 增加字体粗细
                        padding = 10      # 增大背景padding
                        (text_width, text_height), baseline = cv2.getTextSize(text, font, font_scale, thickness)
                        
                        # 计算直线中点
                        mid_x = (p1.x() + p2.x()) // 2
                        mid_y = (p1.y() + p2.y()) // 2
                        
                        # 计算文本位置（在直线中点的上方）
                        text_x = mid_x - text_width // 2  # 文本水平居中
                        text_y = mid_y - text_height - 10  # 文本在线段上方
                        
                        # 计算度数圆点的位置
                        dot_x = text_x + text_width + 5
                        dot_y = text_y - text_height + 3
                        
                        # 绘制文本背景
                        padding = 6
                        cv2.rectangle(frame,
                                    (int(text_x - padding), int(text_y - text_height - padding)),
                                    (int(dot_x + 2 + padding), int(text_y + padding)),
                                    (0, 0, 0),
                                    -1)
                        
                        # 绘制文本
                        cv2.putText(frame,
                                text,
                                (text_x, text_y),
                                font,
                                font_scale,
                                obj.properties['color'],
                                thickness,
                                cv2.LINE_AA)
                        
                        # 绘制度数圆点
                        cv2.circle(frame,
                                (int(dot_x), int(dot_y)),
                                3,
                                obj.properties['color'],
                                1,
                                cv2.LINE_AA)

    def _detect_circle_in_roi(self, frame, roi_points):
        """在ROI区域内检测圆形"""
        try:
            # 从 settings.json 读取参数
            from Tools.settings_manager import SettingsManager
            settings = SettingsManager()
            settings_dict = settings.load_settings_from_file()
            
            # 获取参数
            canny_low = int(settings_dict.get('CannyCircleLow', 100))
            canny_high = int(settings_dict.get('CannyCircleHigh', 200))
            circle_threshold = int(settings_dict.get('CircleDetParam2', 50))
            
            # 新增参数：最小置信度阈值
            min_confidence = int(settings_dict.get('CircleMinConfidence', 30))
            
            print(f"圆检测参数: canny_low={canny_low}, canny_high={canny_high}, circle_threshold={circle_threshold}, min_confidence={min_confidence}")
            
            # 获取ROI区域的坐标
            x1, y1 = roi_points[0].x(), roi_points[0].y()
            x2, y2 = roi_points[1].x(), roi_points[1].y()
            
            # 确保坐标正确排序
            x_min, x_max = min(x1, x2), max(x1, x2)
            y_min, y_max = min(y1, y2), max(y1, y2)
            
            # 计算ROI尺寸
            roi_width = abs(x_max - x_min)
            roi_height = abs(y_max - y_min)
            roi_area = roi_width * roi_height
            
            # 设置ROI面积的最大限制（例如：500x500像素）
            MAX_ROI_AREA = 400000  # 500x500
            MAX_DIMENSION = 1000    # 单边最大500像素
            
            # 检查ROI大小限制
            if roi_area > MAX_ROI_AREA:
                print(f"ROI区域太大: {roi_area}像素 > {MAX_ROI_AREA}像素")
                return None
                
            if roi_width > MAX_DIMENSION or roi_height > MAX_DIMENSION:
                print(f"ROI尺寸超出限制: {roi_width}x{roi_height} > {MAX_DIMENSION}x{MAX_DIMENSION}")
                return None
            
            # 确保ROI区域有效
            if roi_width < 10 or roi_height < 10:
                print("ROI区域太小")
                return None
            
            # 计算合适的圆检测参数
            roi_radius = min(roi_width, roi_height) // 2
            min_radius = max(10, roi_radius // 10)  # 最小半径不小于10像素
            max_radius = roi_radius  # 最大半径不超过ROI的较小边的一半
            
            print(f"ROI区域: {roi_width}x{roi_height}, 面积: {roi_area}像素, 半径范围: {min_radius}-{max_radius}")
            
            # 提取ROI区域
            roi = frame[y_min:y_max, x_min:x_max]
            if roi.size == 0:
                print("ROI区域无效")
                return None
            
            # 检查图像通道数并转换为灰度图
            gray = None
            if len(roi.shape) == 2:
                # 已经是灰度图
                gray = roi
                print("输入图像已经是灰度图")
            elif len(roi.shape) == 3 and roi.shape[2] == 3:
                # 彩色图转灰度图
                gray = cv2.cvtColor(roi, cv2.COLOR_BGR2GRAY)
                print("已将彩色图转换为灰度图")
            else:
                print(f"不支持的图像格式: shape={roi.shape}")
                return None
            
            # 保存原始灰度图用于后续验证
            original_gray = gray.copy()
            
            # 方法1: 直接使用灰度图进行霍夫圆检测
            try:
                print("方法1: 直接使用灰度图进行霍夫圆检测")
                # 确保gray是单通道图像
                if len(gray.shape) != 2:
                    print(f"警告：gray不是单通道图像，shape={gray.shape}")
                    if len(gray.shape) == 3 and gray.shape[2] == 3:
                        gray = cv2.cvtColor(gray, cv2.COLOR_BGR2GRAY)
                        print("已将gray转换为单通道图像")
                
                # 使用固定的param2值，避免过度降低阈值
                circles = cv2.HoughCircles(
                    gray,
                    cv2.HOUGH_GRADIENT,
                    dp=1,
                    minDist=roi_radius//2,
                    param1=canny_high,
                    param2=circle_threshold,
                    minRadius=10,
                    maxRadius=roi_radius
                )
                
                if circles is not None:
                    print(f"方法1: 检测到 {len(circles[0])} 个圆")
                    # 验证检测到的圆的置信度
                    best_circle = None
                    best_confidence = 0
                    
                    for circle in circles[0]:
                        # 计算圆的置信度
                        confidence = self._calculate_circle_confidence(original_gray, circle)
                        print(f"圆 ({int(circle[0])}, {int(circle[1])}, {int(circle[2])}) 的置信度: {confidence}")
                        
                        if confidence > best_confidence:
                            best_confidence = confidence
                            best_circle = circle
                    
                    # 只有当置信度超过阈值时才返回结果
                    if best_confidence >= min_confidence:
                        center_x = int(best_circle[0]) + x_min
                        center_y = int(best_circle[1]) + y_min
                        radius = int(best_circle[2])
                        print(f"选择置信度为 {best_confidence} 的圆")
                        return {'center_x': center_x, 'center_y': center_y, 'radius': radius, 'confidence': best_confidence}
                    else:
                        print(f"最佳圆的置信度 {best_confidence} 低于阈值 {min_confidence}，认为是误检")
                else:
                    print("方法1: 未检测到圆")
            except Exception as e:
                print(f"方法1失败: {str(e)}")
            
            # 方法2: 使用自适应二值化和边缘检测
            try:
                print("方法2: 使用自适应二值化和边缘检测")
                
                # 使用自适应二值化
                binary = cv2.adaptiveThreshold(
                    gray, 
                    255, 
                    cv2.ADAPTIVE_THRESH_GAUSSIAN_C, 
                    cv2.THRESH_BINARY_INV, 
                    11, 
                    2
                )
                print("自适应二值化完成")
                
                # 形态学操作改善圆形
                kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (5,5))
                binary = cv2.morphologyEx(binary, cv2.MORPH_CLOSE, kernel)
                binary = cv2.morphologyEx(binary, cv2.MORPH_OPEN, kernel)
                print("形态学处理完成")
                
                # 边缘检测
                edges = cv2.Canny(binary, canny_low, canny_high)
                print(f"边缘检测完成，edges形状: {edges.shape}")
                
                # 霍夫圆变换检测圆形
                circles = cv2.HoughCircles(
                    edges,
                    cv2.HOUGH_GRADIENT,
                    dp=1,
                    minDist=roi_radius//2,
                    param1=canny_high,
                    param2=circle_threshold,
                    minRadius=10,
                    maxRadius=roi_radius
                )
                
                if circles is not None:
                    print(f"方法2: 检测到 {len(circles[0])} 个圆")
                    # 验证检测到的圆的置信度
                    best_circle = None
                    best_confidence = 0
                    
                    for circle in circles[0]:
                        # 计算圆的置信度
                        confidence = self._calculate_circle_confidence(original_gray, circle)
                        print(f"圆 ({int(circle[0])}, {int(circle[1])}, {int(circle[2])}) 的置信度: {confidence}")
                        
                        if confidence > best_confidence:
                            best_confidence = confidence
                            best_circle = circle
                    
                    # 只有当置信度超过阈值时才返回结果
                    if best_confidence >= min_confidence:
                        center_x = int(best_circle[0]) + x_min
                        center_y = int(best_circle[1]) + y_min
                        radius = int(best_circle[2])
                        print(f"选择置信度为 {best_confidence} 的圆")
                        return {'center_x': center_x, 'center_y': center_y, 'radius': radius, 'confidence': best_confidence}
                    else:
                        print(f"最佳圆的置信度 {best_confidence} 低于阈值 {min_confidence}，认为是误检")
                else:
                    print("方法2: 未检测到圆")
            except Exception as e:
                print(f"方法2失败: {str(e)}")
            
            print("所有方法都未能检测到有效的圆")
            return None
            
        except Exception as e:
            print(f"圆检测失败: {str(e)}")
            import traceback
            traceback.print_exc()
            return None
            
    def _calculate_circle_confidence(self, gray_img, circle):
        """计算检测到的圆的置信度
        
        参数:
            gray_img: 灰度图像
            circle: 检测到的圆 [x, y, r]
            
        返回:
            置信度分数 (0-100)
        """
        try:
            # 提取圆的参数
            center_x, center_y, radius = int(circle[0]), int(circle[1]), int(circle[2])
            height, width = gray_img.shape[:2]
            
            # 创建圆形掩码
            mask = np.zeros((height, width), dtype=np.uint8)
            cv2.circle(mask, (center_x, center_y), radius, 255, 1)
            
            # 计算边缘点数量
            edge_points = np.sum(mask > 0)
            if edge_points == 0:
                return 0
                
            # 使用Canny检测边缘
            edges = cv2.Canny(gray_img, 50, 150)
            
            # 计算圆周上有多少点与边缘重合
            matching_points = np.sum((mask > 0) & (edges > 0))
            
            # 计算置信度
            confidence = (matching_points / edge_points) * 100
            
            # 额外检查：圆内外的灰度差异
            # 创建内部和外部掩码
            inner_mask = np.zeros((height, width), dtype=np.uint8)
            outer_mask = np.zeros((height, width), dtype=np.uint8)
            
            # 内部区域（略小于检测到的圆）
            cv2.circle(inner_mask, (center_x, center_y), max(1, radius - 2), 255, -1)
            
            # 外部区域（略大于检测到的圆）
            cv2.circle(outer_mask, (center_x, center_y), radius + 2, 255, -1)
            outer_mask = outer_mask - cv2.circle(np.zeros_like(outer_mask), (center_x, center_y), radius, 255, -1)
            
            # 计算内部和外部区域的平均灰度
            inner_mean = np.mean(gray_img[inner_mask > 0]) if np.sum(inner_mask > 0) > 0 else 0
            outer_mean = np.mean(gray_img[outer_mask > 0]) if np.sum(outer_mask > 0) > 0 else 0
            
            # 计算灰度差异的绝对值
            gray_diff = abs(inner_mean - outer_mean)
            
            # 灰度差异越大，越可能是真实的圆
            gray_diff_score = min(100, gray_diff * 2)
            
            # 综合评分：边缘匹配度和灰度差异
            final_confidence = (confidence * 0.7) + (gray_diff_score * 0.3)
            
            return final_confidence
            
        except Exception as e:
            print(f"计算圆置信度失败: {str(e)}")
            return 0
    
    def _draw_circle_detect(self, frame, obj: DrawingObject):
        """绘制圆形检测"""
        try:
            if len(obj.points) >= 2:
                p1 = obj.points[0]
                p2 = obj.points[1]
                
                if obj.properties.get('is_roi', True):
                    # 绘制临时ROI圆形框
                    x1, y1 = p1.x(), p1.y()
                    x2, y2 = p2.x(), p2.y()
                    center_x = (x1 + x2) // 2
                    center_y = (y1 + y2) // 2
                    radius = int(np.sqrt((x2-x1)**2 + (y2-y1)**2) / 2)
                    
                    cv2.circle(frame,
                              (center_x, center_y),
                              radius,
                              obj.properties['color'],
                              2,
                              cv2.LINE_AA)
                elif obj.properties.get('circle_detected', False):  # 只有在确认检测到圆时才绘制
                    # 绘制检测到的圆
                    center_x = obj.properties.get('center_x', 0)
                    center_y = obj.properties.get('center_y', 0)
                    radius = obj.properties.get('radius', 0)
                    confidence = obj.properties.get('confidence', 0)
                    
                    # 绘制圆形
                    cv2.circle(frame,
                             (int(center_x), int(center_y)),
                             int(radius),
                             obj.properties['color'],
                             obj.properties['thickness'],
                             cv2.LINE_AA)
                    
                    # 绘制圆心十字标记
                    cross_size = 5
                    cv2.line(frame,
                            (int(center_x - cross_size), int(center_y)),
                            (int(center_x + cross_size), int(center_y)),
                            obj.properties['color'],
                            1,
                            cv2.LINE_AA)
                    cv2.line(frame,
                            (int(center_x), int(center_y - cross_size)),
                            (int(center_x), int(center_y + cross_size)),
                            obj.properties['color'],
                            1,
                            cv2.LINE_AA)
                    
                    # 显示半径信息
                    text = f"R={radius:.1f}px"
                    font = cv2.FONT_HERSHEY_SIMPLEX
                    font_scale = self._calculate_font_scale(frame.shape[0])  # 使用基于图像高度的字体大小
                    thickness = 2     # 增加字体粗细
                    padding = 10      # 增大背景padding
                    
                    # 获取文本大小
                    (text_width, text_height), baseline = cv2.getTextSize(text, font, font_scale, thickness)
                    
                    # 计算文本位置(在圆心右上方)
                    text_x = int(center_x + 10)
                    text_y = int(center_y - 10)
                    
                    # 绘制文本背景
                    padding = 6
                    cv2.rectangle(frame,
                                (text_x - padding, text_y - text_height - padding),
                                (text_x + text_width + padding, text_y + padding),
                                (0, 0, 0),
                                -1)
                    
                    # 绘制文本
                    cv2.putText(frame,
                              text,
                              (text_x, text_y),
                              font,
                              font_scale,
                              obj.properties['color'],
                              thickness,
                              cv2.LINE_AA)
        except Exception as e:
            print(f"绘制圆形检测时出错: {str(e)}")

    def _draw_point(self, frame, obj: DrawingObject):
        """绘制点"""
        if len(obj.points) >= 1:
            p = obj.points[0]
            
            # 如果图元被选中，先绘制高亮效果
            if obj.selected:
                cv2.circle(frame,
                          (p.x(), p.y()),
                          obj.properties.get('radius', 10) + 8,  # 增加高亮边缘的宽度
                          (0, 0, 255),  # 蓝色高亮
                          -1)  # 填充圆
            
            # 绘制点（实心圆）
            cv2.circle(frame,
                      (p.x(), p.y()),
                      obj.properties.get('radius', 10),  # 保持点的基础半径为10
                      obj.properties['color'],
                      -1)  # 填充圆
            
            # 显示坐标信息
            coord_text = f"({p.x()}, {p.y()})"
            
            # 绘制文本（带背景）
            font = cv2.FONT_HERSHEY_SIMPLEX
            font_scale = self._calculate_font_scale(frame.shape[0])  # 使用基于图像高度的字体大小
            thickness = 2
            padding = 8  # 增加内边距
            
            # 计算文本位置（移动到点的右上方）
            text_x = p.x() + 15  # 向右偏移15像素
            text_y = p.y() - 15  # 向上偏移15像素
            
            # 获取文本大小
            (text_width, text_height), baseline = cv2.getTextSize(coord_text, font, font_scale, thickness)
            
            # 绘制文本背景
            cv2.rectangle(frame,
                        (text_x - padding, text_y - text_height - padding),
                        (text_x + text_width + padding, text_y + padding + text_height//4),
                        (0, 0, 0),
                        -1)
            
            # 绘制文本
            cv2.putText(frame,
                      coord_text,
                      (text_x, text_y),
                      font,
                      font_scale,
                      obj.properties['color'],
                      thickness,
                      cv2.LINE_AA)

    def _draw_point_to_line(self, frame, obj: DrawingObject):
        """绘制点到线的垂直距离"""
        if len(obj.points) >= 3:
            point = obj.points[0]  # 点的坐标
            line_p1, line_p2 = obj.points[1], obj.points[2]  # 线的两个端点
            
            # 计算点到线的垂直距离
            # 使用点到直线距离公式：|Ax + By + C|/sqrt(A^2 + B^2)
            # 其中 Ax + By + C = 0 是直线方程
            A = line_p2.y() - line_p1.y()
            B = line_p1.x() - line_p2.x()
            C = line_p2.x() * line_p1.y() - line_p1.x() * line_p2.y()
            
            distance = abs(A * point.x() + B * point.y() + C) / np.sqrt(A * A + B * B)
            
            # 计算垂足坐标
            # 垂足公式：x = (B^2*x0 - A*B*y0 - A*C)/(A^2 + B^2)
            #          y = (A^2*y0 - A*B*x0 - B*C)/(A^2 + B^2)
            foot_x = (B * B * point.x() - A * B * point.y() - A * C) / (A * A + B * B)
            foot_y = (A * A * point.y() - A * B * point.x() - B * C) / (A * A + B * B)
            
            # 确定线条颜色 - 如果对象被选中，使用蓝色
            line_color = (0, 0, 255) if obj.selected else obj.properties.get('color', (0, 255, 0))
            
            # 绘制虚线
            if obj.properties.get('is_dashed', False):
                # 绘制虚线效果
                dash_length = 10  # 增加虚线长度
                gap_length = 10   # 增加间隔长度
                total_length = int(distance)
                num_segments = total_length // (dash_length + gap_length)
                
                # 计算单位向量
                dx = foot_x - point.x()
                dy = foot_y - point.y()
                length = np.sqrt(dx * dx + dy * dy)
                if length > 0:
                    dx, dy = dx / length, dy / length
                    
                    for i in range(num_segments):
                        start_dist = i * (dash_length + gap_length)
                        end_dist = start_dist + dash_length
                        
                        start_x = int(point.x() + dx * start_dist)
                        start_y = int(point.y() + dy * start_dist)
                        end_x = int(point.x() + dx * end_dist)
                        end_y = int(point.y() + dy * end_dist)
                        
                        cv2.line(frame,
                                (start_x, start_y),
                                (end_x, end_y),
                                line_color,
                                obj.properties.get('thickness', 2),
                                cv2.LINE_AA)
            
            # 绘制垂足点
            cv2.circle(frame,
                      (int(foot_x), int(foot_y)),
                      obj.properties.get('radius', 5),
                      (255, 0, 0),
                      -1)  # 填充圆

            # 显示距离文本
            if obj.properties.get('show_distance', False):
                distance_text = f"{distance:.1f}px"
                
                # 计算文本位置（在垂线中点）
                text_x = int((point.x() + foot_x) / 2)
                text_y = int((point.y() + foot_y) / 2)
                
                # 设置文本参数
                font = cv2.FONT_HERSHEY_SIMPLEX
                font_scale = self._calculate_font_scale(frame.shape[0])  # 使用基于图像高度的字体大小
                thickness = 2
                padding = 5
                
                # 获取文本大小
                (text_width, text_height), baseline = cv2.getTextSize(distance_text, font, font_scale, thickness)
                
                # 绘制文本背景
                cv2.rectangle(frame,
                            (text_x - padding, text_y - text_height - padding),
                            (text_x + text_width + padding, text_y + padding),
                            (0, 0, 0),
                            -1)
                
                # 绘制文本
                cv2.putText(frame,
                          distance_text,
                          (text_x, text_y),
                          font,
                          font_scale,
                          obj.properties.get('color', (0, 255, 0)),  # 使用与线条相同的颜色
                          thickness,
                          cv2.LINE_AA)

    def _draw_simple_circle(self, frame, obj: DrawingObject):
        """绘制简单圆的三个点和连接线"""
        # 绘制已有的点
        for point in obj.points:
            # 绘制点
            cv2.circle(frame, (point.x(), point.y()), 5, (0, 255, 0), -1)  # 实心点
            cv2.circle(frame, (point.x(), point.y()), 8, (0, 255, 0), 2)   # 空心圆环
            
            # 在点旁边显示序号
            font = cv2.FONT_HERSHEY_SIMPLEX
            cv2.putText(frame, str(obj.points.index(point) + 1), 
                       (point.x() + 15, point.y() + 15), 
                       font, 0.8, (0, 255, 0), 2)
        
        # 如果有多个点，绘制连接线
        if len(obj.points) >= 2:
            # 连接点
            for i in range(len(obj.points)-1):
                cv2.line(frame,
                        (obj.points[i].x(), obj.points[i].y()),
                        (obj.points[i+1].x(), obj.points[i+1].y()),
                        (0, 255, 0), 2, cv2.LINE_AA)

    def _draw_fine_circle(self, frame, obj: DrawingObject):
        """绘制精细圆的五个点和拟合圆"""
        # 绘制已有的点
        for point in obj.points:
            # 绘制点
            cv2.circle(frame, (point.x(), point.y()), 5, (0, 255, 0), -1)  # 实心点
            cv2.circle(frame, (point.x(), point.y()), 8, (0, 255, 0), 2)   # 空心圆环
            
            # 在点旁边显示序号
            font = cv2.FONT_HERSHEY_SIMPLEX
            cv2.putText(frame, str(obj.points.index(point) + 1), 
                       (point.x() + 15, point.y() + 15), 
                       font, 0.8, (0, 255, 0), 2)
        
        # 如果有多个点，绘制连接线
        if len(obj.points) >= 2:
            # 连接点
            for i in range(len(obj.points)-1):
                cv2.line(frame,
                        (obj.points[i].x(), obj.points[i].y()),
                        (obj.points[i+1].x(), obj.points[i+1].y()),
                        (0, 255, 0), 2, cv2.LINE_AA)
            
            # 如果是最后一个点，连接回第一个点形成闭环
            if len(obj.points) == 5:
                cv2.line(frame,
                        (obj.points[-1].x(), obj.points[-1].y()),
                        (obj.points[0].x(), obj.points[0].y()),
                        (0, 255, 0), 2, cv2.LINE_AA)
        
        # 如果已经拟合了圆，绘制圆
        if 'center' in obj.properties and 'radius' in obj.properties:
            center = obj.properties['center']
            radius = obj.properties['radius']
            
            # 绘制圆
            cv2.circle(frame, 
                      (int(center[0]), int(center[1])), 
                      int(radius), 
                      (0, 0, 255), 2, cv2.LINE_AA)
            
            # 绘制圆心
            cv2.circle(frame, 
                      (int(center[0]), int(center[1])), 
                      5, (0, 0, 255), -1)
            
            # 显示圆心坐标和半径
            text = f"Center: ({center[0]:.1f}, {center[1]:.1f}), R: {radius:.1f}"
            cv2.putText(frame, text, 
                       (int(center[0]) + 20, int(center[1]) - 20), 
                       cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2)

    def hit_test(self, point, tolerance=10):
        """测试点击是否命中图元"""
        hit_objects = []
        
        # 检查所有图元
        for obj in self.drawing_objects + self.detection_objects:
            if not obj.visible:
                continue
                
            try:
                # 根据图元类型进行命中测试
                if obj.type == DrawingType.POINT:
                    if self._hit_test_point(point, obj.points[0], tolerance):
                        hit_objects.append(obj)
                elif obj.type in [DrawingType.LINE, DrawingType.LINE_SEGMENT]:
                    # 确保有足够的点
                    if len(obj.points) >= 2:
                        if self._hit_test_line(point, obj.points[0], obj.points[1], tolerance):
                            hit_objects.append(obj)
                elif obj.type == DrawingType.CIRCLE:
                    if len(obj.points) >= 2:
                        if self._hit_test_circle(point, obj.points[0], obj.points[1], tolerance):
                            hit_objects.append(obj)
                elif obj.type == DrawingType.PARALLEL:
                    # 平行线测试
                    if len(obj.points) >= 2:
                        # 测试第一条线
                        if self._hit_test_line(point, obj.points[0], obj.points[1], tolerance):
                            hit_objects.append(obj)
                            continue
                            
                        # 如果有第二条线，测试第二条线
                        if len(obj.points) >= 3:
                            # 测试第二条线
                            p1, p2 = obj.points[0], obj.points[1]
                            p3 = obj.points[2]
                            
                            # 计算第二条线的方向向量（与第一条线平行）
                            dx = p2.x() - p1.x()
                            dy = p2.y() - p1.y()
                            
                            if dx != 0 or dy != 0:
                                # 计算单位向量
                                length = np.sqrt(dx*dx + dy*dy)
                                dx, dy = dx/length, dy/length
                                
                                # 计算第二条线的起点和终点
                                height, width = 10000, 10000  # 使用一个足够大的值
                                max_length = int(np.sqrt(width**2 + height**2))
                                
                                start_x2 = int(p3.x() - dx * max_length)
                                start_y2 = int(p3.y() - dy * max_length)
                                end_x2 = int(p3.x() + dx * max_length)
                                end_y2 = int(p3.y() + dy * max_length)
                                
                                # 测试第二条线
                                if self._hit_test_line(point, 
                                                     QPoint(start_x2, start_y2), 
                                                     QPoint(end_x2, end_y2), 
                                                     tolerance):
                                    hit_objects.append(obj)
                                    continue
                                
                                # 测试中线
                                # 计算中点
                                mid_x = (p1.x() + p3.x()) // 2
                                mid_y = (p1.y() + p3.y()) // 2
                                
                                # 计算中线的起点和终点
                                start_x_mid = int(mid_x - dx * max_length)
                                start_y_mid = int(mid_y - dy * max_length)
                                end_x_mid = int(mid_x + dx * max_length)
                                end_y_mid = int(mid_y + dy * max_length)
                                
                                # 测试中线
                                if self._hit_test_line(point, 
                                                     QPoint(start_x_mid, start_y_mid), 
                                                     QPoint(end_x_mid, end_y_mid), 
                                                     tolerance):
                                    # 如果点击了中线，创建一个虚拟的直线对象
                                    if 'midline_object' not in obj.properties:
                                        # 创建一个虚拟的直线对象，用于与其他图元交互
                                        midline_obj = DrawingObject(
                                            type=DrawingType.LINE,
                                            points=[QPoint(mid_x, mid_y), 
                                                   QPoint(mid_x + int(dx * 100), mid_y + int(dy * 100))],
                                            properties={'color': (255, 0, 0), 'thickness': 2, 
                                                       'is_midline': True, 'parent_object': obj}
                                        )
                                        obj.properties['midline_object'] = midline_obj
                                    
                                    # 返回中线对象
                                    hit_objects.append(obj.properties['midline_object'])
                                    continue
                elif obj.type == DrawingType.POINT_TO_LINE:
                    # 点到线的垂直距离测量
                    if len(obj.points) >= 3:
                        # 检查是否点击了点
                        if self._hit_test_point(point, obj.points[0], tolerance):
                            hit_objects.append(obj)
                            continue
                            
                        # 检查是否点击了线
                        line_p1, line_p2 = obj.points[1], obj.points[2]
                        if self._hit_test_line(point, line_p1, line_p2, tolerance):
                            hit_objects.append(obj)
                            continue
                            
                        # 计算点到线的垂足
                        A = line_p2.y() - line_p1.y()
                        B = line_p1.x() - line_p2.x()
                        C = line_p2.x() * line_p1.y() - line_p1.x() * line_p2.y()
                        
                        foot_x = (B * B * obj.points[0].x() - A * B * obj.points[0].y() - A * C) / (A * A + B * B)
                        foot_y = (A * A * obj.points[0].y() - A * B * obj.points[0].x() - B * C) / (A * A + B * B)
                        
                        # 检查是否点击了垂直距离线
                        foot_point = QPoint(int(foot_x), int(foot_y))
                        if self._hit_test_line(point, obj.points[0], foot_point, tolerance * 2):
                            hit_objects.append(obj)
                elif obj.type == DrawingType.POINT_TO_CIRCLE:
                    # 点到圆的垂直距离测量
                    if len(obj.points) >= 3:
                        # 检查是否点击了点
                        if self._hit_test_point(point, obj.points[0], tolerance):
                            hit_objects.append(obj)
                            continue
                            
                        # 检查是否点击了圆
                        circle_center = obj.points[1]
                        circle_radius_point = obj.points[2]
                        if self._hit_test_circle(point, circle_center, circle_radius_point, tolerance):
                            hit_objects.append(obj)
                            continue
                            
                        # 检查是否点击了垂直距离线
                        # 计算圆的半径
                        dx = circle_radius_point.x() - circle_center.x()
                        dy = circle_radius_point.y() - circle_center.y()
                        radius = np.sqrt(dx * dx + dy * dy)
                        
                        # 计算点到圆心的距离
                        dx_point = obj.points[0].x() - circle_center.x()
                        dy_point = obj.points[0].y() - circle_center.y()
                        distance_to_center = np.sqrt(dx_point * dx_point + dy_point * dy_point)
                        
                        # 计算垂足坐标
                        if distance_to_center > 0:
                            unit_x = dx_point / distance_to_center
                            unit_y = dy_point / distance_to_center
                            foot_x = circle_center.x() + unit_x * radius
                            foot_y = circle_center.y() + unit_y * radius
                            
                            # 检查是否点击了垂直距离线
                            if self._hit_test_line(point, obj.points[0], QPoint(int(foot_x), int(foot_y)), tolerance * 2):
                                hit_objects.append(obj)
                elif obj.type == DrawingType.LINE_SEGMENT_TO_CIRCLE:
                    # 线段到圆的垂直距离测量
                    if len(obj.points) >= 4:
                        # 检查是否点击了线段
                        line_p1, line_p2 = obj.points[0], obj.points[1]
                        if self._hit_test_line(point, line_p1, line_p2, tolerance):
                            hit_objects.append(obj)
                            continue
                            
                        # 检查是否点击了圆
                        circle_center = obj.points[2]
                        circle_radius_point = obj.points[3]
                        if self._hit_test_circle(point, circle_center, circle_radius_point, tolerance):
                            hit_objects.append(obj)
                            continue
                            
                        # 计算圆的半径
                        dx = circle_radius_point.x() - circle_center.x()
                        dy = circle_radius_point.y() - circle_center.y()
                        radius = np.sqrt(dx * dx + dy * dy)
                        
                        # 计算线段的方向向量
                        line_dx = line_p2.x() - line_p1.x()
                        line_dy = line_p2.y() - line_p1.y()
                        
                        # 计算圆心到线段的垂直距离
                        A = line_p2.y() - line_p1.y()
                        B = line_p1.x() - line_p2.x()
                        C = line_p2.x() * line_p1.y() - line_p1.x() * line_p2.y()
                        
                        # 计算圆心到线段的垂足
                        foot_x = (B * B * circle_center.x() - A * B * circle_center.y() - A * C) / (A * A + B * B)
                        foot_y = (A * A * circle_center.y() - A * B * circle_center.x() - B * C) / (A * A + B * B)
                        
                        # 检查垂足是否在线段上
                        on_segment = False
                        if min(line_p1.x(), line_p2.x()) <= foot_x <= max(line_p1.x(), line_p2.x()) and \
                           min(line_p1.y(), line_p2.y()) <= foot_y <= max(line_p1.y(), line_p2.y()):
                            if abs((foot_x - line_p1.x()) * line_dy - (foot_y - line_p1.y()) * line_dx) < 1e-6:
                                on_segment = True
                        
                        # 如果垂足不在线段上，检查是否点击了延长线
                        if not on_segment:
                            # 计算延长线的起点和终点
                            line_length = np.sqrt(line_dx * line_dx + line_dy * line_dy)
                            if line_length > 0:
                                unit_dx = line_dx / line_length
                                unit_dy = line_dy / line_length
                                
                                if np.dot([foot_x - line_p1.x(), foot_y - line_p1.y()], [unit_dx, unit_dy]) < 0:
                                    # 垂足在线段p1的反方向
                                    ext_line_start = line_p1
                                    ext_line_end = QPoint(int(foot_x), int(foot_y))
                                else:
                                    # 垂足在线段p2的方向
                                    ext_line_start = line_p2
                                    ext_line_end = QPoint(int(foot_x), int(foot_y))
                                
                                # 检查是否点击了延长线
                                if self._hit_test_line(point, ext_line_start, ext_line_end, tolerance * 2):
                                    hit_objects.append(obj)
                                    continue
                        
                        # 计算圆上的垂足点
                        dir_x = foot_x - circle_center.x()
                        dir_y = foot_y - circle_center.y()
                        dir_length = np.sqrt(dir_x * dir_x + dir_y * dir_y)
                        
                        if dir_length > 0:
                            dir_x, dir_y = dir_x / dir_length, dir_y / dir_length
                            circle_foot_x = circle_center.x() + dir_x * radius
                            circle_foot_y = circle_center.y() + dir_y * radius
                            
                            # 检查是否点击了垂直距离线
                            foot_point = QPoint(int(foot_x), int(foot_y))
                            circle_foot_point = QPoint(int(circle_foot_x), int(circle_foot_y))
                            if self._hit_test_line(point, foot_point, circle_foot_point, tolerance * 2):
                                hit_objects.append(obj)
                elif obj.type == DrawingType.LINE_TO_CIRCLE:
                    # 直线到圆的垂直距离测量
                    if len(obj.points) >= 4:
                        # 检查是否点击了直线
                        line_p1, line_p2 = obj.points[0], obj.points[1]
                        if self._hit_test_line(point, line_p1, line_p2, tolerance):
                            hit_objects.append(obj)
                            continue
                            
                        # 检查是否点击了圆
                        circle_center = obj.points[2]
                        circle_radius_point = obj.points[3]
                        if self._hit_test_circle(point, circle_center, circle_radius_point, tolerance):
                            hit_objects.append(obj)
                            continue
                            
                        # 计算圆的半径
                        dx = circle_radius_point.x() - circle_center.x()
                        dy = circle_radius_point.y() - circle_center.y()
                        radius = np.sqrt(dx * dx + dy * dy)
                        
                        # 计算直线的方向向量
                        line_dx = line_p2.x() - line_p1.x()
                        line_dy = line_p2.y() - line_p1.y()
                        
                        # 计算圆心到直线的垂直距离
                        A = line_p2.y() - line_p1.y()
                        B = line_p1.x() - line_p2.x()
                        C = line_p2.x() * line_p1.y() - line_p1.x() * line_p2.y()
                        
                        # 计算圆心到直线的垂足
                        foot_x = (B * B * circle_center.x() - A * B * circle_center.y() - A * C) / (A * A + B * B)
                        foot_y = (A * A * circle_center.y() - A * B * circle_center.x() - B * C) / (A * A + B * B)
                        
                        # 计算圆上的垂足点
                        dir_x = foot_x - circle_center.x()
                        dir_y = foot_y - circle_center.y()
                        dir_length = np.sqrt(dir_x * dir_x + dir_y * dir_y)
                        
                        if dir_length > 0:
                            dir_x, dir_y = dir_x / dir_length, dir_y / dir_length
                            circle_foot_x = circle_center.x() + dir_x * radius
                            circle_foot_y = circle_center.y() + dir_y * radius
                            
                            # 检查是否点击了垂直距离线
                            foot_point = QPoint(int(foot_x), int(foot_y))
                            circle_foot_point = QPoint(int(circle_foot_x), int(circle_foot_y))
                            if self._hit_test_line(point, foot_point, circle_foot_point, tolerance * 2):
                                hit_objects.append(obj)
                            
            except Exception as e:
                print(f"Hit test error for object {obj.type}: {str(e)}")
                continue
                
        return hit_objects

    def _hit_test_point(self, test_point, point, tolerance):
        """测试点是否在目标点附近"""
        try:
            p = np.array([test_point.x(), test_point.y()])
            target = np.array([point.x(), point.y()])
            
            # 计算两点之间的距离
            dist = np.linalg.norm(p - target)
            
            # 增加选中范围到20像素
            return dist <= 20  # 增加选中范围，使点更容易被选中
            
        except Exception as e:
            print(f"Point hit test error: {str(e)}")
            return False

    def _hit_test_line(self, point, line_start, line_end, tolerance):
        """测试点是否在线段附近"""
        try:
            p = np.array([point.x(), point.y()])
            p1 = np.array([line_start.x(), line_start.y()])
            p2 = np.array([line_end.x(), line_end.y()])
            
            # 计算线段向量和点到起点的向量
            line_vec = p2 - p1
            point_vec = p - p1
            
            # 计算线段长度
            line_length = np.linalg.norm(line_vec)
            if line_length < 1e-6:  # 避免除零错误
                return np.linalg.norm(point_vec) <= tolerance
            
            # 将线段向量单位化
            line_unit_vec = line_vec / line_length
            
            # 计算点到线的垂直距离
            dist = abs(np.cross(line_unit_vec, point_vec))
            
            # 对于直线，只需要检查垂直距离
            return dist <= tolerance
            
        except Exception as e:
            print(f"Line hit test error: {str(e)}")
            return False

    def _hit_test_circle(self, point, center, radius_point, tolerance):
        """测试点是否在圆的轮廓附近"""
        p = np.array([point.x(), point.y()])
        c = np.array([center.x(), center.y()])
        r = np.linalg.norm(np.array([radius_point.x(), radius_point.y()]) - c)
        
        dist_to_center = np.linalg.norm(p - c)
        return abs(dist_to_center - r) <= tolerance
        
    def select_object(self, obj, ctrl_pressed=False):
        """选择图元"""
        if not ctrl_pressed:
            # 如果没有按Ctrl，清除之前的选择
            for o in self.selected_objects:
                o.selected = False
            self.selected_objects.clear()
            
        # 检查是否是中线对象
        if obj.properties.get('is_midline', False):
            # 如果是中线对象，标记为选中
            obj.selected = True
            if obj not in self.selected_objects:
                self.selected_objects.append(obj)
                # 打印选中的图元类型
                print(f"选中了: 平行线中线")
        else:
            # 普通对象
            if obj not in self.selected_objects:
                obj.selected = True
                self.selected_objects.append(obj)
                # 打印选中的图元类型
                print(f"选中了: {obj.type.value}")  # .value 获取枚举的字符串值

    def delete_selected_objects(self):
        """删除选中的图元"""
        # 获取要删除的线段对象
        selected_segments = [obj for obj in self.selected_objects if obj.type == DrawingType.LINE_SEGMENT]
        
        # 从绘制列表中移除选中的对象
        self.drawing_objects = [obj for obj in self.drawing_objects 
                              if obj not in self.selected_objects]
        
        # 如果删除了线段，同时删除相关的角度测量对象
        if selected_segments:
            # 遍历所有绘制对象
            remaining_objects = []
            for obj in self.drawing_objects:
                # 如果是线段角度测量对象
                if obj.type == DrawingType.LINE_SEGMENT_ANGLE:
                    # 获取该角度测量对象使用的线段点
                    line1_p1, line1_p2 = obj.points[0], obj.points[1]  # 第一条线段
                    line2_p1, line2_p2 = obj.points[2], obj.points[3]  # 第二条线段
                    
                    # 检查是否使用了被删除的线段
                    should_keep = True
                    for segment in selected_segments:
                        segment_p1, segment_p2 = segment.points[0], segment.points[1]
                        # 如果角度测量对象使用了被删除的线段，则删除该角度测量对象
                        if ((line1_p1 == segment_p1 and line1_p2 == segment_p2) or
                            (line2_p1 == segment_p1 and line2_p2 == segment_p2)):
                            should_keep = False
                            break
                    
                    if should_keep:
                        remaining_objects.append(obj)
                else:
                    remaining_objects.append(obj)
            
            self.drawing_objects = remaining_objects
        
        # 从检测列表中移除选中的对象
        self.detection_objects = [obj for obj in self.detection_objects 
                                if obj not in self.selected_objects]
        # 清空选中列表
        self.selected_objects.clear()

    def clear_selection(self):
        """清除所有选择"""
        for obj in self.selected_objects:
            obj.selected = False
        self.selected_objects.clear()

    def _calculate_circle_from_five_points(self, points):
        """使用最小二乘法从五个点拟合圆"""
        try:
            # 提取五个点的坐标
            x = np.array([point.x() for point in points])
            y = np.array([point.y() for point in points])
            
            # 构建最小二乘法方程组
            # 对于圆方程 (x-h)^2 + (y-k)^2 = r^2
            # 可以转换为 x^2 + y^2 = 2hx + 2ky + (r^2 - h^2 - k^2)
            # 令 A = 2h, B = 2k, C = r^2 - h^2 - k^2
            # 则方程变为 x^2 + y^2 = Ax + By + C
            
            # 构建系数矩阵
            A = np.column_stack((x, y, np.ones(len(x))))
            
            # 构建常数项
            b = x**2 + y**2
            
            # 求解线性方程组
            try:
                # 使用最小二乘法求解
                params, residuals, rank, s = np.linalg.lstsq(A, b, rcond=None)
                
                # 提取参数
                a, b, c = params
                
                # 计算圆心和半径
                h = a / 2
                k = b / 2
                r = np.sqrt(c + h**2 + k**2)
                
                return (h, k), r
            except np.linalg.LinAlgError:
                print("线性代数错误，无法拟合圆")
                return None
        except Exception as e:
            print(f"拟合圆参数时出错: {str(e)}")
            return None

    def _draw_point_to_circle(self, frame, obj: DrawingObject):
        """绘制点到圆的垂直距离"""
        try:
            if len(obj.points) >= 3:
                point = obj.points[0]  # 点的坐标
                circle_center = obj.points[1]  # 圆心坐标
                circle_radius_point = obj.points[2]  # 圆上一点，用于计算半径
                
                # 计算圆的半径
                dx = circle_radius_point.x() - circle_center.x()
                dy = circle_radius_point.y() - circle_center.y()
                radius = np.sqrt(dx * dx + dy * dy)
                
                # 计算点到圆心的距离
                dx_point = point.x() - circle_center.x()
                dy_point = point.y() - circle_center.y()
                distance_to_center = np.sqrt(dx_point * dx_point + dy_point * dy_point)
                
                # 计算点到圆的垂直距离
                distance_to_circle = abs(distance_to_center - radius)
                
                # 计算点到圆心的单位向量
                if distance_to_center > 0:
                    unit_x = dx_point / distance_to_center
                    unit_y = dy_point / distance_to_center
                else:
                    unit_x, unit_y = 0, 0
                
                # 计算垂足坐标（点在圆内或圆外）
                if distance_to_center > radius:  # 点在圆外
                    # 垂足在圆上，沿着点到圆心的方向
                    foot_x = circle_center.x() + unit_x * radius
                    foot_y = circle_center.y() + unit_y * radius
                else:  # 点在圆内或圆上
                    # 垂足在圆上，沿着圆心到点的方向
                    foot_x = circle_center.x() + unit_x * radius
                    foot_y = circle_center.y() + unit_y * radius
                
                # 确定线条颜色 - 如果对象被选中，使用蓝色
                line_color = (0, 0, 255) if obj.selected else obj.properties.get('color', (0, 255, 0))
                
                # 绘制虚线
                if obj.properties.get('is_dashed', True):  # 默认使用虚线
                    # 绘制虚线效果
                    dash_length = 10
                    gap_length = 10
                    total_length = int(distance_to_circle)
                    num_segments = max(1, total_length // (dash_length + gap_length))
                    
                    # 计算单位向量（从点到垂足）
                    dx_line = foot_x - point.x()
                    dy_line = foot_y - point.y()
                    line_length = np.sqrt(dx_line * dx_line + dy_line * dy_line)
                    
                    if line_length > 0:
                        dx_line, dy_line = dx_line / line_length, dy_line / line_length
                        
                        for i in range(num_segments):
                            start_dist = i * (dash_length + gap_length)
                            end_dist = min(start_dist + dash_length, line_length)
                            
                            if end_dist <= start_dist:
                                break
                                
                            start_x = int(point.x() + dx_line * start_dist)
                            start_y = int(point.y() + dy_line * start_dist)
                            end_x = int(point.x() + dx_line * end_dist)
                            end_y = int(point.y() + dy_line * end_dist)
                            
                            cv2.line(frame, (start_x, start_y), (end_x, end_y), 
                                    line_color, 
                                    obj.properties.get('thickness', 2))
                else:
                    # 绘制实线
                    cv2.line(frame, 
                            (point.x(), point.y()), 
                            (int(foot_x), int(foot_y)), 
                            line_color, 
                            obj.properties.get('thickness', 2))
                
                # 绘制垂足点
                cv2.circle(frame,
                          (int(foot_x), int(foot_y)),
                          obj.properties.get('radius', 5),
                          (255, 0, 0),
                          -1)  # 填充圆

                # 显示距离信息
                if obj.properties.get('show_distance', True):
                    # 计算文本位置（在连线中点附近）
                    text_x = int((point.x() + foot_x) / 2)
                    text_y = int((point.y() + foot_y) / 2)
                    
                    # 格式化距离文本
                    distance_text = f"{distance_to_circle:.2f}px"
                    
                    # 绘制文本（带背景）
                    font = cv2.FONT_HERSHEY_SIMPLEX
                    font_scale = self._calculate_font_scale(frame.shape[0])  # 使用基于图像高度的字体大小
                    thickness = 2
                    padding = 5
                    
                    # 获取文本大小
                    (text_width, text_height), baseline = cv2.getTextSize(distance_text, font, font_scale, thickness)
                    
                    # 绘制文本背景
                    cv2.rectangle(frame,
                                (text_x - padding, text_y - text_height - padding),
                                (text_x + text_width + padding, text_y + padding),
                                (0, 0, 0),
                                -1)
                    
                    # 绘制文本
                    cv2.putText(frame,
                              distance_text,
                              (text_x, text_y),
                              font,
                              font_scale,
                              obj.properties.get('color', (0, 255, 0)),
                              thickness,
                              cv2.LINE_AA)
        except Exception as e:
            print(f"绘制点到圆的垂直距离时出错: {str(e)}")

    def _draw_line_segment_to_circle(self, frame, obj: DrawingObject):
        """绘制线段到圆的垂直距离"""
        try:
            if len(obj.points) >= 4:
                # 线段的两个端点
                line_p1, line_p2 = obj.points[0], obj.points[1]
                # 圆心和圆上一点
                circle_center = obj.points[2]
                circle_radius_point = obj.points[3]
                
                # 确定线条颜色 - 如果对象被选中，使用蓝色
                line_color = (0, 0, 255) if obj.selected else obj.properties.get('color', (0, 255, 0))
                
                # 计算圆的半径
                dx = circle_radius_point.x() - circle_center.x()
                dy = circle_radius_point.y() - circle_center.y()
                radius = np.sqrt(dx * dx + dy * dy)
                
                # 绘制圆
                cv2.circle(frame, 
                          (circle_center.x(), circle_center.y()), 
                          int(radius), 
                          line_color, 
                          obj.properties.get('thickness', 2))
                
                # 绘制线段
                cv2.line(frame, 
                        (line_p1.x(), line_p1.y()), 
                        (line_p2.x(), line_p2.y()), 
                        line_color, 
                        obj.properties.get('thickness', 2))
                
                # 计算线段的方向向量
                line_dx = line_p2.x() - line_p1.x()
                line_dy = line_p2.y() - line_p1.y()
                line_length = np.sqrt(line_dx * line_dx + line_dy * line_dy)
                
                if line_length > 0:
                    # 单位方向向量
                    unit_dx = line_dx / line_length
                    unit_dy = line_dy / line_length
                    
                    # 计算圆心到线段的垂直距离
                    # 使用点到直线距离公式：|Ax + By + C|/sqrt(A^2 + B^2)
                    # 其中 Ax + By + C = 0 是直线方程
                    A = line_p2.y() - line_p1.y()
                    B = line_p1.x() - line_p2.x()
                    C = line_p2.x() * line_p1.y() - line_p1.x() * line_p2.y()
                    
                    distance_to_line = abs(A * circle_center.x() + B * circle_center.y() + C) / np.sqrt(A * A + B * B)
                    
                    # 计算圆心到线段的垂足
                    # 垂足公式：x = (B^2*x0 - A*B*y0 - A*C)/(A^2 + B^2)
                    #          y = (A^2*y0 - A*B*x0 - B*C)/(A^2 + B^2)
                    foot_x = (B * B * circle_center.x() - A * B * circle_center.y() - A * C) / (A * A + B * B)
                    foot_y = (A * A * circle_center.y() - A * B * circle_center.x() - B * C) / (A * A + B * B)
                    
                    # 检查垂足是否在线段上
                    # 如果不在线段上，找到最近的线段端点
                    on_segment = False
                    
                    # 检查垂足是否在线段上
                    if min(line_p1.x(), line_p2.x()) <= foot_x <= max(line_p1.x(), line_p2.x()) and \
                       min(line_p1.y(), line_p2.y()) <= foot_y <= max(line_p1.y(), line_p2.y()):
                        # 额外检查垂足是否真的在线段上（处理斜线情况）
                        if abs((foot_x - line_p1.x()) * line_dy - (foot_y - line_p1.y()) * line_dx) < 1e-6:
                            on_segment = True
                    
                    # 如果垂足不在线段上，延长线段
                    if not on_segment:
                        # 绘制延长线（虚线）
                        # 计算延长线的起点和终点
                        if np.dot([foot_x - line_p1.x(), foot_y - line_p1.y()], [unit_dx, unit_dy]) < 0:
                            # 垂足在线段p1的反方向，从p1延长
                            ext_line_start = (line_p1.x(), line_p1.y())
                            ext_line_end = (int(foot_x), int(foot_y))
                        else:
                            # 垂足在线段p2的方向，从p2延长
                            ext_line_start = (line_p2.x(), line_p2.y())
                            ext_line_end = (int(foot_x), int(foot_y))
                        
                        # 绘制延长线（虚线）
                        dash_length = 5
                        gap_length = 5
                        ext_dx = ext_line_end[0] - ext_line_start[0]
                        ext_dy = ext_line_end[1] - ext_line_start[1]
                        ext_length = np.sqrt(ext_dx * ext_dx + ext_dy * ext_dy)
                        
                        if ext_length > 0:
                            ext_unit_dx = ext_dx / ext_length
                            ext_unit_dy = ext_dy / ext_length
                            
                            num_segments = int(ext_length / (dash_length + gap_length)) + 1
                            
                            for i in range(num_segments):
                                start_dist = i * (dash_length + gap_length)
                                end_dist = min(start_dist + dash_length, ext_length)
                                
                                if end_dist <= start_dist:
                                    break
                                    
                                dash_start_x = int(ext_line_start[0] + ext_unit_dx * start_dist)
                                dash_start_y = int(ext_line_start[1] + ext_unit_dy * start_dist)
                                dash_end_x = int(ext_line_start[0] + ext_unit_dx * end_dist)
                                dash_end_y = int(ext_line_start[1] + ext_unit_dy * end_dist)
                                
                                cv2.line(frame, 
                                        (dash_start_x, dash_start_y), 
                                        (dash_end_x, dash_end_y), 
                                        line_color, 
                                        obj.properties.get('thickness', 1),
                                        cv2.LINE_AA)
                    
                    # 计算圆上的垂足点
                    # 从圆心到线段垂足的方向向量
                    dir_x = foot_x - circle_center.x()
                    dir_y = foot_y - circle_center.y()
                    dir_length = np.sqrt(dir_x * dir_x + dir_y * dir_y)
                    
                    if dir_length > 0:
                        dir_x, dir_y = dir_x / dir_length, dir_y / dir_length
                        
                        # 计算圆上的垂足点
                        circle_foot_x = circle_center.x() + dir_x * radius
                        circle_foot_y = circle_center.y() + dir_y * radius
                        
                        # 计算线段到圆的垂直距离
                        distance_to_circle = abs(distance_to_line - radius)
                        
                        # 绘制垂线（从线段垂足到圆上垂足）
                        if obj.properties.get('is_dashed', True):
                            # 绘制虚线效果
                            dash_length = 10
                            gap_length = 10
                            total_length = int(distance_to_circle)
                            num_segments = max(1, total_length // (dash_length + gap_length))
                            
                            # 计算单位向量（从线段垂足到圆上垂足）
                            perp_dx = circle_foot_x - foot_x
                            perp_dy = circle_foot_y - foot_y
                            perp_length = np.sqrt(perp_dx * perp_dx + perp_dy * perp_dy)
                            
                            if perp_length > 0:
                                perp_dx, perp_dy = perp_dx / perp_length, perp_dy / perp_length
                                
                                for i in range(num_segments):
                                    start_dist = i * (dash_length + gap_length)
                                    end_dist = min(start_dist + dash_length, perp_length)
                                    
                                    if end_dist <= start_dist:
                                        break
                                        
                                    start_x = int(foot_x + perp_dx * start_dist)
                                    start_y = int(foot_y + perp_dy * start_dist)
                                    end_x = int(foot_x + perp_dx * end_dist)
                                    end_y = int(foot_y + perp_dy * end_dist)
                                    
                                    cv2.line(frame, (start_x, start_y), (end_x, end_y), 
                                            line_color, 
                                            obj.properties.get('thickness', 2))
                        else:
                            # 绘制实线
                            cv2.line(frame, 
                                    (int(foot_x), int(foot_y)), 
                                    (int(circle_foot_x), int(circle_foot_y)), 
                                    line_color, 
                                    obj.properties.get('thickness', 2))
                        
                        # 绘制垂足点
                        cv2.circle(frame,
                                  (int(foot_x), int(foot_y)),
                                  obj.properties.get('radius', 5),
                                  (255, 0, 0),
                                  -1)  # 填充圆
                        
                        # 绘制圆上的垂足点
                        cv2.circle(frame,
                                  (int(circle_foot_x), int(circle_foot_y)),
                                  obj.properties.get('radius', 5),
                                  line_color,
                                  -1)  # 填充圆

                        # 显示距离信息
                        if obj.properties.get('show_distance', True):
                            # 计算文本位置（在连线中点附近）
                            text_x = int((foot_x + circle_foot_x) / 2)
                            text_y = int((foot_y + circle_foot_y) / 2)
                            
                            # 格式化距离文本
                            distance_text = f"{distance_to_circle:.2f}px"
                            
                            # 绘制文本（带背景）
                            font = cv2.FONT_HERSHEY_SIMPLEX
                            font_scale = self._calculate_font_scale(frame.shape[0])  # 使用基于图像高度的字体大小
                            thickness = 2
                            padding = 5
                            
                            # 获取文本大小
                            (text_width, text_height), baseline = cv2.getTextSize(distance_text, font, font_scale, thickness)
                            
                            # 绘制文本背景
                            cv2.rectangle(frame,
                                        (text_x - padding, text_y - text_height - padding),
                                        (text_x + text_width + padding, text_y + padding),
                                        (0, 0, 0),
                                        -1)
                            
                            # 绘制文本
                            cv2.putText(frame,
                                      distance_text,
                                      (text_x, text_y),
                                      font,
                                      font_scale,
                                      obj.properties.get('color', (0, 255, 0)),
                                      thickness,
                                      cv2.LINE_AA)
                            
        except Exception as e:
            print(f"绘制线段到圆的垂直距离时出错: {str(e)}")

    def _draw_line_to_circle(self, frame, obj: DrawingObject):
        """绘制直线到圆的垂直距离虚线"""
        try:
            if len(obj.points) >= 4:
                # 直线的两个端点
                line_p1, line_p2 = obj.points[0], obj.points[1]
                # 圆心和圆上一点
                circle_center = obj.points[2]
                circle_radius_point = obj.points[3]
                
                # 计算圆的半径
                dx = circle_radius_point.x() - circle_center.x()
                dy = circle_radius_point.y() - circle_center.y()
                radius = np.sqrt(dx * dx + dy * dy)
                
                # 计算直线的方向向量
                line_dx = line_p2.x() - line_p1.x()
                line_dy = line_p2.y() - line_p1.y()
                
                # 计算圆心到直线的垂直距离
                # 使用点到直线距离公式：|Ax + By + C|/sqrt(A^2 + B^2)
                # 其中 Ax + By + C = 0 是直线方程
                A = line_p2.y() - line_p1.y()
                B = line_p1.x() - line_p2.x()
                C = line_p2.x() * line_p1.y() - line_p1.x() * line_p2.y()
                
                distance_to_line = abs(A * circle_center.x() + B * circle_center.y() + C) / np.sqrt(A * A + B * B)
                
                # 计算圆心到直线的垂足
                # 垂足公式：x = (B^2*x0 - A*B*y0 - A*C)/(A^2 + B^2)
                #          y = (A^2*y0 - A*B*x0 - B*C)/(A^2 + B^2)
                foot_x = (B * B * circle_center.x() - A * B * circle_center.y() - A * C) / (A * A + B * B)
                foot_y = (A * A * circle_center.y() - A * B * circle_center.x() - B * C) / (A * A + B * B)
                
                # 计算圆上的垂足点
                # 从圆心到直线垂足的方向向量
                dir_x = foot_x - circle_center.x()
                dir_y = foot_y - circle_center.y()
                dir_length = np.sqrt(dir_x * dir_x + dir_y * dir_y)
                
                if dir_length > 0:
                    dir_x, dir_y = dir_x / dir_length, dir_y / dir_length
                    
                    # 计算圆上的垂足点
                    circle_foot_x = circle_center.x() + dir_x * radius
                    circle_foot_y = circle_center.y() + dir_y * radius
                    
                    # 计算直线到圆的垂直距离
                    distance_to_circle = abs(distance_to_line - radius)

                    # 确定线条颜色 - 如果对象被选中，使用蓝色
                    line_color = (0, 0, 255) if obj.selected else obj.properties.get('color', (0, 255, 0))
                    
                    # 绘制垂直距离虚线
                    if obj.properties.get('is_dashed', True):  # 默认使用虚线
                        # 绘制虚线效果
                        dash_length = 10
                        gap_length = 10
                        total_length = int(distance_to_circle)
                        num_segments = max(1, total_length // (dash_length + gap_length))
                        
                        # 计算单位向量（从直线垂足到圆上垂足）
                        perp_dx = circle_foot_x - foot_x
                        perp_dy = circle_foot_y - foot_y
                        perp_length = np.sqrt(perp_dx * perp_dx + perp_dy * perp_dy)
                        
                        if perp_length > 0:
                            perp_dx, perp_dy = perp_dx / perp_length, perp_dy / perp_length
                            
                            for i in range(num_segments):
                                start_dist = i * (dash_length + gap_length)
                                end_dist = min(start_dist + dash_length, perp_length)
                                
                                if end_dist <= start_dist:
                                    break
                                    
                                start_x = int(foot_x + perp_dx * start_dist)
                                start_y = int(foot_y + perp_dy * start_dist)
                                end_x = int(foot_x + perp_dx * end_dist)
                                end_y = int(foot_y + perp_dy * end_dist)
                                
                                cv2.line(frame, (start_x, start_y), (end_x, end_y), 
                                        line_color, 
                                        obj.properties.get('thickness', 2))
                    else:
                        # 绘制实线
                        cv2.line(frame, 
                                (int(foot_x), int(foot_y)), 
                                (int(circle_foot_x), int(circle_foot_y)), 
                                line_color, 
                                obj.properties.get('thickness', 2))
                    
                    # 显示距离信息
                    if obj.properties.get('show_distance', True):
                        # 计算文本位置（在连线中点附近）
                        text_x = int((foot_x + circle_foot_x) / 2)
                        text_y = int((foot_y + circle_foot_y) / 2)
                        
                        # 格式化距离文本
                        distance_text = f"{distance_to_circle:.2f}px"
                        
                        # 绘制文本（带背景）
                        font = cv2.FONT_HERSHEY_SIMPLEX
                        font_scale = self._calculate_font_scale(frame.shape[0])  # 使用基于图像高度的字体大小
                        thickness = 2
                        padding = 5
                        
                        # 获取文本大小
                        (text_width, text_height), baseline = cv2.getTextSize(distance_text, font, font_scale, thickness)
                        
                        # 绘制文本背景
                        cv2.rectangle(frame,
                                    (text_x - padding, text_y - text_height - padding),
                                    (text_x + text_width + padding, text_y + padding),
                                    (0, 0, 0),
                                    -1)
                        
                        # 绘制文本
                        cv2.putText(frame,
                                  distance_text,
                                  (text_x, text_y),
                                  font,
                                  font_scale,
                                  obj.properties.get('color', (0, 255, 0)),
                                  thickness,
                                  cv2.LINE_AA)
        except Exception as e:
            print(f"绘制直线到圆的垂直距离时出错: {str(e)}")

    def _calculate_font_scale(self, height):
        """根据图像高度计算字体大小"""
        font_scale = max(0.9, height / 70 / 30)  # 除以30是因为OpenCV字体基础大小约为30像素
        return font_scale

class MeasurementManager(QObject):
    def __init__(self, parent=None):
        super().__init__(parent)
        # parent 现在是 DrawingManager 实例，直接从它获取 log_manager
        self.log_manager = parent.log_manager if parent else None
        self.layer_manager = LayerManager()
        self.drawing = False
        self.draw_mode = None
        self.drawing_history = []  # 添加绘画历史列表
        self.is_main_view = False  # 添加标志，指示是否为主界面视图

    def set_main_view(self, is_main):
        """设置是否为主界面视图"""
        self.is_main_view = is_main

    def start_line_measurement(self):
        """启动直线测量模式"""
        self.draw_mode = DrawingType.LINE
        self.drawing = False

    def start_circle_measurement(self):
        """启动圆形测量模式"""
        self.draw_mode = DrawingType.CIRCLE
        self.drawing = False

    def start_line_segment_measurement(self):
        """启动线段测量模式"""
        self.draw_mode = DrawingType.LINE_SEGMENT
        self.drawing = False

    def start_parallel_measurement(self):
        """启动平行线测量模式"""
        self.draw_mode = DrawingType.PARALLEL
        self.drawing = False

    def start_circle_line_measurement(self):
        """启动圆线距离测量模式"""
        self.draw_mode = DrawingType.CIRCLE_LINE
        self.drawing = False

    def start_two_lines_measurement(self):
        """启动两线测量模式"""
        self.draw_mode = DrawingType.TWO_LINES
        self.drawing = False

    def start_line_detection(self):
        """启动直线检测模式"""
        self.draw_mode = DrawingType.LINE_DETECT
        self.drawing = False 
        
    def start_circle_detection(self):
        """启动圆形检测模式"""
        self.draw_mode = DrawingType.CIRCLE_DETECT
        self.drawing = False 

    def start_point_measurement(self):
        """启动点测量模式"""
        self.draw_mode = DrawingType.POINT
        self.drawing = False

    def start_simple_circle_measurement(self):
        """启动简单圆测量模式"""
        self.draw_mode = DrawingType.SIMPLE_CIRCLE
        self.drawing = False

    def start_fine_circle_measurement(self):
        """启动精细圆测量模式"""
        self.draw_mode = DrawingType.FINE_CIRCLE
        self.drawing = False

    def start_line_segment_angle_measurement(self):
        """启动线段角度测量模式"""
        self.draw_mode = DrawingType.LINE_SEGMENT_ANGLE
        self.drawing = False

    def clear_measurements(self):
        """清空所有测量"""
        self.draw_mode = None
        self.drawing = False
        self.layer_manager.clear()  # 保留此方法用于完全清空
        
    def clear_drawings(self):
        """清除所有手动绘制"""
        # 保留检测对象
        self.drawing_objects = [obj for obj in self.drawing_objects if obj.properties.get('is_detection', False)]
        self.current_object = None
        
    def undo_last_drawing(self):
        """撤销上一步绘制"""
        if self.layer_manager.undo_last_drawing():
            return self.layer_manager.render_frame(self.parent().get_current_frame() if self.parent() else None)
        return None

    def handle_mouse_press(self, event_pos, current_frame):
        """处理鼠标按下事件"""
        # 检查是否是右键点击
        if QApplication.mouseButtons() & Qt.RightButton:
            # 如果正在绘制，退出绘制状态
            if self.drawing or self.draw_mode:
                self.exit_drawing_mode()
                return self.layer_manager.render_frame(current_frame)
            return None
        
        # 左键点击的原有逻辑
        if not self.draw_mode and not self.drawing and not self.is_main_view:
            hit_objects = self.layer_manager.hit_test(event_pos)
            if hit_objects:
                ctrl_pressed = QApplication.keyboardModifiers() & Qt.ControlModifier
                self.layer_manager.select_object(hit_objects[0], ctrl_pressed)
                return self.layer_manager.render_frame(current_frame)
            else:
                if self.layer_manager.selected_objects:
                    self.layer_manager.clear_selection()
                    return self.layer_manager.render_frame(current_frame)
        
        # 如果在绘制模式下，执行正常的绘制操作
        if self.draw_mode:
            self.layer_manager.clear_selection()
            if self.draw_mode == DrawingType.SIMPLE_CIRCLE:
                # 简单圆模式不在鼠标按下时处理，而是在鼠标释放时处理
                return None
            elif self.draw_mode == DrawingType.FINE_CIRCLE:
                # 精细圆模式不在鼠标按下时处理，而是在鼠标释放时处理
                return None
            elif self.draw_mode == DrawingType.LINE_DETECT:
                # 直线检测模式
                if not self.layer_manager.current_object:
                    self.drawing = True
                    properties = {
                        'color': (255, 0, 255),  # RGB格式：亮紫色
                        'thickness': 2,
                        'line_detected': False,  # 标记是否已检测到直线
                        'is_roi': True  # 标记当前是ROI框阶段
                    }
                    self.layer_manager.start_drawing(self.draw_mode, properties)
                    self.layer_manager.current_object.points.append(event_pos)
                return self.layer_manager.render_frame(current_frame)
            elif self.draw_mode == DrawingType.CIRCLE_DETECT:
                # 圆形检测模式
                if not self.layer_manager.current_object:
                    self.drawing = True
                    properties = {
                        'color': (255, 0, 255),  # RGB格式：亮紫色
                        'thickness': 2
                    }
                    self.layer_manager.start_drawing(self.draw_mode, properties)
                    self.layer_manager.current_object.points.append(event_pos)
                return self.layer_manager.render_frame(current_frame)
            elif self.draw_mode == DrawingType.TWO_LINES:
                # 两线夹角测量模式
                if not self.layer_manager.current_object:
                    # 第一次按下：开始绘制第一条线
                    self.drawing = True
                    properties = {
                        'color': (0, 255, 0),  # RGB格式：绿色
                        'thickness': 2
                    }
                    self.layer_manager.start_drawing(self.draw_mode, properties)
                    self.layer_manager.current_object.points.append(event_pos)
                elif len(self.layer_manager.current_object.points) == 2:
                    # 第二次按下：开始绘制第二条线
                    self.drawing = True
                    self.layer_manager.current_object.points.append(event_pos)
                return self.layer_manager.render_frame(current_frame)
            elif self.draw_mode == DrawingType.CIRCLE_LINE:
                # 圆线距离测量模式
                if not self.layer_manager.current_object:
                    # 第一次按下：开始绘制圆（三点法）
                    self.drawing = True
                    properties = {
                        'color': (0, 255, 0),  # RGB格式：绿色
                        'thickness': 2,
                        'temp_points': [],  # 临时存储三个点
                        'drawing_stage': 'circle'  # 标记当前是圆绘制阶段
                    }
                    self.layer_manager.start_drawing(self.draw_mode, properties)
                    # 添加第一个点
                    self.layer_manager.current_object.properties['temp_points'].append(event_pos)
                elif self.layer_manager.current_object.properties.get('drawing_stage') == 'circle':
                    # 圆绘制阶段
                    temp_points = self.layer_manager.current_object.properties.get('temp_points', [])
                    if len(temp_points) < 3:
                        # 添加第二个或第三个点
                        temp_points.append(event_pos)
                        
                        # 如果已经有三个点，计算圆并更新当前对象
                        if len(temp_points) == 3:
                            circle_params = self.layer_manager._calculate_circle_from_three_points(temp_points)
                            if circle_params:
                                center, radius = circle_params
                                # 更新当前对象的点列表，使用计算出的圆心和半径点
                                self.layer_manager.current_object.points = [
                                    QPoint(int(center[0]), int(center[1])),  # 圆心
                                    QPoint(int(center[0] + radius), int(center[1]))  # 半径点
                                ]
                                # 切换到直线绘制阶段
                                self.layer_manager.current_object.properties['drawing_stage'] = 'line'
                                # 清除临时点
                                self.layer_manager.current_object.properties['temp_points'] = []
                            else:
                                # 如果计算失败，清除所有临时点，重新开始
                                self.layer_manager.current_object.properties['temp_points'] = []
                    
                elif self.layer_manager.current_object.properties.get('drawing_stage') == 'line':
                    # 直线绘制阶段
                    self.drawing = True
                    # 添加直线起点
                    self.layer_manager.current_object.points.append(event_pos)
                return self.layer_manager.render_frame(current_frame)
            elif self.draw_mode == DrawingType.PARALLEL:
                # 平行线测量模式
                if self.layer_manager.current_object and len(self.layer_manager.current_object.points) == 2:
                    self.layer_manager.update_current_object(event_pos)
                    self.layer_manager.commit_drawing()
                    return self.layer_manager.render_frame(current_frame)
                else:
                    self.drawing = True
                    properties = {
                        'color': (0, 255, 0),  # RGB格式：绿色
                        'thickness': 2
                    }
                    self.layer_manager.start_drawing(self.draw_mode, properties)
                    self.layer_manager.current_object.points.append(event_pos)
            elif self.draw_mode == DrawingType.POINT:
                # 点绘制模式
                self.drawing = True
                properties = {
                    'color': (0, 255, 0),  # RGB格式：绿色
                    'radius': 10,  # 增加点的默认半径到10
                }
                self.layer_manager.start_drawing(self.draw_mode, properties)
                self.layer_manager.current_object.points.append(event_pos)
                self.layer_manager.commit_drawing()  # 立即提交点的绘制
                self.drawing = False
                return self.layer_manager.render_frame(current_frame)
            else:
                # 其他模式正常处理
                self.drawing = True
                properties = {
                    'color': (0, 255, 0),  # RGB格式：绿色
                    'thickness': 2
                }
                self.layer_manager.start_drawing(self.draw_mode, properties)
                self.layer_manager.update_current_object(event_pos)
            
            return self.layer_manager.render_frame(current_frame)
        
    def handle_mouse_move(self, event_pos, current_frame):
        """处理鼠标移动事件"""
        if self.drawing:
            # 如果是简单圆模式，不需要在移动时更新
            if self.draw_mode == DrawingType.SIMPLE_CIRCLE:
                return None
                
            # 如果是精细圆模式，不需要在移动时更新
            if self.draw_mode == DrawingType.FINE_CIRCLE:
                return None
                
            if self.draw_mode == DrawingType.LINE_DETECT:
                # 直线检测模式 - 绘制ROI框
                if len(self.layer_manager.current_object.points) == 1:
                    self.layer_manager.current_object.points.append(event_pos)
                else:
                    self.layer_manager.current_object.points[1] = event_pos
            elif self.draw_mode == DrawingType.CIRCLE_DETECT:
                # 圆形检测模式 - 绘制圆形ROI
                if len(self.layer_manager.current_object.points) == 1:
                    self.layer_manager.current_object.points.append(event_pos)
                else:
                    self.layer_manager.current_object.points[1] = event_pos
            elif self.draw_mode == DrawingType.TWO_LINES:
                # 两线夹角测量模式
                current_obj = self.layer_manager.current_object
                if len(current_obj.points) <= 2:
                    # 第一阶段：绘制第一条线时的拖动
                    if len(current_obj.points) == 1:
                        current_obj.points.append(event_pos)
                    else:
                        current_obj.points[1] = event_pos
                elif len(current_obj.points) >= 3:
                    # 第二阶段：绘制第二条线时的拖动
                    if len(current_obj.points) == 3:
                        current_obj.points.append(event_pos)
                    else:
                        current_obj.points[3] = event_pos
            elif self.draw_mode == DrawingType.CIRCLE_LINE:
                # 圆线距离测量模式
                current_obj = self.layer_manager.current_object
                if current_obj.properties.get('drawing_stage') == 'circle':
                    # 圆绘制阶段，不需要在移动时更新
                    return None
                elif current_obj.properties.get('drawing_stage') == 'line':
                    # 直线绘制阶段
                    if len(current_obj.points) >= 3:
                        # 第二阶段：绘制直线时的拖动
                        if len(current_obj.points) == 3:
                            current_obj.points.append(event_pos)
                        else:
                            current_obj.points[3] = event_pos
                return self.layer_manager.render_frame(current_frame)
            elif self.draw_mode == DrawingType.PARALLEL:
                # 平行线测量模式
                if len(self.layer_manager.current_object.points) <= 2:
                    if len(self.layer_manager.current_object.points) == 1:
                        self.layer_manager.current_object.points.append(event_pos)
                    elif len(self.layer_manager.current_object.points) == 2:
                        self.layer_manager.current_object.points[1] = event_pos
                else:
                    # 其他模式：正常处理
                    self.layer_manager.update_current_object(event_pos)
            elif self.draw_mode == DrawingType.POINT:
                # 点不需要处理移动事件
                return None
            else:
                # 其他模式：正常处理
                self.layer_manager.update_current_object(event_pos)
            return self.layer_manager.render_frame(current_frame)
        return None
        
    def handle_mouse_release(self, event_pos, current_frame):
        """处理鼠标释放事件"""
        try:
            if self.draw_mode == DrawingType.SIMPLE_CIRCLE:
                # 如果是第一个点，创建新的绘制对象
                if not self.layer_manager.current_object:
                    properties = {
                        'color': (0, 255, 0),  # RGB格式：绿色
                        'thickness': 2
                    }
                    self.layer_manager.start_drawing(DrawingType.SIMPLE_CIRCLE, properties)
                
                # 添加新的点
                if len(self.layer_manager.current_object.points) < 3:
                    self.layer_manager.current_object.points.append(event_pos)
                    
                    # 如果已经有三个点，计算圆并创建
                    if len(self.layer_manager.current_object.points) == 3:
                        circle_params = self.layer_manager._calculate_circle_from_three_points(
                            self.layer_manager.current_object.points
                        )
                        if circle_params:
                            center, radius = circle_params
                            # 创建新的圆对象
                            circle_obj = DrawingObject(
                                type=DrawingType.CIRCLE,
                                points=[QPoint(int(center[0]), int(center[1])),
                                       QPoint(int(center[0] + radius), int(center[1]))],
                                properties={
                                    'color': (0, 255, 0),
                                    'thickness': 2,
                                    'radius': radius
                                }
                            )
                            # 添加圆对象并清除当前的三点对象
                            self.layer_manager.drawing_objects.append(circle_obj)
                            self.layer_manager.current_object = None
                            self.drawing = False
                            return self.layer_manager.render_frame(current_frame)
                    
                    return self.layer_manager.render_frame(current_frame)
                
                return None
            elif self.draw_mode == DrawingType.FINE_CIRCLE:
                print(f"精细圆鼠标释放: 当前点数={len(self.layer_manager.current_object.points) if self.layer_manager.current_object else 0}")  # 调试信息
                # 如果是第一个点，创建新的绘制对象
                if not self.layer_manager.current_object:
                    print("创建精细圆对象")  # 调试信息
                    properties = {
                        'color': (0, 255, 0),  # RGB格式：绿色
                        'thickness': 2
                    }
                    self.layer_manager.start_drawing(DrawingType.FINE_CIRCLE, properties)
                
                # 添加新的点
                if self.layer_manager.current_object and len(self.layer_manager.current_object.points) < 5:
                    print(f"添加点 {len(self.layer_manager.current_object.points) + 1}")  # 调试信息
                    self.layer_manager.current_object.points.append(event_pos)
                    
                    # 如果已经有五个点，计算圆并更新属性
                    if len(self.layer_manager.current_object.points) == 5:
                        print("开始拟合圆")  # 调试信息
                        circle_params = self.layer_manager._calculate_circle_from_five_points(
                            self.layer_manager.current_object.points
                        )
                        if circle_params:
                            center, radius = circle_params
                            # 创建新的圆对象
                            circle_obj = DrawingObject(
                                type=DrawingType.CIRCLE,
                                points=[QPoint(int(center[0]), int(center[1])),
                                       QPoint(int(center[0] + radius), int(center[1]))],
                                properties={
                                    'color': (0, 255, 0),
                                    'thickness': 2,
                                    'radius': radius
                                }
                            )
                            # 添加圆对象并清除当前的三点对象
                            self.layer_manager.drawing_objects.append(circle_obj)
                            self.layer_manager.current_object = None
                            self.drawing = False

                            # 获取当前视图名称
                            view_name = self.parent().active_view.objectName() if self.parent() and self.parent().active_view else "未知视图"
                            
                            # 记录日志
                            if self.log_manager:
                                self.log_manager.log_measurement_operation(
                                    f"精细圆拟合成功 - 视图: {view_name}",
                                    f"圆心: ({center[0]:.2f}, {center[1]:.2f}), 半径: {radius:.2f}"
                                )
                    
                    # 每次添加点后都重新渲染，以显示点和序号
                    return self.layer_manager.render_frame(current_frame)
                
                return None
            elif self.drawing:
                # 获取当前视图名称
                view_name = self.parent().active_view.objectName() if self.parent() and self.parent().active_view else "未知视图"
                if self.draw_mode == DrawingType.LINE_DETECT:
                    # 直线检测模式
                    if len(self.layer_manager.current_object.points) == 1:
                        self.layer_manager.current_object.points.append(event_pos)
                    else:
                        self.layer_manager.current_object.points[1] = event_pos
                
                    # 设置标记，准备进行直线检测
                    self.layer_manager.current_object.properties['is_roi'] = False
                    
                    # 立即进行直线检测 - 使用 layer_manager 的方法
                    detected_points = self.layer_manager._detect_line_in_roi(current_frame, 
                                                                           self.layer_manager.current_object.points)
                    if detected_points:
                        # 创建新的直线对象，类型为DrawingType.LINE而不是LINE_DETECT
                        line_obj = DrawingObject(
                            type=DrawingType.LINE,  # 将类型设置为LINE而不是LINE_DETECT
                            points=detected_points,
                            properties={
                                'color': (255, 0, 255),  # 紫色
                                'thickness': 2,
                                'line_detected': True,
                                'is_detection': True  # 添加检测标记
                            }
                        )
                        # 添加直线对象到绘制列表
                        self.layer_manager.drawing_objects.append(line_obj)
                        # 删除当前的ROI对象
                        self.layer_manager.current_object = None
                        
                        if self.log_manager:
                            self.log_manager.log_measurement_operation(
                                f"直线检测成功 - 视图: {view_name}",
                                f"检测到直线: ({detected_points[0].x()}, {detected_points[0].y()}) - "
                                f"({detected_points[1].x()}, {detected_points[1].y()})"
                            )
                    else:
                        if self.log_manager:
                            self.log_manager.log_measurement_operation(
                                f"直线检测失败 - 视图: {view_name}", 
                                "未在ROI区域内检测到直线"
                            )
                        # 在检测失败时也重置current_object，以便能够进行新的ROI框绘制
                        self.layer_manager.current_object = None
                    
                    self.drawing = False
                    return self.layer_manager.render_frame(current_frame)
                elif self.draw_mode == DrawingType.CIRCLE_DETECT:
                    # 圆形检测模式
                    if len(self.layer_manager.current_object.points) == 1:
                        self.layer_manager.current_object.points.append(event_pos)
                    else:
                        self.layer_manager.current_object.points[1] = event_pos
                
                    # 设置标记，准备进行圆形检测
                    self.layer_manager.current_object.properties['is_roi'] = False
                    
                    # 立即进行圆形检测
                    detected_circle = self.layer_manager._detect_circle_in_roi(current_frame, 
                                                                               self.layer_manager.current_object.points)
                    if detected_circle:
                        # 创建新的圆对象，类型为DrawingType.CIRCLE而不是CIRCLE_DETECT
                        center_x = detected_circle['center_x']
                        center_y = detected_circle['center_y']
                        radius = detected_circle['radius']
                        confidence = detected_circle.get('confidence', 0)
                        
                        circle_obj = DrawingObject(
                            type=DrawingType.CIRCLE,  # 将类型设置为CIRCLE而不是CIRCLE_DETECT
                            points=[
                                QPoint(center_x, center_y),
                                QPoint(center_x + radius, center_y)  # 用于确定半径的点
                            ],
                            properties={
                                'color': (255, 0, 255),  # 紫色
                                'thickness': 2,
                                'radius': radius,
                                'circle_detected': True,
                                'confidence': confidence,
                                'is_detection': True  # 添加检测标记
                            }
                        )
                        # 添加圆对象到绘制列表
                        self.layer_manager.drawing_objects.append(circle_obj)
                        # 删除当前的ROI对象
                        self.layer_manager.current_object = None

                        if self.log_manager:
                            self.log_manager.log_measurement_operation(
                                f"圆形检测成功 - 视图: {view_name}",
                                f"检测到圆形: 圆心({detected_circle['center_x']}, {detected_circle['center_y']}), "
                                f"半径: {detected_circle['radius']}, 置信度: {confidence:.1f}"
                            )
                        
                        print(f"更新绘制属性: center=({center_x}, {center_y}), radius={detected_circle['radius']}, 置信度: {confidence:.1f}")
                    else:
                        # 检测失败，显示提示信息
                        if self.log_manager:
                            self.log_manager.log_measurement_operation(
                                f"圆形检测失败 - 视图: {view_name}",
                                f"在选定区域内未检测到有效的圆形"
                            )
                        # 在检测失败时重置current_object，以便能够进行新的ROI框绘制
                        self.layer_manager.current_object = None
                
                    self.drawing = False
                    return self.layer_manager.render_frame(current_frame)
                elif self.draw_mode == DrawingType.TWO_LINES:
                    # 两线夹角测量模式
                    if len(self.layer_manager.current_object.points) <= 2:
                        # 第一条线绘制完成
                        if len(self.layer_manager.current_object.points) == 1:
                            self.layer_manager.current_object.points.append(event_pos)
                        else:
                            self.layer_manager.current_object.points[1] = event_pos
                        self.drawing = False
                        return self.layer_manager.render_frame(current_frame)
                    elif len(self.layer_manager.current_object.points) >= 3:
                        # 第二条线绘制完成
                        if len(self.layer_manager.current_object.points) == 3:
                            self.layer_manager.current_object.points.append(event_pos)
                        else:
                            self.layer_manager.current_object.points[3] = event_pos
                        self.layer_manager.commit_drawing()
                        self.drawing = False
                        if self.log_manager:
                            self.log_manager.log_measurement_operation(
                                f"两线夹角测量完成 - {view_name}", 
                                f"两线夹角测量完成 - 视图: {view_name}"
                            )
                        return self.layer_manager.render_frame(current_frame)
                elif self.draw_mode == DrawingType.CIRCLE_LINE:
                    # 圆线距离测量模式
                    current_obj = self.layer_manager.current_object
                    if current_obj.properties.get('drawing_stage') == 'circle':
                        # 圆绘制阶段，在handle_mouse_press中已处理
                        return self.layer_manager.render_frame(current_frame)
                    elif current_obj.properties.get('drawing_stage') == 'line':
                        # 直线绘制阶段
                        if len(current_obj.points) >= 3:
                            # 完成直线绘制
                            if len(current_obj.points) == 3:
                                current_obj.points.append(event_pos)
                            else:
                                current_obj.points[3] = event_pos
                            self.layer_manager.commit_drawing()
                            self.drawing = False
                            if self.log_manager:
                                self.log_manager.log_measurement_operation(
                                    f"圆线距离测量完成 - {view_name}", 
                                    f"圆线距离测量完成 - 视图: {view_name}"
                                )
                        return self.layer_manager.render_frame(current_frame)
                elif self.draw_mode == DrawingType.PARALLEL:
                    # 平行线测量模式
                    if len(self.layer_manager.current_object.points) <= 2:
                        print(f"平行线第一阶段: 点数={len(self.layer_manager.current_object.points)}")
                        if len(self.layer_manager.current_object.points) == 1:
                            self.layer_manager.current_object.points.append(event_pos)
                        else:
                            self.layer_manager.current_object.points[1] = event_pos
                        self.drawing = False
                        print("完成第一条线的绘制")
                        return self.layer_manager.render_frame(current_frame)
                    else:
                        print(f"平行线第二阶段: 点数={len(self.layer_manager.current_object.points)}")
                        # 当完成第三个点的绘制时，提交绘制并同步
                        if len(self.layer_manager.current_object.points) == 2:
                            print("添加第三个点")
                            self.layer_manager.current_object.points.append(event_pos)
                        else:
                            print("更新第三个点位置")
                            self.layer_manager.current_object.points[2] = event_pos
                        # 提交绘制
                        print("准备提交绘制")
                        self.layer_manager.commit_drawing()
                        self.drawing = False
                        # 记录日志
                        if self.log_manager:
                            view_name = self.parent().active_view.objectName() if self.parent() and self.parent().active_view else "未知视图"
                            print(f"当前视图: {view_name}")
                            self.log_manager.log_measurement_operation(
                                f"平行线测量完成 - {view_name}", 
                                f"平行线测量完成 - 视图: {view_name}"
                            )
                        # 同步到其他视图
                        parent = self.parent()
                        if parent and hasattr(parent, 'drawing_manager'):
                            print("找到 drawing_manager")
                            # 获取当前视图和对应的视图
                            current_view = parent.active_view
                            if current_view:
                                print(f"当前视图: {current_view.objectName()}")
                                paired_view = parent.drawing_manager.view_pairs.get(current_view)
                                if paired_view:
                                    print(f"找到对应视图: {paired_view.objectName()}")
                                    # 同步绘制对象
                                    parent.drawing_manager.sync_drawings(current_view, paired_view)
                                    print("完成同步绘制")
                                    # 强制更新两个视图
                                    current_view.update()
                                    paired_view.update()
                                else:
                                    print("未找到对应的视图")
                            else:
                                print("未找到当前视图")
                        else:
                            print("未找到 drawing_manager")
                        return self.layer_manager.render_frame(current_frame)
                elif self.draw_mode == DrawingType.LINE:
                    # 直线测量模式
                    if self.log_manager:
                        view_name = self.parent().active_view.objectName() if self.parent() and self.parent().active_view else "未知视图"
                        self.log_manager.log_measurement_operation(
                            f"直线测量完成 - {view_name}", 
                            f"直线测量完成 - 视图: {view_name}"
                        )
                elif self.draw_mode == DrawingType.LINE_SEGMENT:
                    # 线段测量模式
                    if self.log_manager:
                        view_name = self.parent().active_view.objectName() if self.parent() and self.parent().active_view else "未知视图"
                        self.log_manager.log_measurement_operation(
                            f"线段测量完成 - {view_name}", 
                            f"线段测量完成 - 视图: {view_name}"
                        )
                    
                    # 检查是否是从标定功能启动的线段绘制
                    # 如果是标定模式启动的，则在完成线段绘制后自动退出绘画模式
                    if hasattr(self, '_is_calibration_mode') and self._is_calibration_mode:
                        self.layer_manager.commit_drawing()
                        self.exit_drawing_mode()
                        if self.log_manager:
                            self.log_manager.log_measurement_operation("标定线段绘制完成，退出绘画模式")
                        self._is_calibration_mode = False
                        
                        # 弹出标定输入对话框
                        self._show_calibration_dialog()
                        
                elif self.draw_mode == DrawingType.CIRCLE:
                    # 圆形测量模式
                    if self.log_manager:
                        view_name = self.parent().active_view.objectName() if self.parent() and self.parent().active_view else "未知视图"
                        self.log_manager.log_measurement_operation(
                            f"圆形测量完成 - {view_name}", 
                            f"圆形测量完成 - 视图: {view_name}"
                        )
                self.layer_manager.commit_drawing()
                self.drawing = False
                return self.layer_manager.render_frame(current_frame)
        except Exception as e:
            print(f"Error in handle_mouse_release: {str(e)}")
        return None 

    def exit_drawing_mode(self):
        """退出绘画状态"""
        self.draw_mode = None
        self.drawing = False
        if hasattr(self, '_is_calibration_mode'):
            self._is_calibration_mode = False  # 清除标定模式标志
        
        if self.layer_manager.current_object:
            # 如果有未完成的绘制，取消它
            self.layer_manager.current_object = None
        
        # 恢复默认鼠标光标
        parent = self.parent()
        if parent and hasattr(parent, 'drawing_manager'):
            # 同步所有视图的状态
            for manager in parent.drawing_manager.measurement_managers.values():
                manager.draw_mode = None
                manager.drawing = False
                if manager.layer_manager.current_object:
                    manager.layer_manager.current_object = None
            
            # 恢复所有视图的鼠标光标
            for label in parent.drawing_manager.measurement_managers.keys():
                label.setCursor(Qt.ArrowCursor)

    def handle_key_press(self, event, current_frame):
        """处理按键事件"""
        if event.key() == Qt.Key_Escape:  # ESC键
            if self.drawing or self.draw_mode:
                self.exit_drawing_mode()
                return self.layer_manager.render_frame(current_frame)
        return None

    def start_calibration(self):
        """启动像素标定模式，使用线段进行标定"""
        self.draw_mode = DrawingType.LINE_SEGMENT
        self.drawing = False  # 初始设置为False，等待鼠标按下时设置为True
        self._is_calibration_mode = True  # 设置标定模式标志
        
        if self.log_manager:
            self.log_manager.log_drawing_operation("启动像素标定模式")
            print("启动像素标定模式，请在视图上绘制一条线段")
            
    def _show_calibration_dialog(self):
        """显示标定输入对话框"""
        from PyQt5.QtWidgets import QDialog, QVBoxLayout, QHBoxLayout, QLabel, QLineEdit, QPushButton, QMessageBox
        
        # 获取最后绘制的线段对象
        if not self.layer_manager.drawing_objects:
            return
            
        line_segment = self.layer_manager.drawing_objects[-1]
        if line_segment.type != DrawingType.LINE_SEGMENT or len(line_segment.points) < 2:
            return
            
        # 计算线段像素长度
        p1, p2 = line_segment.points[0], line_segment.points[1]
        pixel_length = ((p2.x() - p1.x())**2 + (p2.y() - p1.y())**2)**0.5
        
        # 创建对话框
        dialog = QDialog()
        dialog.setWindowTitle("像素标定")
        dialog.setMinimumWidth(300)
        
        # 创建布局
        layout = QVBoxLayout()
        
        # 添加说明标签
        info_label = QLabel(f"线段像素长度: {pixel_length:.2f}像素")
        layout.addWidget(info_label)
        
        # 添加输入提示和文本框
        input_layout = QHBoxLayout()
        input_label = QLabel("请输入实际长度(μm):")
        input_layout.addWidget(input_label)
        
        input_edit = QLineEdit()
        input_edit.setPlaceholderText("输入数值")
        input_layout.addWidget(input_edit)
        
        layout.addLayout(input_layout)
        
        # 添加确定按钮
        button_layout = QHBoxLayout()
        button_layout.addStretch()
        
        ok_button = QPushButton("确定")
        
        def handle_input():
            try:
                # 尝试将输入转换为浮点数
                value_text = input_edit.text()
                real_length = float(value_text)
                if real_length <= 0:
                    raise ValueError("长度必须大于0")
                    
                # 计算像素比例 (μm/pixel)
                scale = real_length / pixel_length
                
                # 记录标定结果
                if self.log_manager:
                    self.log_manager.log_measurement_operation(
                        "像素标定完成", 
                        f"标定结果: {scale:.6f} μm/pixel (像素长度: {pixel_length:.2f}px, 实际长度: {real_length:.2f}μm)"
                    )
                    
                # 只更新当前视图的像素比例，不保存到设置中
                self.layer_manager.set_pixel_scale(scale)
                
                # 更新当前视图中已存在图元的长度数值
                self._update_existing_measurements(scale)
                
                # 关闭对话框
                dialog.accept()
                
            except ValueError as e:
                # 输入无效，显示错误消息
                QMessageBox.warning(dialog, "输入错误", f"请输入有效的数值: {str(e)}")
        
        ok_button.clicked.connect(handle_input)
        button_layout.addWidget(ok_button)
        
        layout.addLayout(button_layout)
        
        # 设置对话框布局
        dialog.setLayout(layout)
        
        # 显示对话框
        dialog.exec_()
        
    def get_pixel_scale(self):
        """获取当前的像素比例（μm/pixel）"""
        # 只从LayerManager获取像素比例，不依赖settings_manager
        if hasattr(self, 'layer_manager') and self.layer_manager:
            return self.layer_manager.get_pixel_scale()
        
        # 默认返回1.0（1像素 = 1微米）
        return 1.0
        
    def _update_existing_measurements(self, new_scale):
        """更新当前视图中已存在图元的长度数值
        
        Args:
            new_scale: 新的像素比例（μm/pixel）
        """
        # 设置LayerManager的像素比例
        self.layer_manager.set_pixel_scale(new_scale)
        
        # 遍历所有绘制对象和检测对象
        all_objects = self.layer_manager.drawing_objects + self.layer_manager.detection_objects
        
        for obj in all_objects:
            # 跳过当前刚绘制的标定线段
            if obj.type == DrawingType.LINE_SEGMENT and obj == self.layer_manager.drawing_objects[-1]:
                continue
                
            # 根据图元类型更新长度数值
            if obj.type == DrawingType.LINE_SEGMENT:
                # 线段
                if len(obj.points) >= 2:
                    p1, p2 = obj.points[0], obj.points[1]
                    pixel_length = ((p2.x() - p1.x())**2 + (p2.y() - p1.y())**2)**0.5
                    real_length = pixel_length * new_scale
                    
                    # 更新属性
                    if 'length' in obj.properties:
                        obj.properties['length'] = real_length
                    else:
                        obj.properties['length'] = real_length
                    
                    # 更新显示文本
                    obj.properties['display_text'] = f"{real_length:.2f}um"
                    
            elif obj.type == DrawingType.LINE:
                # 直线
                if len(obj.points) >= 2:
                    # 直线没有固定长度，可能需要更新其他属性
                    pass
                    
            elif obj.type == DrawingType.CIRCLE:
                # 圆形
                if len(obj.points) >= 2:
                    center, radius_point = obj.points[0], obj.points[1]
                    radius_pixels = ((radius_point.x() - center.x())**2 + (radius_point.y() - center.y())**2)**0.5
                    radius_real = radius_pixels * new_scale
                    
                    # 更新属性
                    if 'radius' in obj.properties:
                        obj.properties['radius'] = radius_real
                    
                    # 更新显示文本
                    obj.properties['display_text'] = f"R={radius_real:.2f}um"
                    
            elif obj.type == DrawingType.PARALLEL:
                # 平行线
                if len(obj.points) >= 4:  # 两条线四个点
                    # 计算两条平行线之间的距离
                    # 这里简化处理，假设平行线是水平或垂直的
                    if 'distance' in obj.properties:
                        pixel_distance = obj.properties.get('pixel_distance', 0)
                        real_distance = pixel_distance * new_scale
                        obj.properties['distance'] = real_distance
                        obj.properties['display_text'] = f"{real_distance:.2f}um"
        
        # 重新渲染视图
        parent = self.parent()
        if parent and hasattr(parent, 'active_view'):
            active_view = parent.active_view
            if active_view:
                active_view.update()