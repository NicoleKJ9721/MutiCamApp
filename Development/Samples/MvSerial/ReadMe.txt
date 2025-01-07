SerialDemo：
1）该示例demo适配利用串口方式配置CameraLink相机的属性参数
2）Demo使用基础介绍：已知CameraLink相机串口和波特率
   Demo界面选定对应COM->选定对应Baudrate->枚举相机->打开串口...
   ->利用配置属性KeyName操作Read/Write...
   ->利用直接输入串口命令的方式 Read/Write操作配置（相关命令格式：需要参考Camera Link工业XX相机用户手册文档说明）
   ->clear 窗口显示....
   ->关闭串口
   
   
使用注意事项：
1.依赖runtime包环境库；若需二次开发，该串口组件的头文件和lib库在SerialDemo示例程序中获取。
2.现波特率大小受限，只支持头文件中MV_SERIAL_BAUDRATE中的示例，Demo中的COM口可自行适配新增。
