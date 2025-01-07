/***************************************************************************************************
* 
* 版权信息：
* 
* 文件名称：ConvertPixel.cpp
* 摘    要：数据格式转换
*
* 当前版本：1.0.0.0
* 作    者：杨伟杰
* 日    期：2017-11-07
* 备    注：新建
***************************************************************************************************/
#include "stdafx.h"
#include "ConvertPixel.h"

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
int ConvertMono8ToHalcon(Halcon::Hobject *Hobj, int nHeight, int nWidth, unsigned char *pData)
{
    if(NULL == Hobj || NULL == pData)
    {
        return MV_E_PARAMETER;
    }

    gen_image1(Hobj, "byte", nWidth, nHeight, (Hlong)pData);

    return MV_OK;
}

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
int ConvertBayer8ToHalcon(Halcon::Hobject *Hobj, int nHeight, int nWidth, MvGvspPixelType nPixelType, unsigned char *pData)
{
    if(NULL == Hobj || NULL == pData)
    {
        return MV_E_PARAMETER;
    }

    gen_image1(Hobj, "byte", nWidth, nHeight, (Hlong)pData);

    if (nPixelType == PixelType_Gvsp_BayerGR8)
    {
        cfa_to_rgb(*Hobj, Hobj, "bayer_gr", "bilinear");
    }
    else if (nPixelType == PixelType_Gvsp_BayerRG8)
    {
        cfa_to_rgb(*Hobj, Hobj, "bayer_rg", "bilinear");
    }
    else if (nPixelType == PixelType_Gvsp_BayerGB8)
    {
        cfa_to_rgb(*Hobj, Hobj, "bayer_gb", "bilinear");
    }
    else if (nPixelType == PixelType_Gvsp_BayerBG8)
    {
        cfa_to_rgb(*Hobj, Hobj, "bayer_bg", "bilinear");
    }

    return MV_OK;
}

/************************************************************************
 *  @fn     ConvertRGBToHalcon()
 *  @brief  RGB转换为Halcon格式数据
 *  @param  Hobj                   [OUT]          转换后的输出Hobject数据
 *  @param  nHeight                [IN]           图像高度
 *  @param  nWidth                 [IN]           图像宽度
 *  @param  pData                  [IN]           源数据
 *  @param  pDataSeparate          [IN]           存储红绿蓝色源数据的缓冲区
 *  @return 成功，返回STATUS_OK；错误，返回STATUS_ERROR 
 ************************************************************************/
int ConvertRGBToHalcon(Halcon::Hobject *Hobj, int nHeight, int nWidth, unsigned char *pData)
{
    if(NULL == Hobj || NULL == pData)
    {
        return MV_E_PARAMETER;
    }

    gen_image_interleaved(Hobj, (Hlong)pData, "rgb", nWidth, nHeight, -1, "byte", 0, 0, 0, 0, -1, 0);

    return MV_OK;
}

/************************************************************************
 *  @fn     IsBayer8PixelFormat()
 *  @brief  判断是否是Bayer8格式
 *  @param  enType                [IN]            像素格式
 *  @return 是，返回true；否，返回false 
 ************************************************************************/
bool IsBayer8PixelFormat(MvGvspPixelType enType)
{
    switch(enType)
    {
        case PixelType_Gvsp_BayerGR8:
        case PixelType_Gvsp_BayerRG8:
        case PixelType_Gvsp_BayerGB8:
        case PixelType_Gvsp_BayerBG8:
            return true;
        default:
            return false;
    }
}

/************************************************************************
 *  @fn     IsColorPixelFormat()
 *  @brief  判断是否是彩色格式
 *  @param  enType                [IN]            像素格式
 *  @return 是，返回true；否，返回false 
 ************************************************************************/
bool IsColorPixelFormat(MvGvspPixelType enType)
{
    switch(enType)
    {
        case PixelType_Gvsp_RGB8_Packed:
        case PixelType_Gvsp_BGR8_Packed:
        case PixelType_Gvsp_RGBA8_Packed:
        case PixelType_Gvsp_BGRA8_Packed:
        case PixelType_Gvsp_YUV422_Packed:
        case PixelType_Gvsp_YUV422_YUYV_Packed:
        case PixelType_Gvsp_BayerGB10:
        case PixelType_Gvsp_BayerGB10_Packed:
        case PixelType_Gvsp_BayerBG10:
        case PixelType_Gvsp_BayerBG10_Packed:
        case PixelType_Gvsp_BayerRG10:
        case PixelType_Gvsp_BayerRG10_Packed:
        case PixelType_Gvsp_BayerGR10:
        case PixelType_Gvsp_BayerGR10_Packed:
        case PixelType_Gvsp_BayerGB12:
        case PixelType_Gvsp_BayerGB12_Packed:
        case PixelType_Gvsp_BayerBG12:
        case PixelType_Gvsp_BayerBG12_Packed:
        case PixelType_Gvsp_BayerRG12:
        case PixelType_Gvsp_BayerRG12_Packed:
        case PixelType_Gvsp_BayerGR12:
        case PixelType_Gvsp_BayerGR12_Packed:
            return true;
        default:
            return false;
    }
}

/************************************************************************
 *  @fn     IsMonoPixelFormat()
 *  @brief  判断是否是黑白格式
 *  @param  enType                [IN]            像素格式
 *  @return 是，返回true；否，返回false 
 ************************************************************************/
bool IsMonoPixelFormat(MvGvspPixelType enType)
{
    switch(enType)
    {
        case PixelType_Gvsp_Mono8:
        case PixelType_Gvsp_Mono10:
        case PixelType_Gvsp_Mono10_Packed:
        case PixelType_Gvsp_Mono12:
        case PixelType_Gvsp_Mono12_Packed:
            return true;
        default:
            return false;
    }
}

/************************************************************************
 *  @fn     HalconDisplay(HTuple *hWindow, const Halcon::Hobject &Hobj, HTuple hImageWidth, HTuple hImageHeight)
 *  @brief  Halcon图像显示
 *  @param  hWindow               [IN]        - 输入数据缓存
 *  @param  Hobj                  [IN]        - Halcon格式数据
 *  @param  hImageWidth           [IN]        - 图像宽
 *  @param  hImageHeight          [IN]        - 图像高
 *  @return 成功，返回STATUS_OK；
 ************************************************************************/
int HalconDisplay(HTuple *hWindow, const Halcon::Hobject &Hobj, const Halcon::HTuple &hImageHeight,const Halcon::HTuple &hImageWidth)
{
    if(NULL == hWindow)
    {
        return MV_E_PARAMETER;
    }
    // ch: 显示 || en:display
    set_part(*hWindow, 0, 0, hImageHeight - 1, hImageWidth - 1);
    disp_obj((const Halcon::Hobject)Hobj, *hWindow);

    return MV_OK;
}
