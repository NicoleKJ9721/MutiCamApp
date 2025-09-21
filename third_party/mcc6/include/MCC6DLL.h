#ifndef _MYCODE_H_
#define _MYCODE_H_

#include "memory"
#include "stdint.h"

#ifdef _WINDOWS
#ifdef MCCDLL_EXPORTS
#define EXPORTS_DEMO _declspec( dllexport )
#else
#define EXPORTS_DEMO _declspec(dllimport)
#endif
#else
#define EXPORTS_DEMO
#endif

typedef uint16_t      McCard_UINT16;
typedef int16_t       McCard_INT16;
typedef uint32_t      McCard_UINT32;
typedef int32_t       McCard_INT32;
typedef uint8_t       McCard_UINT8;
typedef int8_t        McCard_INT8;
typedef float         McCard_FP32;
typedef void          McCard_VOID;

/// <summary>
/// 错误码
/// </summary>
enum ErrInfo {
    funResOk = 0x01,                ///< 函数被成功执行
    funResErrAxisId = 0x02,         ///< 下传的轴序号出错
    funResErrOutGrpId = 0x03,       ///< 下传的输出的序号出错
    funResErrInGrpId = 0x04,        ///< 下传的输入的序号出错
    funResErrInterGrpId = 0x05,     ///< 下传的中断输入的序号出错
    funResErrOpenLogInfo = 0x06,    ///< 打开日志信息失败
    funResErrGCodeErr = 0x07,       ///< G代码内容有误
    funResErrGCodeDecode = 0x8,     ///< G代码解析有误
    funResOpenPortErr = 0x80,       ///< 打开串口失败
    funCallbackOk = 0x81,           ///< 中品回调函数成功
    funCallbackErr = 0x82,          ///< 串口回调函数失败
    funResErr = 0x83,
};

#define MAX_AXIS_NUM        0x06        ///< DLL 支持的最大轴数
#define PARA_CNT_PER_AXIS   0x10        ///< 每个轴支持的参数个数

// MCDLL 库函数
class EXPORTS_DEMO MoCtrCard
{
public:
    McCard_UINT32 const Axis1IntrpltSttBit = 0x01;
    McCard_UINT32 const Axis1DelaySttBit = 0x02;        ///< 第一轴的延时状态位
    McCard_UINT32 const Axis1RunSttBit = 0x04;      ///< 第一轴运行状态位

    McCard_UINT32 const Axis2IntrpltSttBit = 0x10;      ///< 第二轴的插补状态位
    McCard_UINT32 const Axis2DelaySttBit = 0x20;        ///< 第二轴的延时状态位
    McCard_UINT32 const Axis2RunSttBit = 0x40;      ///< 第二轴运行状态位

    McCard_UINT32 const Axis3IntrpltSttBit = 0x100; ///< 第三轴的插补状态位
    McCard_UINT32 const Axis3DelaySttBit = 0x200;   ///< 第三轴的延时状态位
    McCard_UINT32 const Axis3RunSttBit = 0x400; ///< 第三轴运行状态位

    McCard_UINT32 const Axis4IntrpltSttBit = 0x1000;    ///< 第四轴的插补状态位
    McCard_UINT32 const Axis4DelaySttBit = 0x2000;  ///< 第四轴的延时状态位
    McCard_UINT32 const Axis4RunSttBit = 0x4000;    ///< 第四轴运行状态位

    McCard_UINT32 const Axis5IntrpltSttBit = 0x10000;   ///< 第五轴的插补状态位
    McCard_UINT32 const Axis5DelaySttBit = 0x20000; ///< 第五轴的延时状态位
    McCard_UINT32 const Axis5RunSttBit = 0x40000;   ///< 第五轴运行状态位

    McCard_UINT32 const Axis6IntrpltSttBit = 0x100000;  ///< 第六轴的插补状态位
    McCard_UINT32 const Axis6DelaySttBit = 0x200000;    ///< 第六轴的延时状态位
    McCard_UINT32 const Axis6RunSttBit = 0x400000;  ///< 第六轴运行状态位

    McCard_UINT32 const GrpIntrpltSttBit = 0x1000000;  ///< l轴组的插补状态位
    McCard_UINT32 const GrpDelaySttBit = 0x2000000;  ///< l轴组的插补状态位
    McCard_UINT32 const GrpRunSttBit = 0x4000000;  ///< l轴组的插补状态位

    McCard_UINT32 const NcCtrlStt = 0x10000000; ///< NC 控制器的控制状态，为1时表示是联动，为0时表示单轴运动

    McCard_UINT32 const FunResOK = 1;
    McCard_UINT32 const FunResErr = 2;

public:
    MoCtrCard();
    ~MoCtrCard();

    /// <summary>
    /// 查询轴位置函数
    /// </summary>
    /// <param name="AxisId">轴ID号，0-5或0xFF，0xFF返回所有轴信息</param>
    /// <param name="ResPos">输出参数，轴位置</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_GetAxisPos(McCard_UINT8 AxisId, McCard_FP32 ResPos[]);

    /// <summary>
    /// 查询轴实际位置函数
    /// </summary>
    /// <param name="AxisId">轴ID号，0-5或0xFF，0xFF返回所有轴信息</param>
    /// <param name="ResPos">输出参数，轴位置</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_GetAxisActualPos(McCard_UINT8 AxisId, McCard_FP32 ResPos[]);

    /// <summary>
    /// 查询轴速度函数
    /// </summary>
    /// <param name="AxisId">轴ID号，0-5或0xFF，0xFF返回所有轴信息</param>
    /// <param name="ResSpd">输出参数，轴速度</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_GetAxisSpd(McCard_UINT8 AxisId, McCard_FP32 ResSpd[]);

    /// <summary>
    /// 查询所有轴运行状态函数
    /// </summary>
    /// <param name="ResStt">输出参数，所有轴状态</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_GetRunState(McCard_INT32 ResStt[]);

    /// <summary>
    /// 查询所有轴对应的摇杆接口的模拟量值
    /// </summary>
    /// <param name="AxisId">轴ID号，0-5或0xFF，0xFF返回所有轴信息</param>
    /// <param name="ResAd">输出参数，模拟量的值</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_GetAdVal(McCard_UINT8 AxisId, McCard_INT32 ResAd[]);

    /// <summary>
    /// 查询输出端口的状态
    /// </summary>
    /// <param name="OutGrpIndx">0</param>
    /// <param name="ResOut">输出参数，每一BIT对应一个输出口</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_GetOutState(McCard_UINT8 OutGrpIndx, McCard_UINT32 ResOut[]);

    /// <summary>
    /// 查询输入端口的状态
    /// </summary>
    /// <param name="InGrpIndx">0</param>
    /// <param name="ResIn">输出参数，每一BIT对应一个输入口</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_GetInputState(McCard_UINT8 InGrpIndx, McCard_UINT32 ResIn[]);

    /// <summary>
    /// 查询中断式输入的状态，该函数保留，暂不支持
    /// </summary>
    /// <param name="InGrpIndx">0</param>
    /// <param name="ResIn">输出参数，每一BIT对应一个中断式输入</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_GetIntInputState(McCard_UINT8 InGrpIndx, McCard_UINT32 ResIn[]);

    /// <summary>
    /// 查询控制器的软件版本信息
    /// </summary>
    /// <param name="ResVer">输出参数，软件版本信息</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_GetCardVersion(McCard_UINT32 ResVer[]);

    /// <summary>
    /// 查询编码器值
    /// </summary>
    /// <param name="AxisId">轴ID号，0-5或0xFF，0xFF返回所有轴信息</param>
    /// <param name="EncoderPos">输出参数，编码器值</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_GetEncoderVal(McCard_UINT8 AxisId, McCard_INT32 EncoderPos[]);

    /// <summary>
    /// 查询控制器硬件信息，内部使用，用户请误调用
    /// </summary>
    /// <param name="HardInfo">输出参数，硬件信息</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_GetBoardHardInfo(McCard_UINT32 HardInfo[]);

    /// <summary>
    /// 查询DLL的版本信息
    /// </summary>
    /// <returns>DLL的版本信息</returns>
    McCard_UINT32 MoCtrCard_GetDLLVersion(McCard_VOID);

    /// <summary>
    /// 获取动态补偿的脉冲差
    /// </summary>
    /// <param name="AxisId">轴的ID号，0-5</param>
    /// <param name="ResDif">脉冲差</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_GetComDiff(McCard_UINT8 AxisId, McCard_INT32 ResDif[]);

    /// <summary>
    /// 获取各个轴的回零状态
    /// </summary>
    /// <param name="HomeState">回零状态</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_GetHomeState(McCard_UINT32 HomeState[]);

    /// <summary>
    /// 获取指定轴的回零状态
    /// </summary>
    /// <param name="AxisId">轴的ID号，0-5</param>
    /// <param name="HomeState">回零状态</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_GetHomeState(McCard_UINT8 AxisId, McCard_UINT32 HomeState[]);

    /// <summary>
    /// 获取轴的运动状态
    /// </summary>
    /// <param name="AxisId">轴号，0-X轴，1-Y轴，2-Z轴，3-A轴，4-B轴，5-C轴</param>
    /// <param name="outRunning">轴运动状态输出参数，0-停止，1-运动中</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_IsAxisRunning(McCard_UINT8 AxisId, McCard_INT32 outRunning[]);

    /// <summary>
    /// 设置参数接口
    /// </summary>
    /// <param name="AxisId">轴的ID号，0-5</param>
    /// <param name="ParaIndx">参数序号</param>
    /// <param name="ParaVal">参数值</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_SendPara(McCard_UINT8 AxisId, McCard_UINT8 ParaIndx, McCard_FP32 ParaVal);
    McCard_UINT16 MoCtrCard_SendPara(McCard_UINT8 AxisId, McCard_UINT8 ParaIndx, McCard_UINT32 ParaVal);
    McCard_UINT16 MoCtrCard_SendPara(McCard_UINT8 AxisId, McCard_UINT8 ParaIndx, McCard_INT32 ParaVal);
    McCard_UINT16 MoCtrCard_SendPara(McCard_UINT8 AxisId, McCard_UINT8 ParaIndx, McCard_INT8 ParaVal);

    /// <summary>
    /// 控制轴以内部参数定义的速度向某个方向运动“手动距离”定义的距离
    /// </summary>
    /// <param name="AxisId">轴的ID号，0-5</param>
    /// <param name="SpdDir">运动方向，1-正向，-1-负向</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_MCrlAxisMove(McCard_UINT8 AxisId, McCard_INT8 SpdDir);

    /// <summary>
    /// 控制轴以相对运动方式移动
    /// </summary>
    /// <param name="AxisId">轴的ID号，0-5</param>
    /// <param name="DistCmnd">相对运动距离，>0正向，<0负向</param>
    /// <param name="VCmnd">指令速度</param>
    /// <param name="ACmnd">指令加速度</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_MCrlAxisRelMove(McCard_UINT8 AxisId, McCard_FP32 DistCmnd, McCard_FP32 VCmnd, McCard_FP32 ACmnd = 0.0f);

    /// <summary>
    /// 控制轴以绝对运动方式移动
    /// </summary>
    /// <param name="AxisId">轴的ID号，0-5</param>
    /// <param name="PosCmnd">绝对位置</param>
    /// <param name="VCmnd">指令速度</param>
    /// <param name="ACmnd">指令加速度</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_MCrlAxisAbsMove(McCard_UINT8 AxisId, McCard_FP32 PosCmnd, McCard_FP32 VCmnd, McCard_FP32 ACmnd = 0.0f);

    /// <summary>
    /// 控制轴寻找外部零点开关
    /// </summary>
    /// <param name="AxisId">轴的ID号，0-5</param>
    /// <param name="VCmnd">指令速度，>0正向寻零，<0负向寻零</param>
    /// <param name="ACmnd">指令加速度</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_SeekZero(McCard_UINT8 AxisId, McCard_FP32 VCmnd, McCard_FP32 ACmnd = 0.0f);

    /// <summary>
    /// 停止轴的寻零过程
    /// </summary>
    /// <param name="AxisId">轴的ID号，0-5</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_CancelSeekZero(McCard_UINT8 AxisId);

    /// <summary>
    /// 暂时轴当前的运动
    /// </summary>
    /// <param name="AxisId">轴的ID号，0-5</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_PauseAxisMov(McCard_UINT8 AxisId);

    /// <summary>
    /// 重新开始轴被暂停的运动
    /// </summary>
    /// <param name="AxisId">轴的ID号，0-5</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_ReStartAxisMov(McCard_UINT8 AxisId);

    /// <summary>
    /// 停止轴
    /// </summary>
    /// <param name="AxisId">轴的ID号，0-5</param>
    /// <param name="fAcc">停止轴时的减速度</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_StopAxisMov(McCard_UINT8 AxisId, McCard_FP32 fAcc);

    /// <summary>
    /// 急停止轴，无减速过程
    /// </summary>
    /// <param name="AxisId">轴的ID号，0-5</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_EmergencyStopAxisMov(McCard_UINT8 AxisId);

    /// <summary>
    /// 退出轴控制状态，调用后运动的轴急停止，轴的控制状态复位
    /// </summary>
    /// <param name="">无</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_QuiteMotionControl(McCard_VOID);

    /// <summary>
    /// 在轴的运动过程中可以改变指令速度和加速度
    /// </summary>
    /// <param name="AxisId">轴的ID号，0-5</param>
    /// <param name="fVel">指令速度</param>
    /// <param name="fAcc">指令加速度</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_ChangeAxisMovPara(McCard_UINT8 AxisId, McCard_FP32 fVel, McCard_FP32 fAcc);

    /// <summary>
    /// 控制轴开始简协运动
    /// </summary>
    /// <param name="AxisId">轴的ID号，0-5</param>
    /// <param name="bRelAbs">运动模式，0-绝对，1-相对</param>
    /// <param name="fA">振幅</param>
    /// <param name="fW">周期</param>
    /// <param name="fPhase">初始相位</param>
    /// <param name="nCycle">工作循环数，0-无限运动</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_StartSimpleHarmonicMove(McCard_UINT8 AxisId, McCard_UINT8 bRelAbs, McCard_FP32 fA, McCard_FP32 fW, McCard_FP32 fPhase, McCard_UINT32 nCycle);

    /// <summary>
    /// 停止简协运动
    /// </summary>
    /// <param name="AxisId">轴的ID号，0-5</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_StopSimpleHarmonicMove(McCard_UINT8 AxisId);

    /// <summary>
    /// 设置输出
    /// </summary>
    /// <param name="OutputIndex">输出序号，0-15</param>
    /// <param name="OutputVal">输出值，0-关闭，1-打开</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_SetOutput(McCard_UINT8 OutputIndex, McCard_UINT8 OutputVal);

    /// <summary>
    /// 使能手柄功能
    /// </summary>
    /// <param name="AxisId">轴的ID号，0-5</param>
    /// <param name="bEnable">0-关闭手柄功能，1-使能手柄功能</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_SetJoyStickEnable(McCard_UINT8 AxisId, McCard_UINT8 bEnable);

    /// <summary>
    /// 设置轴相关信号极性或功能开关
    /// </summary>
    /// <param name="AxisId">轴的ID号，0-5</param>
    /// <param name="InputType">1-硬限位功能，2-软限位功能，3-补偿功能，4-硬限位信号极性，5-软限位极性</param>
    /// <param name="OpenOrClose">0-关闭（常开），1-打开（关闭）</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_SetAxisRealtiveInputPole(McCard_UINT8 AxisId, McCard_UINT8 InputType, McCard_UINT8 OpenOrClose);

    /// <summary>
    /// 使能补偿功能
    /// </summary>
    /// <param name="AxisId">轴的ID号，0-5</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_EnableCompensateFunction(McCard_UINT8 AxisId);

    /// <summary>
    /// 关闭补偿功能
    /// </summary>
    /// <param name="AxisId">轴的ID号，0-5</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_DisableCompensateFunction(McCard_UINT8 AxisId);

    /// <summary>
    /// 读轴参数
    /// </summary>
    /// <param name="AxisId">轴的ID号，0-5</param>
    /// <param name="ParaIndx">参数序号</param>
    /// <param name="ParaVal">输出参数，参数值</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_ReadPara(McCard_UINT8 AxisId, McCard_UINT8 ParaIndx, McCard_FP32 ParaVal[]);
    McCard_UINT16 MoCtrCard_ReadPara(McCard_UINT8 AxisId, McCard_UINT8 ParaIndx, McCard_INT8* ParaVal);

    /// <summary>
    /// 复位轴的指令位置
    /// </summary>
    /// <param name="AxisId">轴的ID号，0-5</param>
    /// <param name="PosRest">轴的复位位置</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_ResetCoordinate(McCard_UINT8 AxisId, McCard_FP32 PosRest);

    /// <summary>
    /// 将轴参数保存到控制存储区，掉电保存
    /// </summary>
    /// <param name="">无</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_SaveSystemParaToROM(McCard_VOID);

    /// <summary>
    /// 设置轴的编码器值
    /// </summary>
    /// <param name="AxisId">轴的ID号，0-5</param>
    /// <param name="EncoderPos">轴的编码器的设定值</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_SetEncoderPos(McCard_UINT8 AxisId, McCard_INT32 EncoderPos);

    /// <summary>
    /// 复位编码器的Z信号，此功能保留，暂不支持
    /// </summary>
    /// <param name="AxisId">轴的ID号，0-5</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_RstZ(McCard_UINT8 AxisId);

    /// <summary>
    /// 设置轮廓运动曲线参数
    /// </summary>
    /// <param name="AxisId">轴号，0-5</param>
    /// <param name="ProfileIndx">轮廓运动曲线参数序号</param>
    /// <param name="fPara">轮廓曲线参数</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_SetProfileCurvePara(McCard_UINT8 AxisId, McCard_UINT8 ProfileIndx, McCard_FP32 fPara[]);

    /// <summary>
    /// 设置轮廓运动参数
    /// </summary>
    /// <param name="AxisId">轴号，0-5</param>
    /// <param name="nMaxSec">轮廓曲线运动有效运动效数</param>
    /// <param name="nT">轮廓运动的周期，指个运动覆盖的距离或角度</param>
    /// <param name="nWorkT">轮廓曲线运动的循环数，如要为周期类型，该参数指令运动的循环数</param>
    /// <param name="nPeriodicType">轮廓曲线类型，0-非周期，线性；1-周期</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_SetProfilePara(McCard_UINT8 AxisId, McCard_UINT16 nMaxSec, McCard_FP32 nT, McCard_UINT16 nWorkT, McCard_UINT16 nPeriodicType);

    /// <summary>
    /// 启动轮廓曲线运动
    /// </summary>
    /// <param name="AxisId">轴号，0-5</param>
    /// <param name="MoveDir">轮廓曲线运动方向，1-正方向，0-负方向</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_StartProfileMove(McCard_UINT8 AxisId, McCard_UINT8 MoveDir);

    /// <summary>
    /// 停止轮廓曲线运动
    /// </summary>
    /// <param name="AxisId">轴号，0-5</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_StopProfileMove(McCard_UINT8 AxisId);

    /// <summary>
    /// 以正弦规律运动至绝对位置，实际轴的摆动距离与当前的轴指令位置成正弦关系
    /// </summary>
    /// <param name="AxisId">轴号，0-5</param>
    /// <param name="fPos">轴的终点位置</param>
    /// <param name="fSpd">轴的指令速度</param>
    /// <param name="fAcc">轴的指令加速度</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_MCrlAxisSinAbsMove(McCard_UINT8 AxisId, McCard_FP32 fCoefA, McCard_FP32 fPos, McCard_FP32 fSpd = 0.0f, McCard_FP32 fAcc = 0.0f);

    /// <summary>
    /// 以正弦规律运动至相对位置，实际轴的摆动距离与当前的轴指令位置成正弦关系
    /// </summary>
    /// <param name="AxisId">轴号，0-5</param>
    /// <param name="fPos">轴的终点位置</param>
    /// <param name="fSpd">轴的指令速度</param>
    /// <param name="fAcc">轴的指令加速度</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_MCrlAxisSinRelMove(McCard_UINT8 AxisId, McCard_FP32 fCoefA, McCard_FP32 fPos, McCard_FP32 fSpd, McCard_FP32 fAcc);

    /// <summary>
    /// 以速度方式运动
    /// </summary>
    /// <param name="AxisId">轴号，0-5</param>
    /// <param name="fSpd">指令速度</param>
    /// <param name="fAcc">指令加速度，为0.0时，表示使用默认的加速度，为加速度参数定义的加速度</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_MCrlAxisMoveAtSpd(McCard_UINT8 AxisId, McCard_FP32 fSpd, McCard_FP32 fAcc = 0.0f);

    // Master and Slave Mode
    /// <summary>
    /// 设置电子齿轮参数
    /// </summary>
    /// <param name="slvId">从轴ID号，0-5</param>
    /// <param name="mstrId">主轴ID号，0-5</param>
    /// <param name="fGear">电子齿轮比，从轴比主轴</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_SetGearPara(McCard_UINT8 slvId, McCard_UINT8 mstrId, McCard_FP32 fGear);

    /// <summary>
    /// 电子齿轮操作指令
    /// </summary>
    /// <param name="slvId">从轴ID号，0-5</param>
    /// <param name="bGearCmndr">操作指令，1-启动电子齿轮耦合状态，0-退出电子齿轮耦合状态</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_ManGearOp(McCard_UINT8 slvId, McCard_UINT8 bGearCmndr);

    /// <summary>
    /// 启动电子齿轮耦合
    /// </summary>
    /// <param name="slvId">从轴ID号，0-5</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_ManGearStart(McCard_UINT8 slvId);

    /// <summary>
    /// 停止电子齿轮耦合
    /// </summary>
    /// <param name="slvId">从轴ID号，0-5</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_ManGearStop(McCard_UINT8 slvId);

    /// <summary>
    /// 轴组以绝对方式运动，同起同停
    /// </summary>
    /// <param name="bAxisEn">轴使能，非0-使能该轴</param>
    /// <param name="fPos">指令位置</param>
    /// <param name="fSpd">指令速度</param>
    /// <param name="fAcc">指令加速度，缺省为0.0</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_MCrlGroupAbsMove(McCard_UINT8 bAxisEn[], McCard_FP32 fPos[], McCard_FP32 fSpd, McCard_FP32 fAcc = 0.0);

    /// <summary>
    /// 轴组以相对方式运动，同起同停
    /// </summary>
    /// <param name="bAxisEn">轴使能，非0-使能该轴</param>
    /// <param name="fDist">指令距离</param>
    /// <param name="fSpd">指令速度</param>
    /// <param name="fAcc">指令加速度，缺省为0.0</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_MCrlGroupRelMove(McCard_UINT8 bAxisEn[], McCard_FP32 fDist[], McCard_FP32 fSpd, McCard_FP32 fAcc = 0.0);

    /// <summary>
    /// 轴组以绝对方式运动，同起不同停
    /// </summary>
    /// <param name="bAxisEn">轴使能，非0-使能该轴</param>
    /// <param name="fPos">指令位置</param>
    /// <param name="fSpd">指令速度</param>
    /// <param name="fAcc">指令加速度，缺省为不指定加速度，按系统的加速度参数执行</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_MCrlGroupAbsMovePTP(McCard_UINT8 bAxisEn[], McCard_FP32 fPos[], McCard_FP32 fSpd[], McCard_FP32 fAcc[] = NULL);

    /// <summary>
    /// 轴组以相对方式运动，同起不同停
    /// </summary>
    /// <param name="bAxisEn">轴使能，非0-使能该轴</param>
    /// <param name="fDist">指令距离</param>
    /// <param name="fSpd">指令速度</param>
    /// <param name="fAcc">指令加速度，缺省为不指定加速度，按系统的加速度参数执行</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_MCrlGroupRelMovePTP(McCard_UINT8 bAxisEn[], McCard_FP32 fDist[], McCard_FP32 fSpd[], McCard_FP32 fAcc[] = NULL);

    /// <summary>
    /// 轴组以速度方式运动
    /// </summary>
    /// <param name="bAxisEn">轴使能，非0-使能该轴</param>
    /// <param name="fSpd">指令速度</param>
    /// <param name="fAcc">指令加速度</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_MCrlGroupSpdMove(McCard_UINT8 bAxisEn[], McCard_FP32 fSpd[], McCard_FP32 fAcc[]);

    /// <summary>
    /// 测试功能
    /// </summary>
    /// <param name="TestVar">输出参数，值为1、2、3、4</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_Test(McCard_INT32 TestVar[]);

    /// <summary>
    /// 设置控制卡相对运动，在运动过程中可以在指定位置产生脉冲信号
    /// </summary>
    /// <param name="AxisId">轴号，0-X轴，1-Y轴，2-Z轴，3-A轴，4-B轴，5-C轴</param>
    /// <param name="spdDir">运动方向，0-正向，1-负向</param>
    /// <param name="outputIndex">输出序号，0-15</param>
    /// <param name="fDist">运动的距离，正数</param>
    /// <param name="fDistHead">运动的头距离，该距离内是不产生脉冲的，正数</param>
    /// <param name="fDistTail">运动的尾距离，该距离内是不产生脉冲的，正数</param>
    /// <param name="fVel">运动的指令速度</param>
    /// <param name="fDistPulse">产生脉冲的间隔</param>
    /// <param name="nPluseWidth">产生脉冲的宽度</param>
    /// <param name="nMode">脉冲宽度模式，0：周期，1：us</param>
    /// <returns>函数执行是否成功</returns>
    McCard_UINT16 MoCtrCard_MCrlAxisRelMoveAndPulse(McCard_UINT8 AxisId, McCard_UINT8 spdDir, McCard_UINT8 outputIndex, float fDist, float fDistHead, float fDistTail, float fVel, float fDistPulse, McCard_UINT8 nPluseWidth, McCard_UINT8 nMode);

    /// <summary>
    /// 设置控制卡绝对运动，在运动过程中可以在指定位置产生脉冲信号
    /// </summary>
    /// <param name="AxisId">轴号，0-X轴，1-Y轴，2-Z轴，3-A轴，4-B轴，5-C轴</param>
    /// <param name="outputIndex">输出序号，0-15</param>
    /// <param name="fPos">运动的终点位置</param>
    /// <param name="fDistHead">运动的头距离，该距离内是不产生脉冲的，正数</param>
    /// <param name="fDistTail">运动的尾距离，该距离内是不产生脉冲的，正数</param>
    /// <param name="fVel">运动的指令速度</param>
    /// <param name="fDistPulse">产生脉冲的间隔</param>
    /// <param name="nPluseWidth">产生脉冲的宽度</param>
    /// <param name="nMode">脉冲宽度模式，0：周期，1：us</param>
    /// <returns>函数执行是否成功</returns>
    McCard_UINT16 MoCtrCard_MCrlAxisAbsMoveAndPulse(McCard_UINT8 AxisId, McCard_UINT8 outputIndex, float fPos, float fDistHead, float fDistTail, float fVel, float fDistPulse, McCard_UINT8 nPluseWidth, McCard_UINT8 nMode);

    /// <summary>
    /// 设置控制卡按位置产生脉冲信号的参数，只设参数，不产生轴运动
    /// </summary>
    /// <param name="AxisId">轴号，0-X轴，1-Y轴，2-Z轴，3-A轴，4-B轴，5-C轴</param>
    /// <param name="spdDir">运动方向，0-正向，1-负向</param>
    /// <param name="outputIndex">输出序号，0-15</param>
    /// <param name="fDist">运动的距离，正数</param>
    /// <param name="fDistHead">运动的头距离，该距离内是不产生脉冲的，正数</param>
    /// <param name="fDistTail">运动的尾距离，该距离内是不产生脉冲的，正数</param>
    /// <param name="fVel">运动的指令速度</param>
    /// <param name="fDistPulse">产生脉冲的间隔</param>
    /// <param name="nPluseWidth">产生脉冲的宽度</param>
    /// <param name="nMode">脉冲宽度模式，0：周期，1：us</param>
    /// <returns>函数执行是否成功</returns>
    McCard_UINT16 MoCtrCard_SetAxisTriggerParameter(McCard_UINT8 AxisId, McCard_UINT8 spdDir, McCard_UINT8 outputIndex, float fDist, float fDistHead, float fDistTail, float fVel, float fDistPulse, McCard_UINT8 nPluseWidth, McCard_UINT8 nMode);

    /// <summary>
    /// 设置控制卡相对运动，在运动过程中可以在指定位置产生脉冲信号
    /// </summary>
    /// <param name="AxisId">轴号，0-X轴，1-Y轴，2-Z轴，3-A轴，4-B轴，5-C轴</param>
    /// <returns>函数执行是否成功</returns>
    McCard_UINT16 MoCtrCard_StopAxisRelMoveAndPulse(McCard_UINT8 AxisId);

    /// <summary>
    /// 获取脉冲式运动已经触发的脉冲数
    /// </summary>
    /// <param name="AxisId">轴号，0-X轴，1-Y轴，2-Z轴，3-A轴，4-B轴，5-C轴</param>
    /// <param name="nCnt">已经触发的脉冲数</param>
    /// <returns>函数执行是否成功</returns>
    McCard_UINT16 MoCtrCard_GetPulseMoveCount(McCard_UINT8 AxisId, McCard_INT32 nCnt[]);

    /// <summary>
    /// 获取脉冲式运动触发脉冲数的位置
    /// </summary>
    /// <param name="AxisId">轴号，0-X轴，1-Y轴，2-Z轴，3-A轴，4-B轴，5-C轴</param>
    /// <param name="nCnt">触发脉冲产生的位置</param>
    /// <returns>函数执行是否成功</returns>
    McCard_UINT16 MoCtrCard_GetPulseMovePos(McCard_UINT8 AxisId, McCard_FP32 fPos[]);

    // TCPIP 接口

    /// <summary>
    /// 初始化运动控制器，网口
    /// </summary>
    /// <param name="pIPAddr">控制器的IP地址，"192.168.0.100"</param>
    /// <param name="nPort">控制器的端口号</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_Net_Initial(char* pIPAddr, McCard_INT32 nPort);

    /// <summary>
    /// 初始化运动控制器，串口
    /// </summary>
    /// <param name="ComPort">串口号</param>
    /// <returns></returns>
#ifdef _WINDOWS
    McCard_UINT16 MoCtrCard_Initial(McCard_UINT8 ComPort);
#else
    McCard_UINT16 MoCtrCard_Initial(char* pComName);
#endif
    /// <summary>
    /// 卸载运动控制器
    /// </summary>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_Unload();

    /// <summary>
    /// 获取通讯状态
    /// </summary>
    /// <param name="CommStt">输出参数，0-关闭，1-通讯中</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_GetCommState(McCard_INT32 CommStt[]);

    // MDI指令

    /// <summary>
    /// 按自定义控制指令向控制器发送指令，具体格式参考MCCDEMO的编程功能
    /// </summary>
    /// <param name="MDICmndStr">输入的字符串指令</param>
    /// <returns>7，指令字符有误；8，指令解析有误</returns>
    McCard_UINT16 MoCtrCard_SendMDICommand(char MDICmndStr[]);

    /// <summary>
    /// 获取通讯联接状态
    /// </summary>
    /// <param name="nState">非0表示处于联接状态</param>
    /// <returns></returns>
    McCard_UINT16 MoCtrCard_GetLinkState(McCard_UINT8 nState[]);

private:
    class MoCtrCardImpl;
    std::unique_ptr<MoCtrCardImpl> m_pImpl;
};

#endif