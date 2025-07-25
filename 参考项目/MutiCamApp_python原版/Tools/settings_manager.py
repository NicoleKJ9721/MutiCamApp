import os
import json
import logging

class SettingsManager:
    def __init__(self, settings_file="./Settings/settings.json", log_manager=None):
        """初始化设置管理器
        Args:
            settings_file (str): 设置文件路径
            log_manager (LogManager, optional): 日志管理器实例
        """
        self.settings_file = settings_file
        self.log_manager = log_manager
        # 设置默认值
        self.default_settings = {
            # 相机参数
            "VerCamSN": "Vir21084717",
            "LeftCamSN": "Vir21128616",
            "FrontCamSN": "Vir158888",
            
            # 直线查找参数
            "CannyLineLow": 50,
            "CannyLineHigh": 150,
            "LineDetThreshold": 50,
            "LineDetMinLength": 100,
            "LineDetMaxGap": 10,
            
            # 圆查找参数
            "CannyCircleLow": 100,
            "CannyCircleHigh": 200,
            "CircleDetParam2": 50,

            # UI尺寸参数
            "UIWidth": 2230,
            "UIHeight": 1300
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
                print("设置文件不存在或为空，创建默认设置文件")
                settings = self.default_settings
                # 如果文件不存在或为空，创建默认设置文件
                self.save_settings_to_file(self.default_settings)
            
            # 加载相机参数
            self._load_camera_settings(ui, settings)
            
            # 加载直线查找参数
            self._load_line_detection_settings(ui, settings)
            
            # 加载圆查找参数
            self._load_circle_detection_settings(ui, settings)

            # 加载UI尺寸参数
            self._load_ui_size_settings(ui, settings)
                
        except Exception as e:
            logging.error(f"加载设置时出错: {str(e)}")
            # 如果出错，使用默认值
            self.apply_default_settings(ui)

    def _load_camera_settings(self, ui, settings):
        """加载相机参数"""
        # 序列号
        ui.ledVerCamSN.setText(str(settings.get("VerCamSN", 
                                            self.default_settings["VerCamSN"])))
        ui.ledLeftCamSN.setText(str(settings.get("LeftCamSN", 
                                             self.default_settings["LeftCamSN"])))
        ui.ledFrontCamSN.setText(str(settings.get("FrontCamSN", 
                                              self.default_settings["FrontCamSN"])))

    def _load_line_detection_settings(self, ui, settings):
        """加载直线查找参数"""
        # Canny边缘检测参数
        ui.ledCannyLineLow.setText(str(settings.get("CannyLineLow", 
                                                self.default_settings["CannyLineLow"])))
        ui.ledCannyLineHigh.setText(str(settings.get("CannyLineHigh", 
                                                 self.default_settings["CannyLineHigh"])))
        
        # HoughLinesP参数
        ui.ledLineDetThreshold.setText(str(settings.get("LineDetThreshold", 
                                                    self.default_settings["LineDetThreshold"])))
        ui.ledLineDetMinLength.setText(str(settings.get("LineDetMinLength", 
                                                    self.default_settings["LineDetMinLength"])))
        ui.ledLineDetMaxGap.setText(str(settings.get("LineDetMaxGap", 
                                                 self.default_settings["LineDetMaxGap"])))

    def _load_circle_detection_settings(self, ui, settings):
        """加载圆查找参数"""
        # Canny边缘检测参数
        ui.ledCannyCircleLow.setText(str(settings.get("CannyCircleLow", 
                                                self.default_settings["CannyCircleLow"])))
        ui.ledCannyCircleHigh.setText(str(settings.get("CannyCircleHigh", 
                                                 self.default_settings["CannyCircleHigh"])))
        
        # HoughCircles参数
        ui.ledCircleDetParam2.setText(str(settings.get("CircleDetParam2", 
                                                    self.default_settings["CircleDetParam2"])))

    def _load_ui_size_settings(self, ui, settings):
        """加载UI尺寸参数"""
        ui.ledUIWidth.setText(str(settings.get("UIWidth", 
                                             self.default_settings["UIWidth"])))
        ui.ledUIHeight.setText(str(settings.get("UIHeight", 
                                              self.default_settings["UIHeight"])))

    def save_settings(self, ui):
        """保存UI中的设置"""
        try:
            settings = self._get_current_settings(ui)
            self.save_settings_to_file(settings)
            if self.log_manager:
                self.log_manager.log_settings_operation("保存设置", "成功保存所有设置")
        except Exception as e:
            if self.log_manager:
                self.log_manager.log_error(f"保存设置失败: {str(e)}")

    def _get_current_settings(self, ui):
        """获取当前UI中的所有设置值并记录更改"""
        old_settings = {}
        try:
            # 如果存在旧设置文件，读取它
            if os.path.exists(self.settings_file):
                with open(self.settings_file, 'r', encoding='utf-8') as f:
                    old_settings = json.load(f)
        except Exception:
            old_settings = self.default_settings

        # 获取新设置
        new_settings = {
            # 相机参数
            "VerCamSN": ui.ledVerCamSN.text(),
            "LeftCamSN": ui.ledLeftCamSN.text(),
            "FrontCamSN": ui.ledFrontCamSN.text(),
            
            # 直线查找参数
            "CannyLineLow": int(ui.ledCannyLineLow.text()),
            "CannyLineHigh": int(ui.ledCannyLineHigh.text()),
            "LineDetThreshold": int(ui.ledLineDetThreshold.text()),
            "LineDetMinLength": int(ui.ledLineDetMinLength.text()),
            "LineDetMaxGap": int(ui.ledLineDetMaxGap.text()),
            
            # 圆查找参数
            "CannyCircleLow": int(ui.ledCannyCircleLow.text()),
            "CannyCircleHigh": int(ui.ledCannyCircleHigh.text()),
            "CircleDetParam2": int(ui.ledCircleDetParam2.text()),
            
            # UI尺寸参数
            "UIWidth": int(ui.ledUIWidth.text()),
            "UIHeight": int(ui.ledUIHeight.text())
        }

        # 比较并记录更改
        if self.log_manager:
            for key, new_value in new_settings.items():
                old_value = old_settings.get(key, self.default_settings.get(key))
                if new_value != old_value:
                    self.log_manager.log_settings_operation(
                        f"参数修改: {key}",
                        f"从 {old_value} 改为 {new_value}"
                    )

        return new_settings

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
            ui.ledVerCamSN.setText(str(self.default_settings["VerCamSN"]))
            ui.ledLeftCamSN.setText(str(self.default_settings["LeftCamSN"]))
            ui.ledFrontCamSN.setText(str(self.default_settings["FrontCamSN"]))
            
            # 直线查找参数
            ui.ledCannyLineLow.setText(str(self.default_settings["CannyLineLow"]))
            ui.ledCannyLineHigh.setText(str(self.default_settings["CannyLineHigh"]))
            ui.ledLineDetThreshold.setText(str(self.default_settings["LineDetThreshold"]))
            ui.ledLineDetMinLength.setText(str(self.default_settings["LineDetMinLength"]))
            ui.ledLineDetMaxGap.setText(str(self.default_settings["LineDetMaxGap"]))
            
            # 圆查找参数
            ui.ledCannyCircleLow.setText(str(self.default_settings["CannyCircleLow"]))
            ui.ledCannyCircleHigh.setText(str(self.default_settings["CannyCircleHigh"]))
            ui.ledCircleDetParam2.setText(str(self.default_settings["CircleDetParam2"]))
            
            # UI尺寸参数
            ui.ledUIWidth.setText(str(self.default_settings["UIWidth"]))
            ui.ledUIHeight.setText(str(self.default_settings["UIHeight"]))
            
        except Exception as e:
            logging.error(f"应用默认设置时出错: {str(e)}")
            raise

    def load_settings_from_file(self):
        """从文件加载设置"""
        try:
            settings_path = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'Settings', 'settings.json')
            if os.path.exists(settings_path):
                with open(settings_path, 'r') as f:
                    return json.load(f)
            else:
                # 如果文件不存在，返回默认值
                return {
                    'CannyLineLow': 50,
                    'CannyLineHigh': 150,
                    'LineDetThreshold': 50,
                    'LineDetMinLength': 50,
                    'LineDetMaxGap': 10,
                    'CannyCircleLow': 100,
                    'CannyCircleHigh': 200,
                    'CircleDetParam2': 30,
                    'UIWidth': 1000,
                    'UIHeight': 800
                }
        except Exception as e:
            print(f"加载设置文件失败: {str(e)}")
            # 返回默认值
            return {
                'CannyLineLow': 50,
                'CannyLineHigh': 150,
                'LineDetThreshold': 50,
                'LineDetMinLength': 50,
                'LineDetMaxGap': 10,
                'CannyCircleLow': 100,
                'CannyCircleHigh': 200,
                'CircleDetParam2': 30,
                'UIWidth': 1000,
                'UIHeight': 800
            } 