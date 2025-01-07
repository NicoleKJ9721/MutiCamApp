#ifndef _MV_SERIAL_ERROR_DEFINE_
#define _MV_SERIAL_ERROR_DEFINE_

#define MV_SERIAL_OK                   0x0

#define MV_SERIAL_E_HANDLE             0x80000000  ///< 错误或无效的句柄
#define MV_SERIAL_E_SUPPORT            0x80000001  ///< 不支持的功能
#define MV_SERIAL_E_BUFOVER            0x80000002  ///< 缓存已满
#define MV_SERIAL_E_CALLORDER          0x80000003  ///< 函数调用顺序错误
#define MV_SERIAL_E_PARAMETER          0x80000004  ///< 错误的参数
#define MV_SERIAL_E_RESOURCE           0x80000006  ///< 资源申请失败
#define MV_SERIAL_E_NODATA             0x80000007  ///< 无数据
#define MV_SERIAL_E_PRECONDITION       0x80000008  ///< 前置条件有误，或运行环境已发生变化
#define MV_SERIAL_E_VERSION            0x80000009  ///< 版本不匹配
#define MV_SERIAL_E_NOENOUGH_BUF       0x8000000A  ///< 传入的内存空间不足
#define MV_SERIAL_E_UNKNOW             0x800000FF  ///< 未知的错误

//网络相关错误: 0x80000100-0x800001FF
#define MV_SERIAL_E_CREAT_SOCKET       0x80000100	///< 创建Socket错误
#define MV_SERIAL_E_BIND_SOCKET        0x80000101	///< 绑定错误
#define MV_SERIAL_E_CONNECT_SOCKET     0x80000102	///< 连接错误
#define MV_SERIAL_E_GET_HOSTNAME       0x80000103	///< 获取主机名错误
#define MV_SERIAL_E_NET_WRITE          0x80000104	///< 写入数据错误
#define MV_SERIAL_E_NET_READ           0x80000105	///< 读取数据错误
#define MV_SERIAL_E_NET_SELECT         0x80000106	///< Select错误
#define MV_SERIAL_E_NET_TIMEOUT        0x80000107	///< 超时


#endif