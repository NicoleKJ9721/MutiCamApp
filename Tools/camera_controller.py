import sys
import time
import cv2
import numpy as np
from ctypes import *
import os
sys.path.append("./Development/Samples/Python/MvImport")

from MvCameraControl_class import *
from MvErrorDefine_const import *
from CameraParams_header import *
from PixelType_header import *

# 定义回调函数类型
CALL_BACK_FUN = CFUNCTYPE(None, c_void_p, POINTER(MV_FRAME_OUT_INFO_EX), c_void_p)

class HikiCamera:
    def __init__(self, serial_number=None):
        """初始化相机"""
        # 强制使用64位DLL
        if sys.maxsize > 2**32:  # 如果是64位Python
            dll_path = r'C:\Program Files (x86)\Common Files\MVS\Runtime\Win64_x64'
            # 将64位DLL路径放在PATH的最前面
            if dll_path not in os.environ['PATH']:
                os.environ['PATH'] = dll_path + os.pathsep + os.environ['PATH']
            # 显式设置DLL搜索路径
            os.add_dll_directory(dll_path)
        
        self.cam = None
        self.device_list = None
        self.nPayloadSize = None
        self.running = False
        # 添加回调相关的属性
        self.callback_mode = False
        self.frame_buffer = None
        self.frame_ready = False
        # 保存回调函数引用，防止被垃圾回收
        self._image_callback_func = None
        # 固定使用RGB格式
        self.pixel_format = "RGB8"
        
        # 如果提供了序列号，尝试打开相机
        if serial_number is not None:
            if not self.open_camera_by_sn(serial_number):
                print(f"无法自动打开序列号为 {serial_number} 的相机")
        
    def enum_devices(self):
        """枚举所有设备"""
        print("\n=== 开始诊断 ===")
        print(f"当前工作目录: {os.getcwd()}")
        print(f"Python版本: {sys.version}")
        print(f"系统架构: {'64 bit' if sys.maxsize > 2**32 else '32 bit'}")
        
        # 检查DLL文件
        dll_paths = [
            r"C:\Program Files (x86)\Common Files\MVS\Runtime\Win64_x64\MvCameraControl.dll",
            r"C:\Program Files (x86)\Common Files\MVS\Runtime\Win32_i86\MvCameraControl.dll",
            os.path.join(os.getcwd(), "MvCameraControl.dll")
        ]
        
        print("\n检查DLL文件:")
        found_dlls = []
        for path in dll_paths:
            if os.path.exists(path):
                found_dlls.append(path)
                print(f"找到DLL: {path}")
                # 检查DLL位数
                try:
                    import pefile
                    pe = pefile.PE(path)
                    is_64 = pe.FILE_HEADER.Machine == pefile.MACHINE_TYPE['IMAGE_FILE_MACHINE_AMD64']
                    print(f"  DLL架构: {'64位' if is_64 else '32位'}")
                except ImportError:
                    print("  无法检查DLL架构（需要安装pefile包）")
            else:
                print(f"未找到DLL: {path}")
        
        print("\n检查环境变量PATH:")
        path_dirs = os.environ['PATH'].split(';')
        mvs_dirs = []
        for dir in path_dirs:
            if 'MVS' in dir:
                mvs_dirs.append(dir)
                print(f"发现MVS相关目录: {dir}")
                if os.path.exists(dir):
                    dlls = [f for f in os.listdir(dir) if f.endswith('.dll')]
                    print(f"  目录存在，包含 {len(dlls)} 个DLL文件:")
                    for dll in dlls:
                        print(f"    - {dll}")
                else:
                    print(f"  目录不存在")
        
        if not found_dlls:
            print("\n警告: 未找到任何MvCameraControl.dll!")
            print("建议:")
            print("1. 确保已安装海康相机SDK")
            print("2. 检查SDK安装路径是否正确")
            if sys.maxsize > 2**32:
                print("3. 当前Python为64位，请确保安装了64位SDK")
            else:
                print("3. 当前Python为32位，请确保安装了32位SDK")
        
        if not mvs_dirs:
            print("\n警告: 环境变量PATH中未找到MVS目录!")
            print("建议将SDK运行时目录添加到系统环境变量PATH中")
        
        print("\n开始枚举设备...")
        
        print("尝试枚举GigE设备...")
        deviceList_gige = MV_CC_DEVICE_INFO_LIST()
        ret_gige = MvCamera.MV_CC_EnumDevices(MV_GIGE_DEVICE, deviceList_gige)
        if ret_gige == 0:
            print(f"找到 {deviceList_gige.nDeviceNum} 个GigE设备")
        else:
            error_code = ret_gige & 0xFFFFFFFF
            print(f"枚举GigE设备失败: {ret_gige} (0x{error_code:08X})")
            deviceList_gige.nDeviceNum = 0  # 确保失败时设备数为0
        
        print("\n尝试枚举USB设备...")
        deviceList_usb = MV_CC_DEVICE_INFO_LIST()
        ret_usb = MvCamera.MV_CC_EnumDevices(MV_USB_DEVICE, deviceList_usb)
        if ret_usb == 0:
            print(f"找到 {deviceList_usb.nDeviceNum} 个USB设备")
        else:
            error_code = ret_usb & 0xFFFFFFFF
            print(f"枚举USB设备失败: {ret_usb} (0x{error_code:08X})")
            deviceList_usb.nDeviceNum = 0  # 确保失败时设备数为0
        
        print("\n=== 诊断结束 ===\n")
        
        # 创建合并的设备列表
        deviceList = MV_CC_DEVICE_INFO_LIST()
        total_devices = deviceList_gige.nDeviceNum + deviceList_usb.nDeviceNum
        
        if total_devices == 0:
            print("未找到任何设备!")
            return None
        
        deviceList.nDeviceNum = total_devices
        
        # 复制设备信息
        for i in range(deviceList_gige.nDeviceNum):
            deviceList.pDeviceInfo[i] = deviceList_gige.pDeviceInfo[i]
        
        for i in range(deviceList_usb.nDeviceNum):
            deviceList.pDeviceInfo[deviceList_gige.nDeviceNum + i] = deviceList_usb.pDeviceInfo[i]
        
        self.device_list = deviceList  # 保存设备列表
        
        # 打印找到的设备信息
        print(f"\n总共找到 {total_devices} 个设备:")
        for i in range(total_devices):
            mvcc_dev_info = cast(deviceList.pDeviceInfo[i], POINTER(MV_CC_DEVICE_INFO)).contents
            if mvcc_dev_info.nTLayerType == MV_GIGE_DEVICE:
                print(f"\nGigE设备序号: {i}")
                strSerialNumber = ""
                for per in mvcc_dev_info.SpecialInfo.stGigEInfo.chSerialNumber:
                    if per == 0:
                        break
                    strSerialNumber = strSerialNumber + chr(per)
                print(f"序列号: {strSerialNumber}")
            elif mvcc_dev_info.nTLayerType == MV_USB_DEVICE:
                print(f"\nUSB设备序号: {i}")
                strSerialNumber = ""
                for per in mvcc_dev_info.SpecialInfo.stUsb3VInfo.chSerialNumber:
                    if per == 0:
                        break
                    strSerialNumber = strSerialNumber + chr(per)
                print(f"序列号: {strSerialNumber}")
        
        return deviceList

    def open_camera(self, device_index=0):
        """打开相机"""
        if self.device_list is None:
            print("请先枚举设备!")
            return False
        
        # 确保先关闭之前的相机实例
        if self.cam is not None:
            self.close_camera()
        
        # 创建相机实例
        self.cam = MvCamera()
        
        # 检查设备信息是否有效
        if device_index >= self.device_list.nDeviceNum:
            print(f"设备索引 {device_index} 超出范围!")
            return False
        
        # 创建句柄
        ret = self.cam.MV_CC_CreateHandle(cast(self.device_list.pDeviceInfo[device_index], POINTER(MV_CC_DEVICE_INFO)).contents)
        if ret != 0:
            print(f"创建句柄失败! ret={ret}")
            return False

        # 打开设备
        ret = self.cam.MV_CC_OpenDevice(MV_ACCESS_Exclusive, 0)
        if ret != 0:
            print(f"打开设备失败! ret={ret}")
            self.cam.MV_CC_DestroyHandle()
            return False

        # 设置心跳超时时间（针对GigE相机）
        if self.device_list.pDeviceInfo[device_index].contents.nTLayerType == MV_GIGE_DEVICE:
            ret = self.cam.MV_CC_SetIntValue("GevHeartbeatTimeout", 5000)
            if ret != 0:
                print(f"设置心跳超时时间失败! ret={ret}")
        
        # 设置数据包大小（针对GigE相机）
        if self.device_list.pDeviceInfo[device_index].contents.nTLayerType == MV_GIGE_DEVICE:
            ret = self.cam.MV_CC_SetIntValue("GevSCPSPacketSize", 1500)
            if ret != 0:
                print(f"设置数据包大小失败! ret={ret}")

        # 设置触发模式为关
        ret = self.cam.MV_CC_SetEnumValue("TriggerMode", MV_TRIGGER_MODE_OFF)
        if ret != 0:
            print(f"设置触发模式失败! ret={ret}")
            self.close_camera()
            return False

        # 默认设置为RGB8格式
        if not self.set_pixel_format('RGB8'):
            self.close_camera()
            return False

        return True

    def set_exposure_time(self, exposure_time):
        """设置曝光时间"""
        if self.cam is None:
            return False
        ret = self.cam.MV_CC_SetFloatValue("ExposureTime", exposure_time)
        if ret != 0:
            print(f"设置曝光时间失败! ret={ret}")
            return False
        return True

    def _cb_func(self, pData, pFrameInfo, pUser):
        """内部回调函数"""
        try:
            if pFrameInfo:
                frame_info = pFrameInfo.contents
                # 复制图像数据
                data_buf = (c_ubyte * frame_info.nFrameLen)()
                memmove(data_buf, cast(pData, POINTER(c_ubyte)), frame_info.nFrameLen)
                
                # 转换为numpy数组
                frame = np.frombuffer(data_buf, dtype=np.uint8)
                if frame_info.enPixelType == PixelType_Gvsp_Mono8:
                    frame = frame.reshape((frame_info.nHeight, frame_info.nWidth))
                else:
                    frame = frame.reshape((frame_info.nHeight, frame_info.nWidth, 3))
                    # 根据实际的像素格式进行转换
                    if self.pixel_format == "RGB8":
                        frame = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)
                
                # 更新帧缓存
                self.frame_buffer = frame.copy()
                self.frame_ready = True
                
        except Exception as e:
            print(f"回调处理异常: {str(e)}")
            import traceback
            traceback.print_exc()

    def start_grabbing(self, use_callback=False):
        """开始取流
        Args:
            use_callback (bool): 是否使用回调方式取流
        """
        if self.cam is None:
            return False
            
        self.callback_mode = use_callback
        if use_callback:
            # 创建并保存回调函数
            self._image_callback_func = CALL_BACK_FUN(self._cb_func)
            # 注册回调函数
            ret = self.cam.MV_CC_RegisterImageCallBackEx(
                self._image_callback_func, 
                c_void_p(0)
            )
            if ret != 0:
                print(f"注册回调函数失败! ret={ret}")
                return False
        
        ret = self.cam.MV_CC_StartGrabbing()
        if ret != 0:
            print(f"开始取流失败! ret={ret}")
            return False
            
        self.running = True
        self.frame_ready = False
        return True

    def get_frame(self):
        """获取一帧图像"""
        if not self.running:
            return None

        if self.callback_mode:
            # 回调模式：等待新帧
            if self.frame_ready:
                self.frame_ready = False
                try:
                    return self.frame_buffer.copy() if self.frame_buffer is not None else None
                except Exception as e:
                    print(f"复制帧缓存失败: {str(e)}")
                    return None
        else:
            # 轮询模式：直接获取图像
            stOutFrame = MV_FRAME_OUT()
            ret = self.cam.MV_CC_GetImageBuffer(stOutFrame, 1000)
            if ret != 0:
                print(f"获取图像失败! ret={ret}")
                return None

            try:
                if stOutFrame.pBufAddr is None:
                    return None
                    
                data_buf = (c_ubyte * stOutFrame.stFrameInfo.nFrameLen)()
                cdll.msvcrt.memcpy(byref(data_buf), stOutFrame.pBufAddr, stOutFrame.stFrameInfo.nFrameLen)
                
                frame = np.frombuffer(data_buf, dtype=np.uint8)
                if stOutFrame.stFrameInfo.enPixelType == PixelType_Gvsp_Mono8:
                    frame = frame.reshape((stOutFrame.stFrameInfo.nHeight, stOutFrame.stFrameInfo.nWidth))
                else:
                    frame = frame.reshape((stOutFrame.stFrameInfo.nHeight, stOutFrame.stFrameInfo.nWidth, 3))
                    if self.pixel_format == "RGB8":
                        frame = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)
                
                return frame.copy()  # 返回副本避免内存问题
            except Exception as e:
                print(f"图像处理失败: {str(e)}")
                return None
            finally:
                try:
                    self.cam.MV_CC_FreeImageBuffer(stOutFrame)
                except Exception as e:
                    print(f"释放图像缓存失败: {str(e)}")

    def stop_grabbing(self):
        """停止取流"""
        if self.cam is None:
            return False
        ret = self.cam.MV_CC_StopGrabbing()
        if ret != 0:
            print(f"停止取流失败! ret={ret}")
            return False
        self.running = False
        # 清理回调相关资源
        if self.callback_mode:
            self._image_callback_func = None
            self.frame_buffer = None
            self.frame_ready = False
        return True

    def close_camera(self):
        """关闭相机"""
        if self.cam is None:
            return
        
        try:
            # 先停止取流
            if self.running:
                self.stop_grabbing()
            
            # 清理回调资源
            if self.callback_mode:
                try:
                    ret = self.cam.MV_CC_RegisterImageCallBackEx(None, c_void_p(0))
                    if ret != 0:
                        print(f"注销回调函数失败! ret={ret}")
                except Exception as e:
                    print(f"注销回调函数时发生异常: {str(e)}")
                self._image_callback_func = None
                self.frame_buffer = None
                self.frame_ready = False
            
            # 关闭设备
            ret = self.cam.MV_CC_CloseDevice()
            if ret != 0:
                print(f"关闭设备失败! ret={ret}")
            
        except Exception as e:
            print(f"关闭设备时发生异常: {str(e)}")
        finally:
            try:
                ret = self.cam.MV_CC_DestroyHandle()
                if ret != 0:
                    print(f"销毁句柄失败! ret={ret}")
            except Exception as e:
                print(f"销毁句柄时发生异常: {str(e)}")
            
            self.cam = None
            self.running = False
            self.callback_mode = False

    def get_device_info(self, device_index=0):
        """获取指定设备的详细信息"""
        if self.device_list is None or device_index >= self.device_list.nDeviceNum:
            return None
        
        device_info = {}
        mvcc_dev_info = cast(self.device_list.pDeviceInfo[device_index], POINTER(MV_CC_DEVICE_INFO)).contents
        
        # 获取设备类型
        device_info['type'] = 'GigE' if mvcc_dev_info.nTLayerType == MV_GIGE_DEVICE else 'USB'
        
        if mvcc_dev_info.nTLayerType == MV_GIGE_DEVICE:
            # GigE相机信息
            strModeName = ""
            for per in mvcc_dev_info.SpecialInfo.stGigEInfo.chModelName:
                if per == 0:
                    break
                strModeName = strModeName + chr(per)
            device_info['model_name'] = strModeName
            
            # IP地址
            nip1 = ((mvcc_dev_info.SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24)
            nip2 = ((mvcc_dev_info.SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16)
            nip3 = ((mvcc_dev_info.SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8)
            nip4 = (mvcc_dev_info.SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff)
            device_info['ip'] = f"{nip1}.{nip2}.{nip3}.{nip4}"
            
            # 序列号
            strSerialNumber = ""
            for per in mvcc_dev_info.SpecialInfo.stGigEInfo.chSerialNumber:
                if per == 0:
                    break
                strSerialNumber = strSerialNumber + chr(per)
            device_info['serial_number'] = strSerialNumber
            
        else:
            # USB相机信息
            strModeName = ""
            for per in mvcc_dev_info.SpecialInfo.stUsb3VInfo.chModelName:
                if per == 0:
                    break
                strModeName = strModeName + chr(per)
            device_info['model_name'] = strModeName
            
            # 序列号
            strSerialNumber = ""
            for per in mvcc_dev_info.SpecialInfo.stUsb3VInfo.chSerialNumber:
                if per == 0:
                    break
                strSerialNumber = strSerialNumber + chr(per)
            device_info['serial_number'] = strSerialNumber
        
        return device_info

    def set_pixel_format(self, format_str):
        """设置像素格式
        Args:
            format_str (str): 'RGB8' 或 'Mono8'
        Returns:
            bool: 设置是否成功
        """
        if self.cam is None:
            print("相机未打开!")
            return False
        
        if format_str not in ['RGB8', 'Mono8']:
            print("不支持的格式! 只支持 'RGB8' 或 'Mono8'")
            return False
        
        # 设置像素格式
        pixel_format = PixelType_Gvsp_RGB8_Packed if format_str == 'RGB8' else PixelType_Gvsp_Mono8
        ret = self.cam.MV_CC_SetEnumValue("PixelFormat", pixel_format)
        if ret != 0:
            print(f"设置{format_str}格式失败! ret={ret}")
            return False
        
        print(f"成功设置为{format_str}格式")
        self.pixel_format = format_str
        
        # 更新数据包大小
        stParam = MVCC_INTVALUE()
        ret = self.cam.MV_CC_GetIntValue("PayloadSize", stParam)
        if ret != 0:
            print(f"获取数据包大小失败! ret={ret}")
            return False
        self.nPayloadSize = stParam.nCurValue
        
        return True

    def find_device_by_sn(self, serial_number):
        """通过序列号查找设备索引
        Args:
            serial_number (str): 设备序列号
        Returns:
            int: 设备索引，如果未找到返回-1
        """
        if self.device_list is None:
            if not self.enum_devices():
                return -1
            
        for i in range(self.device_list.nDeviceNum):
            mvcc_dev_info = cast(self.device_list.pDeviceInfo[i], POINTER(MV_CC_DEVICE_INFO)).contents
            if mvcc_dev_info.nTLayerType == MV_GIGE_DEVICE:
                # GigE相机
                sn = ""
                for per in mvcc_dev_info.SpecialInfo.stGigEInfo.chSerialNumber:
                    if per == 0:
                        break
                    sn = sn + chr(per)
                if sn == serial_number:
                    return i
            elif mvcc_dev_info.nTLayerType == MV_USB_DEVICE:
                # USB相机
                sn = ""
                for per in mvcc_dev_info.SpecialInfo.stUsb3VInfo.chSerialNumber:
                    if per == 0:
                        break
                    sn = sn + chr(per)
                if sn == serial_number:
                    return i
        return -1

    def open_camera_by_sn(self, serial_number):
        """通过序列号打开相机
        Args:
            serial_number (str): 设备序列号
        Returns:
            bool: 是否成功打开相机
        """
        device_index = self.find_device_by_sn(serial_number)
        if device_index == -1:
            print(f"未找到序列号为 {serial_number} 的相机!")
            return False
        
        return self.open_camera(device_index)

    def check_device_status(self):
        """检查设备状态"""
        if self.cam is None:
            return False
        
        try:
            stParam = MVCC_INTVALUE()
            ret = self.cam.MV_CC_GetIntValue("GevHeartbeatTimeout", stParam)
            return ret == 0
        except:
            return False

    def is_device_connected(self):
        """检查设备是否连接正常"""
        if not self.check_device_status():
            self.close_camera()
            return False
        return True

    def reconnect(self):
        """重新连接相机"""
        try:
            self.close_camera()
            time.sleep(1)  # 等待设备完全释放
            if hasattr(self, '_last_serial_number'):
                return self.open_camera_by_sn(self._last_serial_number)
            return False
        except Exception as e:
            print(f"重连失败: {str(e)}")
            return False


if __name__ == "__main__":
    import traceback
    
    try:
        # 创建相机实例时直接指定序列号
        camera = HikiCamera('Vir9973003')
        if camera.cam is None:
            print("相机打开失败!")
            sys.exit()
            
        camera.set_exposure_time(2000)
        
        # 使用回调方式开始取流
        if not camera.start_grabbing(use_callback=True):
            print("开始取流失败!")
            camera.close_camera()
            sys.exit()
        
        print("开始采集RGB8图像...")
        start_time = time.time()
        while (time.time() - start_time) < 1:  # 只显示1秒
            frame = camera.get_frame()
            if frame is not None:
                cv2.imshow("frame", frame)
                cv2.waitKey(1)
        
        print("停止采集...")
        camera.stop_grabbing()  # 停止采集
        print("关闭相机...")
        camera.close_camera()   # 关闭相机
                
    except Exception as e:
        print(f"发生错误: {str(e)}")
        traceback.print_exc()
    finally:
        print("清理资源...")
        if 'camera' in locals():
            if camera.running:
                camera.stop_grabbing()
            camera.close_camera()
        cv2.destroyAllWindows()
