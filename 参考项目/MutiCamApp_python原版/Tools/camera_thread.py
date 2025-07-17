from PyQt5.QtCore import QThread, pyqtSignal
import numpy as np
import time
from Tools.camera_controller import HikiCamera
import cv2
import psutil
import threading

class CameraThread(QThread):
    """相机线程类"""    
    frame_ready = pyqtSignal(np.ndarray)  # 图像信号
    error_occurred = pyqtSignal(str)      # 错误信号

    def __init__(self, camera_sn):
        super().__init__()
        self.camera = None
        self.camera_sn = camera_sn
        self.running = False
        self.current_frame = None
        self.target_fps = 20  # 降低目标帧率到15fps，减少CPU负担
        self.frame_interval = 1.0 / self.target_fps  # 帧间隔时间
        self.last_frame_time = 0  # 上一帧的时间戳
        self._frame_buffer = None  # 帧缓冲
        self._buffer_lock = threading.Lock()
        self._skip_frames = 0  # 跳帧计数器
        self._pre_initialized = False  # 预初始化标志

    def pre_initialize(self):
        """预初始化相机，创建相机实例但不开始取流"""
        try:
            if self.camera is None and not self._pre_initialized:
                # 创建相机实例
                self.camera = HikiCamera(self.camera_sn)
                if self.camera.cam is None:
                    self.error_occurred.emit(f"无法打开相机 {self.camera_sn}")
                    return False
                
                self._pre_initialized = True
                return True
            return False
        except Exception as e:
            self.error_occurred.emit(f"相机预初始化错误: {str(e)}")
            return False

    def _process_frame(self, frame):
        """处理图像帧"""
        if frame is None:
            return None
            
        try:
            # 获取相机的当前像素格式
            pixel_format = self.camera.get_pixel_format() if self.camera else None
            
            # 对于Mono8格式，不进行颜色空间转换，保持原始格式
            # 这样可以确保绘画系统能够正确处理灰度图像
            
            # 使用固定大小的帧缓冲区
            with self._buffer_lock:
                if self._frame_buffer is None or self._frame_buffer.shape != frame.shape:
                    self._frame_buffer = np.empty_like(frame)
                np.copyto(self._frame_buffer, frame)
                return self._frame_buffer
        except Exception as e:
            self.error_occurred.emit(f"处理图像帧失败: {str(e)}")
            return None

    def run(self):
        try:
            # 如果没有预初始化，则创建相机实例
            if not self._pre_initialized:
                # 创建相机实例
                self.camera = HikiCamera(self.camera_sn)
                if self.camera.cam is None:
                    self.error_occurred.emit(f"无法打开相机 {self.camera_sn}")
                    return

            # 开始取流
            if not self.camera.start_grabbing(use_callback=True):
                self.error_occurred.emit(f"相机 {self.camera_sn} 开始取流失败")
                return

            frame_count = 0
            self.running = True
            
            while self.running:
                current_time = time.time()
                
                # 检查是否需要获取新帧
                if current_time - self.last_frame_time >= self.frame_interval:
                    frame_count += 1
                    
                    # 根据跳帧设置决定是否处理这一帧
                    if frame_count % (self._skip_frames + 1) == 0:
                        frame = self.camera.get_frame()
                        if frame is not None:
                            processed_frame = self._process_frame(frame)
                            if processed_frame is not None:
                                self.current_frame = processed_frame
                                self.frame_ready.emit(processed_frame)
                            
                    self.last_frame_time = current_time
                
                # 动态调整休眠时间，增加最小休眠时间
                sleep_time = max(0.005, self.frame_interval - (time.time() - current_time))
                if sleep_time > 0:
                    time.sleep(sleep_time)

        except Exception as e:
            self.error_occurred.emit(f"相机线程错误: {str(e)}")
        finally:
            if self.camera:
                self.camera.close_camera()
                self._pre_initialized = False

    def stop(self):
        """停止相机线程"""
        self.running = False
        self.wait()
        if self.camera:
            self.camera.close_camera()
            self._pre_initialized = False
        # 清理资源
        with self._buffer_lock:
            self._frame_buffer = None
        self.current_frame = None
            
    def set_target_fps(self, fps):
        """设置目标帧率"""
        self.target_fps = max(1, min(fps, 60))  # 限制帧率在1-60之间
        self.frame_interval = 1.0 / self.target_fps 