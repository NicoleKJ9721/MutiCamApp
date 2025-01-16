import warnings
warnings.filterwarnings("ignore", category=DeprecationWarning)

from PyQt5.QtCore import QObject, QPoint
import cv2
import numpy as np
from dataclasses import dataclass
from enum import Enum
from typing import List, Optional

class DrawingType(Enum):
    LINE = "line"
    CIRCLE = "circle"
    LINE_SEGMENT = "line_segment"
    PARALLEL = "parallel"
    CIRCLE_LINE = "circle_line"
    TWO_LINES = "two_lines"
    LINE_DETECT = "line_detect"  # 添加直线检测类型
    CIRCLE_DETECT = "circle_detect"  # 添加圆形检测类型

@dataclass
class DrawingObject:
    type: DrawingType
    points: List[QPoint]
    properties: dict  # 存储颜色、线宽等属性
    visible: bool = True

class LayerManager:
    def __init__(self):
        self.drawing_objects: List[DrawingObject] = []  # 手动绘制对象
        self.detection_objects: List[DrawingObject] = []  # 自动检测对象
        self.current_object: Optional[DrawingObject] = None
        self.base_frame = None
        self.show_grid = False  # 是否显示网格
        self.grid_density = 0.1  # 网格密度（默认10%）
        
    def set_grid(self, show: bool, density: float = 0.1):
        """设置网格显示"""
        self.show_grid = show
        self.grid_density = density
        
    def _draw_grid(self, frame):
        """绘制网格"""
        if not self.show_grid:
            return frame
            
        h, w = frame.shape[:2]
        
        # 计算网格间距
        cell_size = int(min(w, h) * self.grid_density)
        if cell_size < 10:  # 设置最小网格大小
            cell_size = 10
            
        # 绘制垂直线
        for x in range(0, w, cell_size):
            cv2.line(frame, (x, 0), (x, h), (255, 0, 0), 3, cv2.LINE_AA)
            
        # 绘制水平线
        for y in range(0, h, cell_size):
            cv2.line(frame, (0, y), (w, y), (255, 0, 0), 3, cv2.LINE_AA)
            
        return frame
        
    def clear(self):
        """清空所有绘制对象"""
        self.drawing_objects = []
        self.detection_objects = []
        self.current_object = None

    def clear_drawings(self):
        """清空手动绘制"""
        self.drawing_objects = []
        
    def clear_detections(self):
        """清空自动检测"""
        self.detection_objects = []
        
    def commit_drawing(self):
        """提交当前绘制"""
        if self.current_object:
            if self.current_object.type in [DrawingType.LINE_DETECT, DrawingType.CIRCLE_DETECT]:
                self.detection_objects.append(self.current_object)
            else:
                self.drawing_objects.append(self.current_object)
            self.current_object = None
            
    def undo_last_drawing(self):
        """撤销最后一次手动绘制"""
        if self.drawing_objects:
            self.drawing_objects.pop()
            return True
        return False
        
    def undo_last_detection(self):
        """撤销最后一次自动检测"""
        if self.detection_objects:
            self.detection_objects.pop()
            return True
        return False
        
    def render_object(self, frame, obj: DrawingObject):
        """渲染单个绘制对象"""
        if obj is None or not obj.visible:
            return
            
        # 根据对象类型调用相应的绘制方法
        draw_methods = {
            DrawingType.LINE: self._draw_line,
            DrawingType.CIRCLE: self._draw_circle,
            DrawingType.LINE_SEGMENT: self._draw_line_segment,
            DrawingType.PARALLEL: self._draw_parallel,
            DrawingType.CIRCLE_LINE: self._draw_circle_line,
            DrawingType.TWO_LINES: self._draw_two_lines,
            DrawingType.LINE_DETECT: self._draw_line_detect,
            DrawingType.CIRCLE_DETECT: self._draw_circle_detect
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
            
        # 创建副本以避免修改原始帧
        display_frame = frame.copy()
        
        # 首先绘制网格（如果启用）
        display_frame = self._draw_grid(display_frame)
        
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
            else:
                # 其他模式保持不变
                if len(self.current_object.points) == 0:
                    self.current_object.points.append(point)
                elif len(self.current_object.points) == 1:
                    self.current_object.points.append(point)
                else:
                    self.current_object.points[1] = point
            
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
                
                # 修改角度显示部分
                text = f"{angle:.1f}"  # 先绘制数字
                
                # 计算文本位置（在起点附近）
                text_x = p1.x() + 20
                text_y = p1.y() - 20
                
                # 绘制文本（带背景）
                font = cv2.FONT_HERSHEY_SIMPLEX
                font_scale = 0.8
                thickness = 1
                (text_width, text_height), baseline = cv2.getTextSize(text, font, font_scale, thickness)
                
                # 计算度数圆点的位置（调整到右上方）
                dot_x = text_x + text_width + 5  # 文本右侧偏移1像素
                dot_y = text_y - text_height + 3  # 上移到文本顶部附近
                
                # 绘制文本背景（需要考虑圆点的空间）
                padding = 6
                cv2.rectangle(frame,
                             (int(text_x - padding), int(text_y - text_height - padding)),
                             (int(dot_x + 2 + padding), int(text_y + padding)),  # +2为圆点预留空间
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
                
                # 绘制度数圆点（调整大小和位置）
                cv2.circle(frame,
                          (int(dot_x), int(dot_y)),
                          3,  # 减小圆点半径为1
                          obj.properties['color'],
                          1,  # 实心圆
                          cv2.LINE_AA)

    def _draw_circle(self, frame, obj: DrawingObject):
        """绘制圆形"""
        if len(obj.points) >= 2:
            center = obj.points[0]
            point = obj.points[-1]
            radius = int(np.sqrt(
                (point.x() - center.x())**2 + 
                (point.y() - center.y())**2
            ))
            
            # 绘制圆形
            cv2.circle(frame, 
                      (center.x(), center.y()), 
                      radius, 
                      obj.properties['color'], 
                      obj.properties['thickness'])
            
            # 绘制圆心
            cv2.circle(frame,
                      (center.x(), center.y()),
                      3, 
                      obj.properties['color'], 
                      -1)
            
            # 显示圆心坐标
            coord_text = f"({center.x()}, {center.y()})"
            
            # 计算文本位置（在圆心右侧）
            text_x = center.x() + 20
            text_y = center.y() - 10
            
            # 绘制文本（带背景）
            font = cv2.FONT_HERSHEY_SIMPLEX
            font_scale = 0.8
            thickness = 1
            (text_width, text_height), baseline = cv2.getTextSize(coord_text, font, font_scale, thickness)
            
            # 绘制文本背景
            padding = 6
            cv2.rectangle(frame,
                         (int(text_x - padding), int(text_y - text_height - padding)),
                         (int(text_x + text_width + padding), int(text_y + padding)),
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

    def _draw_line_segment(self, frame, obj: DrawingObject):
        """绘制线段"""
        if len(obj.points) >= 2:
            p1, p2 = obj.points[0], obj.points[-1]
            
            # 如果是正在绘制的线段，显示虚线效果
            if obj == self.current_object:
                # 计算线段长度
                dist = np.sqrt((p2.x() - p1.x())**2 + (p2.y() - p1.y())**2)
                if dist > 0:
                    # 计算单位向量
                    vx = (p2.x() - p1.x()) / dist
                    vy = (p2.y() - p1.y()) / dist
                    
                    # 虚线参数
                    dash_length = 10
                    gap_length = 10
                    segment_length = dash_length + gap_length
                    
                    # 计算虚线段数量
                    num_segments = int(dist / segment_length)
                    
                    # 绘制虚线段
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
            else:
                # 绘制完成的线段显示实线
                cv2.line(frame,
                        (p1.x(), p1.y()),
                        (p2.x(), p2.y()),
                        obj.properties['color'],
                        obj.properties['thickness'],
                        cv2.LINE_AA)
            
            # 绘制端点标记
            radius = max(3, obj.properties['thickness'])
            cv2.circle(frame, (p1.x(), p1.y()), radius, obj.properties['color'], -1)
            cv2.circle(frame, (p2.x(), p2.y()), radius, obj.properties['color'], -1)
            
            # 计算长度和角度
            length = np.sqrt((p2.x() - p1.x())**2 + (p2.y() - p1.y())**2)
            # 计算与水平线的夹角
            angle = np.degrees(np.arctan2(-(p2.y() - p1.y()), p2.x() - p1.x()))  # 取负是因为y轴向下为正
            if angle < 0:
                angle += 180
            
            # 计算文字位置（线段中点）
            text_x = (p1.x() + p2.x()) // 2 + 40  # 向右偏移，避免与线重叠
            text_y = (p1.y() + p2.y()) // 2
            
            # 准备文本
            dist_text = f"{length:.1f}px"
            angle_text = f"{angle:.1f}"
            
            # 设置文本参数
            font = cv2.FONT_HERSHEY_SIMPLEX
            font_scale = 0.8
            thickness = max(1, obj.properties['thickness'] - 1)
            
            # 计算文本大小
            (dist_width, text_height), baseline = cv2.getTextSize(dist_text, font, font_scale, thickness)
            (angle_width, _), _ = cv2.getTextSize(angle_text, font, font_scale, thickness)
            
            # 计算度数符号的额外空间和最大宽度
            dot_space = 10
            max_width = max(dist_width, angle_width + dot_space)
            line_spacing = 10
            
            # 绘制文本背景
            padding = 2
            total_height = text_height * 2 + line_spacing + padding
            cv2.rectangle(frame,
                         (int(text_x - max_width//2 - padding), 
                          int(text_y - total_height)),
                         (int(text_x + max_width//2 + padding), 
                          int(text_y + text_height + padding)),
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
            dot_x = angle_x + angle_width + 3
            dot_y = angle_y - text_height//2 + 3
            cv2.circle(frame,
                      (int(dot_x), int(dot_y)),
                      3,
                      obj.properties['color'],
                      1,
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
            font_scale = 0.8
            thickness = 1
            (text_width, text_height), baseline = cv2.getTextSize(text, font, font_scale, thickness)
            
            # 绘制文本背景
            padding = 6
            cv2.rectangle(frame,
                         (int(text_x - text_width//2 - padding), int(text_y - text_height//2 - padding)),
                         (int(text_x + text_width//2 + padding), int(text_y + text_height//2 + padding)),
                         (0, 0, 0),
                         -1)
            
            # 绘制文本
            cv2.putText(frame,
                       text,
                       (int(text_x - text_width//2), int(text_y + text_height//2)),
                       font,
                       font_scale,
                       obj.properties['color'],
                       thickness)

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
                    
                    # 绘制中线（虚线）
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
                                    obj.properties['color'],
                                    1,  # 使用较细的线宽
                                    cv2.LINE_AA)
                    
                    # 显示距离和角度信息（分两行显示）
                    dist_text = f"{dist:.1f}px"
                    angle_text = f"{angle:.1f}"

                    # 计算文本位置（在两条线中间，但要错开）
                    text_x = mid_x + 40  # 向右偏移，避免与中线重叠
                    text_y = mid_y

                    # 绘制文本（带背景）
                    font = cv2.FONT_HERSHEY_SIMPLEX
                    font_scale = 0.8
                    thickness = 1

                    # 计算文本大小
                    (dist_width, text_height), baseline = cv2.getTextSize(dist_text, font, font_scale, thickness)
                    (angle_width, _), _ = cv2.getTextSize(angle_text, font, font_scale, thickness)

                    # 计算度数符号的额外空间
                    dot_space = 10  # 为度数符号预留的空间

                    # 计算最大宽度（考虑度数符号的空间）
                    max_width = max(dist_width, angle_width + dot_space)

                    # 计算两行文本的垂直间距
                    line_spacing = 10

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
                    dot_x = angle_x + angle_width + 3
                    dot_y = angle_y - text_height//2 + 3  # 微调圆的垂直位置
                    cv2.circle(frame,
                              (int(dot_x), int(dot_y)),
                              3,  # 圆的半径
                              obj.properties['color'],
                              1,  # 线宽为1表示空心圆
                              cv2.LINE_AA)

    def _draw_circle_line(self, frame, obj: DrawingObject):
        """绘制圆与线的距离测量"""
        if len(obj.points) >= 1:
            # 绘制圆心
            center = obj.points[0]
            cv2.circle(frame, 
                      (center.x(), center.y()), 
                      3, 
                      obj.properties['color'], 
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
                          obj.properties['color'], 
                          obj.properties['thickness'],
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
                                obj.properties['color'],
                                obj.properties['thickness'],
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
                                    obj.properties['color'],
                                    1,
                                    cv2.LINE_AA)
                        
                        # 显示距离信息
                        text = f"{dist:.1f}px"  # 直接显示垂直距离，不减去半径
                        font = cv2.FONT_HERSHEY_SIMPLEX
                        font_scale = 0.8
                        thickness = 1
                        
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
                                  obj.properties['color'],
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
        }
        
        if obj.type in draw_methods:
            draw_methods[obj.type](frame, obj)
        
    def _draw_two_lines(self, frame, obj: DrawingObject):
        """绘制两条线的夹角测量"""
        if len(obj.points) >= 2:
            height, width = frame.shape[:2]
            max_length = int(np.sqrt(width**2 + height**2))
            
            # 绘制第一条线
            p1 = obj.points[0]
            p2 = obj.points[1]
            
            # 计算第一条线的方向向量
            dx1 = p2.x() - p1.x()
            dy1 = p2.y() - p1.y()
            
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
                
                # 如果有第二条线
                if len(obj.points) >= 4:
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
                        # 使用参数方程求解
                        # 线1: p1 + t1(dx1,dy1)
                        # 线2: p3 + t2(dx2,dy2)
                        denominator = dx1*dy2 - dy1*dx2
                        if abs(denominator) > 1e-10:  # 避免平行线的情况
                            t1 = ((p3.x() - p1.x())*dy2 - (p3.y() - p1.y())*dx2) / denominator
                            intersection_x = int(p1.x() + dx1 * t1)
                            intersection_y = int(p1.y() + dy1 * t1)
                            
                            # 计算文本位置（在交点附近）
                            text_x = intersection_x + 20  # 向右偏移一点
                            text_y = intersection_y - 20  # 向上偏移一点
                        else:
                            # 如果线段平行，使用中点
                            text_x = (p1.x() + p3.x()) // 2
                            text_y = (p1.y() + p3.y()) // 2
                        
                        # 显示角度信息
                        text = f"{angle:.1f}"
                        font = cv2.FONT_HERSHEY_SIMPLEX
                        font_scale = 0.8
                        thickness = 1
                        
                        # 获取文本大小
                        (text_width, text_height), baseline = cv2.getTextSize(text, font, font_scale, thickness)
                        
                        # 绘制文本背景
                        padding = 6
                        cv2.rectangle(frame,
                                    (int(text_x - text_width//2 - padding), 
                                     int(text_y - text_height//2 - padding)),
                                    (int(text_x + text_width//2 + padding + 10), 
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
                        
                        # 绘制度数符号（空心圆）
                        dot_x = text_x + text_width//2 + 2
                        dot_y = text_y - text_height//2 + 3  # 微调圆的垂直位置
                        cv2.circle(frame,
                                  (int(dot_x), int(dot_y)),
                                  3,  # 圆的半径
                                  obj.properties['color'],
                                  1,  # 线宽为1表示空心圆
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
            
            # 获取ROI区域的坐标
            x1, y1 = roi_points[0].x(), roi_points[0].y()
            x2, y2 = roi_points[1].x(), roi_points[1].y()
            
            # 确保坐标正确排序
            x_min, x_max = min(x1, x2), max(x1, x2)
            y_min, y_max = min(y1, y2), max(y1, y2)
            
            # 提取ROI区域
            roi = frame[y_min:y_max, x_min:x_max]
            if roi.size == 0:
                return None
            
            # 转换为灰度图
            gray = cv2.cvtColor(roi, cv2.COLOR_BGR2GRAY)
            
            # 边缘检测 - 使用设置的参数
            edges = cv2.Canny(gray, canny_low, canny_high, apertureSize=3)
            
            # 霍夫变换检测直线 - 使用设置的参数
            lines = cv2.HoughLinesP(edges, 1, np.pi/180, 
                                   threshold=line_threshold,
                                   minLineLength=min_line_length,
                                   maxLineGap=max_line_gap)
            
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
                    font_scale = 0.8
                    thickness = 1
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
            circle_threshold = int(settings_dict.get('CircleDetParam2', 15))
            
            # 获取ROI区域的坐标
            x1, y1 = roi_points[0].x(), roi_points[0].y()
            x2, y2 = roi_points[1].x(), roi_points[1].y()
            
            # 确保坐标正确排序
            x_min, x_max = min(x1, x2), max(x1, x2)
            y_min, y_max = min(y1, y2), max(y1, y2)
            
            # 计算ROI尺寸
            roi_width = abs(x_max - x_min)
            roi_height = abs(y_max - y_min)
            roi_radius = min(roi_width, roi_height) // 2
            
            # 确保ROI区域有效
            if roi_width < 10 or roi_height < 10:
                print("ROI区域太小")
                return None
            
            # 提取ROI区域
            roi = frame[y_min:y_max, x_min:x_max]
            if roi.size == 0:
                print("ROI区域无效")
                return None
            
            # 转换为灰度图
            gray = cv2.cvtColor(roi, cv2.COLOR_BGR2GRAY)
            
            # 二值化处理
            _, binary = cv2.threshold(gray, 200, 255, cv2.THRESH_BINARY)
            
            # 形态学操作改善圆形
            kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (3,3))
            binary = cv2.morphologyEx(binary, cv2.MORPH_CLOSE, kernel)
            
            # 边缘检测
            edges = cv2.Canny(binary, canny_low, canny_high)
            
            # 霍夫圆变换检测圆形
            circles = cv2.HoughCircles(
                edges,
                cv2.HOUGH_GRADIENT,
                dp=1,
                minDist=roi_radius,
                param1=canny_high,
                param2=circle_threshold,
                minRadius=10,
                maxRadius=roi_radius
            )
            
            if circles is not None:
                print(f"检测到 {len(circles[0])} 个圆")  # 调试信息
                # 只取准确率最高的一个圆
                best_circle = circles[0][0]
                center_x = int(best_circle[0]) + x_min
                center_y = int(best_circle[1]) + y_min
                radius = int(best_circle[2])
                return {'center_x': center_x, 'center_y': center_y, 'radius': radius}
            else:
                print("未检测到圆形")
                return None
            
        except Exception as e:
            print(f"圆形检测失败: {str(e)}")
            return None
    
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
                    text = f"R={radius:.1f}"
                    font = cv2.FONT_HERSHEY_SIMPLEX
                    font_scale = 0.8
                    thickness = 1
                    
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
            if str(e) != "0":  # 只打印非0错误
                print(f"绘制圆形检测时出错: {str(e)}")

class MeasurementManager(QObject):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.log_manager = parent.log_manager if parent else None
        self.layer_manager = LayerManager()
        self.drawing = False
        self.draw_mode = None
        self.drawing_history = []  # 添加绘画历史列表
        
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
        if self.log_manager:
            self.log_manager.log_measurement_operation("启动直线检测")
        self.draw_mode = DrawingType.LINE_DETECT
        self.drawing = False 
        
    def start_circle_detection(self):
        """启动圆形检测模式"""
        if self.log_manager:
            self.log_manager.log_measurement_operation("启动圆形检测")
        self.draw_mode = DrawingType.CIRCLE_DETECT
        self.drawing = False 

    def clear_measurements(self):
        """清空所有测量"""
        self.draw_mode = None
        self.drawing = False
        self.layer_manager.clear()  # 保留此方法用于完全清空
        
    def clear_drawings(self):
        """只清空手动绘制"""
        self.draw_mode = None
        self.drawing = False
        self.layer_manager.clear_drawings()  # 只清空手动绘制

    def undo_last_drawing(self):
        """撤销上一步绘制"""
        return self.layer_manager.undo()

    def handle_mouse_press(self, event_pos, current_frame):
        """处理鼠标按下事件"""
        if not self.draw_mode:
            return None
        
        if self.draw_mode == DrawingType.LINE_DETECT:
            # 直线检测模式
            if not self.layer_manager.current_object:
                self.drawing = True
                properties = {
                    'color': (0, 255, 255),  # RGB格式：青色
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
                    'color': (0, 255, 255),  # RGB格式：青色
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
                    'color': (255, 190, 0),  # RGB格式：暗黄色
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
                # 第一次按下：开始绘制圆
                self.drawing = True
                properties = {
                    'color': (255, 190, 0),  # RGB格式：暗黄色
                    'thickness': 2
                }
                self.layer_manager.start_drawing(self.draw_mode, properties)
                self.layer_manager.current_object.points.append(event_pos)
            elif len(self.layer_manager.current_object.points) == 2:
                # 第二次按下：开始绘制直线
                self.drawing = True
                self.layer_manager.current_object.points.append(event_pos)  # 添加直线起点
            return self.layer_manager.render_frame(current_frame)
        elif self.draw_mode == DrawingType.PARALLEL:
            # 保持平行线处理不变
            if self.layer_manager.current_object and len(self.layer_manager.current_object.points) == 2:
                self.layer_manager.update_current_object(event_pos)
                self.layer_manager.commit_drawing()
                return self.layer_manager.render_frame(current_frame)
            else:
                self.drawing = True
                properties = {
                    'color': (255, 190, 0),  # RGB格式：暗黄色
                    'thickness': 2
                }
                self.layer_manager.start_drawing(self.draw_mode, properties)
                self.layer_manager.current_object.points.append(event_pos)
        else:
            # 其他模式正常处理
            self.drawing = True
            properties = {
                'color': (255, 190, 0),  # RGB格式：暗黄色
                'thickness': 2
            }
            self.layer_manager.start_drawing(self.draw_mode, properties)
            self.layer_manager.update_current_object(event_pos)
        
        return self.layer_manager.render_frame(current_frame)
        
    def handle_mouse_move(self, event_pos, current_frame):
        """处理鼠标移动事件"""
        if self.drawing:
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
                if len(self.layer_manager.current_object.points) <= 2:
                    # 第一阶段：绘制圆时的拖动
                    if len(self.layer_manager.current_object.points) == 1:
                        self.layer_manager.current_object.points.append(event_pos)
                    else:
                        self.layer_manager.current_object.points[1] = event_pos
                elif len(self.layer_manager.current_object.points) >= 3:
                    # 第二阶段：绘制直线时的拖动
                    if len(self.layer_manager.current_object.points) == 3:
                        self.layer_manager.current_object.points.append(event_pos)
                    else:
                        self.layer_manager.current_object.points[3] = event_pos
                return self.layer_manager.render_frame(current_frame)
            elif self.draw_mode == DrawingType.PARALLEL:
                # 保持平行线模式处理不变
                if len(self.layer_manager.current_object.points) <= 2:
                    if len(self.layer_manager.current_object.points) == 1:
                        self.layer_manager.current_object.points.append(event_pos)
                    elif len(self.layer_manager.current_object.points) == 2:
                        self.layer_manager.current_object.points[1] = event_pos
                else:
                    # 其他模式：正常处理
                    self.layer_manager.update_current_object(event_pos)
            else:
                # 其他模式：正常处理
                self.layer_manager.update_current_object(event_pos)
            return self.layer_manager.render_frame(current_frame)
        return None
        
    def handle_mouse_release(self, event_pos, current_frame):
        """处理鼠标释放事件"""
        if self.drawing:
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
                    self.layer_manager.current_object.points = detected_points
                    self.layer_manager.current_object.properties['line_detected'] = True
                
                self.layer_manager.commit_drawing()
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
                    # 更新当前对象的属性
                    self.layer_manager.current_object.properties.update(detected_circle)
                    self.layer_manager.current_object.properties['circle_detected'] = True
                    
                    # 创建新的点列表,使用检测到的圆心
                    center_x = detected_circle['center_x']
                    center_y = detected_circle['center_y']
                    self.layer_manager.current_object.points = [
                        QPoint(center_x, center_y),
                        QPoint(center_x + detected_circle['radius'], center_y)  # 用于确定半径的点
                    ]
                    
                    print(f"更新绘制属性: center=({center_x}, {center_y}), radius={detected_circle['radius']}")
                
                self.layer_manager.commit_drawing()
                self.drawing = False
                return self.layer_manager.render_frame(current_frame)
            elif self.draw_mode == DrawingType.TWO_LINES:
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
                    return self.layer_manager.render_frame(current_frame)
            elif self.draw_mode == DrawingType.CIRCLE_LINE:
                if len(self.layer_manager.current_object.points) <= 2:
                    # 第一阶段：圆绘制完成
                    if len(self.layer_manager.current_object.points) == 1:
                        self.layer_manager.current_object.points.append(event_pos)
                    else:
                        self.layer_manager.current_object.points[1] = event_pos
                    self.drawing = False
                    return self.layer_manager.render_frame(current_frame)
                elif len(self.layer_manager.current_object.points) <= 4:
                    # 第二阶段：直线绘制完成
                    if len(self.layer_manager.current_object.points) == 3:
                        self.layer_manager.current_object.points.append(event_pos)
                    else:
                        self.layer_manager.current_object.points[3] = event_pos
                    self.layer_manager.commit_drawing()
                    self.drawing = False
                    return self.layer_manager.render_frame(current_frame)
            elif self.draw_mode == DrawingType.PARALLEL:
                # 保持平行线处理不变
                if len(self.layer_manager.current_object.points) <= 2:
                    if len(self.layer_manager.current_object.points) == 1:
                        self.layer_manager.current_object.points.append(event_pos)
                    else:
                        self.layer_manager.current_object.points[1] = event_pos
                    self.drawing = False
                    return self.layer_manager.render_frame(current_frame)
            # 其他情况正常提交
            self.layer_manager.commit_drawing()
            self.drawing = False
            return self.layer_manager.render_frame(current_frame)
        return None 

    def set_grid(self, show: bool, density: float = 0.1):
        """设置网格显示"""
        self.layer_manager.set_grid(show, density)