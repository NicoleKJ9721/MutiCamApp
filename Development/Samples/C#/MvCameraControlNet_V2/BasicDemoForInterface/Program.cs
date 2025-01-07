/*
 * 这个示例演示如何通过SDK连接、配置采集卡。
 * This program shows how to connect and configure frame grabber.
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

namespace InterfaceBasicDemo
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
            Application.Run(new InterfaceBasicDemo());
        }
    }
}
