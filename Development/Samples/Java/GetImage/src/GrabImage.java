/***************************************************************************************************
 * @file      Grab_Callback.java
 * @breif     Use functions provided in MvCameraControlWrapper.jar to grab images
 * @author    zhanglei72
 * @date      2020/01/12
 *
 * @warning
 * @version   V1.0.0  2020/01/12 Create this file
 *            V1.0.1  2020/02/10 add parameter checking
 * @since     2020/02/10
 **************************************************************************************************/

import java.util.ArrayList;
import java.util.Scanner;

import MvCameraControlWrapper.*;
import static MvCameraControlWrapper.MvCameraControlDefines.*;

public class GrabImage
{
	public static Scanner scanner;
	static    Handle hCamera = null;
    static int nRet          = MV_OK;
    static boolean flag=true;
	static int camIndex = -1;
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
        else if (stDeviceInfo.transportLayerType == MV_USB_DEVICE)
        {
            System.out.println("\tUserDefinedName: " + stDeviceInfo.usb3VInfo.userDefinedName);
            System.out.println("\tSerial Number:   " + stDeviceInfo.usb3VInfo.serialNumber);
            System.out.println("\tDevice Number:   " + stDeviceInfo.usb3VInfo.deviceNumber);
        }
        else
        {
            System.err.print("Device is not supported! \n");
        }

        System.out.println("\tAccessible:      "
            + MvCameraControl.MV_CC_IsDeviceAccessible(stDeviceInfo, MV_ACCESS_Exclusive));
        System.out.println("");
    }




    public static int chooseCamera(ArrayList<MV_CC_DEVICE_INFO> stDeviceList)
    {
        if (null == stDeviceList)
        {
            return -1;
        }
        
        // Choose a device to operate
        int camIndex = -1;

        while (true) 
		{
            try {
                System.out.print("Please input camera index (-1 to quit):");
                camIndex = scanner.nextInt();
                if ((camIndex >= 0 && camIndex < stDeviceList.size()) || -1 == camIndex) {
                    break;
                } else {
                    System.out.println("Input error: " + camIndex);
                }
            } catch (Exception e) {
                e.printStackTrace();
                camIndex = -1;
                break;
            }
        }
       if (-1 == camIndex) {
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
	
	public static void GetImage(){
		
	MV_FRAME_OUT stFrameOut = new MV_FRAME_OUT();
	while(flag)
	{
			nRet = MvCameraControl.MV_CC_GetImageBuffer(hCamera, stFrameOut, 1000);
        // ch:获取一帧图像 | en:Get one image
        if (MV_OK == nRet)
        {
			System.out.println("\n Get Image Buffer Width:" + stFrameOut.mvFrameOutInfo.ExtendWidth
         			              + " Height: "   + stFrameOut.mvFrameOutInfo.ExtendHeight
								  + " FrameNum: " + stFrameOut.mvFrameOutInfo.frameNum);
			
                   
            nRet = MvCameraControl.MV_CC_FreeImageBuffer(hCamera, stFrameOut);
            if (MV_OK != nRet)
            {
                 System.err.printf("\n Free ImageBuffer failed, errcode: [%#x]\n", nRet);
            }
		}
    }
        
					   
	}


    
    public static void main(String[] args)
    {
        ArrayList<MV_CC_DEVICE_INFO> stDeviceList = null;
		 scanner = new Scanner(System.in);
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
			
            // Enumerate GigE and USB devices
            try
            {
                stDeviceList = MvCameraControl.MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE);
            }
            catch (CameraControlException e)
            {
                System.err.println("Enumrate devices failed!" + e.toString());
                e.printStackTrace();
                break;
            }
			
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

            // choose camera
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
				 System.err.printf("Connect suc.\n");
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
				System.err.printf("StartGrabbing  suc.\n");
			}
			
			
          Thread thread = new Thread(new Runnable(){
				@Override
				public void run(){
					// 在线程中执行函数
					GetImage();
				}
			});
            thread.start();
			
			
            scanner.useDelimiter("");
            System.out.println("Press Enter to exit.");
			
            while(flag)
            {
				String input = scanner.nextLine();
                if(scanner.hasNextLine())
                {
                    flag = false;
                    break;
                    
                }
				else
                {
                    try {
                        Thread.sleep(1 * 10);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
                
            }
			
			
            // Stop grabbing
            nRet = MvCameraControl.MV_CC_StopGrabbing(hCamera);
            if (MV_OK != nRet)
            {
                System.err.printf("StopGrabbing failed, errcode: [%#x]\n", nRet);
                break;
            }
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
		scanner.close();
		
    }
}
