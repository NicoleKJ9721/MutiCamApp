/*
 * 这个示例演示了红外相机的基本功能。
 * This sample shows the basic operations of the Infrared Camera.
 */

using System;
using System.Windows.Forms;

namespace InfraredDemo
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
            Application.Run(new InfraredDemo());
        }
    }
}
