
#ifndef __MV_SERIAL_CTRL_H__
#define __MV_SERIAL_CTRL_H__

#include "MvSerialCtrlDefine.h"
#include "MvSerialCtrlErrCode.h"
/************************************************************************/
/* 动态库导入导出定义                                                   */
/* Import and export definition of the dynamic library                  */
/************************************************************************/
#ifndef MV_SERIAL_CTRL_API

    #ifdef WIN32
        #if defined(MV_SERIAL_CTRL_EXPORTS)
            #define MV_SERIAL_CTRL_API __declspec(dllexport)
        #else
            #define MV_SERIAL_CTRL_API __declspec(dllimport)
        #endif
    #else
        #ifndef __stdcall
            #define __stdcall
        #endif

        #ifndef MV_SERIAL_CTRL_API
            #define  MV_SERIAL_CTRL_API
        #endif
    #endif

#endif

#ifndef IN
    #define IN
#endif

#ifndef OUT
    #define OUT
#endif

#ifndef INOUT
    #define INOUT
#endif

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************//**
 *  @~chinese
 *  @brief  通过串口枚举设备信息
 *  @param  strPortName                       [IN]              COM口名称
 *  @param  strPortName                       [INOUT]           设备信息
 *  @return 成功，返回MV_SERIAL_OK；失败，返回错误码 
 *  @remarks strPortName格式为\\\\.\\COMn，如\\\\.\\COM7
 ************************************************************************/
MV_SERIAL_CTRL_API int __stdcall MV_SERIAL_EnumDevice(char* strPortName, MV_SERIAL_BAUDRATE enBaudrate, MV_SERIAL_DEVICE_INFO* pstDeviceInfo);

/********************************************************************//**
 *  @~chinese
 *  @brief  创建串口句柄
 *  @param  ppHandle                       [OUT]          句柄
 *  @return 成功，返回MV_SERIAL_OK；失败，返回错误码 
 ************************************************************************/
MV_SERIAL_CTRL_API int __stdcall MV_SERIAL_CreateHandle(OUT void** ppHandle);

/********************************************************************//**
 *  @~chinese
 *  @brief  打开设备
 *  @param  handle                      [IN]              句柄
 *  @param  pstDeviceInfo               [INOUT]           设备信息
 *  @return 成功，返回MV_SERIAL_OK；失败，返回错误码 
 ************************************************************************/
MV_SERIAL_CTRL_API int __stdcall MV_SERIAL_Open(IN void* handle, IN MV_SERIAL_DEVICE_INFO* pstDeviceInfo);

/********************************************************************//**
 *  @~chinese
 *  @brief  设置波特率
 *  @param  ppHandle                       [IN]          句柄
 *  @param  enBaudrate                     [IN]          波特率（以相机本身支持的波特率标准为准）
 *  @return 成功，返回MV_SERIAL_OK；失败，返回错误码 
 *  @remarks 设置波特率成功，相机会断开，即异常回调会捕获断线，需关闭设备后，重新以设置的波特率枚举
 *  @remarks 该机制为相机串口通信的限制，即设置波特率串口无是否成功的回复，只能以断线默认成功。
 ************************************************************************/
MV_SERIAL_CTRL_API int __stdcall MV_SERIAL_SetBaudrate(IN void* handle, IN MV_SERIAL_BAUDRATE enBaudrate);

/********************************************************************//**
 *  @~chinese
 *  @brief  关闭设备
 *  @param  handle                      [IN]              句柄
 *  @return 成功，返回MV_SERIAL_OK；失败，返回错误码 
 ************************************************************************/
MV_SERIAL_CTRL_API int __stdcall MV_SERIAL_Close(IN void* handle);

/********************************************************************//**
 *  @~chinese
 *  @brief  注册异常回调
 *  @param  handle                      [IN]              句柄
 *  @param  cbException                 [IN]              回调函数
 *  @param  pUser                       [IN]              参数
 *  @return 成功，返回MV_SERIAL_OK；失败，返回错误码 
 ************************************************************************/
MV_SERIAL_CTRL_API int __stdcall MV_SERIAL_RegisterExceptionCallBack(void* handle, void(__stdcall* cbException)(unsigned int nMsgType, void* pUser), void* pUser);

/********************************************************************//**
 *  @~chinese
 *  @brief  获取参数
 *  @param  handle                      [IN]            句柄
 *  @param  strKey                      [IN]            参数名称
 *  @param  enType                      [IN]            参数类型
 *  @param  pBuffer                     [OUT]           用来保存数据的缓存
 *  @param  nSize                       [IN]            缓存大小
 *  @return 成功,返回MV_SERIAL_OK,失败,返回错误码
 *  @remarks 参数类型参考MV_SERIAL_NODE_TYPE，该类型需要自行参考strKey属性节点对应的参数类型去匹配设置获取
 *  @remarks 如"Width"匹配MV_NODE_TYPE_INT该类型值； 注：不评估其校验值范围（相机串口通信的限制）
 *  @remarks 需要参考Camera Link工业XX相机用户手册文档说明（参数名称、类型和注意事项等都需以文档串口部分的描述为准）
 ************************************************************************/
MV_SERIAL_CTRL_API int __stdcall MV_SERIAL_GetValue(IN void* handle, IN const char* strKey, IN MV_SERIAL_NODE_TYPE  enType, INOUT void* pBuffer, IN unsigned int nSize);

/********************************************************************//**
 *  @~chinese
 *  @brief  设置参数
 *  @param  handle                      [IN]            句柄
 *  @param  strKey                      [IN]            参数名称
 *  @param  enType                      [IN]            参数类型
 *  @param  pBuffer                     [OUT]           要设置的数据缓存
 *  @param  nSize                       [IN]            缓存大小
 *  @return 成功,返回MV_SERIAL_OK,失败,返回错误码
 *  @remarks 参数类型参考MV_SERIAL_NODE_TYPE，该类型需要自行参考strKey属性节点对应的参数类型去匹配设置，接口内部不做匹配校验和限制
 *  @remarks 如"Width"匹配MV_NODE_TYPE_INT该类型值； 注：不评估其校验值范围（相机串口通信的限制）
 *  @remarks 需要参考Camera Link工业XX相机用户手册文档说明（参数名称、类型和注意事项等都需以文档串口部分的描述为准）
 ************************************************************************/
MV_SERIAL_CTRL_API int __stdcall MV_SERIAL_SetValue(IN void* handle, IN const char* strKey, IN MV_SERIAL_NODE_TYPE  enType, IN void* pBuffer, IN unsigned int nSize);

/********************************************************************//**
 *  @~chinese
 *  @brief  发送串口命令读取参数
 *  @param  handle                      [IN]            句柄
 *  @param  pCmdBuffer                  [IN]            要发送的命令
 *  @param  nCmdBufferLen               [IN]            要发送的命令的长度
 *  @param  pBuffer                     [OUT]           用于接收回复的缓存
 *  @param  nSize                       [IN]            用于接收回复的缓存大小
 *  @return 成功,返回MV_SERIAL_OK,失败,返回错误码
 *  @remarks pCmdBuffer命令最后要加上换行符\r
 ************************************************************************/
MV_SERIAL_CTRL_API int __stdcall MV_SERIAL_ReadMem(IN void* handle, IN char* pCmdBuffer, IN unsigned int nCmdBufferLen, INOUT char* pAckBuffer, IN unsigned int nAckBufferSize);

/********************************************************************//**
 *  @~chinese
 *  @brief  发送串口命令设置参数
 *  @param  handle                      [IN]            句柄
 *  @param  pCmdBuffer                  [IN]            要发送的命令
 *  @param  nCmdBufferLen               [IN]            要发送的命令的长度
 *  @param  pBuffer                     [OUT]           用于接收回复的缓存
 *  @param  nSize                       [IN]            用于接收回复的缓存大小
 *  @return 成功,返回MV_SERIAL_OK,失败,返回错误码
 *  @remarks pCmdBuffer命令最后要加上换行符\r
 ************************************************************************/
MV_SERIAL_CTRL_API int __stdcall MV_SERIAL_WriteMem(IN void* handle, IN char* pCmdBuffer, IN unsigned int nCmdBufferLen, INOUT char* pAckBuffer, IN unsigned int nAckBufferSize);

/********************************************************************//**
 *  @~chinese
 *  @brief  销毁句柄
 *  @param  handle                        [IN]          句柄
 *  @return 成功，返回MV_SERIAL_OK；失败，返回错误码 
 ************************************************************************/
MV_SERIAL_CTRL_API int __stdcall MV_SERIAL_DestroyHandle(IN void* handle);

#ifdef __cplusplus
}
#endif 

#endif // _MV_ISP_CTRL_H_
