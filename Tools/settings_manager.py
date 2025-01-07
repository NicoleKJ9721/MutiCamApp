import os
import json
import logging
from PyQt5.QtWidgets import QMessageBox

class SettingsManager:
    def __init__(self, settings_file="./Settings/settings.json"):
        self.settings_file = settings_file
        # 设置默认值
        self.default_settings = {
            # 相机参数
            "VerCamExposureTime": 50000,
            "LeftCamExposureTime": 50000,
            "FrontCamExposureTime": 50000,
            "VerCamSN": "Vir7779343",
            "LeftCamSN": "Vir7898388",
            "FrontCamSN": "Vir7898406",
            "VerCamImageFormat": "RGB",
            "LeftCamImageFormat": "RGB",
            "FrontCamImageFormat": "RGB",
            
            # 直线查找参数
            "LineDetThreshold": 50,
            "LineDetMinLength": 100,
            "LineDetMaxGap": 10,
            
            # 圆查找参数
            "CircleDetMinDist": 20,
            "CircleDetParam1": 50,
            "CircleDetParam2": 30,
            "CircleDetMinRadius": 20,
            "CircleDetMaxRadius": 100
        }

    def load_settings(self, ui):
        """加载设置到UI"""
        try:
            # 确保目录存在
            os.makedirs(os.path.dirname(self.settings_file), exist_ok=True)
            
            # 尝试读取保存的设置
            if os.path.exists(self.settings_file) and os.path.getsize(self.settings_file) > 0:
                with open(self.settings_file, 'r', encoding='utf-8') as f:
                    settings = json.load(f)
            else:
                settings = self.default_settings
                # 如果文件不存在或为空，创建默认设置文件
                self.save_settings_to_file(self.default_settings)
            
            # 加载相机参数
            self._load_camera_settings(ui, settings)
            
            # 加载直线查找参数
            self._load_line_detection_settings(ui, settings)
            
            # 加载圆查找参数
            self._load_circle_detection_settings(ui, settings)
                
        except Exception as e:
            logging.error(f"加载设置时出错: {str(e)}")
            # 如果出错，使用默认值
            self.apply_default_settings(ui)

    def _load_camera_settings(self, ui, settings):
        """加载相机参数"""
        # 曝光时间
        ui.ledVerCamExposureTime.setText(str(settings.get("VerCamExposureTime", 
                                                      self.default_settings["VerCamExposureTime"])))
        ui.ledLeftCamExposureTime.setText(str(settings.get("LeftCamExposureTime", 
                                                       self.default_settings["LeftCamExposureTime"])))
        ui.ledFrontCamExposureTime.setText(str(settings.get("FrontCamExposureTime", 
                                                        self.default_settings["FrontCamExposureTime"])))
        # 序列号
        ui.ledVerCamSN.setText(str(settings.get("VerCamSN", 
                                            self.default_settings["VerCamSN"])))
        ui.ledLeftCamSN.setText(str(settings.get("LeftCamSN", 
                                             self.default_settings["LeftCamSN"])))
        ui.ledFrontCamSN.setText(str(settings.get("FrontCamSN", 
                                              self.default_settings["FrontCamSN"])))
        # 图像格式
        ui.cbVerCamImageFormat.setCurrentText(settings.get("VerCamImageFormat", 
                                                       self.default_settings["VerCamImageFormat"]))
        ui.cbLeftCamImageFormat.setCurrentText(settings.get("LeftCamImageFormat", 
                                                       self.default_settings["LeftCamImageFormat"]))
        ui.cbFrontCamImageFormat.setCurrentText(settings.get("FrontCamImageFormat", 
                                                        self.default_settings["FrontCamImageFormat"]))

    def _load_line_detection_settings(self, ui, settings):
        """加载直线查找参数"""
        # TODO: 添加直线查找参数的UI控件加载
        pass

    def _load_circle_detection_settings(self, ui, settings):
        """加载圆查找参数"""
        # TODO: 添加圆查找参数的UI控件加载
        pass

    def save_settings(self, ui):
        """保存UI中的设置"""
        try:
            settings = self._get_current_settings(ui)
            self.save_settings_to_file(settings)
        except Exception as e:
            logging.error(f"保存设置时出错: {str(e)}")

    def _get_current_settings(self, ui):
        """获取当前UI中的所有设置值"""
        settings = {
            # 相机参数
            "VerCamExposureTime": int(ui.ledVerCamExposureTime.text()),
            "LeftCamExposureTime": int(ui.ledLeftCamExposureTime.text()),
            "FrontCamExposureTime": int(ui.ledFrontCamExposureTime.text()),
            "VerCamSN": ui.ledVerCamSN.text(),
            "LeftCamSN": ui.ledLeftCamSN.text(),
            "FrontCamSN": ui.ledFrontCamSN.text(),
            "VerCamImageFormat": ui.cbVerCamImageFormat.currentText(),
            "LeftCamImageFormat": ui.cbLeftCamImageFormat.currentText(),
            "FrontCamImageFormat": ui.cbFrontCamImageFormat.currentText(),
            
            # TODO: 添加直线查找和圆查找参数的获取
            "LineDetThreshold": self.default_settings["LineDetThreshold"],
            "LineDetMinLength": self.default_settings["LineDetMinLength"],
            "LineDetMaxGap": self.default_settings["LineDetMaxGap"],
            
            "CircleDetMinDist": self.default_settings["CircleDetMinDist"],
            "CircleDetParam1": self.default_settings["CircleDetParam1"],
            "CircleDetParam2": self.default_settings["CircleDetParam2"],
            "CircleDetMinRadius": self.default_settings["CircleDetMinRadius"],
            "CircleDetMaxRadius": self.default_settings["CircleDetMaxRadius"]
        }
        return settings

    def save_settings_to_file(self, settings):
        """将设置保存到文件"""
        try:
            # 确保目录存在
            os.makedirs(os.path.dirname(self.settings_file), exist_ok=True)
            
            # 保存到文件
            with open(self.settings_file, 'w', encoding='utf-8') as f:
                json.dump(settings, f, ensure_ascii=False, indent=4)
                
        except Exception as e:
            logging.error(f"保存设置到文件时出错: {str(e)}")
            raise

    def apply_default_settings(self, ui):
        """应用默认设置到UI"""
        try:
            # 相机参数
            ui.ledVerCamExposureTime.setText(str(self.default_settings["VerCamExposureTime"]))
            ui.ledLeftCamExposureTime.setText(str(self.default_settings["LeftCamExposureTime"]))
            ui.ledFrontCamExposureTime.setText(str(self.default_settings["FrontCamExposureTime"]))
            ui.ledVerCamSN.setText(str(self.default_settings["VerCamSN"]))
            ui.ledLeftCamSN.setText(str(self.default_settings["LeftCamSN"]))
            ui.ledFrontCamSN.setText(str(self.default_settings["FrontCamSN"]))
            ui.cbVerCamImageFormat.setCurrentText(self.default_settings["VerCamImageFormat"])
            ui.cbLeftCamImageFormat.setCurrentText(self.default_settings["LeftCamImageFormat"])
            ui.cbFrontCamImageFormat.setCurrentText(self.default_settings["FrontCamImageFormat"])
            
            # TODO: 添加直线查找和圆查找参数的默认值设置
            
        except Exception as e:
            logging.error(f"应用默认设置时出错: {str(e)}")
            raise

    def init_combo_boxes(self, ui):
        """初始化ComboBox选项"""
        # 相机图像格式选项
        ui.cbVerCamImageFormat.addItems(["RGB", "Mono8"])
        ui.cbLeftCamImageFormat.addItems(["RGB", "Mono8"])
        ui.cbFrontCamImageFormat.addItems(["RGB", "Mono8"])
        
        # TODO: 添加其他ComboBox的初始化 