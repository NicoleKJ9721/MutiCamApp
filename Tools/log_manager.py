import os
import time
from datetime import datetime
from PyQt5.QtWidgets import QScrollArea, QTextEdit
from PyQt5.QtCore import QObject, Qt
from PyQt5.QtGui import QTextCursor

class LogManager:
    def __init__(self, scroll_area=None):
        """
        初始化日志管理器
        Args:
            scroll_area: QScrollArea控件，可选。如果提供则显示日志在UI上
        """
        # UI相关初始化
        self.scroll_area = scroll_area
        if scroll_area:
            self.text_edit = QTextEdit()
            self.scroll_area.setWidget(self.text_edit)
            self.text_edit.setReadOnly(True)
        
        # 创建日志目录
        self.log_dir = "./Logs"
        os.makedirs(self.log_dir, exist_ok=True)
        
        # 获取当前日期作为日志文件名
        self.current_date = datetime.now().strftime("%Y-%m-%d")
        self.log_file = os.path.join(self.log_dir, f"{self.current_date}.txt")

    def log_camera_operation(self, operation, camera_sn="", details=""):
        """记录相机操作"""
        message = f"相机操作 - {operation}"
        if camera_sn:
            message += f" - 相机SN: {camera_sn}"
        if details:
            message += f" - 详情: {details}"
        self.log(message)

    def log_parameter_change(self, param_name, old_value, new_value, camera_sn=""):
        """记录参数修改"""
        message = f"参数修改 - {param_name} - 从 {old_value} 改为 {new_value}"
        if camera_sn:
            message += f" - 相机SN: {camera_sn}"
        self.log(message)

    def log_ui_operation(self, operation, details=""):
        """记录UI操作"""
        message = f"界面操作 - {operation}"
        if details:
            message += f" - 详情: {details}"
        self.log(message)

    def log_error(self, error_msg, details=""):
        """记录错误信息"""
        message = f"错误 - {error_msg}"
        if details:
            message += f" - 详情: {details}"
        self.log(message, level="ERROR")

    def log(self, message, level="INFO"):
        """
        记录日志
        Args:
            message: 日志消息
            level: 日志级别，默认为INFO
        """
        # 检查是否需要创建新的日志文件
        current_date = datetime.now().strftime("%Y-%m-%d")
        if current_date != self.current_date:
            self.current_date = current_date
            self.log_file = os.path.join(self.log_dir, f"{self.current_date}.txt")

        # 获取当前时间
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        log_message = f"[{timestamp}] [{level}] {message}"

        # 如果有UI，更新UI显示
        if hasattr(self, 'text_edit'):
            self.text_edit.append(log_message)
            # 滚动到底部
            self.text_edit.verticalScrollBar().setValue(
                self.text_edit.verticalScrollBar().maximum()
            )

        # 写入文件
        try:
            with open(self.log_file, 'a', encoding='utf-8') as f:
                f.write(log_message + '\n')
        except Exception as e:
            print(f"写入日志文件失败: {str(e)}") 