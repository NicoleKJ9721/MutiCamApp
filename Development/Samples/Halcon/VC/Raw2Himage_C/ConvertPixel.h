#include <HalconCpp.h>
#include <HalconCDefs.h>
#include <HProto.h>
#include <HWindow.h>
#include <stdio.h>
#include "MvCamera.h"

using namespace Halcon;

/************************************************************************
 *  @fn     ConvertMono8ToHalcon()
 *  @brief  Mono8转换为Halcon格式数据
 *  @param  Hobj                   [OUT]          转换后的输出Hobject数据
 *  @param  nHeight                [IN]           图像高度
 *  @param  nWidth                 [IN]           图像宽度
 *  @param  pData                  [IN]           源数据
 *  @return 成功，返回STATUS_OK；错误，返回STATUS_ERROR 
 ************************************************************************/
int ConvertMono8ToHalcon(Halcon::Hobject *Hobj, int nHeight, int nWidth, unsigned char *pData);

/************************************************************************
 *  @fn     ConvertBayer8ToHalcon()
 *  @brief  Bayer8转换为Halcon格式数据
 *  @param  Hobj                   [OUT]          转换后的输出Hobject数据
 *  @param  nHeight                [IN]           图像高度
 *  @param  nWidth                 [IN]           图像宽度
 *  @param  nPixelType             [IN]           源数据格式
 *  @param  pData                  [IN]           源数据
 *  @return 成功，返回STATUS_OK；错误，返回STATUS_ERROR 
 ************************************************************************/
int ConvertBayer8ToHalcon(Halcon::Hobject *Hobj, int nHeight, int nWidth, MvGvspPixelType nPixelType, unsigned char *pData);

/************************************************************************
 *  @fn     ConvertRGBToHalcon()
 *  @brief  RGB转换为Halcon格式数据
 *  @param  Hobj                   [OUT]          转换后的输出Hobject数据
 *  @param  nHeight                [IN]           图像高度
 *  @param  nWidth                 [IN]           图像宽度
 *  @param  pData                  [IN]           源数据
 *  @return 成功，返回STATUS_OK；错误，返回STATUS_ERROR 
 ************************************************************************/
int ConvertRGBToHalcon(Halcon::Hobject *Hobj, int nHeight, int nWidth,
                          unsigned char *pData);

/************************************************************************
 *  @fn     IsBayer8PixelFormat()
 *  @brief  判断是否是Bayer8格式
 *  @param  enType                [IN]            像素格式
 *  @return 是，返回true；否，返回false 
 ************************************************************************/
bool IsBayer8PixelFormat(MvGvspPixelType enType);

/************************************************************************
 *  @fn     IsColorPixelFormat()
 *  @brief  判断是否是彩色格式
 *  @param  enType                [IN]            像素格式
 *  @return 是，返回true；否，返回false 
 ************************************************************************/
bool IsColorPixelFormat(MvGvspPixelType enType);

/************************************************************************
 *  @fn     IsMonoPixelFormat()
 *  @brief  判断是否是黑白格式
 *  @param  enType                [IN]            像素格式
 *  @return 是，返回true；否，返回false 
 ************************************************************************/
bool IsMonoPixelFormat(MvGvspPixelType enType);

/************************************************************************
 *  @fn     HalconDisplay(HTuple *hWindow, const Halcon::Hobject &Hobj, HTuple hImageWidth, HTuple hImageHeight)
 *  @brief  Halcon图像显示
 *  @param  hWindow               [IN]        - 输入数据缓存
 *  @param  Hobj                  [IN]        - Halcon格式数据
 *  @param  hImageWidth           [IN]        - 图像宽
 *  @param  hImageHeight          [IN]        - 图像高
 *  @return 成功，返回STATUS_OK；
 ************************************************************************/
int HalconDisplay(HTuple *hWindow, const Halcon::Hobject &Hobj, const Halcon::HTuple &hImageWidth, const Halcon::HTuple &hImageHeight);
