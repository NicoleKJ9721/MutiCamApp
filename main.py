import warnings
warnings.filterwarnings("ignore", category=DeprecationWarning)
import sys
import os

# 添加项目根目录到系统路径
current_dir = os.path.dirname(os.path.abspath(__file__))
if current_dir not in sys.path:
    sys.path.append(current_dir)

from PyQt5.QtWidgets import *
from PyQt5.QtGui import QImage, QPixmap
from PyQt5.QtCore import Qt, QThread
from mainwindow import Ui_MainWindow
# import cv2
from Tools.camera_thread import CameraThread
from Tools.settings_manager import SettingsManager
from Tools.log_manager import LogManager

class MainApp(QMainWindow, Ui_MainWindow):
    def __init__(self):
        super().__init__()
        self.setupUi(self)
        
        # 初始化日志管理器
        self.log_manager = LogManager()
        self.log_manager.log_ui_operation("程序启动")
        
        # 初始化相机线程
        self.ver_camera_thread = None
        self.left_camera_thread = None
        self.front_camera_thread = None

        # 初始化设置管理器
        self.settings_manager = SettingsManager()
        
        # 初始化UI控件
        self._init_ui()
        
        # 连接信号槽
        self._connect_signals()

    def _init_ui(self):
        """初始化UI控件"""
        # 初始化ComboBox选项
        self.settings_manager.init_combo_boxes(self)
        
        # 加载保存的设置
        self.settings_manager.load_settings(self)
        
        # 初始化按钮状态
        self.btnStopMeasure.setEnabled(False)

    def _connect_signals(self):
        """连接信号槽"""
        # 相机参数修改信号
        self.ledVerCamExposureTime.textChanged.connect(lambda: self.settings_manager.save_settings(self))
        self.ledLeftCamExposureTime.textChanged.connect(lambda: self.settings_manager.save_settings(self))
        self.ledFrontCamExposureTime.textChanged.connect(lambda: self.settings_manager.save_settings(self))
        self.ledVerCamSN.textChanged.connect(lambda: self.settings_manager.save_settings(self))
        self.ledLeftCamSN.textChanged.connect(lambda: self.settings_manager.save_settings(self))
        self.ledFrontCamSN.textChanged.connect(lambda: self.settings_manager.save_settings(self))
        
        # 图像格式修改信号
        self.cbVerCamImageFormat.currentTextChanged.connect(lambda: self.settings_manager.save_settings(self))
        self.cbLeftCamImageFormat.currentTextChanged.connect(lambda: self.settings_manager.save_settings(self))
        self.cbFrontCamImageFormat.currentTextChanged.connect(lambda: self.settings_manager.save_settings(self))

        # 相机控制信号
        self.btnStartMeasure.clicked.connect(self.start_cameras)
        self.btnStopMeasure.clicked.connect(self.stop_cameras)

        # 参数更新信号
        self.ledVerCamExposureTime.editingFinished.connect(self.update_camera_params)
        self.ledLeftCamExposureTime.editingFinished.connect(self.update_camera_params)
        self.ledFrontCamExposureTime.editingFinished.connect(self.update_camera_params)
        self.cbVerCamImageFormat.currentTextChanged.connect(self.update_camera_params)
        self.cbLeftCamImageFormat.currentTextChanged.connect(self.update_camera_params)
        self.cbFrontCamImageFormat.currentTextChanged.connect(self.update_camera_params)

    def update_camera_params(self):
        """更新相机参数"""
        try:
            # 记录参数修改
            if self.ver_camera_thread and self.ver_camera_thread.running:
                old_exposure = self.ver_camera_thread.exposure_time
                new_exposure = int(self.ledVerCamExposureTime.text())
                if old_exposure != new_exposure:
                    self.log_manager.log_parameter_change("垂直相机曝光时间", old_exposure, new_exposure, 
                                                        self.ledVerCamSN.text())
            
            # 如果相机正在运行，先停止
            is_running = False
            if self.ver_camera_thread and self.ver_camera_thread.running:
                is_running = True
                self.stop_cameras()
                
            # 保存新的设置
            self.settings_manager.save_settings(self)
            
            # 如果之前在运行，则重新启动相机
            if is_running:
                self.start_cameras()
                
        except Exception as e:
            error_msg = f"更新相机参数失败: {str(e)}"
            self.log_manager.log_error(error_msg)
            self.show_error(error_msg)

    def start_cameras(self):
        try:
            self.log_manager.log_ui_operation("开始测量")
            
            # 垂直相机
            self.ver_camera_thread = CameraThread(
                camera_sn=self.ledVerCamSN.text(),
                exposure_time=int(self.ledVerCamExposureTime.text()),
                image_format='Mono8' if self.cbVerCamImageFormat.currentText() == 'Mono8' else 'RGB8'
            )
            self.ver_camera_thread.frame_ready.connect(self.update_ver_camera_view)
            self.ver_camera_thread.error_occurred.connect(self.show_error)
            self.ver_camera_thread.start()
            self.log_manager.log_camera_operation("启动", self.ledVerCamSN.text(), 
                                                f"曝光时间: {self.ledVerCamExposureTime.text()}, 格式: {self.cbVerCamImageFormat.currentText()}")

            # 左相机
            self.left_camera_thread = CameraThread(
                camera_sn=self.ledLeftCamSN.text(),
                exposure_time=int(self.ledLeftCamExposureTime.text()),
                image_format='Mono8' if self.cbLeftCamImageFormat.currentText() == 'Mono8' else 'RGB8'
            )
            self.left_camera_thread.frame_ready.connect(self.update_left_camera_view)
            self.left_camera_thread.error_occurred.connect(self.show_error)
            self.left_camera_thread.start()
            self.log_manager.log_camera_operation("启动", self.ledLeftCamSN.text(), 
                                                f"曝光时间: {self.ledLeftCamExposureTime.text()}, 格式: {self.cbLeftCamImageFormat.currentText()}")

            # 前相机
            self.front_camera_thread = CameraThread(
                camera_sn=self.ledFrontCamSN.text(),
                exposure_time=int(self.ledFrontCamExposureTime.text()),
                image_format='Mono8' if self.cbFrontCamImageFormat.currentText() == 'Mono8' else 'RGB8'
            )
            self.front_camera_thread.frame_ready.connect(self.update_front_camera_view)
            self.front_camera_thread.error_occurred.connect(self.show_error)
            self.front_camera_thread.start()
            self.log_manager.log_camera_operation("启动", self.ledFrontCamSN.text(), 
                                                f"曝光时间: {self.ledFrontCamExposureTime.text()}, 格式: {self.cbFrontCamImageFormat.currentText()}")

            self.btnStartMeasure.setEnabled(False)
            self.btnStopMeasure.setEnabled(True)

        except Exception as e:
            error_msg = f"启动相机失败: {str(e)}"
            self.log_manager.log_error(error_msg)
            self.show_error(error_msg)

    def stop_cameras(self):
        """停止所有相机"""
        try:
            self.log_manager.log_ui_operation("停止测量")
            
            # 停止所有相机线程
            for thread, name in [(self.ver_camera_thread, "垂直相机"), 
                               (self.left_camera_thread, "左相机"),
                               (self.front_camera_thread, "前相机")]:
                if thread:
                    if thread.running:
                        thread.stop()
                        self.log_manager.log_camera_operation("停止", details=name)
                    thread.wait()  # 等待线程完全停止
            
            # 清空线程引用
            self.ver_camera_thread = None
            self.left_camera_thread = None
            self.front_camera_thread = None

            # 更新按钮状态
            self.btnStartMeasure.setEnabled(True)
            self.btnStopMeasure.setEnabled(False)
            
        except Exception as e:
            error_msg = f"停止相机失败: {str(e)}"
            self.log_manager.log_error(error_msg)
            self.show_error(error_msg)

    def update_ver_camera_view(self, frame):
        self.display_image(frame, self.lbVerticalView)

    def update_left_camera_view(self, frame):
        self.display_image(frame, self.lbLeftView)

    def update_front_camera_view(self, frame):
        self.display_image(frame, self.lbFrontView)

    def display_image(self, frame, label):
        try:
            height, width = frame.shape[:2]
            if len(frame.shape) == 2:  # Mono8
                bytes_per_line = width
                q_img = QImage(frame.data, width, height, bytes_per_line, QImage.Format_Grayscale8)
            else:  # RGB
                bytes_per_line = 3 * width
                q_img = QImage(frame.data, width, height, bytes_per_line, QImage.Format_RGB888)

            # 缩放图像以适应标签大小
            pixmap = QPixmap.fromImage(q_img)
            label.setPixmap(pixmap.scaled(label.size(), Qt.KeepAspectRatio))

        except Exception as e:
            self.show_error(f"显示图像失败: {str(e)}")

    def show_error(self, message):
        QMessageBox.critical(self, "错误", message)

    def closeEvent(self, event):
        """关闭窗口事件处理"""
        try:
            self.log_manager.log_ui_operation("关闭程序")
            self.stop_cameras()
            QThread.msleep(100)
            event.accept()
        except Exception as e:
            error_msg = f"关闭窗口时出错: {str(e)}"
            self.log_manager.log_error(error_msg)
            event.accept()

def main():
    app = QApplication(sys.argv)
    main_window = MainApp()
    main_window.show()
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()