
工业相机c# 二次开发示例文件说明
===========================================================================
版本号： 4.3.0

支持相机：GigE相机和U3V相机

支持系统：windows 7(32和64位)、windows 10(32和64位)、windows11(64位)  

c#支持两种二次开发的方式：
  一、直接调用dll(MvCameraControl.Net.dll)；
  二、直接引用源码文件MVCamera.cs(MVCameraSDK\Source\)；
详细如下：
===========================================================================

..\Samples\C#\MvCameraControlNet 目录结构如下：
	MVCameraSDK
	|-..\..\..\..\DotNet			：生成MvCameraControl.Net.dll文件，直接可用dll文件进行二次开发；
	|-Source		                ：引用MVCamera.cs文件，即可进行二次开发；
	
	
	
===========================================================================


Machine Vision Camera Windows SDK  c# User Manual
===========================================================================
Version: 4.3.0

Camera supported : GigE and USB3 Camera

OS supported: Windows7(32/64 bits), Windows10 (32/64 bits), windows11 (64 bits)
===========================================================================
  
  
c# supports two ways of Secondary Development
   1.call DLL;
   2.Reference the source code file MVCamera.cs;
Details are as follows:  
===========================================================================

..\Samples\C#\MvCameraControlNet Directory：
	MVCameraSDK
	|-..\..\..\..\DotNet			   :Generate  MvCameraControl.Net.dll，referenced for secondary development；
	|-Source		   
	 -MVCamera.cs                      :MVCamera.cs,Reference this file can be secondary development;
	
	MVCameraSDK_DotNet
	|-Source		   
	 -MVCamera.cs      :MVCamera.cs,Reference this file can be secondary development;
	|- other files     :other files；
	
	
===========================================================================


