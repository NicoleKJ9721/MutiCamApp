from PyQt5.QtCore import QThread, pyqtSignal
import numpy as np
import time
from Tools.camera_controller import HikiCamera

class CameraThread(QThread):
    """相机线程类"""    
    frame_ready = pyqtSignal(np.ndarray)  # 图像信号
    error_occurred = pyqtSignal(str)      # 错误信号

    def __init__(self, camera_sn, exposure_time, image_format):
        super().__init__()
        self.camera = None
        self.camera_sn = camera_sn
        self.exposure_time = exposure_time
        self.image_format = image_format
        self.running = False

    def run(self):
        try:
            # 创建相机实例
            self.camera = HikiCamera(self.camera_sn)
            if self.camera.cam is None:
                self.error_occurred.emit(f"无法打开相机 {self.camera_sn}")
                return

            # 设置相机参数
            self.camera.set_exposure_time(self.exposure_time)
            self.camera.set_pixel_format(self.image_format)

            # 开始取流
            if not self.camera.start_grabbing(use_callback=True):
                self.error_occurred.emit(f"相机 {self.camera_sn} 开始取流失败")
                return

            self.running = True
            while self.running:
                frame = self.camera.get_frame()
                if frame is not None:
                    self.frame_ready.emit(frame)
                time.sleep(0.001)  # 避免CPU占用过高

        except Exception as e:
            self.error_occurred.emit(f"相机线程错误: {str(e)}")
        finally:
            if self.camera:
                self.camera.close_camera()

    def stop(self):
        self.running = False
        self.wait()
        if self.camera:
            self.camera.close_camera() 