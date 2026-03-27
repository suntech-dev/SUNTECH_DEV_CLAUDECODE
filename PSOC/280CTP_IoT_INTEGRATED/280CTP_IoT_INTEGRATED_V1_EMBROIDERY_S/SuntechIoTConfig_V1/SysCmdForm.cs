using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Diagnostics;
using System.IO;

namespace SuntechIoTConfig
{
    partial class MainForm
    {

        private void SendSysCmd(string str)
        {
            if (serialPort.IsOpen)
            {
                try
                {
                    serialPort.Write("{\"request\":\"sysCmd\", \"cmd\":\"" + str + "\"}");
                }
                catch (UnauthorizedAccessException)
                {
                }
            }
        }

        private void btnFirmwareUpgrade_Click(object sender, EventArgs e)
        {
            string exePath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "USBBootloaderHost.exe");

            // // 2) 하위 폴더에 있을 때 (예: tools)
            // string exePath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "tools", "USBBootloaderHost.exe");

            // // 3) 절대경로 예시
            // string exePath = @"C:\Program Files (x86)\YourApp\USBBootloaderHost.exe";

            if (!File.Exists(exePath))
            {
                MessageBox.Show("USBBootloaderHost.exe를 찾을 수 없습니다.\n경로: " + exePath);
                return;
            }

            try
            {
                Process.Start(new ProcessStartInfo
                {
                    FileName = exePath,
                    UseShellExecute = true,                           // 기본 실행
                    WorkingDirectory = Path.GetDirectoryName(exePath) // 현재 작업폴더 맞추기(리소스/INI 의존 대비)
                });
            }
            catch (Exception ex)
            {
                MessageBox.Show("실행 실패: " + ex.Message);
            }

            SendSysCmd("firmwareUpgrade");
          //tabControl.SelectTab(1);
        }

        private void btnApplicationMode_Click(object sender, EventArgs e)
        {
            SendSysCmd("appMode");
        }

        private void btnConfigMode_Click(object sender, EventArgs e)
        {
            SendSysCmd("configMode");
        }


        private void btnReboot_Click(object sender, EventArgs e)
        {
            SendSysCmd("reboot");
            if (serialPort.IsOpen)
            {
                try
                {
                    serialPort.Close();                     //선택한 serialPort 오픈
                    btnOpen.Text = "Open";
                }
                catch (System.IO.IOException)
                {
                    MessageBox.Show("Can't run of Iot Device", "Notice");
                }

            }

        }
        private void btnFactoryResetFlash_Click(object sender, EventArgs e)
        {
            SendSysCmd("factoryReset");
        }


    }
}
