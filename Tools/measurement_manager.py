from PyQt5.QtCore import QObject, Qt, QPoint
from PyQt5.QtGui import QImage, QPixmap
import cv2
import numpy as np
from PIL import Image, ImageDraw, ImageFont

class MeasurementManager(QObject):
    def __init__(self):
        super().__init__()
        self.drawing = False
        self.draw_mode = None
        self.line_start = None
        self.line_end = None
        self.display_image = None
        self.temp_display = None
        # 添加圆形检测相关属性
        self.circle_center = None
        self.circle_radius = None
        # 添加平行线相关属性
        self.first_line = None  # 存储第一条线的起点和终点
        self.parallel_point = None  # 存储第二条线的参考点
        self.is_drawing_first_line = True  # 标记是否在画第一条线
        
    def start_line_measurement(self):
        """启动直线测量模式"""
        print("启动直线测量模式")  # 调试信息
        self.draw_mode = 'line'
        self.drawing = False
        self.line_start = None
        self.line_end = None
        self.temp_display = None
        
    def start_circle_measurement(self):
        """启动圆形测量模式"""
        print("启动圆形测量模式")  # 调试信息
        self.draw_mode = 'circle'
        self.drawing = False
        self.circle_center = None
        self.circle_radius = None
        self.temp_display = None
        
    def start_line_segment_measurement(self):
        """启动线段测量模式"""
        print("启动线段测量模式")  # 调试信息
        self.draw_mode = 'line_segment'
        self.drawing = False
        self.line_start = None
        self.line_end = None
        self.temp_display = None
        
    def start_parallel_measurement(self):
        """启动平行线测量模式"""
        print("启动平行线测量模式")  # 调试信息
        self.draw_mode = 'parallel'
        self.drawing = False
        self.line_start = None
        self.line_end = None
        self.first_line = None
        self.parallel_point = None
        self.is_drawing_first_line = True
        self.temp_display = None
        
    def handle_mouse_press(self, event_pos, current_frame):
        """处理鼠标按下事件"""
        print(f"处理鼠标按下事件: {self.draw_mode}")  # 调试信息
        
        # 确保使用3通道图像
        if len(current_frame.shape) == 2:
            current_frame = cv2.cvtColor(current_frame, cv2.COLOR_GRAY2BGR)
        
        # 初始化显示层
        if self.display_image is None:
            self.display_image = np.zeros_like(current_frame)
        if self.temp_display is None:
            self.temp_display = np.zeros_like(current_frame)
        
        if self.draw_mode == 'line':
            self.line_start = event_pos
            self.drawing = True
        elif self.draw_mode == 'circle':
            self.circle_center = event_pos
            self.line_start = event_pos
            self.drawing = True
        elif self.draw_mode == 'line_segment':  # 添加线段模式处理
            self.line_start = event_pos
            self.drawing = True
        elif self.draw_mode == 'parallel':
            if self.is_drawing_first_line:
                # 画第一条线
                self.line_start = event_pos
                self.drawing = True
            else:
                # 设置第二条线的参考点
                self.parallel_point = event_pos
                self.draw_parallel_line(current_frame)
        
    def handle_mouse_move(self, event_pos, current_frame):
        """处理鼠标移动事件"""
        print(f"处理鼠标移动事件: drawing={self.drawing}, mode={self.draw_mode}")  # 调试信息
        if not self.drawing:
            return None
        
        self.line_end = event_pos
        self.temp_display = np.zeros_like(current_frame)
        
        if self.draw_mode == 'line_segment':
            # 绘制线段
            cv2.line(self.temp_display,
                    (self.line_start.x(), self.line_start.y()),
                    (self.line_end.x(), self.line_end.y()),
                    (255, 190, 0),  # BGR格式
                    3)
            
            # 计算长度和角度
            dx = self.line_end.x() - self.line_start.x()
            dy = self.line_end.y() - self.line_start.y()
            length = np.sqrt(dx*dx + dy*dy)
            angle = np.degrees(np.arctan2(-dy, dx))
            if angle < 0:
                angle += 360
            
            # 添加文字说明
            temp_display_rgb = cv2.cvtColor(self.temp_display, cv2.COLOR_BGR2RGB)
            pil_image = Image.fromarray(temp_display_rgb)
            draw = ImageDraw.Draw(pil_image)
            
            try:
                font = ImageFont.truetype("simhei.ttf", 40)
            except:
                try:
                    font = ImageFont.truetype("simsun.ttc", 40)
                except:
                    font = ImageFont.load_default()
            
            # 将文字显示在线段中点附近
            mid_x = (self.line_start.x() + self.line_end.x()) // 2
            mid_y = (self.line_start.y() + self.line_end.y()) // 2
            text_x = mid_x + 20
            text_y = mid_y
            line_spacing = 50
            
            # 准备文字内容
            length_text = f"长度: {length:.1f}px"
            angle_text = f"角度: {angle:.1f}°"
            
            # 获取文字区域
            length_bbox = draw.textbbox((text_x, text_y - line_spacing), length_text, font=font)
            angle_bbox = draw.textbbox((text_x, text_y), angle_text, font=font)
            
            # 创建覆盖层
            overlay = Image.new('RGBA', pil_image.size, (0, 0, 0, 0))
            overlay_draw = ImageDraw.Draw(overlay)
            
            # 绘制文字背景
            overlay_draw.rectangle(
                [length_bbox[0]-5, length_bbox[1]-5, length_bbox[2]+5, length_bbox[3]+5],
                fill=(0, 0, 0, 160)
            )
            overlay_draw.rectangle(
                [angle_bbox[0]-5, angle_bbox[1]-5, angle_bbox[2]+5, angle_bbox[3]+5],
                fill=(0, 0, 0, 160)
            )
            
            # 合并图层
            pil_image = Image.alpha_composite(pil_image.convert('RGBA'), overlay)
            draw = ImageDraw.Draw(pil_image)
            
            # 绘制文字边框
            for offset in [(1,1), (-1,-1), (1,-1), (-1,1)]:
                draw.text((text_x + offset[0], text_y - line_spacing + offset[1]), 
                         length_text, fill=(0, 0, 0), font=font)
                draw.text((text_x + offset[0], text_y + offset[1]), 
                         angle_text, fill=(0, 0, 0), font=font)
            
            # 绘制主文字
            draw.text((text_x, text_y - line_spacing), length_text, 
                     fill=(230, 190, 50), font=font)
            draw.text((text_x, text_y), angle_text, 
                     fill=(230, 190, 50), font=font)
            
            self.temp_display = cv2.cvtColor(np.array(pil_image.convert('RGB')), cv2.COLOR_RGB2BGR)
            
        elif self.draw_mode == 'line':
            height, width = self.temp_display.shape[:2]
            max_length = int(np.sqrt(width**2 + height**2))
            
            dx = self.line_end.x() - self.line_start.x()
            dy = self.line_end.y() - self.line_start.y()
            
            if dx != 0 or dy != 0:
                length = np.sqrt(dx**2 + dy**2)
                dx, dy = dx/length, dy/length
                
                start_x = int(self.line_start.x() - dx * max_length)
                start_y = int(self.line_start.y() - dy * max_length)
                end_x = int(self.line_start.x() + dx * max_length)
                end_y = int(self.line_start.y() + dy * max_length)
                
                # 使用RGB(255,190,0)作为线条颜色
                cv2.line(self.temp_display, (start_x, start_y), (end_x, end_y),
                        (255, 190, 0), 3)  # BGR格式
                
                # 计算角度和斜率
                angle = np.degrees(np.arctan2(-dy, dx))
                if angle < 0:
                    angle += 360
                slope = -dy/dx if dx != 0 else float('inf')
                
                print(f"绘制线条: 起点({start_x}, {start_y}), 终点({end_x}, {end_y})")  # 调试信息
                print(f"角度: {angle:.2f}°, 斜率: {slope:.2f}")  # 调试信息
                
                # 添加文字说明
                temp_display_rgb = cv2.cvtColor(self.temp_display, cv2.COLOR_BGR2RGB)
                pil_image = Image.fromarray(temp_display_rgb)
                draw = ImageDraw.Draw(pil_image)
                
                try:
                    font = ImageFont.truetype("simhei.ttf", 40)  # 字体大小保持40
                except:
                    try:
                        font = ImageFont.truetype("simsun.ttc", 40)
                    except:
                        font = ImageFont.load_default()
                
                text_x = self.line_start.x() + 20
                text_y = self.line_start.y()
                line_spacing = 50  # 增加行间距
                
                # 准备文字内容
                angle_text = f"角度: {angle:.1f}°"
                slope_text = f"斜率: {slope:.2f}" if slope != float('inf') else "斜率: ∞"
                
                # 获取文字大小以绘制背景
                angle_bbox = draw.textbbox((text_x, text_y - line_spacing), angle_text, font=font)
                slope_bbox = draw.textbbox((text_x, text_y), slope_text, font=font)
                
                # 绘制半透明背景
                overlay = Image.new('RGBA', pil_image.size, (0, 0, 0, 0))
                overlay_draw = ImageDraw.Draw(overlay)
                
                # 为角度文字绘制背景
                overlay_draw.rectangle(
                    [angle_bbox[0]-5, angle_bbox[1]-5, angle_bbox[2]+5, angle_bbox[3]+5],
                    fill=(0, 0, 0, 160)  # 黑色半透明背景
                )
                
                # 为斜率文字绘制背景
                overlay_draw.rectangle(
                    [slope_bbox[0]-5, slope_bbox[1]-5, slope_bbox[2]+5, slope_bbox[3]+5],
                    fill=(0, 0, 0, 160)  # 黑色半透明背景
                )
                
                # 将半透明背景叠加到主图像上
                pil_image = Image.alpha_composite(pil_image.convert('RGBA'), overlay)
                draw = ImageDraw.Draw(pil_image)
                
                # 绘制文字边框和填充
                for offset in [(1,1), (-1,-1), (1,-1), (-1,1)]:  # 文字边框偏移
                    draw.text((text_x + offset[0], text_y - line_spacing + offset[1]), 
                             angle_text, fill=(0, 0, 0), font=font)  # 黑色边框
                    draw.text((text_x + offset[0], text_y + offset[1]), 
                             slope_text, fill=(0, 0, 0), font=font)  # 黑色边框
                
                # 绘制主文字
                draw.text((text_x, text_y - line_spacing), angle_text, 
                         fill=(230, 190, 50), font=font)  # 使用原始RGB颜色
                draw.text((text_x, text_y), slope_text, 
                         fill=(230, 190, 50), font=font)  # 使用原始RGB颜色
                
                # 转换回BGR格式
                self.temp_display = cv2.cvtColor(np.array(pil_image.convert('RGB')), cv2.COLOR_RGB2BGR)
                
        elif self.draw_mode == 'circle' and self.circle_center:
            radius = int(np.sqrt(
                (self.line_end.x() - self.circle_center.x())**2 + 
                (self.line_end.y() - self.circle_center.y())**2
            ))
            
            # 绘制圆
            cv2.circle(self.temp_display, 
                      (self.circle_center.x(), self.circle_center.y()),
                      radius,
                      (255, 190, 0),  # 使用相同的颜色主题
                      3)
            
            # 绘制圆心十字
            cross_size = 5
            cv2.line(self.temp_display,
                    (self.circle_center.x() - cross_size, self.circle_center.y()),
                    (self.circle_center.x() + cross_size, self.circle_center.y()),
                    (255, 190, 0),
                    2)
            cv2.line(self.temp_display,
                    (self.circle_center.x(), self.circle_center.y() - cross_size),
                    (self.circle_center.x(), self.circle_center.y() + cross_size),
                    (255, 190, 0),
                    2)
            
            # 绘制半径线段
            cv2.line(self.temp_display,
                    (self.circle_center.x(), self.circle_center.y()),
                    (self.line_end.x(), self.line_end.y()),
                    (255, 190, 0),
                    1)
            
            # 添加文字说明
            temp_display_rgb = cv2.cvtColor(self.temp_display, cv2.COLOR_BGR2RGB)
            pil_image = Image.fromarray(temp_display_rgb)
            draw = ImageDraw.Draw(pil_image)
            
            try:
                font = ImageFont.truetype("simhei.ttf", 40)
            except:
                try:
                    font = ImageFont.truetype("simsun.ttc", 40)
                except:
                    font = ImageFont.load_default()
            
            text_x = self.circle_center.x() + 20
            text_y = self.circle_center.y()
            line_spacing = 50
            
            # 绘制半透明背景
            text_circle = f"圆心: ({self.circle_center.x()}, {self.circle_center.y()})"
            text_radius = f"半径: {radius}px"
            
            # 获取文字区域
            bbox_circle = draw.textbbox((text_x, text_y - line_spacing), text_circle, font=font)
            bbox_radius = draw.textbbox((text_x, text_y), text_radius, font=font)
            
            # 创建覆盖层
            overlay = Image.new('RGBA', pil_image.size, (0, 0, 0, 0))
            overlay_draw = ImageDraw.Draw(overlay)
            
            # 绘制文字背景
            overlay_draw.rectangle(
                [bbox_circle[0]-5, bbox_circle[1]-5, bbox_circle[2]+5, bbox_circle[3]+5],
                fill=(0, 0, 0, 160)
            )
            overlay_draw.rectangle(
                [bbox_radius[0]-5, bbox_radius[1]-5, bbox_radius[2]+5, bbox_radius[3]+5],
                fill=(0, 0, 0, 160)
            )
            
            # 合并图层
            pil_image = Image.alpha_composite(pil_image.convert('RGBA'), overlay)
            draw = ImageDraw.Draw(pil_image)
            
            # 绘制文字边框
            for offset in [(1,1), (-1,-1), (1,-1), (-1,1)]:
                draw.text((text_x + offset[0], text_y - line_spacing + offset[1]), 
                         text_circle, fill=(0, 0, 0), font=font)
                draw.text((text_x + offset[0], text_y + offset[1]), 
                         text_radius, fill=(0, 0, 0), font=font)
            
            # 绘制主文字
            draw.text((text_x, text_y - line_spacing), text_circle, 
                     fill=(230, 190, 50), font=font)
            draw.text((text_x, text_y), text_radius, 
                     fill=(230, 190, 50), font=font)
            
            self.temp_display = cv2.cvtColor(np.array(pil_image.convert('RGB')), cv2.COLOR_RGB2BGR)
            
        elif self.draw_mode == 'parallel':
            if self.is_drawing_first_line and self.drawing:
                # 绘制第一条线
                self.line_end = event_pos
                display_frame = current_frame.copy()
                cv2.line(display_frame, 
                        (self.line_start.x(), self.line_start.y()),
                        (self.line_end.x(), self.line_end.y()),
                        (0, 255, 0), 2)
                return display_frame
            elif not self.is_drawing_first_line:
                # 实时更新第二条平行线和中线
                self.parallel_point = event_pos
                return self.draw_parallel_line(current_frame)
        
        return self.create_display_frame(current_frame)
        
    def handle_mouse_release(self, event_pos, current_frame):
        """处理鼠标释放事件"""
        print("处理鼠标释放事件")  # 调试信息
        if not self.drawing:
            return None
        
        self.drawing = False
        if self.temp_display is not None:
            # 确保 display_image 已初始化
            if self.display_image is None:
                self.display_image = np.zeros_like(current_frame)
            # 使用 BGR 格式进行掩码操作
            mask = cv2.cvtColor(self.temp_display, cv2.COLOR_BGR2GRAY) > 0
            self.display_image[mask] = self.temp_display[mask]
        
        self.temp_display = None
        self.line_start = None
        self.line_end = None
        self.circle_center = None
        
        return self.create_display_frame(current_frame)
        
    def create_display_frame(self, current_frame):
        """创建显示帧"""
        if current_frame is None:
            print("当前帧为空")
            return None
            
        display_frame = current_frame.copy()
        
        # 叠加永久绘制内容
        if self.display_image is not None:
            try:
                mask = cv2.cvtColor(self.display_image, cv2.COLOR_BGR2GRAY) > 0
                # 增加叠加强度
                display_frame[mask] = cv2.addWeighted(display_frame, 0.2, self.display_image, 0.8, 0)[mask]
            except cv2.error as e:
                print(f"处理永久绘制内容时出错: {str(e)}")
        
        # 叠加临时绘制内容
        if self.temp_display is not None:
            try:
                mask = cv2.cvtColor(self.temp_display, cv2.COLOR_BGR2GRAY) > 0
                # 增加叠加强度
                display_frame[mask] = cv2.addWeighted(display_frame, 0.2, self.temp_display, 0.8, 0)[mask]
            except cv2.error as e:
                print(f"处理临时绘制内容时出错: {str(e)}")
        
        return display_frame
        
    def clear_measurements(self):
        """清除所有测量"""
        print("清除所有测量内容")  # 调试信息
        self.display_image = None
        self.temp_display = None
        self.drawing = False
        self.line_start = None
        self.line_end = None
        self.circle_center = None  # 添加圆心清除
        self.circle_radius = None  # 添加半径清除

    def has_drawing(self):
        """检查是否有绘制内容"""
        return self.display_image is not None or self.temp_display is not None 

    def draw_parallel_line(self, frame):
        """绘制平行线和中线"""
        if self.first_line and self.parallel_point:
            display_frame = frame.copy()
            
            # 计算第一条线的方向向量
            dx = self.first_line[1].x() - self.first_line[0].x()
            dy = self.first_line[1].y() - self.first_line[0].y()
            length = np.sqrt(dx*dx + dy*dy)
            dx, dy = dx/length, dy/length
            
            # 计算平行线的起点和终点
            start_x = self.parallel_point.x() - dx * length
            start_y = self.parallel_point.y() - dy * length
            end_x = self.parallel_point.x() + dx * length
            end_y = self.parallel_point.y() + dy * length
            
            # 绘制第二条平行线
            cv2.line(display_frame, 
                    (int(start_x), int(start_y)),
                    (int(end_x), int(end_y)),
                    (0, 255, 0), 2)
            
            # 计算并绘制中线（虚线）
            mid_start_x = (self.first_line[0].x() + start_x) // 2
            mid_start_y = (self.first_line[0].y() + start_y) // 2
            mid_end_x = (self.first_line[1].x() + end_x) // 2
            mid_end_y = (self.first_line[1].y() + end_y) // 2
            
            # 使用虚线绘制中线
            cv2.line(display_frame,
                    (int(mid_start_x), int(mid_start_y)),
                    (int(mid_end_x), int(mid_end_y)),
                    (255, 0, 0), 2, cv2.LINE_AA | cv2.LINE_DASHED)
            
            return display_frame 