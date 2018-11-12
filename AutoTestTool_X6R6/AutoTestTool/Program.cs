using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace AutoTestTool
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
           
            if (ProcTestData.IsConnectInternet() == false)
            {
                MessageBox.Show("网络未连接!\r\n请确认连接后重试", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            Application.Run(new LoginForm());
            //Application.Run(new MainForm());
        }
    }
}
