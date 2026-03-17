using System;
using System.Windows.Forms;
using System.Collections.Generic;
using System.IO.Ports;
using System.Text.Json;
using System.Linq;

namespace SuntechIoTConfig
{
    public partial class MainForm : Form
    {

        SerialPort serialPort = new SerialPort();
        string InputData = String.Empty;
        delegate void ParsingCallback(string text);

        string g_strReceivedJsonString = String.Empty;

        Boolean serialPortConnected = false;

        public MainForm()
        {
            InitializeComponent();

            GetAvailablePorts();
            GetBaudRate();

            serialPort.DataReceived += new SerialDataReceivedEventHandler(port_DataReceived);
            serialPort.ErrorReceived += new SerialErrorReceivedEventHandler(port_ErrorReceived);
          //  serialPort.PinChanged
          //   serialPort.PinChanged += new SerialPinChangedEventHandler(PinChanged);
        }

        void port_ErrorReceived(object sender, SerialErrorReceivedEventArgs e)
        {
            MessageBox.Show("NO", "Warning");
        }

        void GetAvailablePorts()
        {
            string[] ports = SerialPort.GetPortNames();
            comboBoxPorts.Items.Clear();
            comboBoxPorts.Items.AddRange(ports);
            comboBoxPorts.SelectedIndex = comboBoxPorts.Items.Count - 1;
        }
        void GetBaudRate()
        {
            comboBoxBaudRate.Items.Add("9600");
            comboBoxBaudRate.Items.Add("115200");
            comboBoxBaudRate.SelectedIndex = 1;
        }

        private void serialClose()
        {
            try
            {
                Enable(false);
                serialPort.Close();                     //선택한 serialPort 오픈
                btnOpen.Text = "Open";
                serialPortConnected = false;
            }
            catch (System.IO.IOException)
            {
                MessageBox.Show("Can't run of Iot Device", "Notice");
            }
        }

        public void Delay(int ms)
        {
            DateTime dateTimeNow = DateTime.Now;
            TimeSpan duration = new TimeSpan(0, 0, 0, 0, ms);
            DateTime dateTimeAdd = dateTimeNow.Add(duration);
            while (dateTimeAdd >= dateTimeNow)
            {
                System.Windows.Forms.Application.DoEvents();
                dateTimeNow = DateTime.Now;
            }
            return;
        }

        private void btnOpen_Click(object sender, EventArgs e)
        {
            if (serialPortConnected && serialPort.IsOpen)
            {
                SendSysCmd("disconnect");

                Delay(100);

                serialClose();
            }
            else
            {
                if (comboBoxPorts.Text == "" || comboBoxPorts.Text == null || comboBoxBaudRate.Text == "" || comboBoxBaudRate.Text == null)
                {
                    MessageBox.Show("Can't set port", "Warning");
                } 
                else
                {
                    if(serialPort.IsOpen)  serialPort.Close();

                    string[] ports = SerialPort.GetPortNames();


                    if (Array.IndexOf(ports, comboBoxPorts.Text) == -1)
                    {
                        return;
                    }

                    serialPort.PortName = comboBoxPorts.Text;
                    serialPort.BaudRate = Convert.ToInt32(comboBoxBaudRate.Text);//콤보박스에 있는 데이터는 문자이기때문에 Int로 형변환
                    serialPort.DataBits = 8;   //기본 데이터 비트 지정
                    serialPort.StopBits = StopBits.One;    //기본 스탑비트 지정
                    serialPort.Parity = Parity.None;       //기본 패리티 비트 지정


                    serialPort.DtrEnable = true;
                    try
                    {
                        serialPort.Open();                     //선택한 serialPort 오픈
                        btnOpen.Text = "Close";
                        SendSysCmd("connect");
                        //serialPortConnected = true;
                    }
                    catch (UnauthorizedAccessException)
                    {

                    }

                    textBoxMonitoring.Text = String.Empty;
                }
            }
        }

        private void SendAtCmdToSerialPort(string str)
        {

            if (serialPort.IsOpen)
            {
                try
                {
                    serialPort.Write("{\"request\":\"atCmd\", \"cmd\":\"" + StringEncode(str) + "\"}");
                }
                catch (UnauthorizedAccessException)
                {
                }
            }
        }
     

        private void btnSend_Click(object sender, EventArgs e)
        {
            serialPort.Write("{\"request\":\"at\", \"cmd\":\"at+gmr\" }");
        }

        private void port_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            InputData = serialPort.ReadExisting();
            if (InputData != String.Empty)
            {
                this.BeginInvoke(new ParsingCallback(Parsing), new object[] { InputData });
            }
        }

        private void btnConnect_Click(object sender, EventArgs e)
        {
         //   SendAtCmd("AT+CIPSTART=\"TCP\",\"" + textBoxServerIpAddress.Text + "\"," + textBoxServerPortNumber.Text);
        }


        private void Parsing(string text)
        {
            g_strReceivedJsonString += text;


            try
            {
                JsonDocument jdom = JsonDocument.Parse(g_strReceivedJsonString);

                JsonElement root = jdom.RootElement;

                 Console.WriteLine(root);

                inforMessage.Text = root.GetProperty("response").GetString();
                if (inforMessage.Text == "config")
                {
                    try
                    {
                        textBoxServerIpAddress.Text = root.GetProperty("serverIP").GetString();
                    }
                    catch (KeyNotFoundException)
                    {
                        textBoxServerIpAddress.Text = string.Empty;
                    }
                    try
                    {
                        textBoxServerPortNumber.Text = root.GetProperty("serverPort").GetString();
                    }
                    catch (KeyNotFoundException)
                    {
                        textBoxServerPortNumber.Text = string.Empty;
                    }


                    try
                    {
                        JsonElement ssidList = root.GetProperty("ssidList");
                        comboBoxSSID.Items.Clear();
                        for (int i = 0; i < ssidList.GetArrayLength(); i++)
                        {
                            JsonElement item = ssidList[i];
                            comboBoxSSID.Items.Add(item.ToString());
                        }
                        comboBoxSSID.SelectedIndex = 0;
                    }
                    catch (KeyNotFoundException)
                    {
                    }

                    try
                    {
                        comboBoxSSID.SelectedIndex  = Int32.Parse(root.GetProperty("SSID_Index").GetString());
                    }
                    catch (KeyNotFoundException)
                    {
                    }

                    try
                    {
                        textBoxPassword.Text = root.GetProperty("password").GetString();
                        btnWriteConfig.Enabled = true;
                    }
                    catch (KeyNotFoundException)
                    {
                        textBoxPassword.Text = string.Empty;
                    }

                    //try
                    //{
                    //    textBoxRemark.Text = root.GetProperty("remark").GetString();
                    //}
                    //catch (KeyNotFoundException)
                    //{
                    //    textBoxRemark.Text = string.Empty;
                    //}

                    try
                    {
                        textBoxDeviceName.Text = root.GetProperty("deviceName").GetString();
                    }
                    catch (KeyNotFoundException)
                    {
                        textBoxDeviceName.Text = string.Empty;
                    }

                }
                else if (inforMessage.Text == "connected")
                {
                    Enable(true);
                }
                else if (inforMessage.Text == "disconnect")
                {
                    serialClose();
                }
                else if (inforMessage.Text == "notice")
                {
                    MessageBox.Show(root.GetProperty("msg").GetString(), "notice");
                }
                g_strReceivedJsonString = String.Empty;
            }
            catch (JsonException)
            {
                if (g_strReceivedJsonString[0] != '{') g_strReceivedJsonString = String.Empty;

                //  inforMessage.Text = "Json Conversion Error";
            }
   
        }

        private string StringEncode(string str)
        {
            string encode = string.Empty;

            char[] values = str.ToCharArray();
            foreach (char letter in values)
            {
                int value = Convert.ToInt32(letter);
                encode += string.Format($"{value:X2}");
            }

            return encode;
        }


        void Enable(Boolean b)
        {
            inforMessage.Enabled = b;

            textBoxMonitoring.Enabled = b;

            textBoxServerIpAddress.Enabled = b;
            textBoxServerPortNumber.Enabled = b;
            btnReadConfig.Enabled = b;

            textBoxPassword.Enabled = b && checkBoxSSID.Checked;

            if (b == false) btnWriteConfig.Enabled = b;


           // textBoxRemark.Enabled = b;
            checkBoxWithMAC.Enabled = b && checkBoxSSID.Checked;
            comboBoxSSID.Enabled = b && checkBoxSSID.Checked;

            textBoxDeviceName.Enabled = b;  

            btnFactoryResetFlash.Enabled = b;
            btnReboot.Enabled = b;

            btnFirmwareUpgrade.Enabled = b;

            comboBoxPorts.Enabled = !b;
            comboBoxBaudRate.Enabled = !b;
        }

        private void btnScanPort_Click(object sender, EventArgs e)
        {
            GetAvailablePorts();
        }

        private void btnAndonApiTest_Click(object sender, EventArgs e)
        {
            if (serialPort.IsOpen) serialPort.Write("{\"request\":\"andonApiTest\"}");
        }

        private void btnClear_Click(object sender, EventArgs e)
        {
            textBoxMonitoring.Text = String.Empty;
        }

        private void btnReadConfig_Click(object sender, EventArgs e)
        {
            if (serialPort.IsOpen) serialPort.Write("{\"request\":\"readConfig\"}");
        }
 
        private void btnWriteConfig_Click(object sender, EventArgs e)
        {
            textBoxPassword.Text.Trim();
            textBoxServerIpAddress.Text.Trim();
            textBoxServerPortNumber.Text.Trim();

            if (comboBoxSSID.SelectedIndex < 0)
            {
                MessageBox.Show("SSID is not selected", "Notice");
                return;
            }

            textBoxServerIpAddress.Text.Replace("\\", "/");

            if (serialPort.IsOpen)
            {
                serialPort.Write("{\"request\":\"writeConfig\",");
                if (checkBoxSSID.Checked)
                {
                    serialPort.Write("\"SSIDIndex\":\"" + comboBoxSSID.SelectedIndex.ToString() + "\",");
                    if (checkBoxWithMAC.Checked)
                        serialPort.Write("\"withMAC\":\"1\",");
                    else
                        serialPort.Write("\"withMAC\":\"0\",");
                    serialPort.Write("\"password\":\"" + textBoxPassword.Text + "\",");
                }
                textBoxServerIpAddress.Text.TrimStart(' ');
                serialPort.Write("\"serverIP\":\"" + textBoxServerIpAddress.Text + "\",");
                serialPort.Write("\"serverPort\":\"" + textBoxServerPortNumber.Text + "\",");
             //   serialPort.Write("\"remark\":\"" + textBoxRemark.Text + "\",");
                serialPort.Write("\"deviceName\":\"" + textBoxDeviceName.Text + "\"}");
            }
        }

        private void checkBoxSSID_CheckedChanged(object sender, EventArgs e)
        {
            comboBoxSSID.Enabled = checkBoxSSID.Checked;
            textBoxPassword.Enabled = checkBoxSSID.Checked;
            checkBoxWithMAC.Enabled = checkBoxSSID.Checked;
            label9.Enabled = checkBoxSSID.Checked;  
        }
    }
}
