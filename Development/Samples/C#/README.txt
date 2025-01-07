工业相机SDK提供了两套.Net接口，命名空间分别为：
1. namespace MvCamCtrl.NET：	示例程序位于 MvCameraControlNet 文件夹下。
2. namespace MvCameraControl：	示例程序位于 MvCameraControlNet_V2 文件夹下。

推荐使用 namespace MvCameraControl 的接口


namespace MvCamCtrl.NET：
1. 基于 .Net Framework 3.5 开发，演示如何将SDK的C语言接口封装成.Net接口，再调用封装的.Net接口实现SDK功能
2. 源码位于 MvCameraControlNet\MVCameraSDK\Source\MVCamera.cs


namespace MvCameraControl：
1. 基于 .Net Framework 4.0 开发，是SDK提供的新的.Net程序集（MvCameraControl.Net.dll）
2. 库文件位于客户端安装路径下的 Development\DotNet\AnyCpu\MvCameraControl.Net.dll，同时提供了win32 和 win64版本
3. 具体的调用方法可参考文档目录下的工业相机SDK开发指南(.Net)







The Machine Vision Camera SDK provides two sets of .Net interfaces, with namespaces as follows:
1. namespace MvCamCtrl.NET: 	The sample code is located in the MvCameraControlNet folder.
2. namespace MvCameraControl: 	The sample code is located in the MvCameraControlNet_V2 folder.

Recommended using the namespace MvCameraControl


namespace MvCamCtrl.NET:
1. Developed based on .Net Framework 3.5, demonstrate how to package the C language interface of the SDK into a .Net interface, and then call the packaged .Net interface to implement the SDK function
2. The source code is located in MvCameraControlNet\MVCameraSDK\Source\MVCamera.cs


namespace MvCameraControl:
1. Developed based on .Net Framework 4.0, it is a new .Net assembly (MvCameraControl.Net.dll) provided by the SDK
2. The library file is located in the Development\DotNet\AnyCPU\MvCameraControl.Net.dll folder under the client installation path, and is available in both win32 and win64 versions
3. For specific calling methods, refer to the Machine Vision Camera SDK Development Guide (.Net) under the document directory