/*
 * 这个示例演示如何通过SDK连接、配置线扫相机。
 * This program shows how to connect and configure line scan cameras.
 */

using System;
using System.Windows.Forms;

namespace BasicDemoLineScan
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
            Application.Run(new Form1());
        }
    }
}
