using System;
using System.Drawing;
using System.Windows.Forms;
using System.Runtime.InteropServices;   // Marshal copy
using System.IO;                        // For File handling
using CyUSB;                            // For Cypress USB HID device

namespace SuntechIoTConfig
{
    public partial class Form1 : Form
    {
        /// <summary>
        /// Error values
        /// </summary>
        internal const int ERR_SUCCESS = 0;
        internal const int ERR_OPEN = 1;
        internal const int ERR_CLOSE = 2;
        internal const int ERR_READ = 3;
        internal const int ERR_WRITE = 4;

        public CyHidDevice BootloaderHIDDevice;
        public USBDeviceList usbHIDDevices = null;
        int VID = 0x04B4;
        int PID = 0xB71D;

        // String holding the Cyacd file path/name
        string Chosen_File_Cyacd = "";
        bool Cyacd_found;

        // For checking bootloading progress
        double progressBarProgress;
        double progressBarStepSize;
        Bootloader_Utils.CyBtldr_CommunicationsData comm_data = new Bootloader_Utils.CyBtldr_CommunicationsData();

        public Form1()
        {
            InitializeComponent();

            usbHIDDevices = new USBDeviceList(CyConst.DEVICES_HID);

            //Adding event handlers for device attachment and device removal            
            usbHIDDevices.DeviceAttached += new EventHandler(usbHIDDevices_DeviceChecker);
            usbHIDDevices.DeviceRemoved += new EventHandler(usbHIDDevices_DeviceChecker);
            GetHidDevice();
        }

        /// <summary>
        /// The GetHidDevice Method checks for the presence of a specific USB HID device and updates the status
        /// </summary>
        /// <returns>bool</returns>
        public bool GetHidDevice()
        {
            bool Status = false;
            try
            {
                // Attempt to assign local HID object to HID in devices list with matching VID and PID
                BootloaderHIDDevice = usbHIDDevices[VID, PID] as CyHidDevice;

                // If we find a valid device, update the GUI with device information
                if (BootloaderHIDDevice != null)
                {
                    Status = true;
                    textBox3.Text = "Connected";
                    textBox1.Text = Convert.ToString(BootloaderHIDDevice.VendorID, 16);// BootloaderHIDDevice.VendorID.ToString();
                    textBox1.BackColor = Color.LightBlue;
                    textBox2.Text = Convert.ToString(BootloaderHIDDevice.ProductID, 16);//BootloaderHIDDevice.ProductID.ToString(); 
                    textBox2.BackColor = Color.LightBlue;
                }

                // If there is no matching device, display Disconnected on GUI
                else
                {
                    Status = false;
                    textBox3.Text = "Disconnected";

                    textBox1.BackColor = Color.LightYellow;
                    textBox2.BackColor = Color.LightYellow;
                }

                return (Status);
            }

            catch
            {
                MessageBox.Show(" Device not connected");
                return (Status);
            }
        }

        /// <summary>
        /// Checks if the USB device is connected and opens if it is present
        /// Returns a success or failure
        /// </summary>
        public int OpenConnection()
        {
            int status = 0;
            status = GetHidDevice() ? ERR_SUCCESS : ERR_OPEN;
            return status;
        }

        /// <summary>
        /// Closes the previously opened USB device and returns the status
        /// </summary>
        public int CloseConnection()
        {
            int status = 0;
            BootloaderHIDDevice = null;
            return status;
        }

        /// <summary>
        /// Method that performs Read operation from USB Device
        /// </summary>
        /// <param name="buffer"> Pointer to an array where data read from USB device is copied to </param>
        /// <param name="size"> Size of the Buffer </param>
        /// <returns></returns>
        public int ReadData(IntPtr buffer, int size)
        {
            int status;
            bool local_status = false;
            byte[] data = new byte[size];

            //Check if USB device exists
            if (GetHidDevice())
            {
                local_status = BootloaderHIDDevice.ReadInput();
                data = BootloaderHIDDevice.Inputs.DataBuf;
                Marshal.Copy(data, 1, buffer, Math.Min(size, data.Length));
                status = local_status ? ERR_SUCCESS : ERR_READ;
            }
            else
                status = ERR_READ;

            return status;
        }

        /// <summary>
        /// Method that writes to the USB device
        /// </summary>
        /// <param name="buffer">Pointer to an array where data written to USB device is stored </param>
        /// <param name="size"> Size of the Buffer </param>
        /// <returns></returns>
        public int WriteData(IntPtr buffer, int size)
        {
            byte[] data = new byte[64];
            bool status = true;

            Marshal.Copy(buffer, data, 0, size);

            if (GetHidDevice())
            {
                //First Byte in buffer holds the Report ID
                BootloaderHIDDevice.Outputs.DataBuf[0] = BootloaderHIDDevice.Outputs.ID;

                //Load 64 bytes of data
                for (int i = 1; i <= size; i++)
                {
                    BootloaderHIDDevice.Outputs.DataBuf[i] = data[i - 1];
                }

                status = BootloaderHIDDevice.WriteOutput();

                if (status)
                    return ERR_SUCCESS;
                else
                    return ERR_WRITE;
            }
            else
                return ERR_WRITE;

        }

        /// <summary>
        /// Method that returns the maximum transfer size
        /// </summary>
        public uint MaxTransferSize
        {
            get { return (uint)BootloaderHIDDevice.Features.RptByteLen; }
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            VID = Convert.ToInt32(textBox1.Text, 16);
            GetHidDevice();
        }

        private void textBox2_TextChanged(object sender, EventArgs e)
        {
            PID = Convert.ToInt32(textBox2.Text, 16);
            GetHidDevice();
        }

        /// <summary>
        ///  This is the event handler for Device Connection event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>           
        void usbHIDDevices_DeviceChecker(object sender, EventArgs e)
        {
            GetHidDevice();
        }

        /// <summary>
        /// This method handles the "Load File" Button1 Click event and lets you choose the 
        /// appropriate CYACD file
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void button1_Click(object sender, EventArgs e)
        {
            int lines;
            openFileDialog1.Title = "Open CYACD File";
            openFileDialog1.FileName = "";
            openFileDialog1.Filter = "cyacd file|*.cyacd";

            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                Chosen_File_Cyacd = openFileDialog1.FileName;
                textBox6.Text = Chosen_File_Cyacd;

                lines = File.ReadAllLines(Chosen_File_Cyacd).Length - 1; //Don't count header
                progressBarStepSize = 100.0 / lines;
                Cyacd_found = true;
            }
        }

        /// <summary>
        /// This Method handles the "Program" Button2 Click event
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>

        private void button2_Click(object sender, EventArgs e)
        {
            // The value is allowed 1 or 2. If application is using dual image, 
            // the appID of 2nd image should be 2
            byte appID = 1;

            // Coverting string hex to byte array
            long SKey = Int64.Parse(textBox4.Text, System.Globalization.NumberStyles.HexNumber);
            byte[] SecurityKey = BitConverter.GetBytes(SKey);
            Array.Reverse(SecurityKey);

            // If Security key is not 0, make 6 bytes size array for security key 
            // because the reversed array size is 8.
            if (SKey == 0)
            {
                SecurityKey = null;
            }
            else
            {
                for (int i = 0; i < 6; i++)
                {
                    SecurityKey[i] = SecurityKey[i + 2];
                }
            }

            // Setup comm_data object with our delegate functions
            comm_data.OpenConnection = OpenConnection;
            comm_data.CloseConnection = CloseConnection;
            comm_data.ReadData = ReadData;
            comm_data.WriteData = WriteData;
            comm_data.MaxTransferSize = 64;

            ReturnCodes local_status = 0;
            progressBarProgress = 0;
            progressBar1.Value = 0;
            // Create update object for use with Bootload_Utils.CyBtldr_Program function.  This object
            // is used to report updated array and row information from the PSoC.
            Bootloader_Utils.CyBtldr_ProgressUpdate update = new Bootloader_Utils.CyBtldr_ProgressUpdate(ProgressUpdate);

            // Checking a HW connection and a file exist
            if (Cyacd_found)
            {
                if (GetHidDevice())
                {
                    DateTime saveNow = DateTime.Now;
                    string time_var = saveNow.ToString();
                    textBox5.Text += " Bootload Started at " + time_var + "\r\n";

                    local_status = (ReturnCodes)Bootloader_Utils.CyBtldr_Program(Chosen_File_Cyacd, SecurityKey, appID, ref comm_data, update);
                    saveNow = DateTime.Now;

                    if (local_status == (int)ReturnCodes.CYRET_SUCCESS)
                    {
                        time_var = saveNow.ToString();
                        textBox5.Text += " Bootload successful  !! \r\n Bootload ended at " + time_var + "\r\n";
                    }
                    else
                    {
                        if (ReturnCodes.CYRET_ERR_COMM_MASK == (local_status & ReturnCodes.CYRET_ERR_COMM_MASK))
                        {
                            MessageBox.Show(" Program failed: Communication Error");
                        }
                        else if (ReturnCodes.CYRET_ERR_DATA == (local_status & ReturnCodes.CYRET_ERR_DATA))
                        {
                            MessageBox.Show(" Program failed: Check Security Key");
                        }
                        else
                        {
                            MessageBox.Show(" Program failed  " + local_status.ToString());
                        }
                    }
                }
                else
                {
                    textBox5.Text += "No Device Connected \r\n";
                    MessageBox.Show("No Device Connected");
                }
            }
            else
            {
                MessageBox.Show("No file chosen");
            }
        }

        /// <summary>
        /// Method that updates the progres bar
        /// </summary>
        /// <param name="arrayID"></param>
        /// <param name="rowNum"></param>
        public void ProgressUpdate(byte arrayID, ushort rowNum)
        {
            progressBarProgress += progressBarStepSize;
            progressBar1.Value = (int)(progressBarProgress);
            this.Refresh();

            return;
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }
    }
 }
