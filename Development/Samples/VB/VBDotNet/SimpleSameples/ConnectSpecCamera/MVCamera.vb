Imports System.Runtime.InteropServices
Imports System.Threading.Thread
Imports System.IO

'设备参数类
Public Class CCamera
    #Region "相机参数常用常量定义"
    'chinese 设备传输层协议类型 | english Device Transport Layer Protocol Type
    Public Const MV_UNKNOW_DEVICE As Integer            = &H00000000 'chinese 未知设备类型，保留意义 | english Unknown Device Type, Reserved 
    Public Const MV_GIGE_DEVICE As Integer              = &H00000001 'chinese GigE设备 | english GigE Device
    Public Const MV_1394_DEVICE As Integer              = &H00000002 'chinese 1394-a/b 设备 | english 1394-a/b Device
    Public Const MV_USB_DEVICE As Integer               = &H00000004 'chinese USB 设备 | english USB Device
    Public Const MV_CAMERALINK_DEVICE As Integer        = &H00000008 'chinese CameraLink设备 | english CameraLink Device
    Public Const MV_VIR_GIGE_DEVIC As Integer           = &H00000010 'chinese 虚拟GigE设备 | english Virtual GigE Device
    Public Const MV_VIR_USB_DEVICE As Integer           = &H00000020 'chinese 虚拟USB设备 | english Virtual USB Device
    Public Const MV_GENTL_GIGE_DEVICE As Integer        = &H00000040 'chinese 自研网卡下GigE设备 | english GenTL GigE Device

    Public Const INFO_MAX_BUFFER_SIZE As Integer        = 64 'chinese 最大的数据信息大小 | english Maximum data information size
    Public Const MV_MAX_TLS_NUM As Integer              = 8 'chinese 最多支持的传输层实例个数 | english The maximum number of supported transport layer instances
    Public Const MV_MAX_DEVICE_NUM As Integer           = 256 'chinese 最大支持的设备个数 | english The maximum number of supported devices
    Public Const MV_MAX_GENTL_IF_NUM As Integer         = 256 'chinese 最大支持的GenTL接口数量 | english The maximum number of GenTL interface supported
    Public Const MV_MAX_GENTL_DEV_NUM As Integer        = 256 'chinese 最大支持的GenTL设备数量 | english The maximum number of GenTL devices supported

    'chinese 设备的访问模式 | english Device Access Mode
    Public Const MV_ACCESS_Exclusive As Integer                     = 1 'chinese 独占权限，其他APP只允许读CCP寄存器 | english Exclusive authority, other APP is only allowed to read the CCP register
    Public Const MV_ACCESS_ExclusiveWithSwitch As Integer           = 2 'chinese 可以从5模式下抢占权限，然后以独占权限打开 | english You can seize the authority from the 5 mode, and then open with exclusive authority
    Public Const MV_ACCESS_Control As Integer                       = 3 'chinese 控制权限，其他APP允许读所有寄存器 | english Control authority, allows other APP reading all registers
    Public Const MV_ACCESS_ControlWithSwitch As Integer             = 4 'chinese 可以从5的模式下抢占权限，然后以控制权限打开 | english You can seize the authority from the 5 mode, and then open with control authority
    Public Const MV_ACCESS_ControlSwitchEnable As Integer           = 5 'chinese 以可被抢占的控制权限打开 | english Open with seized control authority
    Public Const MV_ACCESS_ControlSwitchEnableWithKey As Integer    = 6 'chinese 可以从5的模式下抢占权限，然后以可被抢占的控制权限打开 | english You can seize the authority from the 5 mode, and then open with seized control authority
    Public Const MV_ACCESS_Monitor As Integer                       = 7 'chinese 读模式打开设备，适用于控制权限下 | english Open with read mode and is available under control authority

    'chinese 信息类型 | english Information Type
    Public Const MV_MATCH_TYPE_NET_DETECT As Integer                = &H00000001 'chinese 网络流量和丢包信息 | english Network traffic and packet loss information
    Public Const MV_MATCH_TYPE_USB_DETECT As Integer                = &H00000002 'chinese host接收到来自U3V设备的字节总数 | english The total number of bytes host received from U3V device
    
    'chinese GigEVision IP配置 | english GigEVision IP Configuration
    Public Const MV_IP_CFG_STATIC As Integer                        = &H05000000 'chinese 静态 | english Static
    Public Const MV_IP_CFG_DHCP As Integer                          = &H06000000 'chinese DHCP | english DHCP
    Public Const MV_IP_CFG_LLA As Integer                           = &H04000000 'chinese LLA |english LLA

    'chinese GigEVision网络传输模式 | english GigEVision Net Transfer Mode
    Public Const MV_NET_TRANS_DRIVER As Integer                     = &H00000001 'chinese 驱动 | english Driver
    Public Const MV_NET_TRANS_SOCKET As Integer                     = &H00000002 'chinese Socket | english Socket

    'chinese CameraLink波特率 | english CameraLink Baud Rates (CLUINT32)
    Public Const MV_CAML_BAUDRATE_9600 As Integer                   = &H00000001 'chinese 9600 | english 9600
    Public Const MV_CAML_BAUDRATE_19200 As Integer                  = &H00000002 'chinese 19200 | english 19200
    Public Const MV_CAML_BAUDRATE_38400 As Integer                  = &H00000004 'chinese 38400 | english 38400
    Public Const MV_CAML_BAUDRATE_57600 As Integer                  = &H00000008 'chinese 57600 | english 57600
    Public Const MV_CAML_BAUDRATE_115200 As Integer                 = &H00000010 'chinese 115200 | english 115200
    Public Const MV_CAML_BAUDRATE_230400 As Integer                 = &H00000020 'chinese 230400 | english 230400
    Public Const MV_CAML_BAUDRATE_460800 As Integer                 = &H00000040 'chinese 460800 | english 460800
    Public Const MV_CAML_BAUDRATE_921600 As Integer                 = &H00000080 'chinese 921600 | english 921600
    Public Const MV_CAML_BAUDRATE_AUTOMAX As Integer                = &H40000000 'chinese 最大值(默认设置9600) | english Auto Max(Default Set 9600)

    'chinese 异常消息类型 | english Exception message type
    Public Const MV_EXCEPTION_DEV_DISCONNECT As Integer             = &H00008001 'chinese 设备断开连接 | english The device is disconnected
    Public Const MV_EXCEPTION_VERSION_CHECK As Integer              = &H00008002 'chinese SDK与驱动版本不匹配 | english SDK does not match the driver version

    Public Const MV_MAX_XML_SYMBOLIC_NUM As Integer                 = 64 'chinese 最大XML符号数 | english Max XML Symbolic Number 
    Public Const MV_MAX_SYMBOLIC_LEN As Integer                     = 64 'chinese 最大枚举条目对应的符号长度 | english Max Enum Entry Symbolic Number 

    'chinese 分时曝光时最多将源图像拆分的个数 | english The maximum number of source image to be split in time-division exposure
    Public Const MV_MAX_SPLIT_NUM As Integer                        = 8

    Public Const MAX_EVENT_NAME_SIZE As Integer                     = 128 'chinese 设备Event事件名称最大长度 | english Max length of event name
    #End Region

    #Region "相机信息相关结构体的定义"
    '设备列表排序方式 | english Sort Method
    Public Enum MV_SORT_METHOD As Integer
        SortMethod_SerialNumber               = 0 'chinese 按序列号排序 | english Sorting by SerialNumber
        SortMethod_UserID                     = 1 'chinese 按用户自定义名字排序 | english Sorting by UserID
        SortMethod_CurrentIP_ASC              = 2 'chinese 按当前IP地址排序（升序） | english Sorting by current IP（Ascending）
        SortMethod_CurrentIP_DESC             = 3 'chinese 按当前IP地址排序（降序） | english Sorting by current IP（Descending）
    End Enum

    'chinese GigE设备信息 | english GigE device info
    Public Structure MV_GIGE_DEVICE_INFO
        Public nIpCfgOption As UInteger                                                                 'chinese IP配置选项 | english IP Configuration Options
        Public nIpCfgCurrent As UInteger                                                                'chinese 当前IP配置 | english IP Configuration
        Public nCurrentIp As UInteger                                                                   'chinese 当前IP地址 | english Current Ip
        Public nCurrentSubNetMask As UInteger                                                           'chinese 当前子网掩码 | english Curtent Subnet Mask
        Public nDefultGateWay As UInteger                                                               'chinese 当前网关 | english Current Gateway

        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=32)> _
        Public chManufacturerName As String                                                            'chinese 制造商名称 | english Manufacturer Name
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=32)> _
        Public chModelName As String                                                                   'chinese 型号名称 | english Model Name
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=32)> _
        Public chDeviceVersion As String                                                               'chinese 设备版本 | english Device Version
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=48)> _
        Public chManufacturerSpecificInfo As String                                                   'chinese 制造商的具体信息 | english Manufacturer Specific Information
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=16)> _
        Public chSerialNumber As String                                                               'chinese 序列号 | english Serial Number
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=16)> _
        Public chUserDefinedName As String                                                            'chinese 用户自定义名称 | english User Defined Name
        Public nNetExport As Integer                                                                  'chinese 网口IP地址 | english NetWork IP Address
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=4)> _
        Public nReserved() As UInteger                                                                'chinese 预留 | english Reserved
    End Structure
    
    ' chinese USB设备信息 | english USB device info
    Public Structure MV_USB3_DEVICE_INFO
        Public CrtlInEndPoint As Byte                                                                             'chinese 控制输入端点 | english Control input endpoint
        Public CrtlOutEndPoint As Byte                                                                            'chinese 控制输出端点 | english Control output endpoint
        Public StreamEndPoint As Byte                                                                             'chinese 流端点 | english Flow endpoint
        Public EventEndPoint As Byte                                                                              'chinese 事件端点 | english Event endpoint
        Public idVendor As UShort                                                                                  'chinese 供应商ID号 | english Vendor ID Number
        Public idProduct As UShort                                                                                 'chinese 产品ID号 | english Device ID Number
        Public nDeviceNumber As UInteger                                                                           'chinese 设备索引号 | english Device Number

        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=INFO_MAX_BUFFER_SIZE)> _
        Public chDeviceGUID As String                                                                             'chinese 设备GUID号 | english Device GUID Number
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=INFO_MAX_BUFFER_SIZE)> _
        Public chVendorName As String                                                                             'chinese 供应商名字 | english Vendor Name
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=INFO_MAX_BUFFER_SIZE)> _
        Public chModelName As String                                                                              'chinese 型号名字 | english Model Name
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=INFO_MAX_BUFFER_SIZE)> _
        Public chFamilyName As String                                                                             'chinese 家族名字 | english Family Name
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=INFO_MAX_BUFFER_SIZE)> _
        Public chDeviceVersion As String                                                                          'chinese 设备版本 | english Device Version
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=INFO_MAX_BUFFER_SIZE)> _
        Public chManufacturerName As String                                                                       'chinese 制造商名字 | english Manufacturer Name
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=INFO_MAX_BUFFER_SIZE)> _
        Public chSerialNumber As String                                                                           'chinese 序列号 | english Serial Number
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=INFO_MAX_BUFFER_SIZE)> _
        Public chUserDefinedName As String                                                                       'chinese 用户自定义名字 | english User Defined Name
        Public nbcdUSB As Integer                                                                                 'chinese 支持的USB协议 | english Support USB Protocol
        Public nDeviceAddress As Integer                                                                          'chinese 设备地址 | english Device Address
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=2)> _
        Public nReserved() As UInteger                                                                            'chinese 预留 | english Reserved
    End Structure

    'chinese CameraLink设备信息 | english CameraLink device info
    Public Structure MV_CamL_DEV_INFO
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=INFO_MAX_BUFFER_SIZE)> _
        Public chPortID As String                                                                                 'chinese 端口号 | english Port ID
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=INFO_MAX_BUFFER_SIZE)> _
        Public chModelName As String                                                                              'chinese 型号名字 | english Model Name
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=INFO_MAX_BUFFER_SIZE)> _
        Public chFamilyName As String                                                                             'chinese 家族名称 | english Family Name
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=INFO_MAX_BUFFER_SIZE)> _
        Public chDeviceVersion As String                                                                          'chinese 设备版本 | english Device Version
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=INFO_MAX_BUFFER_SIZE)> _
        Public chManufacturerName As String                                                                       'chinese 制造商名字 | english Manufacturer Name
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=INFO_MAX_BUFFER_SIZE)> _
        Public chSerialNumber As String                                                                           'chinese 序列号 | english Serial Number
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=38)> _
        Public nReserved() As UInteger                                                                           'chinese 预留 | english Reserved
    End Structure

    'chinese 设备信息 | english Device info
    <StructLayout(LayoutKind.Sequential)> _
     Public Structure MV_CC_DEVICE_INFO
        Public nMajorVer As UShort                                                                                'chinese 主要版本 | english Major Version
        Public nMinorVer As UShort                                                                                'chinese 次要版本 | english Minor Version
        Public nMacAddrHigh As UInteger                                                                           'chinese 高MAC地址 | english High MAC Address
        Public nMacAddrLow As UInteger                                                                            'chinese 低MAC地址 | english Low MAC Address
        Public nTLayerType As UInteger                                                                            'chinese 设备传输层协议类型 | english Device Transport Layer Protocol Type

        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=4)> _
        Public nReserved() As UInteger                                                                            'chinese 预留 | english Reserved

        <StructLayout(LayoutKind.Explicit)> _
        Structure SPECIAL_INFO
            <FieldOffset(0)> <MarshalAs(UnmanagedType.ByValArray, SizeConst:=216)> _
            Public stGigEInfo() As Byte                                                                           'chinese GigE设备信息 | english GigE Device Info
            <FieldOffset(0)> <MarshalAs(UnmanagedType.ByValArray, SizeConst:=536)> _
            Public stCamLInfo() As Byte                                                                           'chinese CameraLink设备信息 | english CameraLink Device Info
            <FieldOffset(0)> <MarshalAs(UnmanagedType.ByValArray, SizeConst:=540)> _
            Public stUsb3VInfo() As Byte                                                                          'chinese USB设备信息 | english USB Device Info
        End Structure

        Public stSpecialInfo As SPECIAL_INFO                                                                      '设备其他信息
    End Structure

    'chinese 设备信息列表 | english Device Information List
    <StructLayout(LayoutKind.Sequential)> _
    Public Structure MV_CC_DEVICE_INFO_LIST
        Public nDeviceNum As UInteger                                                                  'chinese 在线设备数量 | english Online Device Number
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=MV_MAX_DEVICE_NUM)> _
        Public pDeviceInfo() As IntPtr                                                                 'chinese 支持最多256个设备 | english Support up to 256 devices
    End Structure

    'chinese 通过GenTL枚举到的接口信息 | english Interface Information with GenTL
    Public Structure MV_GENTL_IF_INFO
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=INFO_MAX_BUFFER_SIZE)> _
        Public chInterfaceID As String                                                                      'chinese GenTL接口ID | english Interface ID
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=INFO_MAX_BUFFER_SIZE)> _
        Public chTLType As String                                                                            'chinese 传输层类型 | english GenTL Type
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=INFO_MAX_BUFFER_SIZE)> _
        Public chDisplayName As String                                                                       'chinese Interface显示名称 | english Display Name
        Public nCtiIndex As UInteger                                                                         'chinese GenTL的cti文件索引 | english The Index of Cti Files
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=7)> _
        Public nReserved() As UInteger                                                                       'chinese 预留 | english Reserved

    End Structure

    'chinese 通过GenTL枚举到的接口信息列表 | english Inferface Information List with GenTL
    Public Structure MV_GENTL_IF_INFO_LIST
        Public nInterfaceNum As UInteger                                                                  'chinese 在线接口数量 | english Online Inferface Number
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=MV_MAX_GENTL_IF_NUM)> _
        Public pIFInfo() As IntPtr                                                                        'chinese 支持最多256个接口 | english Support up to 256 inferfaces
    End Structure

    'chinese 通过GenTL枚举到的设备信息 | english Device Information with GenTL
    Public Structure MV_GENTL_DEV_INFO
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=INFO_MAX_BUFFER_SIZE)> _
        Public chInterfaceID As String                                                                   'chinese GenTL接口ID | english Interface ID
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=INFO_MAX_BUFFER_SIZE)> _
        Public chDeviceID As String                                                                      'chinese 设备ID | english Device ID
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=INFO_MAX_BUFFER_SIZE)> _
        Public chVendorName As String                                                                    'chinese 供应商名字 | english Vendor NameName
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=INFO_MAX_BUFFER_SIZE)> _
        Public chModelName As String                                                                     'chinese 型号名字 | english Model Name
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=INFO_MAX_BUFFER_SIZE)> _
        Public chTLType As String                                                                        'chinese 传输层类型 | english GenTL Type
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=INFO_MAX_BUFFER_SIZE)> _
        Public chDisplayName As String                                                                   'chinese 用户自定义名字 | english User Defined Name
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=INFO_MAX_BUFFER_SIZE)> _
        Public chSerialNumber As String                                                                  'chinese 序列号 | english Serial Number
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=INFO_MAX_BUFFER_SIZE)> _
        Public chDeviceVersion As String                                                                 'chinese 设备版本号 | english Device Version
        Public nCtiIndex As UInteger                                                                     'chinese GenTL的cti文件索引 | english The Index of Cti Files
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=8)> _
        Public nReserved() As UInteger                                                                   'chinese 预留 | english Reserved
    End Structure

    'chinese 通过GenTL枚举到的设备信息列表 | english Device Information List with GenTL
    Public Structure MV_GENTL_DEV_INFO_LIST
        Public nDeviceNum As UInteger                                                                           'chinese 在线设备数量 | english Online Device Number
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=MV_MAX_GENTL_DEV_NUM)> _
        Public pDeviceInfo() As UInteger                                                                        'chinese 支持最多256个设备 | english Support up to 256 devicess

    End Structure
#End Region

    #Region "网络包数据相关信息的定义"
    'chinese 网络传输的相关信息 | english Network transmission information
    Public Structure MV_NETTRANS_INFO
        Public nReceiveDataSize As Long                                        'chinese 已接收数据大小[Start和Stop之间] | english Received Data Size
        Public nThrowFrameCount As Integer                                     'chinese 丢帧数量 | english Throw frame number
        Public nNetRecvFrameCount As UInteger                                  'chinese 已接收的帧数 | english Received Frame Count
        Public nRequestResendPacketCount As Long                               'chinese 请求重发包数 | english Request Resend Packet Count
        Public nResendPacketCount As Long                                      'chinese 重发包数 | english Resend Packet Count
    End Structure

    'chinese 全匹配的一种信息结构体 | english A fully matched information structure
    Public Structure MV_ALL_MATCH_INFO
        Public nType As UInteger                                               'chinese 需要输出的信息类型，e.g. MV_MATCH_TYPE_NET_DETECT | english Information type need to output ,e.g. MV_MATCH_TYPE_NET_DETECT
        Public pInfo As IntPtr                                                 'chinese 输出的信息缓存，由调用者分配 | english Output information cache, which is allocated by the caller
        Public nInfoSize As UInteger                                           'chinese 信息缓存的大小 | english Information cache size
    End Structure

    'chinese 网络流量和丢包信息反馈结构体，对应类型为 MV_MATCH_TYPE_NET_DETECT | english Network traffic and packet loss feedback structure, the corresponding type is MV_MATCH_TYPE_NET_DETECT
    Public Structure MV_MATCH_INFO_NET_DETECT
        Public nReceiveDataSize As Long                                        'chinese 已接收数据大小[Start和Stop之间] | english Received Data Size
        Public nLostPacketCount As Long                                        'chinese 丢失的包数量 | english Number of packets lost
        Public nLostFrameCount As UInteger                                     'chinese 丢帧数量 | english Number of frames lost
        Public nNetRecvFrameCount As UInteger                                  'chinese 保留  | english Received Frame Count
        Public nRequestResendPacketCount As Long                               'chinese 请求重发包数 | english Request Resend Packet Count
        Public nResendPacketCount As Long                                      'chinese 重发包数 | english Resend Packet Count
    End Structure

    'chinese host收到从u3v设备端的总字节数，对应类型为 MV_MATCH_TYPE_USB_DETECT | english The total number of bytes host received from the u3v device side, the corresponding type is MV_MATCH_TYPE_USB_DETECT
    Public Structure MV_MATCH_INFO_USB_DETECT
        Public nReceiveDataSize As Long                                        'chinese 已接收数据大小 [Open和Close之间] | english Received data size
        Public nReceivedFrameCount As UInteger                                 'chinese 已收到的帧数 | english Number of frames received
        Public nErrorFrameCount As UInteger                                    'chinese 错误帧数 | english Number of error frames
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=2)> _
        Public nReserved() As UInteger                                         'chinese 预留 | english Reserved
    End Structure

    'chinese U3V流异常类型
    Public Enum MV_CC_STREAM_EXCEPTION_TYPE
        MV_CC_STREAM_EXCEPTION_ABNORMAL_IMAGE = &H4001    'chinese 异常的图像，该帧被丢弃
        MV_CC_STREAM_EXCEPTION_LIST_OVERFLOW = &H4002    'chinese 缓存列表溢出，清除最旧的一帧
        MV_CC_STREAM_EXCEPTION_LIST_EMPTY = &H4003    'chinese 缓存列表为空，该帧被丢弃
        MV_CC_STREAM_EXCEPTION_RECONNECTION = &H4004    'chinese 断流恢复
        MV_CC_STREAM_EXCEPTION_DISCONNECTED = &H4005    'chinese 断流,恢复失败,取流被中止
        MV_CC_STREAM_EXCEPTION_DEVICE = &H4006    'chinese 设备异常,取流被中止
    End Enum

    'chinese Gige的传输类型 | english The transmission type of Gige
    Public Enum MV_GIGE_TRANSMISSION_TYPE
        MV_GIGE_TRANSTYPE_UNICAST = &H0          'chinese 表示单播(默认) | english Unicast mode
        MV_GIGE_TRANSTYPE_MULTICAST = &H1          'chinese 表示组播 | english Multicast mode
        MV_GIGE_TRANSTYPE_LIMITEDBROADCAST = &H2          'chinese 表示局域网内广播，暂不支持 | english Limited broadcast mode,not support
        MV_GIGE_TRANSTYPE_SUBNETBROADCAST = &H3          'chinese 表示子网内广播，暂不支持 | english Subnet broadcast mode,not support
        MV_GIGE_TRANSTYPE_CAMERADEFINED = &H4          'chinese 表示从设备获取，暂不支持 | english Transtype from camera,not support
        MV_GIGE_TRANSTYPE_UNICAST_DEFINED_PORT = &H5          'chinese 表示用户自定义应用端接收图像数据Port号 | english User Defined Receive Data Port
        MV_GIGE_TRANSTYPE_UNICAST_WITHOUT_RECV = &H10000   'chinese 表示设置了单播，但本实例不接收图像数据 | english Unicast without receive data
        MV_GIGE_TRANSTYPE_MULTICAST_WITHOUT_RECV = &H10001   'chinese 表示组播模式，但本实例不接收图像数据 | english Multicast without receive data
    End Enum

    'chinese 网络传输模式 | english Transmission type
    Public Structure MV_TRANSMISSION_TYPE
        Public enTransmissionType As MV_GIGE_TRANSMISSION_TYPE                                    'chinese 传输模式 | english Transmission type
        Public nDestIp As UInteger                                                                'chinese 目标IP，组播模式下有意义 | english Destination IP
        Public nDestPort As Short                                                                 'chinese 目标Port，组播模式下有意义 | english Destination port
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=32)> _
        Public nReserved() As UInteger                                                            'chinese 预留 | english Reserved
    End Structure
    #End Region

    #Region "图像数据获取相关信息的定义"
    'chinese 取流策略 | english The strategy of Grabbing
    Public Enum MV_GRAB_STRATEGY
        MV_GrabStrategy_OneByOne           = 0 'chinese 从旧到新一帧一帧的获取图像 | english Grab One By One
        MV_GrabStrategy_LatestImagesOnly   = 1 'chinese 获取列表中最新的一帧图像 | english Grab The Latest Image
        MV_GrabStrategy_LatestImages       = 2 'chinese 获取列表中最新的图像 | english Grab The Latest Images
        MV_GrabStrategy_UpcomingImage      = 3 'chinese 等待下一帧图像 | english Grab The Upcoming Image
    End Enum

    'chinese 采集模式 | english Acquisition mode
    Public Enum MV_CAM_ACQUISITION_MODE
        MV_ACQ_MODE_SINGLE     = 0 'chinese 单帧模式 | english Single Mode
        MV_ACQ_MODE_MUTLI      = 1 'chinese 多帧模式 | english Multi Mode
        MV_ACQ_MODE_CONTINUOUS = 2 'chinese 持续采集模式 | english Continuous Mode
    End Enum

    'chinese 增益模式 | english Gain Mode
    Public Enum MV_CAM_GAIN_MODE
        MV_GAIN_MODE_OFF        = 0 'chinese 关闭 | english Single Mode
        MV_GAIN_MODE_ONCE       = 1 'chinese 一次 | english Multi Mode
        MV_GAIN_MODE_CONTINUOUS = 2 'chinese 连续 | english Continuous Mode
    End Enum
    
    'chinese 曝光模式 | english Exposure Mode
    Public Enum MV_CAM_EXPOSURE_MODE '曝光模式结构体
        MV_EXPOSURE_MODE_TIMED         = 0 'chinese 时间 | english Timed
        MV_EXPOSURE_MODE_TRIGGER_WIDTH = 1 'chinese 触发脉冲宽度 | english TriggerWidth
    End Enum
    
    'chinese 自动曝光模式 | english Auto Exposure Mode
    Public Enum MV_CAM_EXPOSURE_AUTO_MODE '自动曝光模式结构体
        MV_EXPOSURE_AUTO_MODE_OFF        = 0 'chinese 关闭 | english Off
        MV_EXPOSURE_AUTO_MODE_ONCE       = 1 'chinese 一次 | english Once
        MV_EXPOSURE_AUTO_MODE_CONTINUOUS = 2 'chinese 连续 | english Continuous
    End Enum
    
    'chinese 触发模式 | english Trigger Mode
    Public Enum MV_CAM_TRIGGER_MODE '触发模式结构体
        MV_TRIGGER_MODE_OFF = 0 'chinese 关闭 | english Off
        MV_TRIGGER_MODE_ON  = 1 ' chinese 打开 |english ON
    End Enum
    
    'chinese Gamma选择器 | english Gamma Selector
    Public Enum MV_CAM_GAMMA_SELECTOR
        MV_GAMMA_SELECTOR_USER = 1 'chinese 用户 | english Gamma Selector User
        MV_GAMMA_SELECTOR_SRGB = 2 'chinese sRGB | english Gamma Selector sRGB
    End Enum
    
    'chinese 白平衡 | english White Balance
    Public Enum MV_CAM_BALANCEWHITE_AUTO '白平衡结构体
        MV_BALANCEWHITE_AUTO_OFF        = 0 'chinese 关闭 | english Off
        MV_BALANCEWHITE_AUTO_ONCE       = 2 'chinese 一次 | english Once
        MV_BALANCEWHITE_AUTO_CONTINUOUS = 1 'chinese 连续 | english Continuous
    End Enum
    
    'chinese 触发源 | english Trigger Source
    Public Enum MV_CAM_TRIGGER_SOURCE '触发源结构体
        MV_TRIGGER_SOURCE_LINE0              = 0 'chinese Line0 | english Line0
        MV_TRIGGER_SOURCE_LINE1              = 1 'chinese Line1 | english Line1
        MV_TRIGGER_SOURCE_LINE2              = 2 'chinese Line2 | english Line2
        MV_TRIGGER_SOURCE_LINE3              = 3 'chinese Line3 | english Line3
        MV_TRIGGER_SOURCE_COUNTER0           = 4 'chinese Conuter0 | english Conuter0
        MV_TRIGGER_SOURCE_SOFTWARE           = 7 'chinese 软触发 | english Software
        MV_TRIGGER_SOURCE_FrequencyConverter = 8 'chinese 变频器 | english Frequency Converter
    End Enum

    'chinese Chunk内容 | english The content of ChunkData
    Public Structure MV_CHUNK_DATA_CONTENT
        Public pChunkData As Integer                                                   'chinese Chunk数据 | english Chunk Data
        Public nChunkID As Integer                                                     'chinese Chunk ID | english Chunk ID
        Public nChunkLen As Integer                                                    'chinese Chunk的长度 | english Chunk Length
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=7)> _
        Public nReversed As UInteger()                                                 'chinese 预留 | english Reserved
    End Structure

    'chinese 输出帧的信息 | english Output Frame Information
    Public Structure MV_FRAME_OUT_INFO_EX
        Public nWidth As Short                                                         'chinese 图像宽 | english Image Width
        Public nHeight As Short                                                        'chinese 图像高 | english Image Height
        Public enPixelType As MvGvspPixelType                                          'chinese 像素格式 | english Pixel Type
        Public nFrameNum As UInteger                                                   'chinese 帧号 | english Frame Number
        Public nDevTimeStampHigh As UInteger                                           'chinese 时间戳高32位 | english Timestamp high 32 bits
        Public nDevTimeStampLow As UInteger                                            'chinese 时间戳低32位 | english Timestamp low 32 bits
        Public nReserved0 As UInteger                                                  'chinese 保留，8字节对齐 | english Reserved, 8-byte aligned
        Public nHostTimeStamp As Long                                                  'chinese 主机生成的时间戳 | english Host-generated timestamp
        Public nFrameLen As UInteger                                                   'chinese 帧的长度 | english The Length of Frame
        Public nSecondCount As UInteger                                                'chinese 秒数 | english The Seconds
        Public nCycleCount As UInteger                                                 'chinese 周期数 | english The Count of Cycle
        Public nCycleOffset As UInteger                                                'chinese 周期偏移量 | english The Offset of Cycle
        Public fGain As Single                                                         'chinese 增益 | english Gain
        Public fExposureTime As Single                                                 'chinese 曝光时间 | english Exposure Time
        Public nAverageBrightness As UInteger                                          'chinese 平均亮度 | english Average brightness
        Public nRed As UInteger                                                        'chinese 红色 | english Red
        Public nGreen As UInteger                                                      'chinese 绿色 | english Green
        Public nBlue As UInteger                                                       'chinese 蓝色 | english Blue
        Public nFrameCounter As UInteger                                               'chinese 总帧数 | english Frame Counter
        Public nTriggerIndex As UInteger                                               'chinese 触发计数 | english Trigger Counting
        Public nInput As UInteger                                                      'chinese 输入 | english Input
        Public nOutput As UInteger                                                     'chinese 输出 | english Output
        Public nOffsetX As Short                                                       'chinese 水平偏移量 | english OffsetX
        Public nOffsetY As Short                                                       'chinese 垂直偏移量 | english OffsetY
        Public nChunkWidth As Short                                                    'chinese Chunk宽 | english The Width of Chunk
        Public nChunkHeight As Short                                                   'chinese Chunk高 | english The Height of Chunk
        Public nLostPacket As UInteger                                                 'chinese 本帧丢包数 | english Lost Packet Number In This Frame
        Public nUnparsedChunkNum As UInteger                                           'chinese 未解析的Chunkdata个数 | english Unparsed Chunk Number
        <StructLayout(LayoutKind.Explicit)> _
        Public Structure UNPARSED_CHUNK_LIST
            <FieldOffset(0)> Public pUnParsedChunkContent As IntPtr
            <FieldOffset(0)> Public nAligning As Long
        End Structure

        Public UnparsedChunkList As UNPARSED_CHUNK_LIST                                   'chinese 未解析的Chunk | english Unparsed Chunk Content
		
		Public nExtendWidth As UInteger                                                   'chinese 图像宽(扩展变量) | english Image Width
		Public nExtendHeight As UInteger                                                  'chinese 图像高(扩展变量) | english Image Height
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=34)> _
         Public nReserved() As UInteger                                                   'chinese 预留 | english Reserved
    End Structure

    'chinese 图像结构体，输出图像地址及图像信息 | english Image Struct, output the pointer of Image and the information of the specific image
    Public Structure MV_FRAME_OUT '图像结构体，输出图像指针地址及图像信息
        Public pBufAddr As IntPtr                                                           'chinese 图像指针地址 | english  pointer of image
        Public stFrameInfo As MV_FRAME_OUT_INFO_EX                                          'chinese 图像信息 | english information of the specific image
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=16)> _ 
        Public nReserved() As UInteger                                                      'chinese 预留 | english Reserved

    End Structure

    'chinese 水印信息 | english  Frame-specific information
    Public Structure MV_CC_FRAME_SPEC_INFO
        Public nSecondCount As UInteger                                                     'chinese 秒数 | english The Seconds
        Public nCycleCount As UInteger                                                      'chinese 周期数 | english The Count of Cycle
        Public nCycleOffset As UInteger                                                     'chinese 周期偏移量 | english The Offset of Cycle
        Public fGain As Single                                                              'chinese 增益 | english Gain
        Public fExposureTime As Single                                                      'chinese 曝光时间 | english Exposure Time
        Public nAverageBrightness As UInteger                                               'chinese 平均亮度 | english Average brightness
        Public nRed As UInteger                                                             'chinese 红色 | english Red
        Public nGreen As UInteger                                                           'chinese 绿色 | english Green
        Public nBlue As UInteger                                                            'chinese 蓝色 | english Blue
        Public nFrameCounter As UInteger                                                    'chinese 总帧数 | english Frame Counter
        Public nTriggerIndex As UInteger                                                    'chinese 触发计数 | english Trigger Counting
        Public nInput As UInteger                                                           'chinese 输入 | english Input
        Public nOutput As UInteger                                                          'chinese 输出 | english Output
        Public nOffsetX As UShort                                                           'chinese 水平偏移量 |english OffsetX
        Public nOffsetY As UShort                                                           'chinese 垂直偏移量 | english OffsetY
        Public nFrameWidth As UShort                                                        'chinese 水印宽 | english The Width of Chunk
        Public nFrameHeight As UShort                                                       'chinese 水印高 | english The Height of Chunk
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=16)> _
        Public nReserved() As UInteger                                                      'chinese 预留 |english Reserved
    End Structure

    #End Region

    #Region "图像处理相关信息的定义"
    'chinese 显示帧信息 | english Display frame information
    Public Structure MV_DISPLAY_FRAME_INFO
        Public hWnd As IntPtr                                                            'chinese 窗口句柄 | english HWND
        Public pData As IntPtr                                                           'chinese 显示的数据 | english Data Buffer
        Public nDataLen As UInteger                                                      'chinese 数据长度 | english Data Size
        Public nWidth As UShort                                                          'chinese 图像宽 | english Width
        Public nHeight As UShort                                                         'chinese 图像高 | english Height
        Public enPixelType As MvGvspPixelType                                            'chinese 像素格式 | english Pixel format
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=4)> _
        Public nReserved() As UInteger                                                   'chinese 保留 | english Reserved
    End Structure
	
	Public Structure MV_DISPLAY_FRAME_INFO_EX
		Public nWidth As UInteger                                                        'chinese 图像宽 | english Width
        Public nHeight As UInteger                                                       'chinese 图像高 | english Height
        Public enPixelType As MvGvspPixelType                                            'chinese 像素格式 | english Pixel format
		
        Public pData As IntPtr                                                           'chinese 显示的数据 | english Data Buffer
        Public nDataLen As UInteger                                                      'chinese 数据长度 | english Data Size
        
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=4)> _
        Public nReserved() As UInteger                                                   'chinese 保留 | english Reserved
    End Structure
	
    'chinese 旋转角度 | english Rotation angle
    Public Enum MV_IMG_ROTATION_ANGLE
        MV_IMAGE_ROTATE_90  = 1
        MV_IMAGE_ROTATE_180 = 2
        MV_IMAGE_ROTATE_270 = 3
    End Enum

    'chinese 图像旋转结构体 | english Rotate image structure
    Public Structure MV_CC_ROTATE_IMAGE_PARAM
        Public enPixelType As MvGvspPixelType                       'chinese 像素格式 | english Pixel format
        Public nWidth As UInteger                                   'chinese 图像宽 | english Width
        Public nHeight As UInteger                                  'chinese 图像高 | english Height
        Public pSrcData As IntPtr                                   'chinese 输入数据缓存 | english Input data buffer
        Public nSrcDataLen As UInteger                              'chinese 输入数据长度 | english Input data length

        Public pDstBuffer As IntPtr                                 'chinese 输出数据缓存 | english Output data buffer
        Public nDstLen As UInteger                                  'chinese 输出数据长度 | english Output data length
        Public nDstBufferSize As UInteger                           'chinese 提供的输出缓冲区大小 | english Provided output buffer size
        Public enRotationAngle As MV_IMG_ROTATION_ANGLE             'chinese 旋转角度 | english Rotation angle
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=8)> _
        Public nReserved() As UInteger                              'chinese 保留 | english Reserved

    End Structure

    'chinese 翻转类型 | english Flip type
    Public Enum MV_IMG_FLIP_TYPE
        MV_FLIP_VERTICAL   = 1
        MV_FLIP_HORIZONTAL = 2
    End Enum

    'chinese 图像翻转结构体 | english Flip image structure
    Public Structure MV_CC_FLIP_IMAGE_PARAM
        Public enPixelType As MvGvspPixelType                            'chinese 像素格式 | english Pixel format
        Public nWidth As UInteger                                        'chinese 图像宽 | english Width
        Public nHeight As UInteger                                       'chinese 图像高 | english Height
        Public pSrcData As IntPtr                                        'chinese 输入数据缓存 | english Input data buffer
        Public nSrcDataLen As UInteger                                   'chinese 输入数据长度 | english Input data length

        Public pDstBuffer As IntPtr                                      'chinese 输出数据缓存 | english Output data buffer
        Public nDstLen As UInteger                                       'chinese 输出数据长度 | english Output data length
        Public nDstBufferSize As UInteger                                'chinese 提供的输出缓冲区大小 | english Provided output buffer size
        Public enFlipType As MV_IMG_FLIP_TYPE                            'chinese 翻转类型 | english Flip type
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=8)> _
        Public nReserved() As UInteger                                   'chinese 保留 | english Reserved
    End Structure

    'chinese 像素转换结构体 | english Pixel convert structure
    Public Structure MV_CC_PIXEL_CONVERT_PARAM '图像转换结构体
        Public nWidth As UShort                                           'chinese 图像宽 | english Width
        Public nHeight As UShort                                          'chinese 图像高 | english Height
        Public enSrcPixelType As MvGvspPixelType                          'chinese 源像素格式 | english Source pixel format
        Public pSrcData As IntPtr                                         'chinese 输入数据缓存 | english Input data buffer
        Public nSrcDataLen As Integer                                     'chinese 输入数据长度 | english Input data length

        Public enDstPixelType As MvGvspPixelType                          'chinese 目标像素格式 | english Destination pixel format
        Public pDstBuffer As IntPtr                                       'chinese 输出数据缓存 | english Output data buffer
        Public nDstLen As UInteger                                        'chinese 输出数据长度 | english Output data length
        Public nDstBufferSize As UInteger                                 'chinese 提供的输出缓冲区大小 | english Provided output buffer size
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=4)> _
        Public nReserved() As UInteger                                    'chinese 预留 | english Reserved
    End Structure

    'chinese Gamma类型 | english Gamma type
    Public Enum MV_CC_GAMMA_TYPE
        MV_CC_GAMMA_TYPE_NONE               = 0  'chinese 不启用 | english Disable
        MV_CC_GAMMA_TYPE_VALUE              = 1  'chinese Gamma值 | english Gamma value
        MV_CC_GAMMA_TYPE_USER_CURVE         = 2  'chinese Gamma曲线 | english Gamma curve
                                                 'chinese 8位,长度：256*sizeof(unsigned char) | english 8bit,length:256*sizeof(unsigned char)
                                                 'chinese 10位,长度：1024*sizeof(unsigned short) | english 10bit,length:1024*sizeof(unsigned short)
                                                 'chinese 12位,长度：4096*sizeof(unsigned short) | english 12bit,length:4096*sizeof(unsigned short)
                                                 'chinese 16位,长度：65536*sizeof(unsigned short) | english 16bit,length:65536*sizeof(unsigned short)
        MV_CC_GAMMA_TYPE_LRGB2SRGB          = 3  'chinese linear RGB to sRGB | english linear RGB to sRGB
        MV_CC_GAMMA_TYPE_SRGB2LRGB          = 4  'chinese sRGB to linear RGB(仅色彩插值时支持，色彩校正时无效) | english sRGB to linear RGB
    End Enum

    'chinese Gamma信息结构体 | english Gamma info structure
    Public Structure MV_CC_GAMMA_PARAM
        Public enGammaType As MV_CC_GAMMA_TYPE                                                   'chinese Gamma类型 | english Gamma type
        Public fGammaValue As Single                                                             'chinese Gamma值[0.1,4.0] | english Gamma value[0.1,4.0]
        Public pGammaCurveBuf As IntPtr                                                          'chinese Gamma曲线缓存 | english Gamma curve buffer
        Public nGammaCurveBufLen As UInteger                                                     'chinese Gamma曲线长度 | english Gamma curve buffer size
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=8)> _
        Public nReserved() As UInteger                                                           'chinese 预留 | english Reserved
    End Structure

    'chinese CCM参数 | english CCM param
    Public Structure MV_CC_CCM_PARAM
        Public bCCMEnable As Boolean                                                         'chinese 是否启用CCM | english CCM enable
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=9)> _ 
        Public nCCMat() As Integer                                                           'chinese CCM矩阵(-8192~8192) | english Color correction matrix(-8192~8192)
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=8)> _
        Public nReserved() As UInteger                                                       'chinese 预留 | english Reserved
    End Structure

    'chinese CCM参数 | english CCM param
    Public Structure MV_CC_CCM_PARAM_EX
        Public bCCMEnable As Boolean                                                       'chinese 是否启用CCM | english CCM enable
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=9)> _
        Public nCCMat() As Integer                                                         'chinese CCM矩阵(-65536~65536) | english Color correction matrix(-65536~65536)
        Public nCCMScale As UInteger                                                       'chinese 量化系数（2的整数幂,最大65536） | english Quantitative scale(Integer power of 2, <= 65536)
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=8)> _
        Public nReserved() As UInteger                                                     'chinese 预留 | english Reserved
    End Structure

    'chinese 对比度调节结构体 | english Contrast structure
    Public Structure MV_CC_CONTRAST_PARAM
        Public nWidth As UInteger                                                          'chinese 图像宽度(最小8) | english Image Width
        Public nHeight As UInteger                                                         'chinese 图像高度(最小8) | english Image Height
        Public pSrcBuf As IntPtr                                                           'chinese 输入数据缓存 | english Input data buffer
        Public nSrcBufLen As UInteger                                                      'chinese 输入数据大小 | english Input data length
        Public enPixelType As MvGvspPixelType                                              'chinese 源像素格式 | english Source pixel format

        Public pDstBuf As IntPtr                                                           'chinese 输出数据缓存 | english Output data buffer
        Public nDstBufSize As UInteger                                                     'chinese 提供的输出缓冲区大小 | english Provided output buffer size
        Public nDstBufLen As UInteger                                                      'chinese 输出数据长度 | english Output data length
        Public nContrastFactor As UInteger                                                 'chinese 对比度值，[1,10000] | english Contrast factor,[1,10000]
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=8)> _
        Public nReserved() As UInteger                                                     'chinese 预留 | english Reserved

    End Structure

    'chinese 无损解码参数 | english High Bandwidth decode structure
    Public Structure MV_CC_HB_DECODE_PARAM
        Public pSrcBuf As IntPtr                                                           'chinese 输入数据缓存 | english Input data buffer
        Public nSrcLen As UInteger                                                         'chinese 输入数据大小 | english Input data size
        Public nWidth As UInteger                                                          'chinese 图像宽 | english Width
        Public nHeight As UInteger                                                         'chinese 图像高 | english Height

        Public pDstBuf As IntPtr                                                           'chinese 输出数据缓存 | english Output data buffer
        Public nDstBufSize As UInteger                                                     'chinese 提供的输出缓冲区大小 | english Provided output buffer size
        Public nDstBufLen As UInteger                                                      'chinese 输出数据长度 | english Output data length
        Public enDstPixelType As MvGvspPixelType                                           'chinese 输出源像素格式 | english  Output pixel format

        Public stFrameSpecInfo As MV_CC_FRAME_SPEC_INFO                                    'chinese 水印信息 | english Frame Spec Info
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=8)> _
        Public nReserved() As UInteger                                                     'chinese 预留 | english Reserved
    End Structure

    'chinese 录像格式定义 | english Record Format Type
    Public Enum MV_RECORD_FORMAT_TYPE
        MV_FormatType_Undefined = 0 'chinese 未定义的录像格式 | english Undefined Recode Format Type
        MV_FormatType_AVI = 1 'chinese AVI录像格式 | english AVI Recode Format Type
    End Enum

    'chinese 录像参数 | english Record Parameters
    Public Structure MV_CC_RECORD_PARAM
        Public enPixelType As MvGvspPixelType                         'chinese 输入数据的像素格式 | english Pixel Type
        Public nWidth As UShort                                       'chinese 图像宽(2的倍数) | english Width
        Public nHeight As UShort                                      'chinese 图像高(2的倍数) | english Height
        Public fFrameRate As Single                                   'chinese 帧率fps(大于1/16) | english The Rate of Frame
        Public nBitRate As UInteger                                   'chinese 码率kbps(128-16*1024) | english The Rate of Bitrate
        Public enRecordFmtType As MV_RECORD_FORMAT_TYPE               'chinese 录像格式 | english Recode Format Type
        Public strFilePath As String                                  'chinese 录像文件存放路径(如果路径中存在中文，需转成utf-8) | english File Path

        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=8)> _
        Public nReserved() As UInteger                               'chinese 预留 | english Reserved
    End Structure

    'chinese 传入的图像数据 | english Input Data
    Public Structure MV_CC_INPUT_FRAME_INFO
        Public pData As IntPtr                                                        'chinese 图像数据 | english Record Data
        Public nDataLen As UInteger                                                   'chinese 图像大小 | english The Length of Record Data

        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=8)> _
        Public nReserved() As UInteger                                               'chinese 预留 | english Reserved
    End Structure

    'chinese 辅助线颜色 | english Color of Auxiliary Line
    Public Structure MVCC_COLORF
        Public fR As Single                                   'chinese 红色，根据像素颜色的相对深度，范围为[0.0 , 1.0]，代表着[0, 255]的颜色深度 | english Red，Range[0.0, 1.0]
        Public fG As Single                                   'chinese 绿色，根据像素颜色的相对深度，范围为[0.0 , 1.0]，代表着[0, 255]的颜色深度 | english Green，Range[0.0, 1.0]
        Public fB As Single                                   'chinese 蓝色，根据像素颜色的相对深度，范围为[0.0 , 1.0]，代表着[0, 255]的颜色深度 | english Blue，Range[0.0, 1.0]
        Public fAlpha As Single                               'chinese 透明度，根据像素颜色的相对透明度，范围为[0.0 , 1.0] (此参数功能暂不支持) | english Alpha，Range[0.0, 1.0](Not Support)
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=4)> _
        Public nReserved() As UInteger                       'chinese 预留 | english Reserved
    End Structure

    'chinese 自定义点 | english Point defined
    Public Structure MVCC_POINTF
        Public fX As Single                                 'chinese 该点距离图像左边缘距离，根据图像的相对位置，范围为[0.0 , 1.0] | english Distance From Left，Range[0.0, 1.0]
        Public fY As Single                                 'chinese 该点距离图像上边缘距离，根据图像的相对位置，范围为[0.0 , 1.0] | english Distance From Top，Range[0.0, 1.0]

        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=4)> _
        Public nReserved() As UInteger                      'chinese 预留 | english Reserved
    End Structure

    'chinese 矩形框区域信息 | english Rect Area Info
    Public Structure MVCC_RECT_INFO
        Public fTop As Single                                'chinese 矩形上边缘距离图像上边缘的距离，根据图像的相对位置，范围为[0.0 , 1.0] | english Distance From Top，Range[0, 1.0]
        Public fBottom As Single                             'chinese 矩形下边缘距离图像下边缘的距离，根据图像的相对位置，范围为[0.0 , 1.0] | english Distance From Bottom，Range[0, 1.0]
        Public fLeft As Single                               'chinese 矩形左边缘距离图像左边缘的距离，根据图像的相对位置，范围为[0.0 , 1.0] | english Distance From Left，Range[0, 1.0]
        Public fRight As Single                              'chinese 矩形右边缘距离图像右边缘的距离，根据图像的相对位置，范围为[0.0 , 1.0] | english Distance From Right，Range[0, 1.0]

        Public stColor As MVCC_COLORF                        'chinese 辅助线颜色 | english Color of Auxiliary Line
        Public nLineWidth As UInteger                        'chinese 辅助线宽度，宽度只能是1或2 | english Width of Auxiliary Line, width is 1 or 2
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=4)> _
        Public nReserved() As UInteger                       'chinese 预留 | english Reserved
    End Structure

    'chinese 圆形框区域信息            \~english Circle Area Info
    Public Structure MVCC_CIRCLE_INFO
        Public stCenterPoint As MVCC_POINTF                  'chinese 圆心信息 | english Circle Point Info
        Public fR1 As Single                                 'chinese 宽向半径，根据图像的相对位置[0, 1.0]，半径与圆心的位置有关，需保证画出的圆在显示框范围之内，否则报错 | english Windth Radius, Range[0, 1.0]
        Public fR2 As Single                                 'chinese 高向半径，根据图像的相对位置[0, 1.0]，半径与圆心的位置有关，需保证画出的圆在显示框范围之内，否则报错 | english Height Radius, Range[0, 1.0]
        Public stColor As MVCC_COLORF                        'chinese 辅助线颜色 | english Color of Auxiliary Line
        Public nLineWidth As UInteger                        'chinese 辅助线宽度，宽度只能是1或2 | english Width of Auxiliary Line, width is 1 or 2
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=4)> _
        Public nReserved() As UInteger                      'chinese 预留 | english Reserved
    End Structure

    'chinese 线条辅助线信息 | english Linear Auxiliary Line Info
    Public Structure MVCC_LINES_INFO
        Public stStartPoint As MVCC_POINTF                   'chinese 线条辅助线的起始点坐标 | english The Start Point of Auxiliary Line
        Public stEndPoint As MVCC_POINTF                     'chinese 辅助线颜色信息 | english Color of Auxiliary Line
        Public stColor As MVCC_COLORF                        'chinese 辅助线颜色 | english Color of Auxiliary Line
        Public nLineWidth As UInteger                        'chinese 辅助线宽度，宽度只能是1或2 | english Width of Auxiliary Line, width is 1 or 2
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=4)> _
        Public nReserved() As UInteger                      'chinese 预留 | english Reserved
    End Structure

    'chinese 图像重构的方式 | english Image reconstruction method
    Public Enum MV_IMAGE_RECONSTRUCTION_METHOD
        MV_SPLIT_BY_LINE = 1  'chinese 源图像按行拆分成多张图像 | english Source image split into multiple images by line
    End Enum

    'chinese 图像重构后的图像列表 | english List of images after image reconstruction
    Public Structure MV_OUTPUT_IMAGE_INFO
        Public nWidth As UInteger                             'chinese 图像宽 | english Width
        Public nHeight As UInteger                            'chinese 图像高 | english Height
        Public enPixelType As MvGvspPixelType                 'chinese 像素格式 | english Pixel format

        Public pBuff As IntPtr                                'chinese 输出数据缓存 | english Output data buffer
        Public nBufLen As UInteger                            'chinese 输出数据长度 | english Output data length
        Public nBufSize As UInteger                           'chinese 提供的输出缓冲区大小 | english Provided output buffer size
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=8)> _
        Public nReserved() As UInteger                       'chinese 预留 | english Reserved
    End Structure

    'chinese 重构图像参数信息 | english Restructure image parameters
    Public Structure MV_RECONSTRUCT_IMAGE_PARAM
        Public nWidth As UInteger                                                    'chinese 图像宽 | english Width
        Public nHeight As UInteger                                                   'chinese 图像高 | english Height
        Public enPixelType As MvGvspPixelType                                        'chinese 像素格式 | english Pixel format

        Public pSrcData As IntPtr                                                    'chinese 输入数据缓存 | english Input data buffer
        Public nSrcDataLen As UInteger                                               'chinese 输入数据长度 | english Input data length
        Public nExposureNum As UInteger                                              'chinese 曝光个数(1-8] | english Exposure number
        Public enReconstructMethod As MV_IMAGE_RECONSTRUCTION_METHOD                 'chinese 图像重构方式 | english Image restructuring method
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=MV_MAX_SPLIT_NUM)> _
        Public stDstBufList() As MV_OUTPUT_IMAGE_INFO                                'chinese 输出数据缓存信息 | english Output data info
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=4)> _
        Public nReserved() As UInteger                                                'chinese 预留 | english Reserved
    End Structure
    #End Region

    #Region "万能接口相关信息的定义"
    'chinese 每个节点对应的接口类型 | english Interface type corresponds to each node 
    Public Enum MV_XML_InterfaceType
        IFT_IValue                                                     'chinese Value | english IValue interface
        IFT_IBase                                                      'chinese Base | english IBase interface
        IFT_IInteger                                                   'chinese Integer | english IInteger interface
        IFT_IBoolean                                                   'chinese Boolean | english IBoolean interface
        IFT_ICommand                                                   'chinese Command | english ICommand interface
        IFT_IFloat                                                     'chinese Float | english IFloat interface
        IFT_IString                                                    'chinese String | english IString interface
        IFT_IRegister                                                  'chinese Register | english IRegister interface
        IFT_ICategory                                                  'chinese Category | english ICategory interface
        IFT_IEnumeration                                               'chinese Enumeration | english IEnumeration interface
        IFT_IEnumEntry                                                 'chinese EnumEntry | english IEnumEntry interface
        IFT_IPort                                                      'chinese Port | english IPort interface
    End Enum

    'chinese 节点的访问模式 | english Node Access Mode
    Public Enum MV_XML_AccessMode
        AM_NI                                               'chinese 不可实现 | english Not implemented
        AM_NA                                               'chinese 不可用 | english Not available
        AM_WO                                               'chinese 只写 | english Write Only
        AM_RO                                               'chinese 只读 | english Read Only
        AM_RW                                               'chinese 读写 | english Read and Write
        AM_Undefined                                        'chinese 未定义 | english Object is not yet initialized
        AM_CycleDetect                                      'chinese 内部用于AccessMode循环检测 | english used internally for AccessMode cycle detection
    End Enum

    'chinese 枚举类型值 | english Enumeration Value
    Public Structure MVCC_ENUMVALUE
        Public nCurValue As UInteger                                                                                 'chinese 当前值 | english Current Value
        Public nSupportedNum As UInteger                                                                             'chinese 数据的有效数据个数 | english Number of valid data
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=MV_MAX_XML_SYMBOLIC_NUM)> _
        Public nSupportValue() As UInteger                                                                           'chinese 支持的枚举值 | english Support Value
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=4)> _
        Public nReserved() As UInteger                                                                               'chinese 预留 | english Reserved
    End Structure

    'chinese 枚举类型条目 | english Enumeration Entry
    Public Structure MVCC_ENUMENTRY
        Public nValue As UInteger                                                                                    'chinese 指定值 | english Value
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=MV_MAX_SYMBOLIC_LEN)> _
        Public chSymbolic() As Byte                                                                                  'chinese 指定值对应的符号 | english Symbolic
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=4)> _
        Public nReserved() As UInteger                                                                               'chinese 预留 | english Reserved
    End Structure

    'chinese Int类型值 | english Int Value
    Public Structure MVCC_INTVALUE
        Public nCurValue As UInteger                                                                                 'chinese 当前值 | english Current Value
        Public nMax As UInteger                                                                                      'chinese 最大值 | english Max
        Public nMin As UInteger                                                                                      'chinese 最小值 | english Min
        Public nInc As UInteger                                                                                      'chinese 步进间距 | english Inc
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=4)> _
        Public nReserved() As UInteger                                                                               'chinese 预留 | english Reserved
    End Structure

    'chinese Int类型值扩展 | english Int Value Ex
    Public Structure MVCC_INTVALUE_EX
        Public nCurValue As Long                                                                                    'chinese 当前值 | english Current Value
        Public nMax As Long                                                                                         'chinese 最大值 | english Max
        Public nMin As Long                                                                                         'chinese 最小值 | english Min
        Public nInc As Long                                                                                         'chinese 步进间距 | english Inc
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=16)> _
        Public nReserved() As UInteger                                                                              'chinese 预留 | english Reserved
    End Structure

    'chinese Float类型值 | english Float Value
    Public Structure MVCC_FLOATVALUE 'Float类型结构体
        Public fCurValue As Single                                                                                 'chinese 当前值 | english Current Value
        Public fMax As Single                                                                                      'chinese 最大值 | english Max
        Public fMin As Single                                                                                      'chinese 最小值 | english Min
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=4)> _
        Public nReserved() As UInteger                                                                             'chinese 预留 | english Reserved
    End Structure

    'chinese String类型值 | english String Value
    Public Structure MVCC_STRINGVALUE 'String类型结构体
        Public chCurValue As String                                                                                 'chinese 当前值 | english Current Value
        Public nMaxLength As Long                                                                                   'chinese 最大长度 | english MaxLength
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=2)> _
        Public nReserved() As UInteger                                                                              'chinese 预留 | english Reserved
    End Structure
    #End Region

    #Region "文件传输相关信息的定义"
    'chinese 文件存取 | english File Access
    Public Structure MV_CC_FILE_ACCESS '文件存取结构体
        Public pUserFileName As String                                                                            'chinese 用户文件名 | english User file name
        Public pDevFileName As String                                                                             'chinese 设备文件名 | english Device file name
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=32)> _
        Public nReserved() As UInteger                                                                            'chinese 预留 | english Reserved
    End Structure

    'chinese 文件数据存取 | english File Buffer Access
    Public Structure MV_CC_FILE_ACCESS_EX '文件存取结构体
        Public pUserFileBuf As IntPtr                                                                             'chinese 用户文件数据缓存 | english User file data
        Public nFileBufSize As UInteger                                                                           'chinese 用户数据缓存大小 | english User buffer size
        Public nFileBufLen As UInteger                                                                            'chinese 文件数据大小 | english File data len

        Public pDevFileName As String                                                                             'chinese 设备文件名 | english Device file name
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=32)> _
        Public nReserved() As UInteger                                                                            'chinese 预留 | english Reserved
    End Structure

    'chinese 文件存取进度 | english File Access Progress
    Public Structure MV_CC_FILE_ACCESS_PROGRESS
        Public nCompleted As Long                                                                                'chinese 已完成的长度 | english Completed Length
        Public nTotal As Long                                                                                    'chinese 总长度 | english Total Length
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=8)> _
        Public nReserved() As UInteger                                                                          'chinese 预留 | english Reserved

    End Structure
    #End Region

    #Region "文件图像保存相关信息的定义"
    'chinese 保存图片格式 | english Save image type
    Public Enum MV_SAVE_IAMGE_TYPE '保存图片格式
        MV_Image_Undefined = 0                                                            'chinese 未定义的图像格式 | english Undefined Image Type
        MV_Image_Bmp = 1                                                                  'chinese BMP图像格式 | english BMP Image Type
        MV_Image_Jpeg = 2                                                                 'chinese JPEG图像格式 | english Jpeg Image Type
        MV_Image_Png = 3                                                                  'chinese PNG图像格式 | english Png  Image Type
        MV_Image_Tif = 4                                                                  'chinese TIFF图像格式 | english TIFF Image Type
    End Enum
    
    'chinese 保存的3D数据格式 | english The saved format for 3D data
    Public Enum MV_SAVE_POINT_CLOUD_FILE_TYPE '保存图片格式
        MV_PointCloudFile_Undefined = 0                                                   'chinese 未定义的点云格式 | english Undefined point cloud format
        MV_PointCloudFile_PLY = 1                                                         'chinese PLY点云格式 | english The point cloud format named PLY
        MV_PointCloudFile_CSV = 2                                                         'chinese CSV点云格式 | english The point cloud format named CSV
        MV_PointCloudFile_OBJ = 3                                                         'chinese OBJ点云格式 | english The point cloud format named OBJ
    End Enum

    'chinese 保存3D数据到缓存 | english Save 3D data to buffer
    Public Structure MV_SAVE_POINT_CLOUD_PARAM '保存3D数据到缓存结构体
        Public nLinePntNum As UInteger                                                        'chinese 行点数，即图像宽 | english The number of points in each row,which is the width of the image
        Public nLineNum As UInteger                                                           'chinese 行数，即图像高 | english The number of rows,which is the height of the image
        Public enSrcPixelType As MvGvspPixelType                                              'chinese 输入数据的像素格式 | english The pixel format of the input data
        Public pSrcData As IntPtr                                                             'chinese 输入数据缓存 | english Input data buffer
        Public nSrcDataLen As UInteger                                                        'chinese 输入数据长度 | english Input data length
        Public pDstBuf As IntPtr                                                              'chinese 输出像素数据缓存 | english Output pixel data buffer
        Public nDstBufSize As UInteger                                                        'chinese 提供的输出缓冲区大小(nLinePntNum * nLineNum * (16*3 + 4) + 2048) | english Output buffer size provided(nLinePntNum * nLineNum * (16*3 + 4) + 2048) 
        Public nDstBufLen As UInteger                                                         'chinese 输出像素数据缓存长度 | english Output pixel data buffer size
        Public enPointCloudFileType As MV_SAVE_POINT_CLOUD_FILE_TYPE                          'chinese 提供输出的点云文件类型 | english Output point data file type provided
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=8)> _
        Public nReserved() As UInteger                                                        'chinese 保留字段 |english Reserved
    End Structure

    'chinese 图片保存参数 | english Save Image Parameters
    Public Structure MV_SAVE_IMAGE_PARAM_EX '保存图片参数
        Public pData As IntPtr                                                                 'chinese 输入数据缓存 | english Input Data Buffer
        Public nDataLen As UInteger                                                            'chinese 输入数据长度 | english Input Data length
        Public enPixelType As MvGvspPixelType                                                  'chinese 输入数据的像素格式 | english Input Data Pixel Format
        Public nWidth As UShort                                                                'chinese 图像宽 | english Image Width
        Public nHeight As UShort                                                               'chinese 图像高 | english Image Height
        Public pImageBuffer As IntPtr                                                          'chinese 输出图片缓存 | english Output Image Buffer
        Public nImageLen As UInteger                                                           'chinese 输出图片长度 | english Output Image length
        Public nBufferSize As UInteger                                                         'chinese 提供的输出缓冲区大小 | english Output buffer size provided
        Public enImageType As MV_SAVE_IAMGE_TYPE                                               'chinese 输出图片格式 | english Output Image Format
        Public nJpgQuality As UInteger                                                         'chinese JPG编码质量(50-99]，其它格式无效 | english Encoding quality(50-99]，Other formats are invalid
        Public iMethodValue As UInteger                                                        'chinese 插值方法 0-快速 1-均衡 2-最优（其它值默认为最优） | english Bayer interpolation method  0-Fast 1-Equilibrium 2-Optimal
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=3)> _
        Public nReserved() As UInteger                                                         'chinese 预留 | english Reserved

    End Structure

    'chinese 图片文件保存参数 | english Save Image Parameters
    Public Structure MV_SAVE_IMG_TO_FILE_PARAM '保存BMP、JPEG、PNG、TIFF图片文件的参数
        Public enPixelType As MvGvspPixelType                                                         'chinese输入数据的像素格式 | english The pixel format of the input data
        Public pData As IntPtr                                                                        'chinese 输入数据缓存 | english Input Data Buffer
        Public nDataLen As UInteger                                                                   'chinese 输入数据长度 | english Input Data length
        Public nWidth As UShort                                                                       'chinese 图像宽 | english Image Width
        Public nHeight As UShort                                                                      'chinese 图像高 | english Image Height
        Public enImageType As MV_SAVE_IAMGE_TYPE                                                      'chinese 输入图片格式 | english Input Image Format
        Public nQuality As UInteger                                                                   'chinese JPG编码质量(50-99]，PNG编码质量[0-9]，其它格式无效 | english JPG Encoding quality(50-99],PNG Encoding quality[0-9]，Other formats are invalid
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=256)> _
        Public pImagePath As String                                                                   'chinese 输入文件路径 | english Input file path
        Public iMethodValue As UInteger                                                               'chinese 插值方法 0-快速 1-均衡 2-最优（其它值默认为最优） | english Bayer interpolation method  0-Fast 1-Equilibrium 2-Optimal
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=8)> _
        Public nReserved() As UInteger                                                               'chinese 预留 | english Reserved
    End Structure
    #End Region

    #Region "其他相关信息的定义"
    'chinese Event事件回调信息 | english Event callback infomation
    Public Structure MV_EVENT_OUT_INFO '事件信息结构体
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=MAX_EVENT_NAME_SIZE)> _
        Public EventName As String                                                                        'chinese Event名称 | english Event name
        Public nEventID As UShort                                                                         'chinese 流通道序号 |english Circulation number
        Public nStreamChannel As UShort                                                                   'chinese 流通道序号 |english Circulation number
        Public nBlockIdHigh As UInteger                                                                   'chinese 帧号高位 | english BlockId high
        Public nBlockIdLow As UInteger                                                                    'chinese 帧号低位 | english BlockId low
        Public nTimestampHigh As UInteger                                                                 'chinese 时间戳高位 | english Timestramp high
        Public nTimestampLow As UInteger                                                                  'chinese 时间戳低位 | english Timestramp low
        Public pEventData As IntPtr                                                                       'chinese Event数据 | english Event data
        Public nEventDataSize As UInteger                                                                 'chinese Event数据长度 | english Event data len
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=16)> _
        Public nReserved() As UInteger                                                                    'chinese 预留 | english Reserved
    End Structure

    Public Structure MV_ACTION_CMD_INFO
        Public nDeviceKey As UInteger                                                                     'chinese 设备密钥 | english Device Key
        Public nGroupkey As UInteger                                                                      'chinese 组键 | english Group Key
        Public nGroupMask As UInteger                                                                     'chinese 组掩码 | english Group Mask
        Public nActionTimeEnable As UInteger                                                              'chinese 只有设置成1时Action Time才有效，非1时无效 | english Action Time Enable
        Public nActionTime As Long                                                                        'chinese 预定的时间，和主频有关 | english Action Time
        Public pBroadcastAddress As String                                                                'chinese 广播包地址 | english Broadcast Address
        Public nTimeOut As UInteger                                                                       'chinese 等待ACK的超时时间，如果为0表示不需要ACK | english TimeOut
        Public bSpecialNetEnable As UInteger                                                              'chinese 只有设置成1时指定的网卡IP才有效，非1时无效 | english Special IP Enable
        Public nSpecialNetIP As UInteger                                                                  'chinese 指定的网卡IP | english Special Net IP address

        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=14)> _
        Public nReserved() As UInteger                                                                   'chinese 预留 | english Reserved
    End Structure

    'chinese 动作命令返回信息 | english Action Command Result
    Public Structure MV_ACTION_CMD_RESULT
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=16)> _
        Public strDeviceAddress() As Byte                                                      'chinese 设备IP | english IP address of the device
        Public nStatus As Integer                                                              'chinese 状态码 | english status code Returned by the device
                                                                                                '1.0x0000:success.
                                                                                                '2.0x8001:Command is not supported by the device.
                                                                                                '3.0x8013:The device is not synchronized to a master clock to be used as time reference.
                                                                                                '4.0x8015:A device queue or packet data has overflowed.
                                                                                                '5.0x8016:The requested scheduled action command was requested at a time that is already past.

    <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=4)> _
    Public nReserved() As UInteger                                                               'chinese 预留 | english Reserved
    End Structure

    'chinese 动作命令返回信息列表 | english Action Command Result List
    Public Structure MV_ACTION_CMD_RESULT_LIST
        Public nNumResults As UInteger                                                       'chinese 返回值个数 | english Number of Returned values
        Public pResults As Integer                                                           'chinese 动作命令结果 | english Reslut of action command
    End Structure
    #End Region

    #Region "设备错误码定义"
    Public Const MV_OK As Integer                      = &H00000000  '成功，无错误

    '通用错误码定义:范围&H80000000-&H800000FF
    Public Const MV_E_HANDLE As Integer                 = &H80000000 '错误或无效的句柄
    Public Const MV_E_SUPPORT As Integer                = &H80000001 '不支持的功能
    Public Const MV_E_BUFOVER As Integer                = &H80000002 '缓存已满
    Public Const MV_E_CALLORDER As Integer              = &H80000003 '函数调用顺序错误
    Public Const MV_E_PARAMETER As Integer              = &H80000004 '错误的参数
    Public Const MV_E_RESOURCE As Integer               = &H80000006 '资源申请失败
    Public Const MV_E_NODATA As Integer                 = &H80000007 '无数据
    Public Const MV_E_PRECONDITION As Integer           = &H80000008 '前置条件有误，或运行环境已发生变化
    Public Const MV_E_VERSION As Integer                = &H80000009 '版本不匹配
    Public Const MV_E_NOENOUGH_BUF As Integer           = &H8000000A '传入的内存空间不足
    Public Const MV_E_ABNORMAL_IMAGE As Integer         = &H8000000B '异常图像，可能是丢包导致图像不完整
    Public Const MV_E_LOAD_LIBRARY As Integer           = &H8000000C '动态导入DLL失败
    Public Const MV_E_NOOUTBUF As Integer               = &H8000000D '没有可输出的缓存
    Public Const MV_E_ENCRYPT As Integer                = &H8000000E '加密错误
    Public Const MV_E_UNKNOW As Integer                 = &H800000FF '未知的错误

    'GenICam系列错误:范围&H80000100-&H800001FF
    Public Const MV_E_GC_GENERIC As Integer             = &H80000100 '通用错误
    Public Const MV_E_GC_ARGUMENT As Integer            = &H80000101 '参数非法
    Public Const MV_E_GC_RANGE As Integer               = &H80000102 '值超出范围
    Public Const MV_E_GC_PROPERTY As Integer            = &H80000103 '属性
    Public Const MV_E_GC_RUNTIME As Integer             = &H80000104 '运行环境有问题
    Public Const MV_E_GC_LOGICAL As Integer             = &H80000105 '逻辑错误
    Public Const MV_E_GC_ACCESS As Integer              = &H80000106 '节点访问条件有误
    Public Const MV_E_GC_TIMEOUT As Integer             = &H80000107 '超时
    Public Const MV_E_GC_DYNAMICCAST As Integer         = &H80000108 '转换异常
    Public Const MV_E_GC_UNKNOW As Integer              = &H800001FF 'GenICam未知错误

    ' GigE_STATUS对应的错误码:范围&H80000200-&H800002FF
    Public Const MV_E_NOT_IMPLEMENTED As Integer        = &H80000200 '命令不被设备支持
    Public Const MV_E_INVALID_ADDRESS As Integer        = &H80000201 '访问的目标地址不存在
    Public Const MV_E_WRITE_PROTECT As Integer          = &H80000202 '目标地址不可写
    Public Const MV_E_ACCESS_DENIED As Integer          = &H80000203 '设备无访问权限
    Public Const MV_E_BUSY As Integer                   = &H80000204 '设备忙，或网络断开
    Public Const MV_E_PACKET As Integer                 = &H80000205 '网络包数据错误
    Public Const MV_EER_NET As Integer                  = &H80000206 '网络相关错误
    Public Const MV_E_IP_CONFLICT As Integer            = &H80000221 '设备IP冲突

    ' USB_STATUS对应的错误码:范围&H80000300-&H800003FF
    Public Const MV_E_USB_READ As Integer                = &H80000300 '读usb出错
    Public Const MV_E_USB_WRITE As Integer               = &H80000301 '写usb出错
    Public Const MV_E_USB_DEVICE As Integer              = &H80000302 '设备异常
    Public Const MV_E_USB_GENICAM As Integer             = &H80000303 'GenICam相关错误
    Public Const MV_E_USB_BANDWIDTH As Integer           = &H80000304 '带宽不足
    Public Const MV_E_USB_DRIVER As Integer              = &H80000305 '驱动不匹配或者未装驱动
    Public Const MV_E_USB_UNKNOW As Integer              = &H800003FF 'USB未知的错误

    ' 升级时对应的错误码:范围&H80000400-&H800004FF
    Public Const MV_E_UPG_FILE_MISMATCH As Integer       = &H80000400 '升级固件不匹配
    Public Const MV_E_UPG_LANGUSGE_MISMATCH As Integer   = &H80000401 '升级固件语言不匹配
    Public Const MV_E_UPG_CONFLICT As Integer            = &H80000402 '升级冲突（设备已经在升级了再次请求升级即返回此错误）
    Public Const MV_E_UPG_INNER_ERR As Integer           = &H80000403 '升级时设备内部出现错误
    Public Const MV_E_UPG_UNKNOW As Integer              = &H800004FF '升级时未知错误

    '来自ISP算法库的错误码
    Public Const MV_ALG_OK As Integer                    = &H00000000 '处理正确
    Public Const MV_ALG_ERR As Integer                   = &H10000000 '不确定类型错误
    Public Const MV_ALG_E_ABILITY_ARG As Integer         = &H10000001 '能力集中存在无效参数

    ' 内存检查
    Public Const MV_ALG_E_MEM_NULL As Integer            = &H10000002 '内存地址为空
    Public Const MV_ALG_E_MEM_ALIGN As Integer           = &H10000003 '内存对齐不满足要求
    Public Const MV_ALG_E_MEM_LACK As Integer            = &H10000004 '内存空间大小不够
    Public Const MV_ALG_E_MEM_SIZE_ALIGN As Integer      = &H10000005 '内存空间大小不满足对齐要求
    Public Const MV_ALG_E_MEM_ADDR_ALIGN As Integer      = &H10000006 '内存地址不满足对齐要求

    ' 图像检查
    Public Const MV_ALG_E_IMG_FORMAT As Integer          = &H10000007 '图像格式不正确或者不支持
    Public Const MV_ALG_E_IMG_SIZE As Integer            = &H10000008 '图像宽高不正确或者超出范围
    Public Const MV_ALG_E_IMG_STEP As Integer            = &H10000009 '图像宽高与step参数不匹配
    Public Const MV_ALG_E_IMG_DATA_NULL As Integer       = &H1000000A '图像数据存储地址为空

    ' 输入输出参数检查
    Public Const MV_ALG_E_CFG_TYPE As Integer            = &H1000000B '设置或者获取参数类型不正确
    Public Const MV_ALG_E_CFG_SIZE As Integer            = &H1000000C '设置或者获取参数的输入、输出结构体大小不正确
    Public Const MV_ALG_E_PRC_TYPE As Integer            = &H1000000D '处理类型不正确
    Public Const MV_ALG_E_PRC_SIZE As Integer            = &H1000000E '处理时输入、输出参数大小不正确
    Public Const MV_ALG_E_FUNC_TYPE As Integer           = &H1000000F '子处理类型不正确
    Public Const MV_ALG_E_FUNC_SIZE As Integer           = &H10000010 '子处理时输入、输出参数大小不正确

    ' 运行参数检查
    Public Const MV_ALG_E_PARAM_INDEX As Integer         = &H10000011 'index参数不正确
    Public Const MV_ALG_E_PARAM_VALUE As Integer         = &H10000012 'value参数不正确或者超出范围
    Public Const MV_ALG_E_PARAM_NUM As Integer           = &H10000013 'param_num参数不正确
    ' 接口调用检查
    Public Const MV_ALG_E_NULL_PTR As Integer            = &H10000014 '函数参数指针为空
    Public Const MV_ALG_E_OVER_MAX_MEM As Integer        = &H10000015 '超过限定的最大内存
    Public Const MV_ALG_E_CALL_BACK As Integer           = &H10000016 '回调函数出错

    ' 算法库加密相关检查
    Public Const MV_ALG_E_ENCRYPT As Integer             = &H10000017 '加密错误
    Public Const MV_ALG_E_EXPIRE As Integer              = &H10000018 '算法库使用期限错误

    ' 内部模块返回的基本错误类型
    Public Const MV_ALG_E_BAD_ARG As Integer             = &H10000019 '参数范围不正确
    Public Const MV_ALG_E_DATA_SIZE As Integer           = &H1000001A '数据大小不正确
    Public Const MV_ALG_E_STEP As Integer                = &H1000001B '数据step不正确

    ' cpu指令集支持错误码
    Public Const MV_ALG_E_CPUID As Integer               = &H1000001C 'cpu不支持优化代码中的指令集
    Public Const MV_ALG_WARNING As Integer               = &H1000001D '警告
    Public Const MV_ALG_E_TIME_OUT As Integer            = &H1000001E '算法库超时
    Public Const MV_ALG_E_LIB_VERSION As Integer         = &H1000001F '算法版本号出错
    Public Const MV_ALG_E_MODEL_VERSION As Integer       = &H10000020 '模型版本号出错
    Public Const MV_ALG_E_GPU_MEM_ALLOC As Integer       = &H10000021 'GPU内存分配错误
    Public Const MV_ALG_E_FILE_NON_EXIST As Integer      = &H10000022 '文件不存在
    Public Const MV_ALG_E_NONE_STRING As Integer         = &H10000023 '字符串为空
    Public Const MV_ALG_E_IMAGE_CODEC As Integer         = &H10000024 '图像解码器错误
    Public Const MV_ALG_E_FILE_OPEN As Integer           = &H10000025 '打开文件错误
    Public Const MV_ALG_E_FILE_READ As Integer           = &H10000026 '文件读取错误
    Public Const MV_ALG_E_FILE_WRITE As Integer          = &H10000027 '文件写错误
    Public Const MV_ALG_E_FILE_READ_SIZE As Integer      = &H10000028 '文件读取大小错误
    Public Const MV_ALG_E_FILE_TYPE As Integer           = &H10000029 '文件类型错误
    Public Const MV_ALG_E_MODEL_TYPE As Integer          = &H1000002A '模型类型错误
    Public Const MV_ALG_E_MALLOC_MEM As Integer          = &H1000002B '分配内存错误
    Public Const MV_ALG_E_BIND_CORE_FAILED As Integer    = &H1000002C '线程绑核失败

    '降噪特有错误码
    Public Const MV_ALG_E_DENOISE_NE_IMG_FORMAT As Integer          = &H10402001 '噪声特性图像格式错误
    Public Const MV_ALG_E_DENOISE_NE_FEATURE_TYPE As Integer        = &H10402002 '噪声特性类型错误
    Public Const MV_ALG_E_DENOISE_NE_PROFILE_NUM As Integer         = &H10402003 '噪声特性个数错误
    Public Const MV_ALG_E_DENOISE_NE_GAIN_NUM As Integer            = &H10402004 '噪声特性增益个数错误
    Public Const MV_ALG_E_DENOISE_NE_GAIN_VAL As Integer            = &H10402005 '噪声曲线增益值输入错误
    Public Const MV_ALG_E_DENOISE_NE_BIN_NUM As Integer             = &H10402006 '噪声曲线柱数错误
    Public Const MV_ALG_E_DENOISE_NE_INIT_GAIN As Integer           = &H10402007 '噪声估计初始化增益设置错误
    Public Const MV_ALG_E_DENOISE_NE_NOT_INIT As Integer            = &H10402008 '噪声估计未初始化
    Public Const MV_ALG_E_DENOISE_COLOR_MODE As Integer             = &H10402009 '颜色空间模式错误
    Public Const MV_ALG_E_DENOISE_ROI_NUM As Integer                = &H1040200a '图像ROI个数错误
    Public Const MV_ALG_E_DENOISE_ROI_ORI_PT As Integer             = &H1040200b '图像ROI原点错误
    Public Const MV_ALG_E_DENOISE_ROI_SIZE As Integer               = &H1040200c '图像ROI大小错误
    Public Const MV_ALG_E_DENOISE_GAIN_NOT_EXIST As Integer         = &H1040200d '输入的相机增益不存在(增益个数已达上限)
    Public Const MV_ALG_E_DENOISE_GAIN_BEYOND_RANGE As Integer      = &H1040200e '输入的相机增益不在范围内
    Public Const MV_ALG_E_DENOISE_NP_BUF_SIZE As Integer            = &H1040200f '输入的噪声特性内存大小错误
    #End Region

    #Region "像素格式定义"
    ''' <summary>
    ''' 像素格式定义
    ''' </summary>
    ''' <remarks></remarks>
    Public Enum MvGvspPixelType
        PixelType_Gvsp_Undefined                                       = -1         '未定义像素格式
        PixelType_Gvsp_Mono1p                                          = &H1010037  'Mono1p
        PixelType_Gvsp_Mono2p                                          = &H1020038  'Mono2p
        PixelType_Gvsp_Mono4p                                          = &H1040039  'Mono4p
        PixelType_Gvsp_Mono8                                           = &H1080001  'Mono8
        PixelType_Gvsp_Mono8_Signed                                    = &H1080002  'Mono8_Signed
        PixelType_Gvsp_Mono10                                          = &H1100003  'Mono10
        PixelType_Gvsp_Mono10_Packed                                   = &H10C0004  'Mono10_Packed
        PixelType_Gvsp_Mono12                                          = &H1100005  'Mono12
        PixelType_Gvsp_Mono12_Packed                                   = &H10C0006  'Mono12_Packed
        PixelType_Gvsp_Mono14                                          = &H1100025  'Mono14
        PixelType_Gvsp_Mono16                                          = &H1100007  'Mono16
        PixelType_Gvsp_BayerGR8                                        = &H1080008  'BayerGR8
        PixelType_Gvsp_BayerRG8                                        = &H1080009  'BayerRG8
        PixelType_Gvsp_BayerGB8                                        = &H108000A  'BayerGB8
        PixelType_Gvsp_BayerBG8                                        = &H108000B  'BayerBG8
        PixelType_Gvsp_BayerRBGG8                                      = &H1080046  'BayerRBGG8
        PixelType_Gvsp_BayerGR10                                       = &H110000C  'BayerGR10
        PixelType_Gvsp_BayerRG10                                       = &H110000D  'BayerRG10
        PixelType_Gvsp_BayerGB10                                       = &H110000E  'BayerGB10
        PixelType_Gvsp_BayerBG10                                       = &H110000F  'BayerBG10
        PixelType_Gvsp_BayerGR12                                       = &H1100010  'BayerGR12
        PixelType_Gvsp_BayerRG12                                       = &H1100011  'BayerRG12
        PixelType_Gvsp_BayerGB12                                       = &H1100012  'BayerGB12
        PixelType_Gvsp_BayerBG12                                       = &H1100013  'BayerBG12
        PixelType_Gvsp_BayerGR10_Packed                                = &H10C0026  'BayerGR10_Packed
        PixelType_Gvsp_BayerRG10_Packed                                = &H10C0027  'BayerRG10_Packed
        PixelType_Gvsp_BayerGB10_Packed                                = &H10C0028  'BayerGB10_Packed
        PixelType_Gvsp_BayerBG10_Packed                                = &H10C0029  'BayerBG10_Packed
        PixelType_Gvsp_BayerGR12_Packed                                = &H10C002A  'BayerGR12_Packed
        PixelType_Gvsp_BayerRG12_Packed                                = &H10C002B  'BayerRG12_Packed
        PixelType_Gvsp_BayerGB12_Packed                                = &H10C002C  'BayerGB12_Packed
        PixelType_Gvsp_BayerBG12_Packed                                = &H10C002D  'BayerBG12_Packed
        PixelType_Gvsp_BayerGR16                                       = &H110002E  'BayerGR16
        PixelType_Gvsp_BayerRG16                                       = &H110002F  'BayerRG16
        PixelType_Gvsp_BayerGB16                                       = &H1100030  'BayerGB16
        PixelType_Gvsp_BayerBG16                                       = &H1100031  'BayerBG16
        PixelType_Gvsp_RGB8_Packed                                     = &H2180014  'RGB8_Packed
        PixelType_Gvsp_BGR8_Packed                                     = &H2180015  'BGR8_Packed
        PixelType_Gvsp_RGBA8_Packed                                    = &H2200016  'RGBA8_Packed
        PixelType_Gvsp_BGRA8_Packed                                    = &H2200017  'BGRA8_Packed
        PixelType_Gvsp_RGB10_Packed                                    = &H2300018  'RGB10_Packed
        PixelType_Gvsp_BGR10_Packed                                    = &H2300019  'BGR10_Packed
        PixelType_Gvsp_RGB12_Packed                                    = &H230001A  'RGB12_Packed
        PixelType_Gvsp_BGR12_Packed                                    = &H230001B  'BGR12_Packed
        PixelType_Gvsp_RGB16_Packed                                    = &H2300033  'BGR16_Packed
        PixelType_Gvsp_BGR16_Packed                                    = &H230004B  'BGR16_Packed
        PixelType_Gvsp_RGBA16_Packed                                   = &H2400040  'RGBA16_Packed
        PixelType_Gvsp_BGRA16_Packed                                   = &H2400051  'BGRA16_Packed
        PixelType_Gvsp_RGB10V1_Packed                                  = &H220001C  'RGB10V1_Packe
        PixelType_Gvsp_RGB10V2_Packed                                  = &H220001D  'RGB10V2_Packed
        PixelType_Gvsp_RGB12V1_Packed                                  = &H2240034  'RGB12V1_Packed
        PixelType_Gvsp_RGB565_Packed                                   = &H2100035  'RGB565_Packed
        PixelType_Gvsp_BGR565_Packed                                   = &H2100036  'BGR565_Packed
        PixelType_Gvsp_YUV411_Packed                                   = &H20C001E  'YUV411_Packed
        PixelType_Gvsp_YUV422_Packed                                   = &H210001F  'YUV422_Packed
        PixelType_Gvsp_YUV422_YUYV_Packed                              = &H2100032  'YUV422_YUYV_Packed
        PixelType_Gvsp_YUV444_Packed                                   = &H2180020  'YUV444_Packed
        PixelType_Gvsp_YCBCR8_CBYCR                                    = &H218003A  'YCBCR8_CBYCR
        PixelType_Gvsp_YCBCR422_8                                      = &H210003B  'YCBCR422_8
        PixelType_Gvsp_YCBCR422_8_CBYCRY                               = &H2100043  'YCBCR422_8_CBYCRY
        PixelType_Gvsp_YCBCR411_8_CBYYCRYY                             = &H20C003C  'YCBCR411_8_CBYYCRYY
        PixelType_Gvsp_YCBCR601_8_CBYCR                                = &H218003D  'YCBCR601_8_CBYCR
        PixelType_Gvsp_YCBCR601_422_8                                  = &H210003E  'YCBCR601_422_8
        PixelType_Gvsp_YCBCR601_422_8_CBYCRY                           = &H2100044  'YCBCR601_422_8_CBYCRY
        PixelType_Gvsp_YCBCR601_411_8_CBYYCRYY                         = &H20C003F  'YCBCR601_411_8_CBYYCRYY
        PixelType_Gvsp_YCBCR709_8_CBYCR                                = &H2180040  'YCBCR709_8_CBYCR
        PixelType_Gvsp_YCBCR709_422_8                                  = &H2100041  'YCBCR709_422_8
        PixelType_Gvsp_YCBCR709_422_8_CBYCRY                           = &H2100045  'YCBCR709_422_8_CBYCRY
        PixelType_Gvsp_YCBCR709_411_8_CBYYCRYY                         = &H20C0042  'YCBCR709_411_8_CBYYCRYY
        PixelType_Gvsp_YUV420SP_NV12                                   = &H20C8001  'YUV420SP_NV12
        PixelType_Gvsp_YUV420SP_NV21                                   = &H20C8002  'YUV420SP_NV21
        PixelType_Gvsp_RGB8_Planar                                     = &H2180021  'RGB8_Planar
        PixelType_Gvsp_RGB10_Planar                                    = &H2300022  'RGB10_Planar
        PixelType_Gvsp_RGB12_Planar                                    = &H2300023  'RGB12_Planar
        PixelType_Gvsp_RGB16_Planar                                    = &H2300024  'RGB16_Planar
        PixelType_Gvsp_Jpeg                                            = &H80180001  'JPEG
        PixelType_Gvsp_Coord3D_ABC32f                                  = &H26000C0   'Coord3D_ABC32f
        PixelType_Gvsp_Coord3D_ABC32f_Planar                           = &H26000C1   'Coord3D_ABC32f_Planar
        PixelType_Gvsp_Coord3D_AC32f                                   = &H024000C2  'Coord3D_AC32f
        PixelType_Gvsp_COORD3D_DEPTH_PLUS_MASK                         = &H821c0001  'COORD3D_DEPTH_PLUS_MASK
        PixelType_Gvsp_Coord3D_ABC32                                   = &H82603001  'Coord3D_ABC32
        PixelType_Gvsp_Coord3D_AB32f                                   = &H82403002  'Coord3D_AB32f
        PixelType_Gvsp_Coord3D_AB32                                    = &H82403003  'Coord3D_AB32
        PixelType_Gvsp_Coord3D_AC32f_64                                = &H024000C2  'Coord3D_AC32f_64
        PixelType_Gvsp_Coord3D_AC32f_Planar                            = &H024000C3  'Coord3D_AC32f_Planar
        PixelType_Gvsp_Coord3D_AC32                                    = &H82403004  'Coord3D_AC32
        PixelType_Gvsp_Coord3D_A32f                                    = &H12000BD    'Coord3D_A32f
        PixelType_Gvsp_Coord3D_A32                                     = &H81203005   'Coord3D_A32
        PixelType_Gvsp_Coord3D_C32f                                    = &H12000BF    'Coord3D_C32f
        PixelType_Gvsp_Coord3D_C32                                     = &H81203006   'Coord3D_C32
        PixelType_Gvsp_Coord3D_ABC16                                   = &H23000B9   'Coord3D_ABC16
        PixelType_Gvsp_Coord3D_C16                                     = &H11000B8   'Coord3D_C16
        PixelType_Gvsp_Float32                                         = &H81200001  'Float32
        PixelType_Gvsp_HB_Mono8                                        = &H81080001  'HB_Mono8
        PixelType_Gvsp_HB_Mono10                                       = &H81100003  'HB_Mono10
        PixelType_Gvsp_HB_Mono10_Packed                                = &H810c0004  'HB_Mono10_Packed
        PixelType_Gvsp_HB_Mono12                                       = &H81100005  'HB_Mono12
        PixelType_Gvsp_HB_Mono12_Packed                                = &H810c0006  'HB_Mono12_Packed
        PixelType_Gvsp_HB_Mono16                                       = &H81100007  'HB_Mono16
        PixelType_Gvsp_HB_BayerGR8                                     = &H81080008  'HB_BayerGR8
        PixelType_Gvsp_HB_BayerRG8                                     = &H81080009  'HB_BayerRG8
        PixelType_Gvsp_HB_BayerGB8                                     = &H8108000A  'HB_BayerGB8
        PixelType_Gvsp_HB_BayerBG8                                     = &H8108000B  'HB_BayerBG8
        PixelType_Gvsp_HB_BayerRBGG8                                   = &H81080046  'HB_BayerRBGG8
        PixelType_Gvsp_HB_BayerGR10                                    = &H8110000C  'HB_BayerGR10
        PixelType_Gvsp_HB_BayerRG10                                    = &H8110000D  'HB_BayerRG10
        PixelType_Gvsp_HB_BayerGB10                                    = &H8110000E   'HB_BayerGB10
        PixelType_Gvsp_HB_BayerBG10                                    = &H8110000F   'HB_BayerBG10
        PixelType_Gvsp_HB_BayerGR12                                    = &H81100010   'HB_BayerGR12
        PixelType_Gvsp_HB_BayerRG12                                    = &H81100011   'HB_BayerRG12
        PixelType_Gvsp_HB_BayerGB12                                    = &H81100012   'HB_BayerGB12
        PixelType_Gvsp_HB_BayerBG12                                    = &H81100013   'HB_BayerBG12
        PixelType_Gvsp_HB_BayerGR10_Packed                             = &H810c0026  'HB_BayerGR10_Packed
        PixelType_Gvsp_HB_BayerRG10_Packed                             = &H810c0027  'HB_BayerRG10_Packed
        PixelType_Gvsp_HB_BayerGB10_Packed                             = &H810c0028  'HB_BayerGB10_Packed
        PixelType_Gvsp_HB_BayerBG10_Packed                             = &H810c0029  'HB_BayerBG10_Packed
        PixelType_Gvsp_HB_BayerGR12_Packed                             = &H810c002A  'HB_BayerGR12_Packed
        PixelType_Gvsp_HB_BayerRG12_Packed                             = &H810c002B  'HB_BayerRG12_Packed
        PixelType_Gvsp_HB_BayerGB12_Packed                             = &H810c002C  'HB_BayerGB12_Packed
        PixelType_Gvsp_HB_BayerBG12_Packed                             = &H810c002D  'HB_BayerBG12_Packed
        PixelType_Gvsp_HB_YUV422_Packed                                = &H8210001F  'HB_YUV422_Packed
        PixelType_Gvsp_HB_YUV422_YUYV_Packed                           = &H82100032  'HB_YUV422_YUYV_Packed
        PixelType_Gvsp_HB_RGB8_Packed                                  = &H82180014  'HB_RGB8_Packed
        PixelType_Gvsp_HB_BGR8_Packed                                  = &H82180015  'HB_BGR8_Packed
        PixelType_Gvsp_HB_RGBA8_Packed                                 = &H82200016  'HB_RGBA8_Packed
        PixelType_Gvsp_HB_BGRA8_Packed                                 = &H82200017  'HB_BGRA8_Packed
        PixelType_Gvsp_HB_RGB16_Packed                                 = &H82300033  'HB_RGB16_Packed
        PixelType_Gvsp_HB_BGR16_Packed                                 = &H8230004B  'HB_BGR16_Packed
        PixelType_Gvsp_HB_RGBA16_Packed                                = &H82400064  'HB_RGBA16_Packed
        PixelType_Gvsp_HB_BGRA16_Packed                                = &H82400051  'HB_BGRA16_Packed
    End Enum
    #End Region

    #Region "类构造函数接口"
    ''' <summary>
    ''' 构造函数
    ''' </summary>
    ''' <remarks></remarks>
    Public Sub New()
        m_devHandle = IntPtr.Zero
    End Sub

    ''' <summary>
    ''' 析构函数
    ''' </summary>
    ''' <remarks></remarks>
    Protected Overrides Sub Finalize()
    End Sub

    ''' <summary>
    ''' 返回相机设备句柄
    ''' </summary>
    ''' <Returns>返回相机设备句柄</Returns>
    ''' <remarks></remarks>
    Public Function GetCameraHandle() As IntPtr
        Return m_devHandle
    End Function
    #End Region
    
    #Region "委托声明"
    ''' <summary>
    ''' Grab callback
    ''' </summary>
    ''' <param name="pData">Image data</param>
    ''' <param name="stFrameInfo">Frame info</param>
    ''' <param name="pUser">User defined variable</param>
    ''' <remarks></remarks>
    Public Delegate Sub cbOutputdelegate(ByVal pData As IntPtr, ByRef stFrameInfo As MV_FRAME_OUT, ByVal pUser As IntPtr)

    ''' <summary>
    ''' Grab callback
    ''' </summary>
    ''' <param name="pData">Image data</param>
    ''' <param name="stFrameInfoEx">Frame info</param>
    ''' <param name="pUser">User defined variable</param>
    Public Delegate Sub cbOutputExdelegate(ByVal pData As IntPtr, ByRef stFrameInfoEx As MV_FRAME_OUT_INFO_EX, ByVal pUser As IntPtr)

    ''' <summary>
    ''' Exception callback
    ''' </summary>
    ''' <param name="nMsgType">Msg type</param>
    ''' <param name="pUser">User defined variable</param>
    Public Delegate Sub cbExceptiondelegate(ByVal nMsgType As UInteger, ByVal pUser As IntPtr)

    ''' <summary>
    ''' Event callback (Interfaces not recommended)
    ''' </summary>
    ''' <param name="nUserDefinedId">User defined ID</param>
    ''' <param name="pUser">User defined variable</param>
    Public Delegate Sub cbEventdelegate(ByVal nUserDefinedId As UInteger, ByVal pUser As IntPtr)

    ''' <summary>
    ''' Event callback
    ''' </summary>
    ''' <param name="pEventInfo">Event Info</param>
    ''' <param name="pUser">User defined variable</param>
    Public Delegate Sub cbEventdelegateEx(ByRef pEventInfo As MV_EVENT_OUT_INFO, ByVal pUser As IntPtr)

    ''' <summary>
    ''' Stream Exception callback
    ''' </summary>
    ''' <param name="enExceptionType">Msg type</param>
    ''' <param name="pUser">User defined variable</param>
    Public Delegate Sub cbStreamException(ByVal enExceptionType As MV_CC_STREAM_EXCEPTION_TYPE, ByVal pUser As IntPtr)
    #End Region

    #Region "相机的基本指令和操作接口"
    ''' <summary>
    ''' Get SDK Version
    ''' </summary>
    ''' <Returns>Always Return 4 Bytes of version number |Main  |Sub   |Rev   |Test|
    '''                                                   8bits  8bits  8bits  8bits 
    ''' </Returns>
    Public Shared Function GetSDKVersion() As Integer
        Return MV_CC_GetSDKVersion()
    End Function

    ''' <summary>
    ''' Get supported Transport Layer
    ''' </summary>
    ''' <Returns>Supported Transport Layer number</Returns>
    Public Shared Function EnumerateTls() As Integer
        Return MV_CC_EnumerateTls()
    End Function

    ''' <summary>
    ''' Enumerate Device
    ''' </summary>
    ''' <param name="nTLayerType">Enumerate TLs</param>
    ''' <param name="stDevList">Device List</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Shared Function EnumDevices(ByVal nTLayerType As Integer, ByRef stDevList As MV_CC_DEVICE_INFO_LIST) As Integer
        Return MV_CC_EnumDevices(nTLayerType, stDevList)
    End Function

    ''' <summary>
    ''' Enumerate device according to manufacture name
    ''' </summary>
    ''' <param name="nTLayerType">Enumerate TLs</param>
    ''' <param name="stDevList">Device List</param>
    ''' <param name="pManufacturerName">Manufacture Name</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Shared Function EnumDevicesEx(ByVal nTLayerType As Integer, ByRef stDevList As MV_CC_DEVICE_INFO_LIST, ByVal pManufacturerName As String) As Integer
        Return MV_CC_EnumDevicesEx(nTLayerType,  stDevList, pManufacturerName)
    End Function

    ''' <summary>
    ''' Enumerate device according to the specified ordering
    ''' </summary>
    ''' <param name="nTLayerType">Transmission layer of enumeration(All layer protocol type can input)</param>
    ''' <param name="stDevList">Device list</param>
    ''' <param name="pManufacturerName">Manufacture Name</param>
    ''' <param name="enSortMethod">Sorting Method</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Shared Function EnumDevicesEx2(ByVal nTLayerType As Integer, ByRef stDevList As MV_CC_DEVICE_INFO_LIST, ByVal pManufacturerName As String, ByVal enSortMethod As MV_SORT_METHOD) As Integer
        Return MV_CC_EnumDevicesEx2(nTLayerType, stDevList, pManufacturerName, enSortMethod)
    End Function

    ''' <summary>
    ''' Is the device accessible
    ''' </summary>
    ''' <param name="stDevInfo">Device Information</param>
    ''' <param name="nAccessMode">Access Right</param>
    ''' <Returns>Access, Return true. Not access, Return false</Returns>
    Public Shared Function IsDeviceAccessible(ByRef stDevInfo As MV_CC_DEVICE_INFO,ByVal nAccessMode As UInteger) As Boolean
        Return MV_CC_IsDeviceAccessible(stDevInfo, nAccessMode)
    End Function

    ''' <summary>
    ''' Create Device
    ''' </summary>
    ''' <param name="stDevInfo">Device Information</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function CreateDevice(ByRef stDevInfo As MV_CC_DEVICE_INFO) As Integer
        If IntPtr.Zero <> m_devHandle Then
            MV_CC_DestroyHandle(m_devHandle)
            m_devHandle = IntPtr.Zero
        End If

        Return MV_CC_CreateHandle(m_devHandle, stDevInfo)
    End Function

    ''' <summary>
    ''' Create Device without log
    ''' </summary>
    ''' <param name="stDevInfo">Device Information</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function CreateDeviceWithoutLog(ByRef stDevInfo As MV_CC_DEVICE_INFO) As Integer
        If IntPtr.Zero <> m_devHandle Then
            MV_CC_DestroyHandle(m_devHandle)
            m_devHandle = IntPtr.Zero
        End If

        Return MV_CC_CreateHandleWithoutLog(m_devHandle,  stDevInfo)
    End Function

    ''' <summary>
    ''' Destroy Device
    ''' </summary>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function DestroyDevice() As Integer
        Dim nRet As Integer = MV_OK
        If IntPtr.Zero <> m_devHandle Then
            nRet = MV_CC_DestroyHandle(m_devHandle)
            m_devHandle = IntPtr.Zero
        End If
        Return nRet
    End Function

    ''' <summary>
    ''' Open Device
    ''' </summary>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function OpenDevice() As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_OpenDevice(m_devHandle, 1, 0)
    End Function

    ''' <summary>
    ''' Open Device
    ''' </summary>
    ''' <param name="nAccessMode">Access Right</param>
    ''' <param name="nSwitchoverKey">Switch key of access right</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function OpenDevice(ByVal nAccessMode As UInteger, ByVal nSwitchoverKey As UShort) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_OpenDevice(m_devHandle, nAccessMode, nSwitchoverKey)
    End Function

    ''' <summary>
    ''' Close Device
    ''' </summary>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function CloseDevice() As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_CloseDevice(m_devHandle)
    End Function

    ''' <summary>
    ''' Is the device connected
    ''' </summary>
    ''' <Returns>Connected, Return true. Not Connected or DIsconnected, Return false</Returns>
    public Function MV_CC_IsDeviceConnected() As Boolean
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_IsDeviceConnected(m_devHandle)
    End Function

    ''' <summary>
    ''' Register the image callback function
    ''' </summary>
    ''' <param name="cbOutput">Callback function pointer</param>
    ''' <param name="pUser">User defined variable</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function RegisterImageCallBackEx(ByVal cbOutput As cbOutputExdelegate, ByVal pUser As IntPtr) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_RegisterImageCallBackEx(m_devHandle, cbOutput, pUser)
    End Function

    ''' <summary>
    ''' Register the RGB image callback function
    ''' </summary>
    ''' <param name="cbOutput">Callback function pointer</param>
    ''' <param name="pUser">User defined variable</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function RegisterImageCallBackForRGB(ByVal cbOutput As cbOutputExdelegate, ByVal pUser As IntPtr) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_RegisterImageCallBackForRGB(m_devHandle, cbOutput, pUser)
    End Function

    ''' <summary>
    ''' Register the BGR image callback function
    ''' </summary>
    ''' <param name="cbOutput">Callback function pointer</param>
    ''' <param name="pUser">User defined variable</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function RegisterImageCallBackForBGR(ByVal cbOutput As cbOutputExdelegate, ByVal pUser As IntPtr) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_RegisterImageCallBackForBGR(m_devHandle, cbOutput, pUser)
    End Function

    ''' <summary>
    ''' Start Grabbing
    ''' </summary>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function StartGrabbing() As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_StartGrabbing(m_devHandle)
    End Function

    ''' <summary>
    ''' Stop Grabbing
    ''' </summary>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function StopGrabbing() As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_StopGrabbing(m_devHandle)
    End Function

    ''' <summary>
    ''' Get one frame of RGB image, this function is using query to get data
    ''' query whether the internal cache has data, get data if there has, Return error code if no data
    ''' </summary>
    ''' <param name="pData">Image data receiving buffer</param>
    ''' <param name="nDataSize">Buffer size</param>
    ''' <param name="pFrameInfo">Image information</param>
    ''' <param name="nMsec">Waiting timeout</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function GetImageForRGB(ByVal pData As IntPtr, ByVal nDataSize As UInteger, ByRef pFrameInfo As MV_FRAME_OUT_INFO_EX, ByVal nMsec As Integer) As  Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_GetImageForRGB(m_devHandle, pData, nDataSize,  pFrameInfo, nMsec)
    End Function

    ''' <summary>
    ''' Get one frame of BGR image, this function is using query to get data
    ''' query whether the internal cache has data, get data if there has, Return error code if no data
    ''' </summary>
    ''' <param name="pData">Image data receiving buffer</param>
    ''' <param name="nDataSize">Buffer size</param>
    ''' <param name="pFrameInfo">Image information</param>
    ''' <param name="nMsec">Waiting timeout</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error cod</Returns>
    Public Function GetImageForBGR(ByVal pData As IntPtr, ByVal nDataSize As UInt32, ByRef pFrameInfo As MV_FRAME_OUT_INFO_EX, ByVal nMsec As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_GetImageForBGR(m_devHandle, pData, nDataSize,  pFrameInfo, nMsec)
    End Function

    ''' <summary>
    ''' Get a frame of an image using an internal cache
    ''' </summary>
    ''' <param name="pFrame">Image data and image information</param>
    ''' <param name="nMsec">Waiting timeout</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function GetImageBuffer(ByRef pFrame As MV_FRAME_OUT, ByVal nMsec As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_GetImageBuffer(m_devHandle,  pFrame, nMsec)
    End Function

    ''' <summary>
    ''' Free image buffer（used with MV_CC_GetImageBuffer）
    ''' </summary>
    ''' <param name="pFrame">Image data and image information</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function FreeImageBuffer(ByRef pFrame As MV_FRAME_OUT) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_FreeImageBuffer(m_devHandle,  pFrame)
    End Function

    ''' <summary>
    ''' Get a frame of an image
    ''' </summary>
    ''' <param name="pData">Image data receiving buffer</param>
    ''' <param name="nDataSize">Buffer size</param>
    ''' <param name="pFrameInfo">Image information</param>
    ''' <param name="nMsec">Waiting timeout</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function GetOneFrameTimeout(ByVal pData As IntPtr, ByVal nDataSize As UInteger, ByRef pFrameInfo As MV_FRAME_OUT_INFO_EX, ByVal nMsec As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_GetOneFrameTimeout(m_devHandle, pData, nDataSize,  pFrameInfo, nMsec)
    End Function

    ''' <summary>
    ''' Clear image Buffers to clear old data
    ''' </summary>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function ClearImageBuffer() As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_ClearImageBuffer(m_devHandle)
    End Function

    ''' <summary>
    ''' Get the number of valid images in the current image buffer
    ''' </summary>
    ''' <param name="pnValidImageNum">The number of valid images in the current image buffer</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function GetValidImageNum(ByVal pnValidImageNum As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_GetValidImageNum(m_devHandle,  pnValidImageNum)
    End Function

    ''' <summary>
    ''' Display one frame image
    ''' </summary>
    ''' <param name="pDisplayInfo">Image information</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function DisplayOneFrame(ByRef pDisplayInfo As MV_DISPLAY_FRAME_INFO) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_DisplayOneFrame(m_devHandle,  pDisplayInfo)
    End Function
	
	''' <summary>
    ''' Display one frame image
    ''' </summary>
    ''' <param name="pDisplayInfoEx">Image information</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
	Public Function DisplayOneFrameEx(ByVal hWnd As IntPtr, ByRef pDisplayInfoEx As MV_DISPLAY_FRAME_INFO_EX) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_DisplayOneFrameEx(m_devHandle, hWnd, pDisplayInfoEx)
    End Function
	
    ''' <summary>
    ''' Set the number of the internal image cache nodes in SDK(Greater than or equal to 1, to be called before the capture)
    ''' </summary>
    ''' <param name="nNum">Number of cache nodes</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function SetImageNodeNum(ByVal nNum As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_SetImageNodeNum(m_devHandle, nNum)
    End Function

    ''' <summary>
    ''' Set Grab Strategy
    ''' </summary>
    ''' <param name="enGrabStrategy">The value of grab strategy</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function SetGrabStrategy(ByVal enGrabStrategy As MV_GRAB_STRATEGY) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_SetGrabStrategy(m_devHandle, enGrabStrategy)
    End Function

    ''' <summary>
    ''' Set The Size of Output Queue(Only work under the strategy of MV_GrabStrategy_LatestImages，rang：1-ImageNodeNum)
    ''' </summary>
    ''' <param name="nOutputQueueSize">The Size of Output Queue</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function SetOutputQueueSize(ByVal nOutputQueueSize As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_SetOutputQueueSize(m_devHandle, nOutputQueueSize)
    End Function

    ''' <summary>
    ''' Get device information(Called before start grabbing)
    ''' </summary>
    ''' <param name="stDevInfo">device information</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function GetDeviceInfo(ByRef stDevInfo As MV_CC_DEVICE_INFO) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_GetDeviceInfo(m_devHandle,  stDevInfo)
    End Function

    ''' <summary>
    ''' Get various type of information
    ''' </summary>
    ''' <param name="stInfo">Various type of information</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function GetAllMatchInfo(ByRef stInfo As MV_ALL_MATCH_INFO) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_GetAllMatchInfo(m_devHandle, stInfo)
    End Function
    #End Region

    #Region "设置和获取相机参数的万能接口"
    ''' <summary>
    ''' Get Integer value
    ''' </summary>
    ''' <param name="strKey">Key value, for example, using "Width" to get width</param>
    ''' <param name="stValue">Value of device features</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function GetIntValueEx(ByVal strKey As String, ByRef stValue As MVCC_INTVALUE_EX) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_GetIntValueEx(m_devHandle, strKey,  stValue)
    End Function

    ''' <summary>
    ''' Set Integer value
    ''' </summary>
    ''' <param name="strKey">Key value, for example, using "Width" to set width</param>
    ''' <param name="nValue">Feature value to set</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function SetIntValueEx(ByVal strKey As String, ByVal nValue As Long) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_SetIntValueEx(m_devHandle, strKey, nValue)
    End Function

    ''' <summary>
    ''' Get Enum value
    ''' </summary>
    ''' <param name="strKey">Key value, for example, using "PixelFormat" to get pixel format</param>
    ''' <param name="stValue">Value of device features</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function GetEnumValue(ByVal strKey As String, ByRef stValue As MVCC_ENUMVALUE) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_GetEnumValue(m_devHandle, strKey,  stValue)
    End Function

    ''' <summary>
    ''' Set Enum value
    ''' </summary>
    ''' <param name="strKey">Key value, for example, using "PixelFormat" to set pixel format</param>
    ''' <param name="nValue">Feature value to set</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function SetEnumValue(ByVal strKey As String, ByVal nValue As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_SetEnumValue(m_devHandle, strKey, nValue)
    End Function

    ''' <summary>
    ''' Get the symbolic of the specified value of the Enum type node
    ''' </summary>
    ''' <param name="strKey">Key value, for example, using "PixelFormat" to set pixel format</param>
    ''' <param name="stEnumEntry">Symbolic to get</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function GetEnumEntrySymbolic(ByVal strKey As String, ByRef stEnumEntry As MVCC_ENUMENTRY) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_GetEnumEntrySymbolic(m_devHandle, strKey, stEnumEntry)
    End Function

    ''' <summary>
    ''' Set Enum value
    ''' </summary>
    ''' <param name="strKey">Key value, for example, using "PixelFormat" to set pixel format</param>
    ''' <param name="strValue">Feature String to set</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function SetEnumValueByString(ByVal strKey As String, ByVal strValue As String) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_SetEnumValueByString(m_devHandle, strKey, strValue)
    End Function

    ''' <summary>
    ''' Get Float value
    ''' </summary>
    ''' <param name="strKey">Key value</param>
    ''' <param name="stValue">Value of device features</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function GetFloatValue(ByVal strKey As String, ByRef stValue As MVCC_FLOATVALUE) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_GetFloatValue(m_devHandle, strKey, stValue)
    End Function

    ''' <summary>
    ''' Set float value
    ''' </summary>
    ''' <param name="strKey">Key value</param>
    ''' <param name="fValue">Feature value to set</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function SetFloatValue(ByVal strKey As String, ByVal fValue As Single) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_SetFloatValue(m_devHandle, strKey, fValue)
    End Function

    ''' <summary>
    ''' Get Boolean value
    ''' </summary>
    ''' <param name="strKey">Key value</param>
    ''' <param name="pbValue">Value of device features</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function GetBoolValue(ByVal strKey As String, ByRef pbValue As Boolean) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_GetBoolValue(m_devHandle, strKey, pbValue)
    End Function

    ''' <summary>
    ''' Set Boolean value
    ''' </summary>
    ''' <param name="strKey">Key value</param>
    ''' <param name="bValue">Feature value to set</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function SetBoolValue(ByVal strKey As String, ByVal bValue As Boolean) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_SetBoolValue(m_devHandle, strKey, bValue)
    End Function

    ''' <summary>
    ''' Get String value
    ''' </summary>
    ''' <param name="strKey">Key value</param>
    ''' <param name="stValue">Value of device features</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function GetStringValue(ByVal strKey As String, ByRef stValue As MVCC_STRINGVALUE) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_GetStringValue(m_devHandle, strKey, stValue)
    End Function

    ''' <summary>
    ''' Set String value
    ''' </summary>
    ''' <param name="strKey">Key value</param>
    ''' <param name="strValue">Feature value to set</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function SetStringValue(ByVal strKey As String, ByVal strValue As String) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_SetStringValue(m_devHandle, strKey, strValue)
    End Function

    ''' <summary>
    ''' Send Command
    ''' </summary>
    ''' <param name="strKey">Key value</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function SetCommandValue(ByVal strKey As String) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_SetCommandValue(m_devHandle, strKey)
    End Function

    ''' <summary>
    ''' Invalidate GenICam Nodes
    ''' </summary>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function InvalidateNodes() As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_InvalidateNodes(m_devHandle)
    End Function

    #End Region

    #Region "设备升级 和 寄存器读写 和异常、事件回调"
    ''' <summary>
    ''' Device Local Upgrade
    ''' </summary>
    ''' <param name="strFilePathName">File path and name</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function LocalUpgrade(ByVal strFilePathName As String) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_LocalUpgrade(m_devHandle, strFilePathName)
    End Function

    ''' <summary>
    ''' Get Upgrade Progress
    ''' </summary>
    ''' <param name="pnProcess">Value of Progress</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function GetUpgradeProcess(ByRef pnProcess As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_GetUpgradeProcess(m_devHandle, pnProcess)
    End Function

    ''' <summary>
    ''' Read Memory
    ''' </summary>
    ''' <param name="pBuffer">Used as a Return value, save the read-in memory value(Memory value is stored in accordance with the big end model)</param>
    ''' <param name="nAddress">Memory address to be read, which can be obtained from the Camera.xml file of the device, the form xml node value of xxx_RegAddr</param>
    ''' <param name="nLength">Length of the memory to be read</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function ReadMemory(ByVal pBuffer As IntPtr, ByVal nAddress As Long, ByVal nLength As Long) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_ReadMemory(m_devHandle, pBuffer, nAddress, nLength)
    End Function

    ''' <summary>
    ''' Write Memory
    ''' </summary>
    ''' <param name="pBuffer">Memory value to be written ( Note the memory value to be stored in accordance with the big end model)</param>
    ''' <param name="nAddress">Memory address to be written, which can be obtained from the Camera.xml file of the device, the form xml node value of xxx_RegAddr</param>
    ''' <param name="nLength">Length of the memory to be written</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function WriteMemory(ByVal pBuffer As IntPtr, ByVal nAddress As Long, ByVal nLength As Long) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_WriteMemory(m_devHandle, pBuffer, nAddress, nLength)
    End Function

    ''' <summary>
    ''' Register Exception Message CallBack, call after open device
    ''' </summary>
    ''' <param name="cbException">Exception Message CallBack Function</param>
    ''' <param name="pUser">User defined variable</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function RegisterExceptionCallBack(ByVal cbException As cbExceptiondelegate, ByVal pUser As IntPtr) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_RegisterExceptionCallBack(m_devHandle, cbException, pUser)
    End Function

    ''' <summary>
    ''' Register event callback, which is called after the device is opened
    ''' </summary>
    ''' <param name="cbEvent">Event CallBack Function</param>
    ''' <param name="pUser">User defined variable</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function RegisterAllEventCallBack(ByVal cbEvent As cbEventdelegateEx, ByVal pUser As IntPtr) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_RegisterAllEventCallBack(m_devHandle, cbEvent, pUser)
    End Function

    ''' <summary>
    ''' Register single event callback, which is called after the device is opened
    ''' </summary>
    ''' <param name="strEventName">Event name</param>
    ''' <param name="cbEvent">Event CallBack Function</param>
    ''' <param name="pUser">User defined variable</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function RegisterEventCallBackEx(ByVal strEventName As String, ByVal cbEvent As cbEventdelegateEx, ByVal pUser As IntPtr) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_RegisterEventCallBackEx(m_devHandle, strEventName, cbEvent, pUser)
    End Function
    #End Region

    #Region "GigEVision 设备独有的接口"
    ''' <summary>
    ''' Set gige enum devices Time out
    ''' </summary>
    ''' <param name="nMilTimeout">Time out(ms)</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Shared Function GIGE_SetEnumDevTimeout(ByVal nMilTimeout As UInteger) As Integer
        Return MV_GIGE_SetEnumDevTimeout(nMilTimeout)
    End Function

    ''' <summary>
    ''' Force IP
    ''' </summary>
    ''' <param name="nIP">IP to set</param>
    ''' <param name="nSubNetMask">Subnet mask</param>
    ''' <param name="nDefaultGateWay">Default gateway</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function GIGE_ForceIpEx(ByVal nIP As UInteger, ByVal nSubNetMask As UInteger, ByVal nDefaultGateWay As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_GIGE_ForceIpEx(m_devHandle, nIP, nSubNetMask, nDefaultGateWay)
    End Function

    ''' <summary>
    ''' IP configuration method
    ''' </summary>
    ''' <param name="nType">IP type, refer to MV_IP_CFG_x</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function GIGE_SetIpConfig(ByVal nType As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_GIGE_SetIpConfig(m_devHandle, nType)
    End Function

    ''' <summary>
    ''' Set to use only one mode,type: MV_NET_TRANS_x. When do not set, priority is to use driver by default
    ''' </summary>
    ''' <param name="nType">Net transmission mode, refer to MV_NET_TRANS_x</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function GIGE_SetNetTransMode(ByVal nType As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_GIGE_SetNetTransMode(m_devHandle, nType)
    End Function

    ''' <summary>
    ''' Get net transmission information
    ''' </summary>
    ''' <param name="stInfo">Transmission information</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function GIGE_GetNetTransInfo(ByRef stInfo As MV_NETTRANS_INFO) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_GIGE_GetNetTransInfo(m_devHandle, stInfo)
    End Function

    ''' <summary>
    ''' Setting the ACK mode of devices Discovery
    ''' </summary>
    ''' <param name="nMode">ACK mode（Default-Broadcast）,0-Unicast,1-Broadcast</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Shared Function GIGE_SetDiscoveryMode(ByVal nMode As UInteger) As Integer
        Return MV_GIGE_SetDiscoveryMode(nMode)
    End Function

    ''' <summary>
    ''' Set GVSP streaming timeout
    ''' </summary>
    ''' <param name="nMillisec">Timeout, default 300ms, range: >10ms</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function GIGE_SetGvspTimeout(ByVal nMillisec As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_GIGE_SetGvspTimeout(m_devHandle, nMillisec)
    End Function

    ''' <summary>
    ''' Get GVSP streaming timeout
    ''' </summary>
    ''' <param name="pMillisec">Timeout, ms as unit</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function GIGE_GetGvspTimeout(ByRef pMillisec As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_GIGE_GetGvspTimeout(m_devHandle, pMillisec)
    End Function

    ''' <summary>
    ''' Set GVCP cammand timeout
    ''' </summary>
    ''' <param name="nMillisec">Timeout, ms as unit, range: 0-10000</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function GIGE_SetGvcpTimeout(ByVal nMillisec As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_GIGE_SetGvcpTimeout(m_devHandle, nMillisec)
    End Function

    ''' <summary>
    ''' Get GVCP cammand timeout
    ''' </summary>
    ''' <param name="pMillisec">Timeout, ms as unit</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function GIGE_GetGvcpTimeout(ByRef pMillisec As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_GIGE_GetGvcpTimeout(m_devHandle, pMillisec)
    End Function

    ''' <summary>
    ''' Set the number of retry GVCP cammand
    ''' </summary>
    ''' <param name="nRetryGvcpTimes">The number of retries，rang：0-100</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function GIGE_SetRetryGvcpTimes(ByVal nRetryGvcpTimes As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_GIGE_SetRetryGvcpTimes(m_devHandle, nRetryGvcpTimes)
    End Function

    ''' <summary>
    ''' Get the number of retry GVCP cammand
    ''' </summary>
    ''' <param name="pRetryGvcpTimes">The number of retries</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function GIGE_GetRetryGvcpTimes(ByRef pRetryGvcpTimes As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_GIGE_GetRetryGvcpTimes(m_devHandle, pRetryGvcpTimes)
    End Function

    ''' <summary>
    ''' Get the optimal Packet Size, Only support GigE Camera
    ''' </summary>
    ''' <Returns>Optimal packet size</Returns>
    Public Function GIGE_GetOptimalPacketSize() As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_GetOptimalPacketSize(m_devHandle)
    End Function

    ''' <summary>
    ''' Set whethe to enable resend, and set resend
    ''' </summary>
    ''' <param name="bEnable">Enable resend</param>
    ''' <param name="nMaxResendPercent">Max resend persent</param>
    ''' <param name="nResendTimeout">Resend timeout</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function GIGE_SetResend(ByVal bEnable As UInteger, ByVal nMaxResendPercent As UInteger, ByVal nResendTimeout As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_GIGE_SetResend(m_devHandle, bEnable, nMaxResendPercent, nResendTimeout)
    End Function

    ''' <summary>
    ''' Set the max resend retry times
    ''' </summary>
    ''' <param name="nRetryTimes">The max times to retry resending lost packets，default 20</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function GIGE_SetResendMaxRetryTimes(ByVal nRetryTimes As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_GIGE_SetResendMaxRetryTimes(m_devHandle, nRetryTimes)
    End Function

    ''' <summary>
    ''' Get the max resend retry times
    ''' </summary>
    ''' <param name="pnRetryTimes">the max times to retry resending lost packets</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function GIGE_GetResendMaxRetryTimes(ByRef pnRetryTimes As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_GIGE_GetResendMaxRetryTimes(m_devHandle, pnRetryTimes)
    End Function

    ''' <summary>
    ''' Set time interval between same resend requests
    ''' </summary>
    ''' <param name="nMillisec">The time interval between same resend requests,default 10ms</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function GIGE_SetResendTimeInterval(ByVal nMillisec As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_GIGE_SetResendTimeInterval(m_devHandle, nMillisec)
    End Function

    ''' <summary>
    ''' Get time interval between same resend requests
    ''' </summary>
    ''' <param name="pnMillisec">The time interval between same resend requests</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function GIGE_GetResendTimeInterval(ByRef pnMillisec As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_GIGE_GetResendTimeInterval(m_devHandle, pnMillisec)
    End Function

    ''' <summary>
    ''' Set transmission type,Unicast or Multicast
    ''' </summary>
    ''' <param name="stTransmissionType">Struct of transmission type</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function GIGE_SetTransmissionType(ByRef stTransmissionType As MV_TRANSMISSION_TYPE) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_GIGE_SetTransmissionType(m_devHandle, stTransmissionType)
    End Function

    ''' <summary>
    ''' Issue Action Command
    ''' </summary>
    ''' <param name="stActionCmdInfo">Action Command info</param>
    ''' <param name="stActionCmdResults">Action Command Result List</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function GIGE_IssueActionCommand(ByRef stActionCmdInfo As MV_ACTION_CMD_INFO, ByRef stActionCmdResults As MV_ACTION_CMD_RESULT_LIST) As Integer
        Return MV_GIGE_IssueActionCommand(stActionCmdInfo, stActionCmdResults)
    End Function

    ''' <summary>
    ''' Get Multicast Status
    ''' </summary>
    ''' <param name="stDevInfo">Device Information</param>
    ''' <param name="pStatus">Status of Multicast</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Shared Function GIGE_GetMulticastStatus(ByRef stDevInfo As MV_CC_DEVICE_INFO, ByRef pStatus As Boolean) As Integer
        Return MV_GIGE_GetMulticastStatus(stDevInfo, pStatus)
    End Function
    #End Region

    #Region "CameraLink独有的接口"
    ''' <summary>
    ''' Set device baudrate using one of the CL_BAUDRATE_XXXX value
    ''' </summary>
    ''' <param name="nBaudrate">Baudrate to set. Refer to the 'CameraParams.h' for parameter definitions, for example, #define MV_CAML_BAUDRATE_9600  0x00000001</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function CAML_SetDeviceBauderate(ByVal nBaudrate As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CAML_SetDeviceBaudrate(m_devHandle, nBaudrate)
    End Function

    ''' <summary>
    ''' Get device baudrate, using one of the CL_BAUDRATE_XXXX value
    ''' </summary>
    ''' <param name="pnCurrentBaudrate">Return pointer of baud rate to user. 
    '''                                 Refer to the 'CameraParams.h' for parameter definitions, for example, #define MV_CAML_BAUDRATE_9600  0x00000001</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function CAML_GetDeviceBauderate(ByRef pnCurrentBaudrate As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CAML_GetDeviceBaudrate(m_devHandle, pnCurrentBaudrate)
    End Function

    ''' <summary>
    ''' Get supported baudrates of the combined device and host interface
    ''' </summary>
    ''' <param name="pnBaudrateAblity">Return pointer of the supported baudrates to user. 'OR' operation results of the supported baudrates. 
    '''                                Refer to the 'CameraParams.h' for single value definitions, for example, #define MV_CAML_BAUDRATE_9600  0x00000001</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function CAML_GetSupportBaudrates(ByRef pnBaudrateAblity As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CAML_GetSupportBaudrates(m_devHandle, pnBaudrateAblity)
    End Function

    Public Function CAML_GetSupportBauderates(ByRef pnBaudrateAblity As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CAML_GetSupportBaudrates(m_devHandle, pnBaudrateAblity)
    End Function

    ''' <summary>
    ''' Sets the timeout for operations on the serial port
    ''' </summary>
    ''' <param name="nMillisec">Timeout in [ms] for operations on the serial port.</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function CAML_SetGenCPTimeOut(ByVal nMillisec As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CAML_SetGenCPTimeOut(m_devHandle, nMillisec)
    End Function
    #End Region

    #Region "U3V独有的接口"
    ''' <summary>
    ''' Set transfer size of U3V device
    ''' </summary>
    ''' <param name="nTransferSize">Transfer size，Byte，default：1M，rang：>=0x10000</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function USB_SetTransferSize(ByRef nTransferSize As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_USB_SetTransferSize(m_devHandle, nTransferSize)
    End Function

    ''' <summary>
    ''' Get transfer size of U3V device
    ''' </summary>
    ''' <param name="pTransferSize">Transfer size，Byte</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function USB_GetTransferSize(ByRef pTransferSize As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_USB_GetTransferSize(m_devHandle, pTransferSize)
    End Function

    ''' <summary>
    ''' Set transfer ways of U3V device
    ''' </summary>
    ''' <param name="nTransferWays">Transfer ways，rang：1-10</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function USB_SetTransferWays(ByVal nTransferWays As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_USB_SetTransferWays(m_devHandle, nTransferWays)
    End Function

    ''' <summary>
    ''' Get transfer ways of U3V device
    ''' </summary>
    ''' <param name="pTransferWays">Transfer ways</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function USB_GetTransferWays(ByRef pTransferWays As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_USB_GetTransferWays(m_devHandle, pTransferWays)
    End Function

    ''' <summary>
    ''' Register Stream Exception Message CallBack
    ''' </summary>
    ''' <param name="cbException">Stream Exception Message CallBack Function</param>
    ''' <param name="pUser">User defined variable</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function USB_RegisterStreamExceptionCallBack(ByVal cbException As cbStreamException, ByVal pUser As Intptr) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_USB_RegisterStreamExceptionCallBack(m_devHandle, cbException, pUser)
    End Function

    ''' <summary>
    ''' Set the number of U3V device event cache nodes
    ''' </summary>
    ''' <param name="nEventNodeNum">Event Node Number</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function USB_SetEventNodeNum(ByVal nEventNodeNum As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_USB_SetEventNodeNum(m_devHandle, nEventNodeNum)
    End Function

    ''' <summary>
    ''' Set U3V Camera Synchronisation timeout
    ''' </summary>
    ''' <param name="nMills">Synchronisation time(ms), default 1000ms</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function USB_SetSyncTimeOut(ByVal nMills As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_USB_SetSyncTimeOut(m_devHandle, nMills)
    End Function

    ''' <summary>
    ''' Get U3V Camera Synchronisation timeout
    ''' </summary>
    ''' <param name="pnMills">Synchronisation time(ms), default 1000ms</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function USB_GetSyncTimeOut(ByRef pnMills As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_USB_GetSyncTimeOut(m_devHandle, pnMills)
    End Function
    #End Region

    #Region "GenTL相关接口，其它接口可以复用（部分接口不支持）"
    ''' <summary>
    ''' Enumerate interfaces by GenTL
    ''' </summary>
    ''' <param name="stIFInfoList"> Interface information list</param>
    ''' <param name="strGenTLPath">Path of GenTL's cti file</param>
    ''' <Returns></Returns>
    Public Shared Function EnumInterfacesByGenTL(ByRef stIFInfoList As MV_GENTL_IF_INFO_LIST, ByVal strGenTLPath As String) As Integer
        Return MV_CC_EnumInterfacesByGenTL(stIFInfoList, strGenTLPath)
    End Function

    ''' <summary>
    ''' Enumerate Device Based On GenTL
    ''' </summary>
    ''' <param name="stIFInfo">Interface information</param>
    ''' <param name="stDevList">Device List</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Shared Function EnumDevicesByGenTL(ByRef stIFInfo As MV_GENTL_IF_INFO, ByRef stDevList As MV_GENTL_DEV_INFO_LIST) As Integer
        Return MV_CC_EnumDevicesByGenTL(stIFInfo, stDevList)
    End Function

    ''' <summary>
    ''' Unload cti library
    ''' </summary>
    ''' <param name="strGenTLPath">GenTL cti file path</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Shared Function UnloadGenTLLibrary(ByVal strGenTLPath As String) As Integer
        Return MV_CC_UnloadGenTLLibrary(strGenTLPath)
    End Function

    ''' <summary>
    ''' Create Device Handle Based On GenTL Device Info
    ''' </summary>
    ''' <param name="stDevInfo">Device Information Structure</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function CreateDeviceByGenTL(ByRef stDevInfo As MV_GENTL_DEV_INFO) As Integer
        If (IntPtr.Zero <> m_devHandle) Then
            MV_CC_DestroyHandle(m_devHandle)
            m_devHandle = IntPtr.Zero
        End If
        Return MV_CC_CreateHandleByGenTL(m_devHandle, stDevInfo)
    End Function
    #End Region

    #Region "XML解析树的生成"
    ''' <summary>
    ''' Get camera feature tree XML
    ''' </summary>
    ''' <param name="pData">XML data receiving buffer</param>
    ''' <param name="nDataSize">Buffer size</param>
    ''' <param name="pnDataLen">Actual data length</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function XML_GetGenICamXML(ByVal pData As IntPtr, ByVal nDataSize As UInteger, ByRef pnDataLen As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_XML_GetGenICamXML(m_devHandle, pData, nDataSize, pnDataLen)
    End Function

    ''' <summary>
    ''' Get Access mode of cur node
    ''' </summary>
    ''' <param name="strName">Name of node</param>
    ''' <param name="pAccessMode">Access mode of the node</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function XML_GetNodeAccessMode(ByVal strName As String, ByRef pAccessMode As MV_XML_AccessMode) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_XML_GetNodeAccessMode(m_devHandle, strName, pAccessMode)
    End Function

    ''' <summary>
    ''' Get Interface Type of cur node
    ''' </summary>
    ''' <param name="strName">Name of node</param>
    ''' <param name="pInterfaceType">Interface Type of the node</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function XML_GetNodeInterfaceType(ByVal strName As String, ByRef pInterfaceType As MV_XML_InterfaceType) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_XML_GetNodeInterfaceType(m_devHandle, strName, pInterfaceType)
    End Function
    #End Region

    #Region "附加接口"
    ''' <summary>
    ''' Save image, support Bmp and Jpeg. Encoding quality(50-99]
    ''' </summary>
    ''' <param name="stSaveParam">Save image parameters structure</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function SaveImageEx(ByRef stSaveParam As MV_SAVE_IMAGE_PARAM_EX) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_SaveImageEx2(m_devHandle, stSaveParam)
    End Function

    ''' <summary>
    ''' Save the image file, support Bmp、 Jpeg、Png and Tiff. Encoding quality(50-99]
    ''' </summary>
    ''' <param name="stSaveFileParam">Save the image file parameter structure</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function SaveImageToFile(ByRef stSaveFileParam As MV_SAVE_IMG_TO_FILE_PARAM) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_SaveImageToFile(m_devHandle, stSaveFileParam)
    End Function

    ''' <summary>
    ''' Save 3D point data, support PLY、CSV and OBJ
    ''' </summary>
    ''' <param name="stPointDataParam">Save 3D point data parameters structure</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function SavePointCloudData(ByRef stPointDataParam As MV_SAVE_POINT_CLOUD_PARAM) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_SavePointCloudData(m_devHandle, stPointDataParam)
    End Function

    ''' <summary>
    ''' Rotate Image
    ''' </summary>
    ''' <param name="stRotateParam">Rotate image parameter structure</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function RotateImage(ByRef stRotateParam As MV_CC_ROTATE_IMAGE_PARAM) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_RotateImage(m_devHandle, stRotateParam)
    End Function

    ''' <summary>
    ''' Flip Image
    ''' </summary>
    ''' <param name="stFlipParam">Flip image parameter structure</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function FlipImage(ByRef stFlipParam As MV_CC_FLIP_IMAGE_PARAM) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_FlipImage(m_devHandle, stFlipParam)
    End Function

    ''' <summary>
    ''' Pixel format conversion
    ''' </summary>
    ''' <param name="stCvtParam">Convert Pixel Type parameter structure</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function ConvertPixelType(ByRef stCvtParam As MV_CC_PIXEL_CONVERT_PARAM) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_ConvertPixelType(m_devHandle, stCvtParam)
    End Function

    ''' <summary>
    ''' Interpolation algorithm type setting
    ''' </summary>
    ''' <param name="nBayerCvtQuality">Bayer interpolation method  0-Fast 1-Equilibrium 2-Optimal</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function SetBayerCvtQuality(ByVal nBayerCvtQuality As UInteger) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_SetBayerCvtQuality(m_devHandle, nBayerCvtQuality)
    End Function

    ''' <summary>
    ''' Filter type of the bell interpolation quality algorithm setting
    ''' </summary>
    ''' <param name="bFilterEnable">Filter type enable</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function SetBayerFilterEnable(ByVal bFilterEnable As Boolean) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_SetBayerFilterEnable(m_devHandle, bFilterEnable)
    End Function

    ''' <summary>
    ''' Set Gamma value
    ''' </summary>
    ''' <param name="fBayerGammaValue">Gamma value[0.1,4.0]</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function MV_CC_SetGammaValue(ByVal enSrcPixelType As MvGvspPixelType, ByVal fGammaValue As Single) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_SetGammaValue(m_devHandle, enSrcPixelType, fGammaValue)
    End Function

    ''' <summary>
    ''' Set Gamma param
    ''' </summary>
    ''' <param name="stGammaParam">Gamma parameter structure</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function SetBayerGammaParam(ByRef stGammaParam As MV_CC_GAMMA_PARAM) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_SetBayerGammaParam(m_devHandle, stGammaParam)
    End Function

    ''' <summary>
    ''' Set CCM param
    ''' </summary>
    ''' <param name="stCCMParam">CCM parameter structure</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function SetBayerCCMParam(ByRef stCCMParam As MV_CC_CCM_PARAM ) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_SetBayerCCMParam(m_devHandle, stCCMParam)
    End Function

    ''' <summary>
    ''' Set CCM param
    ''' </summary>
    ''' <param name="stCCMParam">CCM parameter structure</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function SetBayerCCMParamEx(ByRef stCCMParam As MV_CC_CCM_PARAM_EX) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_SetBayerCCMParamEx(m_devHandle, stCCMParam)
    End Function

    ''' <summary>
    ''' Adjust image contrast
    ''' </summary>
    ''' <param name="stContrastParam">Contrast parameter structure</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function ImageContrast(ByRef stContrastParam As MV_CC_CONTRAST_PARAM) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_ImageContrast(m_devHandle, stContrastParam)
    End Function

    ''' <summary>
    ''' High Bandwidth Decode
    ''' </summary>
    ''' <param name="stDecodeParam">High Bandwidth Decode parameter structure</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function HB_Decode(ByRef stDecodeParam As MV_CC_HB_DECODE_PARAM ) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_HB_Decode(m_devHandle, stDecodeParam)
    End Function

    ''' <summary>
    ''' Draw Rect Auxiliary Line
    ''' </summary>
    ''' <param name="stRectInfo">Rect Auxiliary Line Info</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function DrawRect(ByRef stRectInfo As MVCC_RECT_INFO) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_DrawRect(m_devHandle, stRectInfo)
    End Function

    ''' <summary>
    ''' Draw Circle Auxiliary Line
    ''' </summary>
    ''' <param name="stCircleInfo">Circle Auxiliary Line Info</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function DrawCircle(ByRef stCircleInfo As MVCC_CIRCLE_INFO) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_DrawCircle(m_devHandle, stCircleInfo)
    End Function

    ''' <summary>
    ''' Draw Line Auxiliary Line
    ''' </summary>
    ''' <param name="stLinesInfo">Linear Auxiliary Line Info</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function DrawLines(ByRef stLinesInfo As MVCC_LINES_INFO) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_DrawLines(m_devHandle, stLinesInfo)
    End Function

    ''' <summary>
    ''' Save camera feature
    ''' </summary>
    ''' <param name="strFileName">File name</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function FeatureSave(ByVal strFileName As String) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_FeatureSave(m_devHandle, strFileName)
    End Function

    ''' <summary>
    ''' Load camera feature
    ''' </summary>
    ''' <param name="strFileName">File name</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function FeatureLoad(ByVal strFileName As String) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_FeatureLoad(m_devHandle, strFileName)
    End Function

    ''' <summary>
    ''' Read the file from the camera
    ''' </summary>
    ''' <param name="stFileAccess">File access structure</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function FileAccessRead(ByRef stFileAccess As MV_CC_FILE_ACCESS) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_FileAccessRead(m_devHandle, stFileAccess)
    End Function

    ''' <summary>
    ''' Read the file from the camera
    ''' </summary>
    ''' <param name="stFileAccessEx">File access structure</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function FileAccessReadEx(ByRef stFileAccessEx As MV_CC_FILE_ACCESS_EX) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_FileAccessReadEx(m_devHandle, stFileAccessEx)
    End Function

    ''' <summary>
    ''' Write the file to camera
    ''' </summary>
    ''' <param name="stFileAccess">File access structure</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function FileAccessWrite(ByRef stFileAccess As MV_CC_FILE_ACCESS) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_FileAccessWrite(m_devHandle, stFileAccess)
    End Function

    ''' <summary>
    ''' Write the file to camera
    ''' </summary>
    ''' <param name="stFileAccessEx">File access structure</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function FileAccessWriteEx(ByRef stFileAccessEx As MV_CC_FILE_ACCESS_EX) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_FileAccessWriteEx(m_devHandle, stFileAccessEx)
    End Function

    ''' <summary>
    ''' Get File Access Progress 
    ''' </summary>
    ''' <param name="stFileAccessProgress">File access Progress</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function GetFileAccessProgress(ByRef stFileAccessProgress As MV_CC_FILE_ACCESS_PROGRESS) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_GetFileAccessProgress(m_devHandle, stFileAccessProgress)
    End Function

    ''' <summary>
    ''' Start Record
    ''' </summary>
    ''' <param name="stRecordParam">Record param structure</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function StartRecord(ByRef stRecordParam As MV_CC_RECORD_PARAM ) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_StartRecord(m_devHandle, stRecordParam)
    End Function

    ''' <summary>
    ''' Input RAW data to Record
    ''' </summary>
    ''' <param name="stInputFrameInfo">Record data structure</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function InputOneFrame(ByRef stInputFrameInfo As MV_CC_INPUT_FRAME_INFO ) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_InputOneFrame(m_devHandle, stInputFrameInfo)
    End Function

    ''' <summary>
    ''' Stop Record
    ''' </summary>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code </Returns>
    Public Function StopRecord() As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_StopRecord(m_devHandle)
    End Function

    ''' <summary>
    ''' Open the GUI interface for getting or setting camera parameters
    ''' </summary>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function OpenParamsGUI() As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_OpenParamsGUI(m_devHandle)
    End Function

    ''' <summary>
    ''' Reconstruct Image(For time-division exposure function)
    ''' </summary>
    ''' <param name="stReconstructParam">Reconstruct image parameters</param>
    ''' <Returns>Success, Return MV_OK. Failure, Return error code</Returns>
    Public Function ReconstructImage(ByRef stReconstructParam As MV_RECONSTRUCT_IMAGE_PARAM) As Integer
        If IntPtr.Zero = m_devHandle Then
            Return MV_E_HANDLE
        End If
        Return MV_CC_ReconstructImage(m_devHandle, stReconstructParam)
    End Function
    #End Region

    #Region "常用公共功能接口函数"
    ''' <summary>
    ''' 对象类型转为字符数组
    ''' </summary>
    ''' <param name="objStructure">对象</param>
    ''' <Returns>字符数组</Returns>
    ''' <remarks></remarks>
    Public Shared Function StructureToByteArray(ByVal objStructure As Object) As Byte()
        Dim nObjSize As Integer = Marshal.SizeOf(objStructure)
        Dim arrByte(nObjSize - 1) As Byte
        Dim bufferPtr As IntPtr = Marshal.AllocHGlobal(nObjSize)
        Marshal.StructureToPtr(objStructure, bufferPtr, False)
        Marshal.Copy(bufferPtr, arrByte, 0, nObjSize)
        Marshal.FreeHGlobal(bufferPtr)
        Return arrByte
    End Function

    ''' <summary>
    ''' 字符数组类型转为结构体类型
    ''' </summary>
    ''' <param name="arrByte">字符数组类型</param>
    ''' <param name="objType">对象类型</param>
    ''' <Returns>对象类型</Returns>
    ''' <remarks></remarks>
    Public Shared Function ByteArrayToStructure(ByVal arrByte() As Byte, ByVal objType As Type) As Object
        Dim nObjSize As Integer = Marshal.SizeOf(objType)
        If nObjSize > arrByte.Length Then Return Nothing
        Dim bufferPtr As IntPtr = Marshal.AllocHGlobal(nObjSize)
        Marshal.Copy(arrByte, 0, bufferPtr, nObjSize)
        Dim objStructure As Object = Marshal.PtrToStructure(bufferPtr, objType)
        Marshal.FreeHGlobal(bufferPtr)
        Return objStructure
    End Function

    ''' <summary>
    ''' 判断像素格式是否为无损压缩格式
    ''' </summary>
    ''' <param name="enPixelType">像素格式</param>
    ''' <Returns>成功: 返回true； 失败: 返回false</Returns>
    ''' <remarks></remarks>
    Public Shared Function IsHBPixelType(ByVal enPixelType As MvGvspPixelType) As Boolean
        Select Case enPixelType
            Case MvGvspPixelType.PixelType_Gvsp_HB_Mono8
            Case MvGvspPixelType.PixelType_Gvsp_HB_Mono10
            Case MvGvspPixelType.PixelType_Gvsp_HB_Mono10_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_Mono12
            Case MvGvspPixelType.PixelType_Gvsp_HB_Mono12_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_Mono16
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerGR8
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerRG8
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerGB8
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerBG8
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerGR10
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerRG10
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerGB10
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerBG10
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerGR12
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerRG12
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerGB12
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerBG12
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerGR10_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerRG10_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerGB10_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerBG10_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerGR12_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerRG12_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerGB12_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerBG12_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_YUV422_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_YUV422_YUYV_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_RGB8_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_BGR8_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_RGBA8_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_BGRA8_Packed
                Return True
            Case Else
                Return False
        End Select
    End Function

    ''' <summary>
    ''' 判断字符编码为UTF-8
    ''' </summary>
    ''' <param name="arrByte">字符数组</param>
    ''' <Returns>成功: 返回True 失败: 返回False</Returns>
    ''' <remarks></remarks>
    Public Shared Function IsTextUTF8(ByVal arrByte() As Byte) As Boolean
        Dim encodingBytesCount As Integer = 0
        Dim allTextsAreASCIIChars As Boolean = True

        Dim i As Integer = 0
        For i = 0 To arrByte.Length - 1
            Dim current As Byte = arrByte(i)
            If (current And &H80) = &H80 Then
                allTextsAreASCIIChars = False
            End If

            If encodingBytesCount = 0 Then
                If (current And &H80) = 0 Then
                    Continue For
                End If

                If (current And &HC0) = &HC0 Then
                    encodingBytesCount = 1
                    current = current << 2
                    While (current And &H80) = &H80
                        current = current << 1
                        encodingBytesCount = encodingBytesCount + 1
                    End While
                Else
                    Return False
                End If
            Else
                If (current And &HC0) = &H80 Then
                    encodingBytesCount = encodingBytesCount - 1
                Else
                    Return False
                End If
            End If

            If encodingBytesCount <> 0 Then
                Return False
            End If

            If allTextsAreASCIIChars = False Then
                Return True
            Else
                Return False
            End If
        Next
    End Function

    ''' <summary>
    ''' 获取图像大小
    ''' </summary>
    ''' <param name="enPixelType">像素格式</param>
    ''' <param name="nWidth">图像宽度</param>
    ''' <param name="nHeight">图像高度</param>
    ''' <Returns>图像大小</Returns>
    ''' <remarks></remarks>
    Public Shared Function GetPixelSize(ByVal enPixelType As MvGvspPixelType, ByVal nWidth As UInteger, ByVal nHeight As UInteger) As ULong
        Dim nlWidth As ULong = nWidth
        Dim nlHeight As ULong = nHeight

        Dim nlSize As ULong = nWidth * nHeight

        Select Case (enPixelType)
            Case MvGvspPixelType.PixelType_Gvsp_HB_Mono8
            Case MvGvspPixelType.PixelType_Gvsp_BayerGR8
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerGR8
            Case MvGvspPixelType.PixelType_Gvsp_BayerRG8
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerRG8
            Case MvGvspPixelType.PixelType_Gvsp_BayerGB8
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerGB8
            Case MvGvspPixelType.PixelType_Gvsp_BayerBG8
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerBG8
            Case MvGvspPixelType.PixelType_Gvsp_BayerRBGG8
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerRBGG8
                Return nlSize
            Case MvGvspPixelType.PixelType_Gvsp_Mono10
            Case MvGvspPixelType.PixelType_Gvsp_HB_Mono10
            Case MvGvspPixelType.PixelType_Gvsp_Mono12
            Case MvGvspPixelType.PixelType_Gvsp_HB_Mono12
            Case MvGvspPixelType.PixelType_Gvsp_Mono16
            Case MvGvspPixelType.PixelType_Gvsp_HB_Mono16
            Case MvGvspPixelType.PixelType_Gvsp_BayerBG10
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerBG10
            Case MvGvspPixelType.PixelType_Gvsp_BayerGB10
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerGB10
            Case MvGvspPixelType.PixelType_Gvsp_BayerRG10
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerRG10
            Case MvGvspPixelType.PixelType_Gvsp_BayerGR10
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerGR10
            Case MvGvspPixelType.PixelType_Gvsp_BayerGR12
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerGR12
            Case MvGvspPixelType.PixelType_Gvsp_BayerRG12
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerRG12
            Case MvGvspPixelType.PixelType_Gvsp_BayerGB12
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerGB12
            Case MvGvspPixelType.PixelType_Gvsp_BayerBG12
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerBG12
            Case MvGvspPixelType.PixelType_Gvsp_BayerGR16
            Case MvGvspPixelType.PixelType_Gvsp_BayerRG16
            Case MvGvspPixelType.PixelType_Gvsp_BayerGB16
            Case MvGvspPixelType.PixelType_Gvsp_BayerBG16
            Case MvGvspPixelType.PixelType_Gvsp_YUV422_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_YUV422_Packed
            Case MvGvspPixelType.PixelType_Gvsp_YUV422_YUYV_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_YUV422_YUYV_Packed
                Return (nlSize * 2)
            Case MvGvspPixelType.PixelType_Gvsp_Mono10_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_Mono10_Packed
            Case MvGvspPixelType.PixelType_Gvsp_Mono12_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_Mono12_Packed
            Case MvGvspPixelType.PixelType_Gvsp_BayerBG10_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerBG10_Packed
            Case MvGvspPixelType.PixelType_Gvsp_BayerGB10_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerGB10_Packed
            Case MvGvspPixelType.PixelType_Gvsp_BayerRG10_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerRG10_Packed
            Case MvGvspPixelType.PixelType_Gvsp_BayerGR10_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerGR10_Packed
            Case MvGvspPixelType.PixelType_Gvsp_BayerBG12_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerBG12_Packed
            Case MvGvspPixelType.PixelType_Gvsp_BayerGB12_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerGB12_Packed
            Case MvGvspPixelType.PixelType_Gvsp_BayerRG12_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerRG12_Packed
            Case MvGvspPixelType.PixelType_Gvsp_BayerGR12_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_BayerGR12_Packed
                Return (nlSize * 3 \ 2)
            Case MvGvspPixelType.PixelType_Gvsp_RGB8_Planar
            Case MvGvspPixelType.PixelType_Gvsp_RGB8_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_RGB8_Packed
            Case MvGvspPixelType.PixelType_Gvsp_BGR8_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_BGR8_Packed
                Return (nlSize * 3)
            Case MvGvspPixelType.PixelType_Gvsp_RGBA8_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_RGBA8_Packed
            Case MvGvspPixelType.PixelType_Gvsp_BGRA8_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_BGRA8_Packed
                Return (nlSize * 4)
            Case MvGvspPixelType.PixelType_Gvsp_Coord3D_A32f
            Case MvGvspPixelType.PixelType_Gvsp_Coord3D_A32
            Case MvGvspPixelType.PixelType_Gvsp_Coord3D_C32f
            Case MvGvspPixelType.PixelType_Gvsp_Coord3D_C32
                Return (nlSize * 1 * 4)
            Case MvGvspPixelType.PixelType_Gvsp_Coord3D_AC32f
            Case MvGvspPixelType.PixelType_Gvsp_Coord3D_AC32
            Case MvGvspPixelType.PixelType_Gvsp_Coord3D_AB32f
            Case MvGvspPixelType.PixelType_Gvsp_Coord3D_AB32
            Case MvGvspPixelType.PixelType_Gvsp_RGBA16_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_RGBA16_Packed
            Case MvGvspPixelType.PixelType_Gvsp_BGRA16_Packed
            Case MvGvspPixelType.PixelType_Gvsp_HB_BGRA16_Packed
                Return (nlSize * 2 * 4)
            Case MvGvspPixelType.PixelType_Gvsp_Coord3D_ABC32f
            Case MvGvspPixelType.PixelType_Gvsp_Coord3D_ABC32
                Return (nlSize * 3 * 4)
            Case MvGvspPixelType.PixelType_Gvsp_Coord3D_ABC16
                Return (nlSize * 3 * 2)
            Case Else
                Return 0
        End Select
    End Function
    #End Region

    #Region "私有成员设备句柄"
    Private m_devHandle As IntPtr  '定义设备句柄
    #End Region

    #Region "导出c/c++动态库"
    #Region "导出系统动态库"
    Public Declare Sub CopyMem Lib "KERNEL32" Alias "RtlMoveMemory" (ByVal pDst As Object, ByVal pSrc As Object, ByVal ByteLen As Integer)
    Public Declare Function LoadLibrary Lib "kernel32" Alias "LoadLibraryA" (ByVal lpLibFileName As String) As Long
    Public Declare Function GetProcAddress Lib "kernel32" (ByVal hModule As Long, ByVal lpProcName As String) As Long
    Public Declare Function CallWindowProc Lib "user32" Alias "CallWindowProcA" (ByVal lpPrevWndFunc As Long, ByVal hWnd As Long, ByVal Msg As Long, ByVal wParam As Long, ByVal lParam As Long) As Long
    Public Declare Function FreeLibrary Lib "kernel32" (ByVal hLibModule As Long) As Long
    Public Declare Sub Sleep Lib "kernel32" (ByVal dwMilliseconds As Long)
    Public Declare Function VirtualAlloc Lib "kernel32" (ByVal lpAddress As Long, ByVal dwSize As Long, ByVal flAllocationType As Long, ByVal flProtect As Long) As Long
    #End Region

    #Region "设备的基本指令和操作动态库导出"
    Private Declare Function MV_CC_GetSDKVersion Lib "MvCameraControl.dll" () As UInteger

    Private Declare Function MV_CC_EnumerateTls Lib "MvCameraControl.dll" () As Integer

    Private Declare Function MV_CC_EnumDevices Lib "MvCameraControl.dll" (ByVal nTLayerType As Integer, ByRef stDevList As MV_CC_DEVICE_INFO_LIST) As Integer

    Private Declare Function MV_CC_EnumDevicesEx Lib "MvCameraControl.dll" (ByVal nTLayerType As Integer, ByRef stDevList As MV_CC_DEVICE_INFO_LIST， ByVal strManufacturerName As String) As Integer

    Private Declare Function MV_CC_EnumDevicesEx2 Lib "MvCameraControl.dll" (ByVal nTLayerType As Integer, ByRef stDevList As MV_CC_DEVICE_INFO_LIST， ByVal strManufacturerName As String, ByVal enSortMethod As MV_SORT_METHOD) As Integer

    Private Declare Function MV_CC_IsDeviceAccessible Lib "MvCameraControl.dll" (ByRef stDevInfo As MV_CC_DEVICE_INFO, ByVal nAccessMode As Integer) As Integer

    Private Declare Function MV_CC_SetSDKLogPath Lib "MvCameraControl.dll" (ByVal strSDKLogPath As String) As Integer

    Private Declare Function MV_CC_CreateHandle Lib "MvCameraControl.dll" (ByRef handle As IntPtr, ByRef stDeviceInfo As MV_CC_DEVICE_INFO) As Integer

    Private Declare Function MV_CC_CreateHandleWithoutLog Lib "MvCameraControl.dll" (ByRef handle As IntPtr, ByRef stDeviceInfo As MV_CC_DEVICE_INFO) As Integer

    Private Declare Function MV_CC_DestroyHandle Lib "MvCameraControl.dll" (ByVal handle As IntPtr) As Integer

    Private Declare Function MV_CC_OpenDevice Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal nAccessMode As Integer, ByVal nSwitchoverKey As Short) As Integer

    Private Declare Function MV_CC_CloseDevice Lib "MvCameraControl.dll" (ByVal handle As IntPtr) As Integer

    Private Declare Function MV_CC_IsDeviceConnected Lib "MvCameraControl.dll" (ByVal handle As IntPtr) As Integer

    Private Declare Function MV_CC_RegisterImageCallBackEx Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal cbOutput As cbOutputExdelegate, ByVal pUser As IntPtr) As Integer

    Private Declare Function MV_CC_RegisterImageCallBackForRGB Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal cbOutput As cbOutputExdelegate, ByVal pUser As IntPtr) As Integer

    Private Declare Function MV_CC_RegisterImageCallBackForBGR Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal cbOutput As cbOutputExdelegate, ByVal pUser As IntPtr) As Integer

    Private Declare Function MV_CC_StartGrabbing Lib "MvCameraControl.dll" (ByVal handle As IntPtr) As Integer

    Private Declare Function MV_CC_StopGrabbing Lib "MvCameraControl.dll" (ByVal handle As IntPtr) As Integer

    Private Declare Function MV_CC_GetImageForRGB Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal pData As IntPtr, ByVal nDataSize As Integer, ByRef pFrameInfo As MV_FRAME_OUT_INFO_EX, ByVal nMsec As Integer) As Integer

    Private Declare Function MV_CC_GetImageForBGR Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal pData As IntPtr, ByVal nDataSize As Integer, ByRef pFrameInfo As MV_FRAME_OUT_INFO_EX, ByVal nMsec As Integer) As Integer

    Private Declare Function MV_CC_GetImageBuffer Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef stFrame As MV_FRAME_OUT, ByVal nMsec As Integer) As Integer

    Private Declare Function MV_CC_FreeImageBuffer Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef stFrame As MV_FRAME_OUT) As Integer

    Private Declare Function MV_CC_GetOneFrameTimeout Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal pData As IntPtr, ByVal nDataSize As Integer, ByRef pFrameInfo As MV_FRAME_OUT_INFO_EX, ByVal nMsec As Integer) As Integer

    Private Declare Function MV_CC_ClearImageBuffer Lib "MvCameraControl.dll" (ByVal handle As IntPtr) As Integer

    Private Declare Function MV_CC_GetValidImageNum Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef pnValidImageNum As UInteger) As Integer

    Private Declare Function MV_CC_DisplayOneFrame Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef stDisplayInfo As MV_DISPLAY_FRAME_INFO) As Integer
	
	Private Declare Function MV_CC_DisplayOneFrameEx Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal hWnd As IntPtr, ByRef stDisplayInfoEx As MV_DISPLAY_FRAME_INFO_EX) As Integer

    Private Declare Function MV_CC_SetImageNodeNum Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal nNum As UInteger) As Integer

    Private Declare Function MV_CC_SetGrabStrategy Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal enGrabStrategy As MV_GRAB_STRATEGY) As Integer

    Private Declare Function MV_CC_SetOutputQueueSize Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal nOutputQueueSize As UInteger) As Integer

    Private Declare Function MV_CC_GetDeviceInfo Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef stDevInfo As MV_CC_DEVICE_INFO) As Integer

    Private Declare Function MV_CC_GetAllMatchInfo Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef stInfo As MV_ALL_MATCH_INFO) As Integer

    #End Region


    #Region "设置和获取设备参数的万能接口"
    Private Declare Function MV_CC_GetIntValueEx Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal strName As String, ByRef stValueEx As MVCC_INTVALUE_EX) As Integer

    '设置Integer型属性值
    Private Declare Function MV_CC_SetIntValueEx Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal strName As String, ByVal nValue As Long) As Integer

    '获取Enum属性值
    Private Declare Function MV_CC_GetEnumValue Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal strName As String, ByRef stValue As MVCC_ENUMVALUE) As Integer

    '设置Enum型属性值
    Private Declare Function MV_CC_SetEnumValue Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal strName As String, ByVal nValue As UInteger) As Integer

    Private Declare Function MV_CC_GetEnumEntrySymbolic Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal strName As String, ByRef stEnumEntry As MVCC_ENUMENTRY) As Integer

    Private Declare Function MV_CC_SetEnumValueByString Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal strName As String, ByVal strValue As String) As Integer

     '获取Float属性值
    Private Declare Function MV_CC_GetFloatValue Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal strName As String, ByRef stValue As MVCC_FLOATVALUE) As Integer

    '设置float型属性值
    Private Declare Function MV_CC_SetFloatValue Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal strName As String, ByVal fValue As Single) As Integer

    '获取Boolean属性值
    Private Declare Function MV_CC_GetBoolValue Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal strName As String, ByRef pbValue As Boolean) As Integer

    '设置Boolean型属性值
    Private Declare Function MV_CC_SetBoolValue Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal strName As String, ByVal bValue As Boolean) As Integer

    '获取String属性值
    Private Declare Function MV_CC_GetStringValue Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal strName As String, ByRef stValue As MVCC_STRINGVALUE) As Integer

    '设置String型属性值
    Private Declare Function MV_CC_SetStringValue Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal strName As String, ByVal strValue As String) As Integer

    '设置Command型属性值
    Private Declare Function MV_CC_SetCommandValue Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal strName As String) As Integer

    Private Declare Function MV_CC_InvalidateNodes Lib "MvCameraControl.dll" (ByVal handle As IntPtr) As Integer

    #End Region

    #Region "设备升级 和 寄存器读写 和异常、事件回调"
    Private Declare Function MV_CC_LocalUpgrade Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal strFilePathName As String) As Integer

    Private Declare Function MV_CC_GetUpgradeProcess Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef pnProcess As UInteger) As Integer

    Private Declare Function MV_CC_ReadMemory Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal pBuffer As IntPtr, ByVal nAddress As Long, ByVal nLength As Long) As Integer

    Private Declare Function MV_CC_WriteMemory Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal pBuffer As IntPtr, ByVal nAddress As Long, ByVal nLength As Long) As Integer

    Private Declare Function MV_CC_RegisterExceptionCallBack Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal cbException As cbExceptiondelegate, ByVal pUser As IntPtr) As Integer

    Private Declare Function MV_CC_RegisterAllEventCallBack Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal cbEvent As cbEventdelegateEx, ByVal pUser As IntPtr) As Integer

    Private Declare Function MV_CC_RegisterEventCallBackEx Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal strEventName As String, ByVal cbEvent As cbEventdelegateEx, ByVal pUser As IntPtr) As Integer
    #End Region

    #Region "GigEVision 设备独有的接口"
    Private Declare Function MV_GIGE_SetEnumDevTimeout Lib "MvCameraControl.dll" (ByVal nMilTimeout As UInteger) As Integer

    Private Declare Function MV_GIGE_ForceIpEx Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal nIP As UInteger, ByVal nSubNetMask As UInteger, ByVal nDefaultGateWay As UInteger) As Integer

    Private Declare Function MV_GIGE_SetIpConfig Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal nType As UInteger) As Integer

    Private Declare Function MV_GIGE_SetNetTransMode Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal nType As UInteger) As Integer

    Private Declare Function MV_GIGE_GetNetTransInfo Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef stInfo As MV_NETTRANS_INFO) As Integer

    Private Declare Function MV_GIGE_SetDiscoveryMode Lib "MvCameraControl.dll" (ByVal nMode As UInteger) As Integer

    Private Declare Function MV_GIGE_SetGvspTimeout Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal nMillisec As UInteger) As Integer

    Private Declare Function MV_GIGE_GetGvspTimeout Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef pnMillisec As UInteger) As Integer

    Private Declare Function MV_GIGE_SetGvcpTimeout Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal nMillisec As UInteger) As Integer

    Private Declare Function MV_GIGE_GetGvcpTimeout Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef pnMillisec As UInteger) As Integer

    Private Declare Function MV_GIGE_SetRetryGvcpTimes Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal nRetryGvcpTimes As UInteger) As Integer

    Private Declare Function MV_GIGE_GetRetryGvcpTimes Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef pnRetryGvcpTimes As UInteger) As Integer

    Private Declare Function MV_CC_GetOptimalPacketSize Lib "MvCameraControl.dll" (ByVal handle As IntPtr) As Integer

    Private Declare Function MV_GIGE_SetResend Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal bEnable As UInteger, ByVal nMaxResendPercent As UInteger, ByVal nResendTimeout As UInteger) As Integer

    Private Declare Function MV_GIGE_SetResendMaxRetryTimes Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal nRetryTimes As UInteger) As Integer

    Private Declare Function MV_GIGE_GetResendMaxRetryTimes Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef pnRetryTimes As UInteger) As Integer

    Private Declare Function MV_GIGE_SetResendTimeInterval Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal nMillisec As UInteger) As Integer

    Private Declare Function MV_GIGE_GetResendTimeInterval Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef nMillisec As UInteger) As Integer

    Private Declare Function MV_GIGE_SetTransmissionType Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef stTransmissionType As MV_TRANSMISSION_TYPE) As Integer

    Private Declare Function MV_GIGE_IssueActionCommand Lib "MvCameraControl.dll" (ByVal stActionCmdInfo As MV_ACTION_CMD_INFO , ByRef stActionCmdResults As MV_ACTION_CMD_RESULT_LIST) As Integer

    Private Declare Function MV_GIGE_GetMulticastStatus Lib "MvCameraControl.dll" (ByRef stDevInfo As MV_CC_DEVICE_INFO , ByRef pbStatus As Boolean) As Integer

    #End Region

    #Region "CameraLink 设备独有的接口"
    Private Declare Function MV_CAML_SetDeviceBaudrate Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal nBaudrate As UInteger) As Integer

    Private Declare Function MV_CAML_GetDeviceBaudrate Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef pnCurrentBaudrate As UInteger) As Integer

    Private Declare Function MV_CAML_GetSupportBaudrates Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef pnBaudrateAblity As UInteger) As Integer

    Private Declare Function MV_CAML_SetGenCPTimeOut Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal nMillisec As UInteger) As Integer
    #End Region

    #Region "U3V 设备独有的接口"
    Private Declare Function MV_USB_SetTransferSize Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal nTransferSize As UInteger) As Integer

    Private Declare Function MV_USB_GetTransferSize Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef pnTransferSize As UInteger) As Integer

    Private Declare Function MV_USB_SetTransferWays Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal nTransferWays As UInteger) As Integer

    Private Declare Function MV_USB_GetTransferWays Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef pnTransferWays As UInteger) As Integer

    Private Declare Function MV_USB_RegisterStreamExceptionCallBack Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal cbException As cbStreamException, ByVal pUser As IntPtr) As Integer

    Private Declare Function MV_USB_SetEventNodeNum Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal nEventNodeNum As UInteger) As Integer

    Private Declare Function MV_USB_SetSyncTimeOut Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal nMills As UInteger) As Integer

    Private Declare Function MV_USB_GetSyncTimeOut Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef pnMills As UInteger) As Integer
    #End Region

    #Region "GenTL相关接口，其它接口可以复用（部分接口不支持）"
    Private Declare Function MV_CC_EnumInterfacesByGenTL Lib "MvCameraControl.dll" (ByRef stIFList As MV_GENTL_IF_INFO_LIST , ByVal strGenTLPath As String) As Integer

    Private Declare Function MV_CC_EnumDevicesByGenTL Lib "MvCameraControl.dll" (ByRef stIFInfo As MV_GENTL_IF_INFO , ByRef stDevList As MV_GENTL_DEV_INFO_LIST) As Integer

    Private Declare Function MV_CC_UnloadGenTLLibrary Lib "MvCameraControl.dll" (ByVal strGenTLPath As String) As Integer

    Private Declare Function MV_CC_CreateHandleByGenTL Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef stDevInfo As MV_GENTL_DEV_INFO) As Integer
    #End Region

    #Region “XML解析树的生成”
    Private Declare Function MV_XML_GetGenICamXML Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal pData As IntPtr, ByVal nDataSize As UInteger, ByRef pnDataLen As UInteger) As Integer

    Private Declare Function MV_XML_GetNodeAccessMode Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal strName As String, ByRef penAccessMode As MV_XML_AccessMode) As Integer

    Private Declare Function MV_XML_GetNodeInterfaceType Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal strName As String, ByRef penInterfaceType As MV_XML_InterfaceType) As Integer
    #End Region

    #Region "附加接口"
    Private Declare Function MV_CC_SaveImageEx2 Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef stSaveParam As MV_SAVE_IMAGE_PARAM_EX) As Integer

    Private Declare Function MV_CC_SaveImageToFile Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef stSaveFileParam As MV_SAVE_IMG_TO_FILE_PARAM) As Integer

    Private Declare Function MV_CC_SavePointCloudData Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef stPointDataParam As MV_SAVE_POINT_CLOUD_PARAM) As Integer

    Private Declare Function MV_CC_RotateImage Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef stRotateParam As MV_CC_ROTATE_IMAGE_PARAM) As Integer

    Private Declare Function MV_CC_FlipImage Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef stFlipParam As MV_CC_FLIP_IMAGE_PARAM) As Integer

    Private Declare Function MV_CC_ConvertPixelType Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef stCvtParam As MV_CC_PIXEL_CONVERT_PARAM) As Integer

    Private Declare Function MV_CC_SetBayerCvtQuality Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal nBayerCvtQuality As Single) As Integer

    Private Declare Function MV_CC_SetBayerFilterEnable Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal bFilterEnable As Boolean) As Integer

    Private Declare Function MV_CC_SetGammaValue Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal enPixelType As MvGvspPixelType, ByVal fGammaValue As Single) As Integer

    Private Declare Function MV_CC_SetBayerGammaParam Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef stGammaParam As MV_CC_GAMMA_PARAM) As Integer

    Private Declare Function MV_CC_SetBayerCCMParam Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef stCCMParam As MV_CC_CCM_PARAM) As Integer

    Private Declare Function MV_CC_SetBayerCCMParamEx Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef stCCMParam As MV_CC_CCM_PARAM_EX) As Integer

    Private Declare Function MV_CC_ImageContrast Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef stContrastParam As MV_CC_CONTRAST_PARAM) As Integer

    Private Declare Function MV_CC_HB_Decode Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef stDecodeParam As MV_CC_HB_DECODE_PARAM) As Integer

    Private Declare Function MV_CC_DrawRect Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef pRectInfo As MVCC_RECT_INFO) As Integer

    Private Declare Function MV_CC_DrawCircle Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef pCircleInfo As MVCC_CIRCLE_INFO) As Integer

    Private Declare Function MV_CC_DrawLines Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef pLinesInfo As MVCC_LINES_INFO) As Integer

    Private Declare Function MV_CC_FeatureSave Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal strFileName As String) As Integer

    Private Declare Function MV_CC_FeatureLoad Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByVal strFileName As String) As Integer

    Private Declare Function MV_CC_FileAccessRead Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef stFileAccess As MV_CC_FILE_ACCESS) As Integer

    Private Declare Function MV_CC_FileAccessReadEx Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef stFileAccessEx As MV_CC_FILE_ACCESS_EX) As Integer

    Private Declare Function MV_CC_FileAccessWrite Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef stFileAccess As MV_CC_FILE_ACCESS) As Integer

    Private Declare Function MV_CC_FileAccessWriteEx Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef stFileAccessEx As MV_CC_FILE_ACCESS_EX) As Integer

    Private Declare Function MV_CC_GetFileAccessProgress Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef stFileAccessProgress As MV_CC_FILE_ACCESS_PROGRESS) As Integer

    Private Declare Function MV_CC_StartRecord Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef stRecordParam As MV_CC_RECORD_PARAM) As Integer

    Private Declare Function MV_CC_InputOneFrame Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef stInputFrameInfo As MV_CC_INPUT_FRAME_INFO) As Integer

    Private Declare Function MV_CC_StopRecord Lib "MvCameraControl.dll" (ByVal handle As IntPtr) As Integer

    Private Declare Function MV_CC_OpenParamsGUI Lib "MvCameraControl.dll" (ByVal handle As IntPtr) As Integer

    Private Declare Function MV_CC_ReconstructImage Lib "MvCameraControl.dll" (ByVal handle As IntPtr, ByRef stReconstructParam As MV_RECONSTRUCT_IMAGE_PARAM) As Integer
    #End Region
    #End Region
End Class



