
namespace SuntechIoTConfig
{
    partial class MainForm
    {
        /// <summary>
        /// 필수 디자이너 변수입니다.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 사용 중인 모든 리소스를 정리합니다.
        /// </summary>
        /// <param name="disposing">관리되는 리소스를 삭제해야 하면 true이고, 그렇지 않으면 false입니다.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form 디자이너에서 생성한 코드

        /// <summary>
        /// 디자이너 지원에 필요한 메서드입니다. 
        /// 이 메서드의 내용을 코드 편집기로 수정하지 마세요.
        /// </summary>
        private void InitializeComponent()
        {
            this.textBoxFIlePath = new System.Windows.Forms.TextBox();
            this.progressBar = new System.Windows.Forms.ProgressBar();
            this.btnProgram = new System.Windows.Forms.Button();
            this.btnLoadFile = new System.Windows.Forms.Button();
            this.textBoxStatusLog = new System.Windows.Forms.TextBox();
            this.textBoxSecurityKey = new System.Windows.Forms.TextBox();
            this.textBoxUSB_HID_Stat = new System.Windows.Forms.TextBox();
            this.textBoxProductID = new System.Windows.Forms.TextBox();
            this.textBoxVendorID = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.lablel1 = new System.Windows.Forms.Label();
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.tabControl = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.inforMessage = new System.Windows.Forms.TextBox();
            this.btnFirmwareUpgrade = new System.Windows.Forms.Button();
            this.btnReboot = new System.Windows.Forms.Button();
            this.btnFactoryResetFlash = new System.Windows.Forms.Button();
            this.comboBoxSSID = new System.Windows.Forms.ComboBox();
            this.label9 = new System.Windows.Forms.Label();
            this.textBoxPassword = new System.Windows.Forms.TextBox();
            this.label11 = new System.Windows.Forms.Label();
            this.textBoxServerIpAddress = new System.Windows.Forms.TextBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.checkBoxSSID = new System.Windows.Forms.CheckBox();
            this.label13 = new System.Windows.Forms.Label();
            this.textBoxDeviceName = new System.Windows.Forms.TextBox();
            this.btnReadConfig = new System.Windows.Forms.Button();
            this.btnWriteConfig = new System.Windows.Forms.Button();
            this.textBoxServerPortNumber = new System.Windows.Forms.TextBox();
            this.label10 = new System.Windows.Forms.Label();
            this.checkBoxWithMAC = new System.Windows.Forms.CheckBox();
            this.btnScanPort = new System.Windows.Forms.Button();
            this.textBoxMonitoring = new System.Windows.Forms.TextBox();
            this.comboBoxBaudRate = new System.Windows.Forms.ComboBox();
            this.label1 = new System.Windows.Forms.Label();
            this.comboBoxPorts = new System.Windows.Forms.ComboBox();
            this.label6 = new System.Windows.Forms.Label();
            this.btnOpen = new System.Windows.Forms.Button();
            this.tabControl.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // textBoxFIlePath
            // 
            this.textBoxFIlePath.Location = new System.Drawing.Point(9, 67);
            this.textBoxFIlePath.Margin = new System.Windows.Forms.Padding(4, 2, 4, 2);
            this.textBoxFIlePath.Name = "textBoxFIlePath";
            this.textBoxFIlePath.ReadOnly = true;
            this.textBoxFIlePath.Size = new System.Drawing.Size(483, 21);
            this.textBoxFIlePath.TabIndex = 27;
            // 
            // progressBar
            // 
            this.progressBar.Location = new System.Drawing.Point(11, 440);
            this.progressBar.Margin = new System.Windows.Forms.Padding(4, 2, 4, 2);
            this.progressBar.Name = "progressBar";
            this.progressBar.Size = new System.Drawing.Size(578, 22);
            this.progressBar.TabIndex = 26;
            // 
            // btnProgram
            // 
            this.btnProgram.Location = new System.Drawing.Point(0, 0);
            this.btnProgram.Name = "btnProgram";
            this.btnProgram.Size = new System.Drawing.Size(75, 23);
            this.btnProgram.TabIndex = 0;
            // 
            // btnLoadFile
            // 
            this.btnLoadFile.Location = new System.Drawing.Point(0, 0);
            this.btnLoadFile.Name = "btnLoadFile";
            this.btnLoadFile.Size = new System.Drawing.Size(75, 23);
            this.btnLoadFile.TabIndex = 0;
            // 
            // textBoxStatusLog
            // 
            this.textBoxStatusLog.Location = new System.Drawing.Point(9, 145);
            this.textBoxStatusLog.Margin = new System.Windows.Forms.Padding(4, 2, 4, 2);
            this.textBoxStatusLog.Multiline = true;
            this.textBoxStatusLog.Name = "textBoxStatusLog";
            this.textBoxStatusLog.Size = new System.Drawing.Size(582, 282);
            this.textBoxStatusLog.TabIndex = 23;
            // 
            // textBoxSecurityKey
            // 
            this.textBoxSecurityKey.Location = new System.Drawing.Point(0, 0);
            this.textBoxSecurityKey.Name = "textBoxSecurityKey";
            this.textBoxSecurityKey.Size = new System.Drawing.Size(100, 21);
            this.textBoxSecurityKey.TabIndex = 0;
            // 
            // textBoxUSB_HID_Stat
            // 
            this.textBoxUSB_HID_Stat.Location = new System.Drawing.Point(0, 0);
            this.textBoxUSB_HID_Stat.Name = "textBoxUSB_HID_Stat";
            this.textBoxUSB_HID_Stat.Size = new System.Drawing.Size(100, 21);
            this.textBoxUSB_HID_Stat.TabIndex = 0;
            // 
            // textBoxProductID
            // 
            this.textBoxProductID.Location = new System.Drawing.Point(0, 0);
            this.textBoxProductID.Name = "textBoxProductID";
            this.textBoxProductID.Size = new System.Drawing.Size(100, 21);
            this.textBoxProductID.TabIndex = 0;
            // 
            // textBoxVendorID
            // 
            this.textBoxVendorID.Location = new System.Drawing.Point(0, 0);
            this.textBoxVendorID.Name = "textBoxVendorID";
            this.textBoxVendorID.Size = new System.Drawing.Size(100, 21);
            this.textBoxVendorID.TabIndex = 0;
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point(0, 0);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(100, 23);
            this.label5.TabIndex = 0;
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(0, 0);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(100, 23);
            this.label4.TabIndex = 0;
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(0, 0);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(100, 23);
            this.label3.TabIndex = 0;
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(0, 0);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(100, 23);
            this.label2.TabIndex = 0;
            // 
            // lablel1
            // 
            this.lablel1.Location = new System.Drawing.Point(0, 0);
            this.lablel1.Name = "lablel1";
            this.lablel1.Size = new System.Drawing.Size(100, 23);
            this.lablel1.TabIndex = 0;
            // 
            // tabControl
            // 
            this.tabControl.Controls.Add(this.tabPage1);
            this.tabControl.Location = new System.Drawing.Point(2, 10);
            this.tabControl.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.tabControl.Name = "tabControl";
            this.tabControl.SelectedIndex = 0;
            this.tabControl.Size = new System.Drawing.Size(628, 770);
            this.tabControl.TabIndex = 28;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.inforMessage);
            this.tabPage1.Controls.Add(this.btnFirmwareUpgrade);
            this.tabPage1.Controls.Add(this.btnReboot);
            this.tabPage1.Controls.Add(this.btnFactoryResetFlash);
            this.tabPage1.Controls.Add(this.comboBoxSSID);
            this.tabPage1.Controls.Add(this.label9);
            this.tabPage1.Controls.Add(this.textBoxPassword);
            this.tabPage1.Controls.Add(this.label11);
            this.tabPage1.Controls.Add(this.textBoxServerIpAddress);
            this.tabPage1.Controls.Add(this.groupBox1);
            this.tabPage1.Controls.Add(this.btnScanPort);
            this.tabPage1.Controls.Add(this.textBoxMonitoring);
            this.tabPage1.Controls.Add(this.comboBoxBaudRate);
            this.tabPage1.Controls.Add(this.label1);
            this.tabPage1.Controls.Add(this.comboBoxPorts);
            this.tabPage1.Controls.Add(this.label6);
            this.tabPage1.Controls.Add(this.btnOpen);
            this.tabPage1.Location = new System.Drawing.Point(4, 22);
            this.tabPage1.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.tabPage1.Size = new System.Drawing.Size(620, 744);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "WITI Setting";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // inforMessage
            // 
            this.inforMessage.Location = new System.Drawing.Point(27, 638);
            this.inforMessage.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.inforMessage.Name = "inforMessage";
            this.inforMessage.ReadOnly = true;
            this.inforMessage.Size = new System.Drawing.Size(563, 21);
            this.inforMessage.TabIndex = 74;
            // 
            // btnFirmwareUpgrade
            // 
            this.btnFirmwareUpgrade.Enabled = false;
            this.btnFirmwareUpgrade.Location = new System.Drawing.Point(384, 595);
            this.btnFirmwareUpgrade.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.btnFirmwareUpgrade.Name = "btnFirmwareUpgrade";
            this.btnFirmwareUpgrade.Size = new System.Drawing.Size(128, 30);
            this.btnFirmwareUpgrade.TabIndex = 73;
            this.btnFirmwareUpgrade.Text = "Firmware Upgrade";
            this.btnFirmwareUpgrade.UseVisualStyleBackColor = true;
            this.btnFirmwareUpgrade.Click += new System.EventHandler(this.btnFirmwareUpgrade_Click);
            // 
            // btnReboot
            // 
            this.btnReboot.Enabled = false;
            this.btnReboot.Location = new System.Drawing.Point(518, 595);
            this.btnReboot.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.btnReboot.Name = "btnReboot";
            this.btnReboot.Size = new System.Drawing.Size(74, 30);
            this.btnReboot.TabIndex = 70;
            this.btnReboot.Text = "Reboot";
            this.btnReboot.UseVisualStyleBackColor = true;
            this.btnReboot.Click += new System.EventHandler(this.btnReboot_Click);
            // 
            // btnFactoryResetFlash
            // 
            this.btnFactoryResetFlash.Enabled = false;
            this.btnFactoryResetFlash.Location = new System.Drawing.Point(232, 595);
            this.btnFactoryResetFlash.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.btnFactoryResetFlash.Name = "btnFactoryResetFlash";
            this.btnFactoryResetFlash.Size = new System.Drawing.Size(146, 30);
            this.btnFactoryResetFlash.TabIndex = 69;
            this.btnFactoryResetFlash.Text = "Factory Reset Flash";
            this.btnFactoryResetFlash.UseVisualStyleBackColor = true;
            this.btnFactoryResetFlash.Click += new System.EventHandler(this.btnFactoryResetFlash_Click);
            // 
            // comboBoxSSID
            // 
            this.comboBoxSSID.Enabled = false;
            this.comboBoxSSID.FormattingEnabled = true;
            this.comboBoxSSID.Location = new System.Drawing.Point(112, 520);
            this.comboBoxSSID.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.comboBoxSSID.Name = "comboBoxSSID";
            this.comboBoxSSID.Size = new System.Drawing.Size(168, 20);
            this.comboBoxSSID.TabIndex = 67;
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Enabled = false;
            this.label9.Location = new System.Drawing.Point(41, 557);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(62, 12);
            this.label9.TabIndex = 61;
            this.label9.Text = "Password";
            // 
            // textBoxPassword
            // 
            this.textBoxPassword.Enabled = false;
            this.textBoxPassword.Location = new System.Drawing.Point(114, 554);
            this.textBoxPassword.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.textBoxPassword.MaxLength = 17;
            this.textBoxPassword.Name = "textBoxPassword";
            this.textBoxPassword.Size = new System.Drawing.Size(166, 21);
            this.textBoxPassword.TabIndex = 60;
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Location = new System.Drawing.Point(38, 484);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(56, 12);
            this.label11.TabIndex = 56;
            this.label11.Text = "Server IP";
            // 
            // textBoxServerIpAddress
            // 
            this.textBoxServerIpAddress.Enabled = false;
            this.textBoxServerIpAddress.Location = new System.Drawing.Point(112, 482);
            this.textBoxServerIpAddress.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.textBoxServerIpAddress.MaxLength = 50;
            this.textBoxServerIpAddress.Name = "textBoxServerIpAddress";
            this.textBoxServerIpAddress.Size = new System.Drawing.Size(168, 21);
            this.textBoxServerIpAddress.TabIndex = 55;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.checkBoxSSID);
            this.groupBox1.Controls.Add(this.label13);
            this.groupBox1.Controls.Add(this.textBoxDeviceName);
            this.groupBox1.Controls.Add(this.btnReadConfig);
            this.groupBox1.Controls.Add(this.btnWriteConfig);
            this.groupBox1.Controls.Add(this.textBoxServerPortNumber);
            this.groupBox1.Controls.Add(this.label10);
            this.groupBox1.Controls.Add(this.checkBoxWithMAC);
            this.groupBox1.Location = new System.Drawing.Point(29, 456);
            this.groupBox1.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Padding = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.groupBox1.Size = new System.Drawing.Size(561, 135);
            this.groupBox1.TabIndex = 68;
            this.groupBox1.TabStop = false;
            // 
            // checkBoxSSID
            // 
            this.checkBoxSSID.AutoSize = true;
            this.checkBoxSSID.Location = new System.Drawing.Point(15, 66);
            this.checkBoxSSID.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.checkBoxSSID.Name = "checkBoxSSID";
            this.checkBoxSSID.Size = new System.Drawing.Size(51, 16);
            this.checkBoxSSID.TabIndex = 69;
            this.checkBoxSSID.Text = "SSID";
            this.checkBoxSSID.UseVisualStyleBackColor = true;
            this.checkBoxSSID.CheckedChanged += new System.EventHandler(this.checkBoxSSID_CheckedChanged);
            // 
            // label13
            // 
            this.label13.AutoSize = true;
            this.label13.Location = new System.Drawing.Point(256, 107);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(81, 12);
            this.label13.TabIndex = 68;
            this.label13.Text = "Device Name";
            // 
            // textBoxDeviceName
            // 
            this.textBoxDeviceName.Enabled = false;
            this.textBoxDeviceName.Location = new System.Drawing.Point(342, 101);
            this.textBoxDeviceName.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.textBoxDeviceName.MaxLength = 17;
            this.textBoxDeviceName.Name = "textBoxDeviceName";
            this.textBoxDeviceName.Size = new System.Drawing.Size(108, 21);
            this.textBoxDeviceName.TabIndex = 67;
            // 
            // btnReadConfig
            // 
            this.btnReadConfig.Enabled = false;
            this.btnReadConfig.Location = new System.Drawing.Point(456, 16);
            this.btnReadConfig.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.btnReadConfig.Name = "btnReadConfig";
            this.btnReadConfig.Size = new System.Drawing.Size(89, 40);
            this.btnReadConfig.TabIndex = 59;
            this.btnReadConfig.Text = "Read Config";
            this.btnReadConfig.UseVisualStyleBackColor = true;
            this.btnReadConfig.Click += new System.EventHandler(this.btnReadConfig_Click);
            // 
            // btnWriteConfig
            // 
            this.btnWriteConfig.Enabled = false;
            this.btnWriteConfig.Location = new System.Drawing.Point(456, 76);
            this.btnWriteConfig.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.btnWriteConfig.Name = "btnWriteConfig";
            this.btnWriteConfig.Size = new System.Drawing.Size(89, 42);
            this.btnWriteConfig.TabIndex = 63;
            this.btnWriteConfig.Text = "Write Config";
            this.btnWriteConfig.UseVisualStyleBackColor = true;
            this.btnWriteConfig.Click += new System.EventHandler(this.btnWriteConfig_Click);
            // 
            // textBoxServerPortNumber
            // 
            this.textBoxServerPortNumber.Enabled = false;
            this.textBoxServerPortNumber.Location = new System.Drawing.Point(310, 26);
            this.textBoxServerPortNumber.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.textBoxServerPortNumber.MaxLength = 5;
            this.textBoxServerPortNumber.Name = "textBoxServerPortNumber";
            this.textBoxServerPortNumber.Size = new System.Drawing.Size(130, 21);
            this.textBoxServerPortNumber.TabIndex = 57;
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(256, 30);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(27, 12);
            this.label10.TabIndex = 58;
            this.label10.Text = "Port";
            // 
            // checkBoxWithMAC
            // 
            this.checkBoxWithMAC.AutoSize = true;
            this.checkBoxWithMAC.Enabled = false;
            this.checkBoxWithMAC.Location = new System.Drawing.Point(259, 67);
            this.checkBoxWithMAC.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.checkBoxWithMAC.Name = "checkBoxWithMAC";
            this.checkBoxWithMAC.Size = new System.Drawing.Size(79, 16);
            this.checkBoxWithMAC.TabIndex = 66;
            this.checkBoxWithMAC.Text = "with MAC";
            this.checkBoxWithMAC.UseVisualStyleBackColor = true;
            // 
            // btnScanPort
            // 
            this.btnScanPort.Location = new System.Drawing.Point(225, 30);
            this.btnScanPort.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.btnScanPort.Name = "btnScanPort";
            this.btnScanPort.Size = new System.Drawing.Size(28, 18);
            this.btnScanPort.TabIndex = 49;
            this.btnScanPort.Text = "()";
            this.btnScanPort.UseVisualStyleBackColor = true;
            this.btnScanPort.Click += new System.EventHandler(this.btnScanPort_Click);
            // 
            // textBoxMonitoring
            // 
            this.textBoxMonitoring.AllowDrop = true;
            this.textBoxMonitoring.Enabled = false;
            this.textBoxMonitoring.Location = new System.Drawing.Point(28, 102);
            this.textBoxMonitoring.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.textBoxMonitoring.Multiline = true;
            this.textBoxMonitoring.Name = "textBoxMonitoring";
            this.textBoxMonitoring.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.textBoxMonitoring.Size = new System.Drawing.Size(571, 276);
            this.textBoxMonitoring.TabIndex = 46;
            // 
            // comboBoxBaudRate
            // 
            this.comboBoxBaudRate.FormattingEnabled = true;
            this.comboBoxBaudRate.Location = new System.Drawing.Point(342, 30);
            this.comboBoxBaudRate.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.comboBoxBaudRate.Name = "comboBoxBaudRate";
            this.comboBoxBaudRate.Size = new System.Drawing.Size(106, 20);
            this.comboBoxBaudRate.TabIndex = 43;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(258, 33);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(63, 12);
            this.label1.TabIndex = 42;
            this.label1.Text = "Baud Rate";
            // 
            // comboBoxPorts
            // 
            this.comboBoxPorts.FormattingEnabled = true;
            this.comboBoxPorts.Location = new System.Drawing.Point(114, 30);
            this.comboBoxPorts.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.comboBoxPorts.Name = "comboBoxPorts";
            this.comboBoxPorts.Size = new System.Drawing.Size(106, 20);
            this.comboBoxPorts.TabIndex = 41;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(29, 33);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(65, 12);
            this.label6.TabIndex = 40;
            this.label6.Text = "Port Name";
            // 
            // btnOpen
            // 
            this.btnOpen.Location = new System.Drawing.Point(481, 23);
            this.btnOpen.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.btnOpen.Name = "btnOpen";
            this.btnOpen.Size = new System.Drawing.Size(93, 30);
            this.btnOpen.TabIndex = 39;
            this.btnOpen.Text = "Open";
            this.btnOpen.UseVisualStyleBackColor = true;
            this.btnOpen.Click += new System.EventHandler(this.btnOpen_Click);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(639, 704);
            this.Controls.Add(this.tabControl);
            this.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.Name = "MainForm";
            this.Text = "SuntechIoT Config";
            this.tabControl.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.tabPage1.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TextBox textBoxFIlePath;
        private System.Windows.Forms.ProgressBar progressBar;
        private System.Windows.Forms.Button btnProgram;
        private System.Windows.Forms.Button btnLoadFile;
        private System.Windows.Forms.TextBox textBoxStatusLog;
        private System.Windows.Forms.TextBox textBoxSecurityKey;
        private System.Windows.Forms.TextBox textBoxUSB_HID_Stat;
        private System.Windows.Forms.TextBox textBoxProductID;
        private System.Windows.Forms.TextBox textBoxVendorID;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label lablel1;
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private System.Windows.Forms.TabControl tabControl;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.ComboBox comboBoxSSID;
        private System.Windows.Forms.CheckBox checkBoxWithMAC;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.TextBox textBoxPassword;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.TextBox textBoxServerIpAddress;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button btnReadConfig;
        private System.Windows.Forms.Button btnWriteConfig;
        private System.Windows.Forms.TextBox textBoxServerPortNumber;
        private System.Windows.Forms.Button btnScanPort;
        private System.Windows.Forms.TextBox textBoxMonitoring;
        private System.Windows.Forms.ComboBox comboBoxBaudRate;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox comboBoxPorts;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Button btnOpen;
        private System.Windows.Forms.Button btnFirmwareUpgrade;
        private System.Windows.Forms.Button btnReboot;
        private System.Windows.Forms.Button btnFactoryResetFlash;
        private System.Windows.Forms.TextBox inforMessage;
        private System.Windows.Forms.Label label13;
        private System.Windows.Forms.TextBox textBoxDeviceName;
        private System.Windows.Forms.CheckBox checkBoxSSID;
    }
}

