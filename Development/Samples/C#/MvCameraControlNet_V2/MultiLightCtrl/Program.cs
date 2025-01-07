/*
 * 这个示例演示了如何从支持“MultiLightControl”功能的相机中获取图像，并重建图像。
 * This sample shows how to get an image from the camera that supports the "MultiLightControl" feature and reconstruct the image.
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace MultiLightCtrl
{
    static class Program
    {
        /// <summary>
        /// 应用程序的主入口点。
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MultiLightCtrl());
        }
    }
}
