/***************************************************************************************************
 * @file      InterfaceAndDevice.java
 * @breif     Use functions provided in MvCameraControlWrapper.jar to grab images
 * @author    guohongli
 * @date      2024/02/22
 *
 * @warning
 * @version   V4.3.2.  2024/02/22 Create this file
 *            
 **************************************************************************************************/

import java.util.ArrayList;
import java.util.Scanner;

import MvCameraControlWrapper.*;
import static MvCameraControlWrapper.MvCameraControlDefines.*;

public class InterfaceAndDevice
{
    private static void printDeviceInfo(MV_CC_DEVICE_INFO stDeviceInfo)
    {
        if (null == stDeviceInfo) {
            System.out.println("stDeviceInfo is null");
            return;
        }

        if (stDeviceInfo.transportLayerType == MV_GIGE_DEVICE)
        {
            System.out.println("\tCurrentIp:       " + stDeviceInfo.gigEInfo.currentIp);
            System.out.println("\tModel:           " + stDeviceInfo.gigEInfo.modelName);
            System.out.println("\tUserDefinedName: " + stDeviceInfo.gigEInfo.userDefinedName);
        }
		else if (stDeviceInfo.transportLayerType == MV_CAMERALINK_DEVICE)
	   {
			System.out.println("\tportID:" + stDeviceInfo.camLinfo.portID);
			System.out.println("\tSerial Number:" + stDeviceInfo.camLinfo.serialNumber);
			System.out.println("\tModel Name:" + stDeviceInfo.camLinfo.modelName);
	   }
		else if (stDeviceInfo.transportLayerType == MV_GENTL_CAMERALINK_DEVICE)
	   {
			System.out.println("\tinterfaceID:" + stDeviceInfo.cmlInfo.interfaceID);
			System.out.println("\tSerial Number:" + stDeviceInfo.cmlInfo.serialNumber);
			System.out.println("\tModel Name:" + stDeviceInfo.cmlInfo.modelName);
	   }
	   else if (stDeviceInfo.transportLayerType == MV_GENTL_CXP_DEVICE)
	   {
			System.out.println("\tinterfaceID:" + stDeviceInfo.cxpInfo.interfaceID);
			System.out.println("\tSerial Number:" + stDeviceInfo.cxpInfo.serialNumber);
			System.out.println("\tModel Name:" + stDeviceInfo.cxpInfo.modelName);
		}
	    else if (stDeviceInfo.transportLayerType == MV_GENTL_XOF_DEVICE)
	    {
			System.out.println("\tinterfaceID:" + stDeviceInfo.xofInfo.interfaceID);
			System.out.println("\tSerial Number:" +  stDeviceInfo.xofInfo.serialNumber);
			System.out.println("\tModel Name:" + stDeviceInfo.xofInfo.modelName);
	    }
		else
		{
			System.out.println("Not support.\n");
		}
  
        //System.out.println("\tAccessible:      "
           // + MvCameraControl.MV_CC_IsDeviceAccessible(stDeviceInfo, MV_ACCESS_Exclusive));
        System.out.println("");
    }


    public static int chooseEnumInterfaceType()
	{
		int EnumInterfaceTypeIndex = -1;
		 Scanner scanner = new Scanner(System.in);
		while (true)
        {
            try
            {
                System.out.print("Please Input Enum Interfaces Type: (0 to 3):");
                EnumInterfaceTypeIndex = scanner.nextInt();
                if ((EnumInterfaceTypeIndex >= 0 && EnumInterfaceTypeIndex < 3) || -1 == EnumInterfaceTypeIndex)
                {
                    break;
                }
                else
                {
                    System.out.println("Input error: " + EnumInterfaceTypeIndex);
                }
            }
            catch (Exception e)
            {
                e.printStackTrace();
                EnumInterfaceTypeIndex = -1;
                break;
            }
        }
        //scanner.close();
		 if (-1 == EnumInterfaceTypeIndex)
        {
            System.out.println("Bye.");
            return EnumInterfaceTypeIndex;
        }
		
		return EnumInterfaceTypeIndex;
	}
	
    public static int chooseCamera(ArrayList<MV_CC_DEVICE_INFO> stDeviceList)
    {
        if (null == stDeviceList)
        {
            return -1;
        }
        
        // Choose a device to operate
        int camIndex = -1;
        Scanner scanner = new Scanner(System.in);

        while (true)
        {
            try
            {
                System.out.print("Please input camera index (-1 to quit):");
                camIndex = scanner.nextInt();
                if ((camIndex >= 0 && camIndex < stDeviceList.size()) || -1 == camIndex)
                {
                    break;
                }
                else
                {
                    System.out.println("Input error: " + camIndex);
                }
            }
            catch (Exception e)
            {
                e.printStackTrace();
                camIndex = -1;
                break;
            }
        }
        scanner.close();

        if (-1 == camIndex)
        {
            System.out.println("Bye.");
            return camIndex;
        }

        if (0 <= camIndex && stDeviceList.size() > camIndex)
        {
            if (MV_GIGE_DEVICE == stDeviceList.get(camIndex).transportLayerType)
            {
                System.out.println("Connect to camera[" + camIndex + "]: " + stDeviceList.get(camIndex).gigEInfo.userDefinedName);
            }
            else if (MV_USB_DEVICE == stDeviceList.get(camIndex).transportLayerType)
            {
                System.out.println("Connect to camera[" + camIndex + "]: " + stDeviceList.get(camIndex).usb3VInfo.userDefinedName);
            }
			else if (MV_CAMERALINK_DEVICE == stDeviceList.get(camIndex).transportLayerType)
            {
                System.out.println("Connect to camera[" + camIndex + "]: " + stDeviceList.get(camIndex).camLinfo.serialNumber);
            }
			else if (MV_GENTL_CAMERALINK_DEVICE == stDeviceList.get(camIndex).transportLayerType)  //interface
            {
                System.out.println("Connect to camera[" + camIndex + "]: " + stDeviceList.get(camIndex).cmlInfo.serialNumber);
            }
			else if (MV_GENTL_CXP_DEVICE == stDeviceList.get(camIndex).transportLayerType)
            {
                System.out.println("Connect to camera[" + camIndex + "]: " + stDeviceList.get(camIndex).cxpInfo.serialNumber);
            }
			else if (MV_GENTL_XOF_DEVICE == stDeviceList.get(camIndex).transportLayerType)
            {
                System.out.println("Connect to camera[" + camIndex + "]: " + stDeviceList.get(camIndex).xofInfo.serialNumber);
            }
            else
            {
                System.out.println("Device is not supported.");
            }
        }
        else
        {
            System.out.println("Invalid index " + camIndex);
            camIndex = -1;
        }

        return camIndex;
    }
    
    public static void main(String[] args)
    {
        int nRet = MV_OK;
		Handle hCamera = null;
       
		System.out.println("[0]: Enum GIGE Interface Devices\n");
		System.out.println("[1]: Enum CAMERALINK Interface Devices\n");
	    System.out.println("[2]: Enum CXP Interface Devices\n");
	    System.out.println("[3]: Enum XOF Interface Devices\n\n");

        do
        {
			System.out.println("SDK Version " + MvCameraControl.MV_CC_GetSDKVersion());
            
			// Initialize SDK
		    nRet = MvCameraControl.MV_CC_Initialize();
		    if (MV_OK != nRet)
		    {
			   System.err.printf("Initialize SDK fail! nRet [0x%x]\n\n",nRet);
               break;
		    }
		
            
			ArrayList<MV_CC_DEVICE_INFO> stDeviceList = null;
			
			
				
            // Enumerate GigE and USB devices
            try
            {
				// choose interface
				int nEnumInterfaceType = chooseEnumInterfaceType();
				if (-1 == nEnumInterfaceType)
				{
					break;
				}
				
				switch(nEnumInterfaceType)
				{
				 case 0:
				 {
					stDeviceList = MvCameraControl.MV_CC_EnumDevices(MV_GENTL_GIGE_DEVICE);
					break;
				 }
				 case 1:
				 {
					stDeviceList  = MvCameraControl.MV_CC_EnumDevices(MV_GENTL_CAMERALINK_DEVICE);
					break;
				 }
				 case 2:
				 {
					stDeviceList = MvCameraControl.MV_CC_EnumDevices(MV_GENTL_CXP_DEVICE);
					break;
				 }
				 case 3:
				 {
					stDeviceList = MvCameraControl.MV_CC_EnumDevices(MV_GENTL_XOF_DEVICE);
					break;
				 }
				 default:
				 {
					System.out.println("Input InerfaceType error!");
					break;
				 }
			   }

			   //枚举采集卡设备
			   if (0 >= stDeviceList.size())
               {
                   System.out.println("No devices found!");
                   break;
                }
		
		
                int i = 0;
			    for (MV_CC_DEVICE_INFO stDeviceInfo : stDeviceList)
                {
                    if (null == stDeviceInfo)
                    {
                      continue;
                    }
                    System.out.println("[camera " + (i++) + "]");
                    printDeviceInfo(stDeviceInfo);
                }
            }
            catch (CameraControlException e)
            {
                System.err.println("Enumrate devices failed!" + e.toString());
                e.printStackTrace();
                break;
            }

            // choose camera
			int camIndex =-1;
            camIndex = chooseCamera(stDeviceList);
            if (-1 == camIndex)
            {
                break;
            }

            // Create device handle
            try
            {
                hCamera = MvCameraControl.MV_CC_CreateHandle(stDeviceList.get(camIndex));
            }
            catch (CameraControlException e)
            {
                System.err.println("Create handle failed!" + e.toString());
                e.printStackTrace();
                hCamera = null;
                break;
            }

            // Open selected device
            nRet = MvCameraControl.MV_CC_OpenDevice(hCamera);
            if (MV_OK != nRet)
            {
                System.err.printf("Connect to camera failed, errcode: [%#x]\n",nRet);
                break;
            }
			else
			{
				 System.out.println("Connect success.\n");
			}

           
            // Turn on trigger mode
            nRet = MvCameraControl.MV_CC_SetEnumValueByString(hCamera, "TriggerMode", "Off");
            if (MV_OK != nRet)
            {
               System.err.printf("SetTriggerMode failed, errcode: [%#x]\n", nRet);
               break;
            }

     

            // Start grabbing
            nRet = MvCameraControl.MV_CC_StartGrabbing(hCamera);
            if (MV_OK != nRet)
            {
                System.err.printf("StartGrabbing failed, errcode: [%#x]\n", nRet);
                break;
            }
			else
			{
				System.out.println("StartGrabbing  success.\n");
			}

            MV_FRAME_OUT stFrameOut = new MV_FRAME_OUT();
			for(int n = 0; n < 10; n++)
			{

			   	nRet = MvCameraControl.MV_CC_GetImageBuffer(hCamera, stFrameOut, 1000);
                 // ch:获取一帧图像 | en:Get one image
                 if (MV_OK == nRet)
                 {
                    System.out.println("Get Image Buffer: Width: " + stFrameOut.mvFrameOutInfo.ExtendWidth
					                                 + ", Height: " +stFrameOut.mvFrameOutInfo.ExtendHeight
													 + ", FrameNum: "+ stFrameOut.mvFrameOutInfo.frameNum);
                   
                   nRet = MvCameraControl.MV_CC_FreeImageBuffer(hCamera, stFrameOut);
                   if (MV_OK != nRet)
                    {
                       System.err.printf("Free ImageBuffer failed, errcode: [%#x]\n", nRet);
                    }
               }
               else
               {
                    System.err.printf("\n Get ImageBuffer failed, errcode: [%#x]\n", nRet);
               }
					   
			}


            // Stop grabbing
            nRet = MvCameraControl.MV_CC_StopGrabbing(hCamera);
            if (MV_OK != nRet)
            {
                System.err.printf("StopGrabbing failed, errcode: [%#x]\n", nRet);
                break;
            }
			
			// close device
			nRet = MvCameraControl.MV_CC_CloseDevice(hCamera);
            if (MV_OK != nRet)
            {
                System.err.printf("Close Device failed, errcode: [%#x]\n", nRet);
                break;
            }
			
			// Destroy handle
			nRet = MvCameraControl.MV_CC_DestroyHandle(hCamera);
            if (MV_OK != nRet) {
                System.err.printf("DestroyHandle failed, errcode: [%#x]\n", nRet);
            }
			hCamera = null;
			
			
        } while (false);

        if (null != hCamera)
        {
            // Destroy handle
            nRet = MvCameraControl.MV_CC_DestroyHandle(hCamera);
            if (MV_OK != nRet) {
                System.err.printf("DestroyHandle failed, errcode: [%#x]\n", nRet);
            }
        }
		
		MvCameraControl.MV_CC_Finalize();
    }
}
