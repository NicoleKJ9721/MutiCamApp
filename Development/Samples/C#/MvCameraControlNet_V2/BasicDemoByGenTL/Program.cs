/*
 * 这个示例演示如何通过GenTL模块连接第三方采集卡和相机。
 * This program shows how to connect a third-party frame grabber and camera through the GenTL module.
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

namespace BasicDemoByGenTL
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
