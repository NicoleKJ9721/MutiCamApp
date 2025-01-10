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
        self.drawing_history = []  # 绘画历史记录
        self.point1 = None  # 第一个点
        self.point2 = None  # 第二个点
        self.preview_point = None  # 预览点（鼠标当前位置）
        self.step = 1  # 当前步骤：1=第一个点，2=第二个点，3=第三个点
        
        # 修改圆线测量相关属性
        self.drawing_circle = True  # 标记是否在画圆
        self.circle_center = None   # 圆心
        self.circle_radius = None   # 圆的半径
        self.line_start = None      # 直线起点
        self.line_end = None        # 直线终点
        
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
        print("启动平行线测量模式")
        self.draw_mode = 'parallel'
        self.drawing = False
        self.point1 = None
        self.point2 = None
        self.preview_point = None
        self.step = 1
        self.temp_display = None
        self.is_drawing_first_line = True
        self.first_line = None
        self.parallel_point = None
        
    def start_circle_line_measurement(self):
        """启动圆线距离测量模式"""
        print("启动圆线距离测量模式")
        self.draw_mode = 'circle_line'
        self.drawing = False
        self.circle_center = None
        self.circle_radius = None
        self.line_start = None
        self.line_end = None
        self.drawing_circle = True  # 标记是否在画圆
        self.temp_display = None
        
    def start_two_lines_measurement(self):
        """启动线与线测量模式"""
        print("启动线与线测量模式")
        self.draw_mode = 'two_lines'
        self.drawing = False
        self.first_line_start = None
        self.first_line_end = None
        self.second_line_start = None
        self.second_line_end = None
        self.is_drawing_first_line = True  # 标记是否在画第一条线
        self.temp_display = None
        
    def handle_mouse_press(self, event_pos, current_frame):
        """处理鼠标按下事件"""
        print(f"处理鼠标按下事件: {self.draw_mode}")
        
        # 确保使用3通道图像
        if len(current_frame.shape) == 2:
            current_frame = cv2.cvtColor(current_frame, cv2.COLOR_GRAY2BGR)
        
        # 初始化显示层
        if self.display_image is None:
            self.display_image = np.zeros_like(current_frame)
        if self.temp_display is None:
            self.temp_display = np.zeros_like(current_frame)
        
        if self.draw_mode == 'circle_line':
            if self.drawing_circle:
                # 第一次点击，设置圆心
                self.circle_center = event_pos
                self.line_end = event_pos
                self.drawing = True
                # 立即显示圆心点
                self.temp_display = np.zeros_like(current_frame)
                cv2.circle(self.temp_display,
                          (self.circle_center.x(), self.circle_center.y()),
                          3,
                          (255, 190, 0),
                          -1)
                return self.create_display_frame(current_frame)
            else:
                # 第三次点击，开始画直线
                self.line_start = event_pos
                self.line_end = event_pos
                self.drawing = True
                return self.create_display_frame(current_frame)
        
        elif self.draw_mode == 'parallel':
            if self.step == 1:
                # 第一个点
                self.point1 = event_pos
                self.step = 2
                # 立即显示第一个点
                self.temp_display = np.zeros_like(current_frame)
                cv2.circle(self.temp_display, 
                          (self.point1.x(), self.point1.y()),
                          5, (255, 190, 0), -1)
                return self.create_display_frame(current_frame)
                
            elif self.step == 2:
                # 第二个点，确定第一条线
                self.point2 = event_pos
                self.step = 3
                # 立即显示无限长直线
                self.temp_display = np.zeros_like(current_frame)
                self.draw_infinite_line(self.point1, self.point2)
                return self.create_display_frame(current_frame)
                
            elif self.step == 3:
                # 第三个点，完成平行线绘制
                if self.display_image is None:
                    self.display_image = np.zeros_like(current_frame)
                
                # 保存第一条线
                self.temp_display = np.zeros_like(current_frame)
                self.draw_infinite_line(self.point1, self.point2)
                
                # 绘制平行线和中线
                self.draw_preview_parallel_line(event_pos)
                
                # 保存到显示图像
                mask = cv2.cvtColor(self.temp_display, cv2.COLOR_BGR2GRAY) > 0
                self.display_image[mask] = self.temp_display[mask]
                
                # 保存到历史记录
                self.drawing_history.append(self.display_image.copy())
                
                # 重置状态
                self.step = 1
                self.point1 = None
                self.point2 = None
                self.preview_point = None
                
                return self.create_display_frame(current_frame)
        
        elif self.draw_mode == 'two_lines':
            if self.is_drawing_first_line:
                # 开始画第一条线
                self.first_line_start = event_pos
                self.drawing = True
            else:
                # 开始画第二条线
                self.second_line_start = event_pos
                self.drawing = True
            return self.create_display_frame(current_frame)
        
        else:  # 其他绘画模式
            self.line_start = event_pos
            self.drawing = True
            if self.draw_mode == 'circle':
                self.circle_center = event_pos
        
        return self.create_display_frame(current_frame)
        
    def handle_mouse_move(self, event_pos, current_frame):
        """处理鼠标移动事件"""
        print(f"处理鼠标移动事件: drawing={self.drawing}, mode={self.draw_mode}, step={self.step}")
        
        # 初始化临时显示层
        if self.temp_display is None:
            self.temp_display = np.zeros_like(current_frame)
        
        if self.draw_mode == 'parallel':
            self.temp_display = np.zeros_like(current_frame)
            
            if self.step == 2 and self.point1:
                # 绘制第一个点和预览线
                cv2.circle(self.temp_display, 
                         (self.point1.x(), self.point1.y()),
                         5, (255, 190, 0), -1)
                # 绘制预览无限长直线
                self.draw_infinite_line(self.point1, event_pos)
                return self.create_display_frame(current_frame)
            
            elif self.step == 3 and self.point1 and self.point2:
                # 绘制第一条线和预览的平行线组合
                self.draw_infinite_line(self.point1, self.point2)
                self.draw_preview_parallel_line(event_pos)
                return self.create_display_frame(current_frame)
        
        elif self.draw_mode == 'circle_line' and self.drawing:
            self.line_end = event_pos
            self.temp_display = np.zeros_like(current_frame)
            
            if self.drawing_circle:
                # 绘制预览圆
                if self.circle_center:
                    radius = int(np.sqrt(
                        (event_pos.x() - self.circle_center.x())**2 + 
                        (event_pos.y() - self.circle_center.y())**2
                    ))
                    cv2.circle(self.temp_display, 
                              (self.circle_center.x(), self.circle_center.y()),
                              radius,
                              (255, 190, 0),
                              2)
                    # 绘制圆心
                    cv2.circle(self.temp_display,
                              (self.circle_center.x(), self.circle_center.y()),
                              3,
                              (255, 190, 0),
                              -1)
            else:
                # 绘制已确定的圆
                cv2.circle(self.temp_display, 
                          (self.circle_center.x(), self.circle_center.y()),
                          self.circle_radius,
                          (255, 190, 0),
                          2)
                # 绘制圆心
                cv2.circle(self.temp_display,
                          (self.circle_center.x(), self.circle_center.y()),
                          3,
                          (255, 190, 0),
                          -1)
                
                # 绘制预览直线
                if self.line_start:
                    # 计算无限长直线的参数
                    height, width = self.temp_display.shape[:2]
                    max_length = int(np.sqrt(width**2 + height**2))
                    
                    dx = event_pos.x() - self.line_start.x()
                    dy = event_pos.y() - self.line_start.y()
                    length = np.sqrt(dx*dx + dy*dy)
                    if length > 0:
                        dx, dy = dx/length, dy/length
                        
                        # 计算无限长直线的起点和终点
                        start_x = int(self.line_start.x() - dx * max_length)
                        start_y = int(self.line_start.y() - dy * max_length)
                        end_x = int(self.line_start.x() + dx * max_length)
                        end_y = int(self.line_start.y() + dy * max_length)
                        
                        # 绘制无限长直线
                        cv2.line(self.temp_display,
                                (start_x, start_y),
                                (end_x, end_y),
                                (255, 190, 0), 2)
                        
                        # 计算圆心到直线的垂直距离
                        distance = abs(dy * self.circle_center.x() - dx * self.circle_center.y() + 
                                     dx * self.line_start.y() - dy * self.line_start.x()) / np.sqrt(dx*dx + dy*dy)
                        
                        # 计算垂线的起点（圆心）和终点
                        # 计算垂足点
                        t = (dx * (self.circle_center.x() - self.line_start.x()) + 
                             dy * (self.circle_center.y() - self.line_start.y())) / (dx * dx + dy * dy)
                        foot_x = self.line_start.x() + t * dx
                        foot_y = self.line_start.y() + t * dy
                        
                        # 绘制垂线（虚线）
                        self.draw_dashed_line(
                            (self.circle_center.x(), self.circle_center.y()),
                            (int(foot_x), int(foot_y))
                        )
                        
                        # 使用PIL绘制中文
                        temp_display_rgb = cv2.cvtColor(self.temp_display, cv2.COLOR_BGR2RGB)
                        pil_image = Image.fromarray(temp_display_rgb)
                        draw = ImageDraw.Draw(pil_image)
                        
                        try:
                            fontpath = "C:/Windows/Fonts/msyh.ttc"  # 微软雅黑
                            font = ImageFont.truetype(fontpath, 40)
                        except:
                            try:
                                fontpath = "C:/Windows/Fonts/simsun.ttc"  # 宋体
                                font = ImageFont.truetype(fontpath, 40)
                            except:
                                font = ImageFont.load_default()
                        
                        text_x = self.circle_center.x() + 10
                        text_y = self.circle_center.y()
                        text = f"距离: {distance:.1f}px"  # 修改文字
                        
                        # 获取文字区域
                        bbox = draw.textbbox((text_x, text_y), text, font=font)
                        
                        # 创建半透明背景
                        overlay = Image.new('RGBA', pil_image.size, (0, 0, 0, 0))
                        overlay_draw = ImageDraw.Draw(overlay)
                        
                        # 绘制文字背景
                        overlay_draw.rectangle(
                            [bbox[0]-5, bbox[1]-5, bbox[2]+5, bbox[3]+5],
                            fill=(0, 0, 0, 160)
                        )
                        
                        # 合并图层
                        pil_image = Image.alpha_composite(pil_image.convert('RGBA'), overlay)
                        draw = ImageDraw.Draw(pil_image)
                        
                        # 绘制文字边框
                        for offset in [(1,1), (-1,-1), (1,-1), (-1,1)]:
                            draw.text((text_x + offset[0], text_y + offset[1]), 
                                    text, fill=(0, 0, 0), font=font)
                        
                        # 绘制主文字
                        draw.text((text_x, text_y), text, 
                                 fill=(230, 190, 50), font=font)
                        
                        # 更新显示图像
                        self.temp_display = cv2.cvtColor(np.array(pil_image.convert('RGB')), cv2.COLOR_RGB2BGR)
            
            return self.create_display_frame(current_frame)
        
        elif self.draw_mode == 'two_lines' and self.drawing:
            # 使用 draw_two_lines 方法来处理所有绘制
            self.draw_two_lines(current_frame, preview_pos=event_pos)
            return self.create_display_frame(current_frame)
        
        # 其他绘画模式的处理
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
            
        return self.create_display_frame(current_frame)
        
    def handle_mouse_release(self, event_pos, current_frame):
        """处理鼠标释放事件"""
        if not self.drawing:
            return None
            
        self.drawing = False
        if self.temp_display is not None:
            if self.draw_mode == 'two_lines':
                if self.is_drawing_first_line:
                    # 完成第一条线
                    self.first_line_end = event_pos
                    self.is_drawing_first_line = False
                    
                    # 保存第一条线到显示图像
                    if self.display_image is None:
                        self.display_image = np.zeros_like(current_frame)
                    mask = cv2.cvtColor(self.temp_display, cv2.COLOR_BGR2GRAY) > 0
                    self.display_image[mask] = self.temp_display[mask]
                    
                    return self.create_display_frame(current_frame)
                else:
                    # 完成第二条线
                    self.second_line_end = event_pos
                    
                    # 计算并显示交点和夹角
                    intersection, angle = self.calculate_lines_intersection(
                        self.first_line_start, self.first_line_end,
                        self.second_line_start, self.second_line_end
                    )
                    
                    if intersection:
                        # 绘制交点
                        cv2.circle(self.temp_display,
                                  (int(intersection[0]), int(intersection[1])),
                                  5, (255, 190, 0), -1)
                        
                        # 添加文字说明
                        temp_display_rgb = cv2.cvtColor(self.temp_display, cv2.COLOR_BGR2RGB)
                        pil_image = Image.fromarray(temp_display_rgb)
                        draw = ImageDraw.Draw(pil_image)
                        
                        try:
                            fontpath = "C:/Windows/Fonts/msyh.ttc"
                            font = ImageFont.truetype(fontpath, 40)
                        except:
                            try:
                                fontpath = "C:/Windows/Fonts/simsun.ttc"
                                font = ImageFont.truetype(fontpath, 40)
                            except:
                                font = ImageFont.load_default()
                        
                        text_x = int(intersection[0]) + 10
                        text_y = int(intersection[1])
                        text = f"夹角: {angle:.1f}°"
                        
                        # 获取文字区域
                        bbox = draw.textbbox((text_x, text_y), text, font=font)
                        
                        # 创建半透明背景
                        overlay = Image.new('RGBA', pil_image.size, (0, 0, 0, 0))
                        overlay_draw = ImageDraw.Draw(overlay)
                        overlay_draw.rectangle(
                            [bbox[0]-5, bbox[1]-5, bbox[2]+5, bbox[3]+5],
                            fill=(0, 0, 0, 160)
                        )
                        
                        # 合并图层
                        pil_image = Image.alpha_composite(pil_image.convert('RGBA'), overlay)
                        draw = ImageDraw.Draw(pil_image)
                        
                        # 绘制文字边框和主文字
                        for offset in [(1,1), (-1,-1), (1,-1), (-1,1)]:
                            draw.text((text_x + offset[0], text_y + offset[1]), 
                                    text, fill=(0, 0, 0), font=font)
                        draw.text((text_x, text_y), text, 
                                 fill=(230, 190, 50), font=font)
                        
                        self.temp_display = cv2.cvtColor(np.array(pil_image.convert('RGB')), 
                                                       cv2.COLOR_RGB2BGR)
                    
                    # 保存到显示图像
                    if self.display_image is None:
                        self.display_image = np.zeros_like(current_frame)
                    mask = cv2.cvtColor(self.temp_display, cv2.COLOR_BGR2GRAY) > 0
                    self.display_image[mask] = self.temp_display[mask]
                    
                    # 保存到历史记录
                    self.drawing_history.append(self.display_image.copy())
                    
                    # 重置状态
                    self.is_drawing_first_line = True
                    self.first_line_start = None
                    self.first_line_end = None
                    self.second_line_start = None
                    self.second_line_end = None
                    
                    return self.create_display_frame(current_frame)
            
            elif self.draw_mode == 'circle_line':
                if self.drawing_circle:
                    # 第二次点击，确定圆的半径
                    self.circle_radius = int(np.sqrt(
                        (event_pos.x() - self.circle_center.x())**2 + 
                        (event_pos.y() - self.circle_center.y())**2
                    ))
                    self.drawing_circle = False
                    
                    # 保存当前状态到显示图像
                    if self.display_image is None:
                        self.display_image = np.zeros_like(current_frame)
                    mask = cv2.cvtColor(self.temp_display, cv2.COLOR_BGR2GRAY) > 0
                    self.display_image[mask] = self.temp_display[mask]
                    
                    return self.create_display_frame(current_frame)
                else:
                    # 第四次点击，完成直线绘制
                    # 保存到显示图像
                    if self.display_image is None:
                        self.display_image = np.zeros_like(current_frame)
                    mask = cv2.cvtColor(self.temp_display, cv2.COLOR_BGR2GRAY) > 0
                    self.display_image[mask] = self.temp_display[mask]
                    
                    # 保存到历史记录
                    self.drawing_history.append(self.display_image.copy())
                    
                    # 重置状态
                    self.drawing_circle = True
                    self.circle_center = None
                    self.circle_radius = None
                    self.line_start = None
                    self.line_end = None
                    
                    return self.create_display_frame(current_frame)
            
            else:  # 其他测量模式（直线、圆、线段等）
                # 保存到显示图像
                if self.display_image is None:
                    self.display_image = np.zeros_like(current_frame)
                mask = cv2.cvtColor(self.temp_display, cv2.COLOR_BGR2GRAY) > 0
                self.display_image[mask] = self.temp_display[mask]
                
                # 保存到历史记录
                self.drawing_history.append(self.display_image.copy())
                
                # 重置状态
                self.line_start = None
                self.line_end = None
                
                return self.create_display_frame(current_frame)
        
        return None

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

    def undo_last_drawing(self):
        """撤销上一步绘画"""
        print("--- MeasurementManager.undo_last_drawing 开始 ---")
        if not self.drawing_history:  # 如果历史记录为空
            print("没有可撤销的历史记录")
            self.display_image = None
            self.temp_display = None
            print("--- MeasurementManager.undo_last_drawing 结束 ---")
            return False
        
        print(f"撤销绘画，当前历史记录数: {len(self.drawing_history)}")
        # 恢复上一个状态
        self.display_image = self.drawing_history.pop()
        # 清理所有临时状态
        self.temp_display = None
        self.line_start = None
        self.line_end = None
        self.circle_center = None
        self.drawing = False
        print("--- MeasurementManager.undo_last_drawing 结束 ---")
        return True 

    def draw_preview_line(self, start, end):
        """绘制预览虚线"""
        # 计算方向向量
        dx = end.x() - start.x()
        dy = end.y() - start.y()
        length = np.sqrt(dx*dx + dy*dy)
        if length > 0:
            dx, dy = dx/length, dy/length
            
            # 绘制虚线
            dash_length = 10
            gap_length = 5
            pos = 0
            total_length = length
            while pos < total_length:
                curr_start_x = int(start.x() + dx * pos)
                curr_start_y = int(start.y() + dy * pos)
                curr_end_pos = min(pos + dash_length, total_length)
                curr_end_x = int(start.x() + dx * curr_end_pos)
                curr_end_y = int(start.y() + dy * curr_end_pos)
                cv2.line(self.temp_display,
                        (curr_start_x, curr_start_y),
                        (curr_end_x, curr_end_y),
                        (255, 190, 0), 2)
                pos += dash_length + gap_length

    def draw_infinite_line(self, p1, p2):
        """绘制无限长直线"""
        height, width = self.temp_display.shape[:2]
        max_length = int(np.sqrt(width**2 + height**2))
        
        dx = p2.x() - p1.x()
        dy = p2.y() - p1.y()
        length = np.sqrt(dx*dx + dy*dy)
        if length > 0:
            dx, dy = dx/length, dy/length
            
            start_x = int(p1.x() - dx * max_length)
            start_y = int(p1.y() - dy * max_length)
            end_x = int(p1.x() + dx * max_length)
            end_y = int(p1.y() + dy * max_length)
            
            cv2.line(self.temp_display,
                    (start_x, start_y),
                    (end_x, end_y),
                    (255, 190, 0), 3)

    def draw_preview_parallel_line(self, point):
        """绘制预览的平行线和中线"""
        height, width = self.temp_display.shape[:2]
        max_length = int(np.sqrt(width**2 + height**2))
        
        # 计算原始直线的方向向量
        dx = self.point2.x() - self.point1.x()
        dy = self.point2.y() - self.point1.y()
        length = np.sqrt(dx*dx + dy*dy)
        if length > 0:
            dx, dy = dx/length, dy/length
            
            # 计算角度
            angle = np.degrees(np.arctan2(-dy, dx))
            if angle < 0:
                angle += 360
            
            # 计算平行线间距
            # 使用点到直线的距离公式
            distance = abs(dy * point.x() - dx * point.y() + 
                         (self.point1.y() * dx - self.point1.x() * dy)) / np.sqrt(dx*dx + dy*dy)
            
            # 绘制平行线
            start_x = int(point.x() - dx * max_length)
            start_y = int(point.y() - dy * max_length)
            end_x = int(point.x() + dx * max_length)
            end_y = int(point.y() + dy * max_length)
            
            cv2.line(self.temp_display,
                    (start_x, start_y),
                    (end_x, end_y),
                    (255, 190, 0), 3)
            
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
            
            # 在中点位置显示文字
            text_x = (self.point1.x() + point.x()) // 2 + 20
            text_y = (self.point1.y() + point.y()) // 2
            line_spacing = 50
            
            # 准备文字内容
            angle_text = f"角度: {angle:.1f}°"
            distance_text = f"距离: {distance:.1f}px"
            
            # 获取文字区域
            angle_bbox = draw.textbbox((text_x, text_y - line_spacing), angle_text, font=font)
            distance_bbox = draw.textbbox((text_x, text_y), distance_text, font=font)
            
            # 创建半透明背景
            overlay = Image.new('RGBA', pil_image.size, (0, 0, 0, 0))
            overlay_draw = ImageDraw.Draw(overlay)
            
            # 绘制文字背景
            overlay_draw.rectangle(
                [angle_bbox[0]-5, angle_bbox[1]-5, angle_bbox[2]+5, angle_bbox[3]+5],
                fill=(0, 0, 0, 160)
            )
            overlay_draw.rectangle(
                [distance_bbox[0]-5, distance_bbox[1]-5, distance_bbox[2]+5, distance_bbox[3]+5],
                fill=(0, 0, 0, 160)
            )
            
            # 合并图层
            pil_image = Image.alpha_composite(pil_image.convert('RGBA'), overlay)
            draw = ImageDraw.Draw(pil_image)
            
            # 绘制文字边框
            for offset in [(1,1), (-1,-1), (1,-1), (-1,1)]:
                draw.text((text_x + offset[0], text_y - line_spacing + offset[1]), 
                         angle_text, fill=(0, 0, 0), font=font)
                draw.text((text_x + offset[0], text_y + offset[1]), 
                         distance_text, fill=(0, 0, 0), font=font)
            
            # 绘制主文字
            draw.text((text_x, text_y - line_spacing), angle_text, 
                     fill=(230, 190, 50), font=font)
            draw.text((text_x, text_y), distance_text, 
                     fill=(230, 190, 50), font=font)
            
            # 更新显示图像
            self.temp_display = cv2.cvtColor(np.array(pil_image.convert('RGB')), cv2.COLOR_RGB2BGR)
            
            # 绘制中线
            mid_start_x = (self.point1.x() + start_x) // 2
            mid_start_y = (self.point1.y() + start_y) // 2
            mid_end_x = (self.point2.x() + end_x) // 2
            mid_end_y = (self.point2.y() + end_y) // 2
            
            # 绘制虚线中线
            self.draw_dashed_line(
                (mid_start_x, mid_start_y),
                (mid_end_x, mid_end_y)
            )

    def draw_dashed_line(self, start, end):
        """绘制虚线"""
        dx = end[0] - start[0]
        dy = end[1] - start[1]
        length = np.sqrt(dx*dx + dy*dy)
        if length > 0:
            dx, dy = dx/length, dy/length
            
            dash_length = 10
            gap_length = 5
            pos = 0
            while pos < length:
                curr_start_x = int(start[0] + dx * pos)
                curr_start_y = int(start[1] + dy * pos)
                curr_end_pos = min(pos + dash_length, length)
                curr_end_x = int(start[0] + dx * curr_end_pos)
                curr_end_y = int(start[1] + dy * curr_end_pos)
                cv2.line(self.temp_display,
                        (curr_start_x, curr_start_y),
                        (curr_end_x, curr_end_y),
                        (255, 190, 0), 2)
                pos += dash_length + gap_length 

    def calculate_lines_intersection(self, p1, p2, p3, p4):
        """计算两条直线的交点和夹角
        返回: (交点坐标, 夹角) 或 (None, None)
        """
        # 计算直线参数
        dx1 = p2.x() - p1.x()
        dy1 = p2.y() - p1.y()
        dx2 = p4.x() - p3.x()
        dy2 = p4.y() - p3.y()
        
        # 计算行列式
        det = dx1 * dy2 - dy1 * dx2
        
        if abs(det) < 1e-6:  # 平行或重合
            return None, None
        
        # 计算交点
        t1 = ((p3.x() - p1.x()) * dy2 - (p3.y() - p1.y()) * dx2) / det
        x = p1.x() + t1 * dx1
        y = p1.y() + t1 * dy1
        
        # 计算夹角
        angle1 = np.degrees(np.arctan2(-dy1, dx1))
        angle2 = np.degrees(np.arctan2(-dy2, dx2))
        angle = abs(angle1 - angle2)
        if angle > 180:
            angle = 360 - angle
        if angle > 90:
            angle = 180 - angle
        
        return (x, y), angle 

    def draw_two_lines(self, current_frame, preview_pos=None, final=False):
        """绘制两条线并计算夹角"""
        self.temp_display = np.zeros_like(current_frame)
        height, width = self.temp_display.shape[:2]
        max_length = int(np.sqrt(width**2 + height**2))

        # 绘制第一条直线
        if self.first_line_start:
            end_point = self.first_line_end if self.first_line_end else preview_pos
            if end_point:
                dx1 = end_point.x() - self.first_line_start.x()
                dy1 = end_point.y() - self.first_line_start.y()
                length1 = np.sqrt(dx1**2 + dy1**2)
                if length1 > 0:
                    dx1, dy1 = dx1/length1, dy1/length1
                    start_x = int(self.first_line_start.x() - dx1 * max_length)
                    start_y = int(self.first_line_start.y() - dy1 * max_length)
                    end_x = int(self.first_line_start.x() + dx1 * max_length)
                    end_y = int(self.first_line_start.y() + dy1 * max_length)
                    cv2.line(self.temp_display, (start_x, start_y), 
                            (end_x, end_y), (255, 190, 0), 2)

        # 绘制第二条直线
        if self.second_line_start:
            end_point = self.second_line_end if self.second_line_end else preview_pos
            if end_point:
                dx2 = end_point.x() - self.second_line_start.x()
                dy2 = end_point.y() - self.second_line_start.y()
                length2 = np.sqrt(dx2**2 + dy2**2)
                if length2 > 0:
                    dx2, dy2 = dx2/length2, dy2/length2
                    start_x = int(self.second_line_start.x() - dx2 * max_length)
                    start_y = int(self.second_line_start.y() - dy2 * max_length)
                    end_x = int(self.second_line_start.x() + dx2 * max_length)
                    end_y = int(self.second_line_start.y() + dy2 * max_length)
                    cv2.line(self.temp_display, (start_x, start_y), 
                            (end_x, end_y), (255, 190, 0), 2)

            if self.first_line_end:  # 第一条线已确定
                # 计算交点和夹角
                intersection, angle = self.calculate_lines_intersection(
                    self.first_line_start, self.first_line_end,
                    self.second_line_start, end_point
                )
                
                if intersection:
                    # 绘制交点
                    cv2.circle(self.temp_display,
                             (int(intersection[0]), int(intersection[1])),
                             5, (255, 190, 0), -1)
                    
                    # 添加文字说明
                    temp_display_rgb = cv2.cvtColor(self.temp_display, cv2.COLOR_BGR2RGB)
                    pil_image = Image.fromarray(temp_display_rgb)
                    draw = ImageDraw.Draw(pil_image)
                    
                    try:
                        fontpath = "C:/Windows/Fonts/msyh.ttc"
                        font = ImageFont.truetype(fontpath, 40)
                    except:
                        try:
                            fontpath = "C:/Windows/Fonts/simsun.ttc"
                            font = ImageFont.truetype(fontpath, 40)
                        except:
                            font = ImageFont.load_default()
                    
                    text_x = int(intersection[0]) + 10
                    text_y = int(intersection[1])
                    text = f"夹角: {angle:.1f}°"
                    
                    # 获取文字区域
                    bbox = draw.textbbox((text_x, text_y), text, font=font)
                    
                    # 创建半透明背景
                    overlay = Image.new('RGBA', pil_image.size, (0, 0, 0, 0))
                    overlay_draw = ImageDraw.Draw(overlay)
                    overlay_draw.rectangle(
                        [bbox[0]-5, bbox[1]-5, bbox[2]+5, bbox[3]+5],
                        fill=(0, 0, 0, 160)
                    )
                    
                    # 合并图层
                    pil_image = Image.alpha_composite(pil_image.convert('RGBA'), overlay)
                    draw = ImageDraw.Draw(pil_image)
                    
                    # 绘制文字边框和主文字
                    for offset in [(1,1), (-1,-1), (1,-1), (-1,1)]:
                        draw.text((text_x + offset[0], text_y + offset[1]), 
                                text, fill=(0, 0, 0), font=font)
                    draw.text((text_x, text_y), text, 
                             fill=(230, 190, 50), font=font)
                    
                    self.temp_display = cv2.cvtColor(np.array(pil_image.convert('RGB')), 
                                                   cv2.COLOR_RGB2BGR)

        if final:
            # 保存到显示图像
            if self.display_image is None:
                self.display_image = np.zeros_like(current_frame)
            mask = cv2.cvtColor(self.temp_display, cv2.COLOR_BGR2GRAY) > 0
            self.display_image[mask] = self.temp_display[mask]
            # 保存到历史记录
            self.drawing_history.append(self.display_image.copy()) 