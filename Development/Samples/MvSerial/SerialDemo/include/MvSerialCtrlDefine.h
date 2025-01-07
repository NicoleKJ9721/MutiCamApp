#ifndef _MV_SERIAL_DEFINE_H_
#define _MV_SERIAL_DEFINE_H_

#define MV_SERIAL_DEVICE_INFO_SIZE 64

typedef enum _MV_SERIAL_STATUS_
{
    MV_STATUS_ERROR         = -1,     // Unknown error occurred
    MV_STATUS_AVAILABLE     =  0,     // Port is available
    MV_STATUS_NOT_AVAILABLE  =  1,     // Port is not present
    MV_STATUS_IN_USE         =  2,     // Port is in use
}MV_SERIAL_STATUS;

// Only the following baud rates are supported
typedef enum _MV_SERIAL_BAUDRATE_
{
    MV_Baud9600    = 9600,    // 9600 bits/sMV_c
    MV_Baud14400   = 14400,   // 14400 bits/sMV_c
    MV_Baud19200   = 19200,   // 19200 bits/sMV_c (dMV_fault)
    MV_Baud38400   = 38400,   // 38400 bits/sMV_c
    MV_Baud56000   = 56000,   // 56000 bits/sMV_c
    MV_Baud57600   = 57600,   // 57600 bits/sMV_c
    MV_Baud115200  = 115200,  // 115200 bits/sMV_c
    MV_Baud128000  = 128000,  // 128000 bits/sMV_c
    MV_Baud256000  = 256000,  // 256000 bits/sMV_c
} MV_SERIAL_BAUDRATE;

typedef struct _MV_SERIAL_DEVICE_INFO_
{
    MV_SERIAL_STATUS        enStatus;                                           ///< [OUT] \~chinese 端口状态               \~english Port ID
    MV_SERIAL_BAUDRATE      enBaudrate;                                         ///< [OUT] \~chinese 波特率                 \~english enBaudrate
    char                    chPortName[MV_SERIAL_DEVICE_INFO_SIZE];             ///< [OUT] \~chinese 端口号                 \~english Port ID
    char                    chDeviceModelName[MV_SERIAL_DEVICE_INFO_SIZE];      ///< [OUT] \~chinese 型号名字               \~english Model Name
    char                    chFamilyName[MV_SERIAL_DEVICE_INFO_SIZE];           ///< [OUT] \~chinese 名称                   \~english Family Name
    char                    chDeviceVersion[MV_SERIAL_DEVICE_INFO_SIZE];        ///< [OUT] \~chinese 设备版本               \~english Device Version
    char                    chManufacturerName[MV_SERIAL_DEVICE_INFO_SIZE];     ///< [OUT] \~chinese 制造商名字             \~english Manufacturer Name
    char                    chSerialNumber[MV_SERIAL_DEVICE_INFO_SIZE];         ///< [OUT] \~chinese 序列号                 \~english Serial Number[MV_SERIAL_DEVICE_INFO_SIZE];


    unsigned int            nRes[16];
}MV_SERIAL_DEVICE_INFO;

typedef enum _MV_SERIAL_NODE_TYPE_
{
    MV_NODE_TYPE_INT         =  0,     // 节点属性int类型
    MV_NODE_TYPE_FLOAT       =  1,     // 节点属性float类型
    MV_NODE_TYPE_BOOL        =  2,     // 节点属性bool类型
    MV_NODE_TYPE_STRING      =  3,     // 节点属性string类型
}MV_SERIAL_NODE_TYPE;

//异常回调
#define MV_SERIAL_EXCEPTION_DEV_DISCONNECT         0x00008001      // 设备断开连接

#endif