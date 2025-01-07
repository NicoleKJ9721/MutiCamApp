/*
 * 这个示例演示SDK的常用功能，包括枚举、连接相机、配置参数、取图、保存图像等功能。
 * This program shows the basic functions of SDK, including enumeration, camera connection, parameter configuration, image grabbing, and image saving.
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

namespace BasicDemo
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new Form1());
        }
    }
}
