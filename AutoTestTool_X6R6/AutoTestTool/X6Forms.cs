using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Threading.Tasks;
using System.Collections;
using System.IO;
using System.Threading;

namespace AutoTestTool
{
    public partial class X6Forms : Form
    {
        public X6Forms()
        {
            InitializeComponent();
        }

        /*****************************************定义类型*******************************************/
        enum X6Command
        {
            CMD_KEY_TEST = 0x01,               //按键测试
            CMD_CARD_TEST = 0x02,              //刷卡测试   
            CMD_LCD_TEST = 0x03,               //LCD测试
            CMD_2G_TEST = 0x04,                //2G测试
            CMD_TRUMPET_TEST = 0x05,           //喇叭测试
            CMD_RELAY_TEST = 0x06,             //继电器测试     
            CMD_SET_PCB = 0x07,                //设置PCB                    
            CMD_SET_SN = 0x08,                 //设置桩号
            CMD_BT_TEST = 0x09,                //蓝牙测试
            CMD_GET_FW = 0x0A,                 //获取软件版本
            CMD_READ_PCB_CODE = 0x0B,          //读取PCB编码
            CMD_SET_REGISTER_CODE = 0x0C,      //设置注册码
            CMD_SET_DEV_TYPE = 0x0D,           //设置设备类型
            CMD_SET_2_4G_GW_ADD = 0x0E,        //设置2.4G网关地址        
            CMD_SET_TERMINAL_INFO = 0x0F,      //设置中端信息
            CMD_SET_SERVER_ADDR = 0x10,        //设置服务器地址
            CMD_SET_SERVER_PORT = 0x11,        //设置服务器端口
            CMD_SET_PRINT_SWITCH = 0x12,       //设置打印开关
            CMD_REBOOT = 0x13,                 //设备重启
            CMD_SET_DEVICE_ID = 0x14,          //设置识别码
            CMD_SET_RTC = 0x15,                //设置RTC
            CMD_GET_RTC = 0x16,                //读取RTC
            CMD_FLASH_TEST = 0x17,             //flash测试


            CMD_LED_TEST = 0x1A,             //指示灯测试
            CMD_485_TEST = 0x1B,             //485测试
            CMD_ONLINE_TEST = 0x1C,             //联网测试
            CMD_4G_COMPATIBLE_2G = 0x1D,         //4G兼容2G
            CMD_GET_CHARGER_SN = 0x1E,          //获取桩编号
            CMD_GET_DEVICE_CODE = 0x1F,         //获取识别码
            CMD_START_AGING_TEST = 0x20,        //启动老化测试
            CMD_GET_AGING_TEST_RESULT = 0x21,   //获取老化结果


            CMD_FW_UPDATE_REQ = 0xF1,               //固件升级请求
            CMD_FW_SEND = 0xF2,                     //固件下发
            CMD_24G_COMMUNICATION_TEST = 0xF3,                     //2.4G通信测试
            TestMode = 0x99
        };

        enum X6TEST_MODE
        {
            TEST_MODE_START = 0x00,
            TEST_MODE_STOP
        };

        //struct DelayTimesFlag
        //{
        //    public int MainBoardPowerDelayFlag;
        //    public int MainBoardLEDDelayFlag;
        //    public int MainBoardTrumpetDelayFlag;
        //    public int MainBoardRelayDelayFlag;
        //    public int MainBoardFlashDelayFlag;

        //}


        struct X6GetResult
        {
            public int testMode;
            public int testModeAllow;
            public int key;
            public int[] keyValue;
            public int tapCard;
            public string cardNum;
            public int lcd;
            public int _2G;
            public int _2gCSQ;
            public string _2G_Iccid;
            public int trumpet;
            public int relay;
            public int measurementChip;
            public int[] getPower;
            public int SetCID;
            public int SetPcbCode;
            public int SetRegisterCode;
            public string MainBoardCode;
            public string InterfaceBoardCode;
            public int BLE;
            public string FwVersion;
            public UInt32 UsedTime_interface;
            public UInt32 UsedTime_main;
            public UInt32 UsedTime_Charger;
            public int BLE_24G;
        };
        struct X6CountDownTime
        {
            public int testMode;
            public int key;
            public int tapCard;
            public int lcd;
            public int _2G;
            public int trumpet;
            public int relay;
            public int PowerSource;
            public int SetCID;
            public int SetPcbCode;
            public int BLE;
            public int _2_4G;
            public int flash;
            public int setRtc;
            public int getRtc;

        };
        /*****************************************变量声明*******************************************/
        Dictionary<string, object> X6TestSettingInfo = new Dictionary<string, object>
        {
            {"ChargerModel","X6" },
            {"CountDown",30 },
            {"CardNum", "A1000000" },
            //{"CsqLowerLimit",20 },
            //{"CsqUpperLimit",60 },
            {"PowerLowerLimit",0 },
            {"PowerUpperLimit",1000 },
        };
        public static List<byte> arraybuffer = new List<byte> { };

        //DelayTimesFlag MainBoardTimesDelayFlag = new DelayTimesFlag
        //{
        //   MainBoardPowerDelayFlag = 0,
        //   MainBoardLEDDelayFlag = 0,
        //   MainBoardTrumpetDelayFlag = 0,
        //   MainBoardRelayDelayFlag = 0,
        //   MainBoardFlashDelayFlag = 0,
        //};

        X6GetResult X6GetResultObj = new X6GetResult
        {
            testMode = -1,
            testModeAllow = -1,
            key = -1,
            keyValue = new int[12],
            tapCard = -1,
            lcd = -1,
            _2G = -1,
            _2gCSQ = -1,
            _2G_Iccid = "",
            trumpet = -1,
            relay = -1,
            measurementChip = -1,
            SetPcbCode = -1,
            BLE = -1,
            SetRegisterCode = -1,
            cardNum = "",
            getPower = new int[12],
            FwVersion = "",
            UsedTime_interface = 0,
            UsedTime_main = 0,
            UsedTime_Charger = 0,
            MainBoardCode = "",
            InterfaceBoardCode = ""
        };

        X6CountDownTime X6countDownTime_MB = new X6CountDownTime
        {
            testMode = 0,
            key = 0,
            tapCard = 0,
            lcd = 0,
            _2G = 0,
            PowerSource = 0,
            trumpet = 0,
            relay = 0,
            SetCID = 0,
            SetPcbCode = 0,
            BLE = 0,
            //_2_4G = 0,
            flash = 0,
            setRtc = 0,
            getRtc = 0
        };
        X6CountDownTime X6countDownTime_SB = new X6CountDownTime
        {
            testMode = 0,
            key = 0,
            tapCard = 0,
            lcd = 0,
            _2G = 0,
            PowerSource = 0,
            trumpet = 0,
            relay = 0,
            SetCID = 0,
            SetPcbCode = 0,
            BLE = 0,
            //_2_4G = 0,
            flash = 0,
            setRtc = 0,
            getRtc = 0
        };
        X6CountDownTime X6countDownTimeCharger = new X6CountDownTime
        {
            testMode = 0,
            key = 0,
            tapCard = 0,
            lcd = 0,
            _2G = 0,
            PowerSource = 0,
            trumpet = 0,
            relay = 0,
            SetCID = 0,
            SetPcbCode = 0,
            BLE = 0,
            //_2_4G = 0,
            flash = 0,
            setRtc = 0,
            getRtc = 0
        };

        static byte X6sequence = 0;
        public static bool X6MBTestingFlag = false;
        public static bool X6SBTestingFlag = false;
        public static bool X6ChargerTestingFlag = false;
        Thread X6MBTestThread;
        Thread X6SBTestThread;
        Thread X6ChargerTestThread;
        static int X6MBTabSelectIndex;
        static int X6SBTabSelectIndex;
        static int X6chargerTestSelectIndex;
        static int X6PreMBTabSelectIndex = 0;
        static int X6PreSBTabSelectIndex;
        static int X6PrechargerTestSelectIndex;
        static int X6TestMeunSelectIndex;
        static int X6PCBATestSelectIndex;

        Dictionary<string, string> X6MBTestResultDir = new Dictionary<string, string>();
        Dictionary<string, string> X6SBTestResultDir = new Dictionary<string, string>();
        Dictionary<string, string> X6ChargerTestResultDir = new Dictionary<string, string>();

        UInt32 X6RtcCount = 0;//定时器计数
        int X6tick = 0;
        int X6countdownTime;
        //  bool MsgDebug = true;
        bool X6MsgDebug = false;
        string X6reportPath = @".\智能报表";
        bool X6OnlineFlg = false;
        bool X6onlineDectecFlag = false;
        Thread X6onlineDetectThread;
        public int X6OnlineDetectTime;
        UInt32 X6TestTimeTicks = 0;
        UInt32 X6cardNumTimeTicks = 0;
        UInt32 X6BlueToothTimeTicks = 0;
        UInt32 X624GTimeTicks = 0;
        UInt32 X6MBPowerTimeTicks = 0;
        UInt32 X6SBGetfwTicks = 0;
        UInt32 X6WholeCardNumTimeTicks = 0;

        private void X6Form1_Load(object sender, EventArgs e)
        {
            X6skinTabControl_X6TestMenu.SelectTab(X6skinTabPage_CurrentUser);
            X6skinTabPage_CurrentUser.Text = "用户:" + ProcTestData.PresentAccount;
            //if (ProcTestData.PresentAccount == "Admin")
            //{
            //    X6skinButton_AccountSettings.Visible = true;
            //}
            //else
            //{
            //    X6skinButton_AccountSettings.Visible = false;
            //}

            X6TestSettingInfo = ProcTestData.ReadConfig(ProcTestData.X6testConfigFile, X6TestSettingInfo);

            X6skinComboBox_ChgType.SelectedItem = X6TestSettingInfo["ChargerModel"];
            X6skinComboBox_ChgType.SelectedIndex = 0;
            X6skinNumericUpDown_TestOverTime.Value = Convert.ToDecimal(X6TestSettingInfo["CountDown"]);
            X6textBox_TestCardNum.Text = X6TestSettingInfo["CardNum"].ToString();
            X6skinNumericUpDown_PowerLowerLimit.Value = Convert.ToDecimal(X6TestSettingInfo["PowerLowerLimit"]);
            X6skinNumericUpDown_PowerUpperLimit.Value = Convert.ToDecimal(X6TestSettingInfo["PowerUpperLimit"]);

            X6timer.Enabled = true;
            X6timer.Start();

            try
            {
                if (Directory.Exists(X6reportPath) == false)
                {
                    Directory.CreateDirectory(X6reportPath);
                }

                //添加串口项目  
                foreach (string s in System.IO.Ports.SerialPort.GetPortNames())
                {//获取有多少个COM口  
                    skinComboBox_X6SerialPortSelect.Items.Add(s);
                }
                
                if (skinComboBox_X6SerialPortSelect.Items.Count > 0)
                {
                    //LOG("qqqqqqqqq " + skinComboBox_X6SerialPortSelect.Items.Count);
                    skinComboBox_X6SerialPortSelect.SelectedIndex = 0;
                    skinComboBox_X6SerialPortSelect.SelectedIndex = 0;
                }
            }
            catch (Exception ex)
            {
                //MessageBox.Show(ex.Message);
            }

            ProcTestData.DealBackUpData(ProcTestData.backupMysqlCmdFile);
        }

        private void X6Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            try
            {
                if (X6MBTestThread != null)
                {
                    if (X6MBTestThread.IsAlive)
                    {
                        X6MBTestThread.Abort();
                    }
                }

                if (X6SBTestThread != null)
                {
                    if (X6SBTestThread.IsAlive)
                    {
                        X6SBTestThread.Abort();
                    }
                }

                if (X6ChargerTestThread != null)
                {
                    if (X6ChargerTestThread.IsAlive)
                    {
                        X6ChargerTestThread.Abort();
                    }
                }


                this.Dispose();
                this.Close();
            }
            catch (Exception ex)
            {
                //MessageBox.Show(ex.Message, "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
            finally
            {
                Application.Exit();
            }
        }

        private void label1_Click(object sender, EventArgs e)
        {

        }

        private void X6ButtonOpenSerial_Click(object sender, EventArgs e)
        {
            try
            {
                if (!X6serialPort.IsOpen)
                {
                    X6serialPort.BaudRate = int.Parse(skinComboBox_X6SerialBuateSelect.SelectedItem.ToString());
                    X6serialPort.PortName = skinComboBox_X6SerialPortSelect.SelectedItem.ToString();
                    X6serialPort.Open();
                    if (X6serialPort.IsOpen)
                    {
                        X6ButtonOpenSerial.Text = "关闭串口";
                    }
                }
                else
                {
                    X6serialPort.Close();

                    if (!X6serialPort.IsOpen)
                    {
                        X6ButtonOpenSerial.Text = "打开串口";
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "异常提示", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void skinTabControl_X6TestMenu_SelectedIndexChanged(object sender, EventArgs e)
        {
            X6MBTestingFlag = false;
            X6SBTestingFlag = false;
            X6ChargerTestingFlag = false;
            X6TestMeunSelectIndex = X6skinTabControl_X6TestMenu.SelectedIndex;
            switch (X6skinTabControl_X6TestMenu.SelectedIndex)
            {
                case 0://测试设置
                    skinComboBox_X6SerialPortSelect.Focus();
                    break;
                case 1://PCBA测试
                    X6TabControl_PCBATest.SelectedIndex = 0;
                    X6skinTabControl_MB.SelectedIndex = 0;
                    X6textBox_MB_QRCode.Focus();
                    updateControlText(X6textBox_MB_QRCode, "");
                    updateControlText(X6textBox_SB_QR, "");
                    break;
                case 2://整机测试         
                    X6chargerTestSelectIndex = 0;
                    X6skinTabControl_WholeChg.SelectedIndex = 0;
                    X6textBox_WholeChg_SN_QR.Focus();
                    updateControlText(X6textBox_WholeChg_SN_QR, "");
                    break;
                
                default:
                    break;

            }
        }

        private void skinX6TabPage_MainBoard_Click(object sender, EventArgs e)
        {

        }
        
        //PCBA测试项索引监听
        private void X6TabControl_PCBATest_SelectedIndexChanged(object sender, EventArgs e)
        {
            X6PCBATestSelectIndex = X6TabControl_PCBATest.SelectedIndex;
            updateControlText(X6textBox_MB_QRCode, "");
            updateControlText(X6textBox_SB_QR, "");
            X6MBTestingFlag = false;
            X6SBTestingFlag = false;
            X6MBTabSelectIndex = 0;
            X6SBTabSelectIndex = 0;
            switch (X6TabControl_PCBATest.SelectedIndex)
            {
                case 0://主板
                    X6skinTabControl_MB.SelectedIndex = 0;
                    X6textBox_MB_QRCode.Focus();
                    break;

                case 1://副板
                    X6skinTabControl_SB.SelectedIndex = 0;
                    X6textBox_SB_QR.Focus();
                    break;
                default:
                    break;
            }
        }

        private void X6skinTabPage_SubBoard_Click(object sender, EventArgs e)
        {

        }

        private void skinTabControl1_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void X6skinTabPage_MainBoard_PCBANum_Click(object sender, EventArgs e)
        {

        }

        private void skinSplitContainer1_Panel1_Paint(object sender, PaintEventArgs e)
        {

        }

        private void X6skinTabPage_MB_STOPTEST_Click(object sender, EventArgs e)
        {

        }

        private void splitContainer1_Panel1_Paint(object sender, PaintEventArgs e)
        {
                    }

        private void skinSplitContainer1_Panel2_Paint(object sender, PaintEventArgs e)
        {

        }

        private void skinSplitContainer2_Panel1_Paint(object sender, PaintEventArgs e)
        {

        }

        private void skinLabel1_Click(object sender, EventArgs e)
        {

        }

        public static UInt32 GetCurrentTimeStamp()
        {
            TimeSpan ts = DateTime.UtcNow - new DateTime(1970, 1, 1, 0, 0, 0, 0);
            return Convert.ToUInt32(ts.TotalSeconds);
        }

        public void LOG(String text)
        {
            try
            {
                this.X6textBoxDebug.Invoke(
                    new MethodInvoker(delegate
                    {
                        this.X6textBoxDebug.AppendText(text + "\r\n");
                    }
                 )
                );

                this.X6textBoxDebugInfo.Invoke(
                    new MethodInvoker(delegate
                    {
                        this.X6textBoxDebugInfo.AppendText(text + "\r\n");
                    }
                 )
                );

                this.X6textBoxConfigPrint.Invoke(
                    new MethodInvoker(delegate
                    {
                        this.X6textBoxConfigPrint.AppendText(text + "\r\n");
                    }
                 )
                );
            }
            catch (Exception ex)
            {
                LOG(ex.Message);
                //MessageBox.Show(ex.Message);
            }
        }

        public void LOG_qwe(String text)
        {
            try
            {
                this.X6textBoxDebug.Invoke(
                    new MethodInvoker(delegate
                    {
                        this.X6textBoxDebug.AppendText(text + "\r\n");
                    }
                 )
                );
            }
            catch (Exception ex)
            {
                LOG(ex.Message);
                //MessageBox.Show(ex.Message);
            }
        }

        //串口发送
        private bool SendSerialData(byte[] data)
        {
            bool ret = false;

            try
            {
                if (X6serialPort != null)
                {
                    X6serialPort.Write(data, 0, data.Length);

                    if (X6MsgDebug)
                    {
                        string send = "";
                        for (int j = 0; j < data.Length; j++)
                        {
                            send += data[j].ToString("X2") + " ";
                        }
                        LOG("Send: " + send);
                    }
                    ret = true;
                }
            }
            catch (Exception ex)
            {
                LOG(ex.Message);
                ret = false;
            }
            return ret;
        }

        private static byte[] MakeSendArray(byte cmd, byte[] data)
        {
            UInt16 length;
            List<byte> list = new List<byte> { };
            byte[] srtDes = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

            list.Add(0xAA);
            list.Add(0x55);

            list.AddRange(srtDes);
            byte ver = 0x01;
            X6sequence++;

            if (data != null)
            {
                length = (UInt16)(1 + 1 + 1 + data.Length + 1);
            }
            else
            {
                length = 2;
            }

            list.Add((byte)(length));
            list.Add((byte)(length >> 8));
            list.Add(ver);
            list.Add(X6sequence);
            list.Add(cmd);
            if (data != null)
            {
                list.AddRange(data);
            }

            list.Add(ProcTestData.caculatedCRC(list.ToArray(), list.Count));

            return list.ToArray();
        }

        //发送测试请求
        private void SendTestModeReq(byte mode)
        {
            byte[] data = { mode };
            int wait = 0, n = 0;

            X6GetResultObj.testMode = -1;
            X6GetResultObj.testModeAllow = -1;

            SendSerialData(MakeSendArray((byte)X6Command.TestMode, data));

            while (X6GetResultObj.testMode == -1)
            {
                Thread.Sleep(500);
                if (wait++ > 10)
                {
                    wait = 0;
                    n++;
                    SendSerialData(MakeSendArray((byte)X6Command.TestMode, data));
                }

                if (n > 3)
                {
                    break;
                }
            }

            if (n > 10)
            {
                if (MessageBox.Show((mode == 0) ? "请求开始失败！\r\n是否重试" : "请求结束失败！\r\n是否重试", "提示",
                    MessageBoxButtons.RetryCancel, MessageBoxIcon.Warning) == DialogResult.Retry)
                {
                    SendTestModeReq(mode);
                }
            }
        }

        //发送刷卡测试指令0x02
        private void SendLedTestReq()
        {
            SendSerialData(MakeSendArray((byte)X6Command.CMD_LED_TEST, null));
        }

        public int getX6_24G_Flag = -1;
        //发送读取蓝牙指令0x09
        private void X6Send24G_COMMUNICATION_TestReq()
        {
            int wait = 0, n = 0;
            SendSerialData(MakeSendArray((byte)X6Command.CMD_24G_COMMUNICATION_TEST, null));

            while (getX6_24G_Flag == -1)
            {
                Thread.Sleep(30);
                if (wait++ > 10)
                {
                    wait = 0;
                    n++;
                    SendSerialData(MakeSendArray((byte)X6Command.CMD_24G_COMMUNICATION_TEST, null));
                }
                if (n > 10)
                {
                    break;
                }

            }
        }

        private void X6SendTapCard()
        {
            X6GetResultObj.tapCard = -1;
            SendSerialData(MakeSendArray((byte)X6Command.CMD_CARD_TEST, null));
            int wait = 0, n = 0;

            while (X6GetResultObj.tapCard == -1)
            {
                Thread.Sleep(50);
                if (wait++ > 10)
                {
                    wait = 0;
                    n++;
                    SendSerialData(MakeSendArray((byte)X6Command.CMD_CARD_TEST, null));

                }
                if (n > 10)
                {
                    break;
                }
            }
        }

        public int getBlueToothFlag = -1;
        //发送读取蓝牙指令0x09
        private void X6SendBlueToothTestReq()
        {
            int wait = 0, n = 0;
            SendSerialData(MakeSendArray((byte)X6Command.CMD_BT_TEST, null));

            while (getBlueToothFlag == -1)
            {
                Thread.Sleep(30);
                if (wait++ > 10)
                {
                    wait = 0;
                    n++;
                    SendSerialData(MakeSendArray((byte)X6Command.CMD_BT_TEST, null));
                    LOG("整机发送蓝牙测试请求!");
                }
                if (n > 10)
                {
                    break;
                }

            }
        }

        //发送喇叭测试指令0x05
        private void SendTrumptTestReq()
        {
            SendSerialData(MakeSendArray((byte)X6Command.CMD_TRUMPET_TEST, null));
        }

        private void X6SendLedTestReq(byte operate, byte ch)
        {
            byte[] data = { operate, ch };
            X6GetResultObj.relay = -1;
            SendSerialData(MakeSendArray((byte)X6Command.CMD_LED_TEST, data));
        }

        private void X6ShowMainboardResult()
        {
            updateControlText(X6MB_PCB_RESULT_VAL, X6MBTestResultDir["PCB编号"], Color.Black);
            updateControlText(X6MB_TESTOR_RESULT_VAL, X6MBTestResultDir["测试员"], Color.Black);
            updateControlText(X6MB_FW_RESULT_VAL, X6MBTestResultDir["软件版本"], Color.Black);
            updateControlText(X6MB_ALL_RESULT_VAL, X6MBTestResultDir["测试结果"], X6decideColor(X6MBTestResultDir["测试结果"]));
            updateControlText(X6MB_POWER_RESULT_VAL, X6MBTestResultDir["电源"], X6decideColor(X6MBTestResultDir["电源"]));
            updateControlText(X6MB_LED_RESULT_VAL, X6MBTestResultDir["指示灯"], X6decideColor(X6MBTestResultDir["指示灯"]));
            updateControlText(X6MB_TRUMPT_RESULT_VAL, X6MBTestResultDir["喇叭"], X6decideColor(X6MBTestResultDir["喇叭"]));
            updateControlText(X6MB_RELAY_RESULT_VAL, X6MBTestResultDir["继电器"], X6decideColor(X6MBTestResultDir["继电器"]));
            updateControlText(X6MB_FLASH_RESULT_VAL, X6MBTestResultDir["FLASH"], X6decideColor(X6MBTestResultDir["FLASH"]));
            updateControlText(X6MB_SET_RTC_RESULT_VAL, X6MBTestResultDir["SETRTC"], X6decideColor(X6MBTestResultDir["SETRTC"]));
            updateControlText(X6MB_GET_RTC_RESULT_VAL, X6MBTestResultDir["GETRTC"], X6decideColor(X6MBTestResultDir["GETRTC"]));
            updateControlText(X6MB_TEST_USED_TIME_VAL, X6MBTestResultDir["测试用时"], Color.Black);
            updateControlText(X6skinLabel_MB_TEST_START_TIME, X6MBTestResultDir["测试时间"], Color.Black);

        }

        private void X6ShowSubBoardResult()
        {
            updateControlText(X6skinLabel_SB_RESULT_VAL, X6SBTestResultDir["PCB编号"], Color.Black);
            updateControlText(X6skinLabel_SB_TESTOR_RESULT_VAL, X6SBTestResultDir["测试员"], Color.Black);
            updateControlText(X6skinLabel_SB_FW_RESULT_VAL, X6SBTestResultDir["软件版本"], Color.Black);
            updateControlText(X6skinLabel_SB_ALL_RESULT_VAL, X6SBTestResultDir["测试结果"], X6decideColor(X6SBTestResultDir["测试结果"]));
            updateControlText(X6skinLabel_SB_Card_RESULT_VAL, X6SBTestResultDir["刷卡"] + "  卡号:" + X6SBTestResultDir["卡号"], X6decideColor(X6SBTestResultDir["刷卡"]));
            updateControlText(X6skinLabel_SB__BT_RESULT_VAL, X6SBTestResultDir["蓝牙"], X6decideColor(X6SBTestResultDir["蓝牙"]));
            updateControlText(X6skinLabel_SB_2_4G_VAL, X6SBTestResultDir["2.4G"], X6decideColor(X6SBTestResultDir["2.4G"]));
            updateControlText(X6skinLabel_SB_TEST_USETIME_VAL, X6SBTestResultDir["测试用时"], Color.Black);
            updateControlText(X6skinLabel_SB_TEST_START_TIME, X6SBTestResultDir["测试时间"], Color.Black);
        }

        private void X6ShowChgBoardResult()
        {
            updateControlText(X6skinLabel_CHG_STATION_ID_RESLUT_VAL, X6ChargerTestResultDir["电桩号"], Color.Black);
            updateControlText(X6skinLabel_CHG_MB_QR_RES_VAL, X6ChargerTestResultDir["主板编号"], Color.Black);
            updateControlText(X6skinLabel_CHG_TESTOR_RES_VAL, X6ChargerTestResultDir["测试员"], Color.Black);
            updateControlText(X6skinLabel_CHG_FW_RES_VAL, X6ChargerTestResultDir["软件版本"], Color.Black);
            updateControlText(X6skinLabeL_CHG_TEST_RES_VAL, X6ChargerTestResultDir["测试结果"], X6decideColor(X6ChargerTestResultDir["测试结果"]));
            updateControlText(X6skinLabel_CHG_LED_RES_VAL, X6ChargerTestResultDir["指示灯"], X6decideColor(X6ChargerTestResultDir["指示灯"]));
            updateControlText(X6skinLabel_CHG_TRUMPET_RES_VAL, X6ChargerTestResultDir["喇叭"], X6decideColor(X6ChargerTestResultDir["喇叭"]));
            updateControlText(X6skinLabel_CHG_BT_RES_VAL, X6ChargerTestResultDir["蓝牙"], X6decideColor(X6ChargerTestResultDir["蓝牙"]));
            updateControlText(X6skinLabel_WHOLE_TEST_Card_RESULT_VAL, X6ChargerTestResultDir["刷卡"] + "  卡号:" + X6ChargerTestResultDir["卡号"], X6decideColor(X6ChargerTestResultDir["刷卡"]));
            updateControlText(X6skinLabel_CHG_24G_RES_VAL, X6ChargerTestResultDir["2.4G"], X6decideColor(X6ChargerTestResultDir["2.4G"]));
            updateControlText(X6skinLabel_CHG_RELAY_RES_VAL, X6ChargerTestResultDir["继电器"], X6decideColor(X6ChargerTestResultDir["继电器"]));
            updateControlText(X6WHOLE_SET_RTC_RESULT_VAL, X6ChargerTestResultDir["SETRTC"], X6decideColor(X6ChargerTestResultDir["SETRTC"]));
            updateControlText(X6WHOL_TEST_GET_RTC_RESULT_VAL, X6ChargerTestResultDir["GETRTC"], X6decideColor(X6ChargerTestResultDir["GETRTC"]));
            updateControlText(X6skinLabel_CHG_TEST_USEDTIME_RES_VAL, X6ChargerTestResultDir["测试用时"], Color.Black);
            updateControlText(X6skinLabel_CHG_TEST_TIME_RES_VAL, X6ChargerTestResultDir["测试时间"], Color.Black);
        }

        private void X6SendGetPcdCode(byte operate)
        {
            byte[] data = { operate };
            SendSerialData(MakeSendArray((byte)X6Command.CMD_READ_PCB_CODE, data));
            int waittime = 0, n = 0;

            if (operate == 0)
            {
                X6GetResultObj.MainBoardCode = "";
                SendSerialData(MakeSendArray((byte)X6Command.CMD_READ_PCB_CODE, data));
                while (X6GetResultObj.MainBoardCode == "")
                {
                    Thread.Sleep(300);
                    waittime++;
                    if (waittime > 10)
                    {
                        n++;
                        waittime = 0;
                        SendSerialData(MakeSendArray((byte)X6Command.CMD_READ_PCB_CODE, data));
                    }
                    if (n > 3)
                    {
                        break;
                    }
                }
            }
            else if (operate == 1)
            {
                X6GetResultObj.InterfaceBoardCode = "";
                SendSerialData(MakeSendArray((byte)X6Command.CMD_READ_PCB_CODE, data));
                while ((X6GetResultObj.InterfaceBoardCode == ""))
                {
                    Thread.Sleep(300);
                    waittime++;
                    if (waittime > 10)
                    {
                        n++;
                        waittime = 0;
                        SendSerialData(MakeSendArray((byte)X6Command.CMD_READ_PCB_CODE, data));
                    }
                    if (n > 3)
                    {
                        break;
                    }
                }
            }

            if (n > 3)
            {
                if (MessageBox.Show("获取PCB编号失败！\r\n是否重试", "提示", MessageBoxButtons.RetryCancel, MessageBoxIcon.Warning) == DialogResult.Retry)
                {
                    X6SendGetPcdCode(operate);
                }
            }
        }

        private void X6SendGetFwVersionReq(byte operate)
        {
            byte[] data = { operate };
            X6GetResultObj.FwVersion = "";
            SendSerialData(MakeSendArray((byte)X6Command.CMD_GET_FW, data));
            int waittime = 0, n = 0;
            while (X6GetResultObj.FwVersion == "")
            {
                Thread.Sleep(100);
                waittime++;
                if (waittime > 10)
                {
                    LOG("获取软件版本.\n" + GetCurrentTimeStamp());
                    n++;
                    waittime = 0;
                    SendSerialData(MakeSendArray((byte)X6Command.CMD_GET_FW, data));
                }
                if (n > 6)
                {
                    break;
                }
            }
            if (n > 5)
            {
                if (MessageBox.Show("获取PCB软件版本失败！\r\n是否重试", "提示", MessageBoxButtons.RetryCancel, MessageBoxIcon.Warning) == DialogResult.Retry)
                {
                    X6SendGetFwVersionReq(operate);
                }
            }
        }

        private void X6SendSetPcbCodeReq(byte type, string code)
        {
            List<byte> data = new List<byte>();
            data.Add(type);
            string str = ProcTestData.fillString(code, 16, '0', 0);
            data.AddRange(ProcTestData.stringToBCD(str));
            X6GetResultObj.SetPcbCode = -1;
            SendSerialData(MakeSendArray((byte)X6Command.CMD_SET_PCB, data.ToArray()));
            int wait = 0, n = 0;
            while (X6GetResultObj.SetPcbCode == -1)
            {
                Thread.Sleep(300);
                if (wait++ > 10)
                {
                    wait = 0;
                    n++;
                    SendSerialData(MakeSendArray((byte)X6Command.CMD_SET_PCB, data.ToArray()));

                }
                if (n > 3)
                {
                    break;
                }

            }

            if (n > 10)
            {
                if (MessageBox.Show("PCB编号设置失败！\r\n是否重试", "提示", MessageBoxButtons.RetryCancel, MessageBoxIcon.Warning) == DialogResult.Retry)
                {
                    X6SendSetPcbCodeReq(type, code);
                }
            }
        }

        //发送指令0x13
        private void SendDevReboot()
        {
            SendSerialData(MakeSendArray((byte)X6Command.CMD_REBOOT, null));
        }

        int X6SendOldTestFlag = 0;
        private void X6SendOldTest()
        {
            try
            {
                byte[] data = { 0x00 };
                //byte[] data = Encoding.Default.GetBytes(addr);
                SendSerialData(MakeSendArray((byte)X6Command.CMD_START_AGING_TEST, data));
                int wait = 0, n = 0;
                while (X6SendOldTestFlag == 0)
                {
                    Thread.Sleep(200);
                    if (wait++ > 10)
                    {
                        wait = 0;
                        LOG("正在请求老机测试");
                        SendSerialData(MakeSendArray((byte)X6Command.CMD_START_AGING_TEST, data));
                        n++;
                    }
                    if (n > 10)
                    {
                        break;
                    }
                }
            }
            catch (Exception ex)
            {
                //updateText("异常：" + ex.Message);
            }
        }

        int X6SetLocal2_4GAddrFlag = 0;
        private void SendSetLocal2_4GAddr(string addrStr)
        {
            try
            {
                string str = ProcTestData.fillString(addrStr, 10, '0', 0);
                byte[] data = ProcTestData.stringToBCD(str);
                //byte[] data = Encoding.Default.GetBytes(addr);
                SendSerialData(MakeSendArray((byte)X6Command.CMD_SET_TERMINAL_INFO, data));
                int wait = 0, n = 0;
                while (X6SetLocal2_4GAddrFlag == 0)
                {
                    Thread.Sleep(200);
                    if (wait++ > 10)
                    {
                        wait = 0;
                        LOG("正在设置X6 2.4g地址...");
                        SendSerialData(MakeSendArray((byte)X6Command.CMD_SET_TERMINAL_INFO, data));
                        n++;
                    }
                    if (n > 10)
                    {
                        break;
                    }
                }
            }
            catch (Exception ex)
            {
                //updateText("异常：" + ex.Message);
            }
        }

        int X6SetGwAddrFlag = 0;
        private void SendSetGwAddr(string addrStr)
        {
            try
            {
                //string[] addr = addrStr.Split('.');
                //byte[] data = new byte[addr.Length];

                //for (int i = 0; i < data.Length; i++)
                //{
                //    data[i] = Convert.ToByte(addr[i]);
                //}

                string str = ProcTestData.fillString(addrStr, 10, '0', 0);
                byte[] data = ProcTestData.stringToBCD(str);
                //byte[] data = Encoding.Default.GetBytes(addr);
                SendSerialData(MakeSendArray((byte)X6Command.CMD_SET_2_4G_GW_ADD, data));
                int wait = 0, n = 0;
                while (X6SetGwAddrFlag == 0)
                {
                    Thread.Sleep(200);
                    if (wait++ > 10)
                    {
                        wait = 0;
                        LOG("正在设置网关地址...");
                        SendSerialData(MakeSendArray((byte)X6Command.CMD_SET_2_4G_GW_ADD, data));
                        n++;
                    }
                    if (n > 3)
                    {
                        break;
                    }
                }
            }
            catch (Exception ex)
            {

                //updateText("异常：" + ex.Message);
            }

        }

        //设置桩号
        private void SendSetID(string id)
        {
            string str = ProcTestData.fillString(id, 16, '0', 0);
            byte[] data = ProcTestData.stringToBCD(str);
            X6GetResultObj.SetCID = -1;

            SendSerialData(MakeSendArray((byte)X6Command.CMD_SET_SN, data));
            int wait = 0, n = 0;
            while (X6GetResultObj.SetCID == -1)
            {
                Thread.Sleep(200);
                if (wait++ > 10)
                {
                    wait = 0;
                    SendSerialData(MakeSendArray((byte)X6Command.CMD_SET_SN, data));
                    n++;
                }
                if (n > 3)
                {
                    break;
                }
            }

            if (n > 3)
            {
                if (MessageBox.Show("桩号设置失败！\r\n是否重试", "提示", MessageBoxButtons.RetryCancel, MessageBoxIcon.Warning) == DialogResult.Retry)
                {
                    SendSetID(id);
                }
            }
        }
        
        //发送继电器测试指令0x06
        private void SendRelayTestReq(byte operate, byte ch)
        {
            byte[] data = { operate, ch };
            X6GetResultObj.relay = -1;
            SendSerialData(MakeSendArray((byte)X6Command.CMD_RELAY_TEST, data));
        }


        //发送读取Rtc指令0x16
        private void SendGetRtcTestReq()
        {
            SendSerialData(MakeSendArray((byte)X6Command.CMD_GET_RTC, null));
        }

        //发送Flash测试指令0x17
        private void SendFlashTestReq()
        {
            SendSerialData(MakeSendArray((byte)X6Command.CMD_FLASH_TEST, null));
        }

        //发送设置Rtc指令0x15
        private void SendSetRtcTestReq()
        {
            DateTime dt = DateTime.UtcNow;
            byte[] data = new byte[4];

            UInt32 currentUtc = GetCurrentTimeStamp();
            LOG("currentUtc:" + currentUtc.ToString());

            data[0] = (byte)((currentUtc >> 24) & 0xFF);
            data[1] = (byte)((currentUtc >> 16) & 0xFF);
            data[2] = (byte)((currentUtc >> 8) & 0xFF);
            data[3] = (byte)((currentUtc >> 0) & 0xFF);

            SendSerialData(MakeSendArray((byte)X6Command.CMD_SET_RTC, data));
        }

        private void X6skinButton_MB_Confirm_Click(object sender, EventArgs e)
        {
            X6skinButton_PCBA_STARTTEST_Click(sender, e);
        }
        
        private void textBoxDebug_TextChanged(object sender, EventArgs e)
        {

        }

        private void tableLayoutPanel1_Paint(object sender, PaintEventArgs e)
        {

        }

        private void X6skinButton_PCBA_STARTTEST_Click(object sender, EventArgs e)
        {
            if ((X6textBox_MB_QRCode.Text == "" && X6TabControl_PCBATest.SelectedTab == X6skinTabPage_MainBoard)
                    || (X6textBox_SB_QR.Text == "" && X6TabControl_PCBATest.SelectedTab == X6skinTabPage_SubBoard))
            {
                MessageBox.Show("PCB编码不能为空", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                X6textBox_MB_QRCode.Text = "";
                X6textBox_SB_QR.Text = "";
                X6TestSettingInfo["ChargerModel"] = X6skinComboBox_ChgType.SelectedItem;
                return;
            }

            X6PCBATestSelectIndex = X6TabControl_PCBATest.SelectedIndex;

            if (X6TabControl_PCBATest.SelectedIndex == 0)//主板
            {
                if (X6MBTestingFlag == false)
                {
                    LOG("主板请求开始测试.");
                    SendTestModeReq((byte)X6TEST_MODE.TEST_MODE_START);
                }
                else
                {
                    LOG("主板请求结束测试.");
                    SendTestModeReq((byte)X6TEST_MODE.TEST_MODE_STOP);
                }
            }
            else if (X6TabControl_PCBATest.SelectedIndex == 1) //副板
            {
                if (X6SBTestingFlag == false)
                {
                    LOG("副板请求开始测试.");
                    SendTestModeReq((byte)X6TEST_MODE.TEST_MODE_START);
                }
                else
                {
                    LOG("副板请求结束测试.");
                    SendTestModeReq((byte)X6TEST_MODE.TEST_MODE_STOP);
                }
            }
        }

        private void X6skinButton_PCBA_CLEAR_LOG_Click(object sender, EventArgs e)
        {
             X6textBoxDebug.Text = "";
        }

        private void X6skinButtonPCBA_REPORTDIR_Click(object sender, EventArgs e)
        {
            try
            {
                System.Diagnostics.Process.Start("explorer.exe", X6reportPath);
            }
            catch (Exception ex)
            {
                LOG(ex.Message);
            }
        }

        private void skinSplitContainer9_Panel2_Paint(object sender, PaintEventArgs e)
        {

        }
        
        //副板测试项索引更新监听
        private void X6skinTabControl_SB_SelectedIndexChanged(object sender, EventArgs e)
        {
            X6SBTabSelectIndex = X6skinTabControl_SB.SelectedIndex;
            switch (X6skinTabControl_SB.SelectedIndex)
            {
                case 0:
                    X6textBox_SB_QR.Focus();
                    break;
                default:
                    break;
            }

        }
        private void skinSplitContainer2_Panel2_Paint(object sender, PaintEventArgs e)
        {

        }

        private void skinSplitContainer2_Panel1_Paint_1(object sender, PaintEventArgs e)
        {

        }
        /*
        //PCBA开始测试
        private void skinButton_PCBA_STARTTEST_Click(object sender, EventArgs e)
        {
            if ((X6textBox_MB_QRCode.Text == "" && X6TabControl_PCBATest.SelectedTab == X6skinTabPage_MainBoard)
                    || (X6textBox_SB_QR.Text == "" && X6TabControl_PCBATest.SelectedTab == X6skinTabPage_SubBoard))
            {
                MessageBox.Show("PCB编码不能为空", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                X6textBox_MB_QRCode.Text = "";
                X6textBox_SB_QR.Text = "";
                X6TestSettingInfo["ChargerModel"] = X6skinComboBox_ChgType.SelectedItem;
                return;
            }

            X6PCBATestSelectIndex = X6TabControl_PCBATest.SelectedIndex;

            if (X6TabControl_PCBATest.SelectedIndex == 0)//主板
            {
                if (X6MBTestingFlag == false)
                {
                    LOG("主板请求开始测试.");
                    SendTestModeReq((byte)X6TEST_MODE.TEST_MODE_START);
                }
                else
                {
                    LOG("主板请求结束测试.");
                    SendTestModeReq((byte)X6TEST_MODE.TEST_MODE_STOP);
                }
            }
            else if (X6TabControl_PCBATest.SelectedIndex == 1) //副板
            {
                if (X6SBTestingFlag == false)
                {
                    LOG("副板请求开始测试.");
                    SendTestModeReq((byte)X6TEST_MODE.TEST_MODE_START);
                }
                else
                {
                    LOG("副板请求结束测试.");
                    SendTestModeReq((byte)X6TEST_MODE.TEST_MODE_STOP);
                }
            }
        }
        */

        private void X6skinButton_SB_CONFIRM_Click(object sender, EventArgs e)
        {
            X6skinButton_PCBA_STARTTEST_Click(sender, e);
        }

        private void X6textBox_SB_QR_TextChanged(object sender, EventArgs e)
        {

        }

        private void label5_Click(object sender, EventArgs e)
        {

        }

        private void skinSplitContainer3_Panel2_Paint(object sender, PaintEventArgs e)
        {

        }
        //更新控件的文字内容
        private void updateControlText(Control control, string text)
        {
            try
            {
                control.Invoke(
                new MethodInvoker(delegate {
                    control.Text = text;
                }
              )
           );
            }
            catch (Exception ex)
            {
                //MessageBox.Show(ex.Message);
                LOG(ex.Message);
            }
        }
        //更新控件的文字内容及颜色
        private void updateControlText(Control control, string text, Color color)
        {
            try
            {
                control.Invoke(
                new MethodInvoker(delegate {

                    control.Text = text;
                    control.ForeColor = color;
                }
              )
           );
            }
            catch (Exception ex)
            {
                //MessageBox.Show(ex.Message);
                LOG(ex.Message);
            }

        }

        //更新table控件的索引
        private void updateTableSelectedIndex(TabControl tabControl, int index)
        {
            try
            {
                tabControl.Invoke(
                new MethodInvoker(delegate {

                    tabControl.SelectedIndex = index;
                }
              )
           );
            }
            catch (Exception ex)
            {
                //MessageBox.Show(ex.Message);
                LOG(ex.Message);
            }
        }

        private void X6skinButton_MB_Power_Success_Click(object sender, EventArgs e)
        {
            LOG("主板检测电源成功.");
            X6MBTestResultDir["电源"] = "通过";
            updateControlText(X6skinLabel_MB_POWER_RESULT, "通过", Color.Green);
            if (6 <= (GetCurrentTimeStamp() - X6MBPowerTimeTicks))
            {
                X6MBPowerTimeTicks = GetCurrentTimeStamp();
                updateTableSelectedIndex(X6skinTabControl_MB, ++X6MBTabSelectIndex);
            }
        }

        //主板电源测试失败
        private void X6skinButton_MB_Power_Fail_Click(object sender, EventArgs e)
        {
            LOG("主板检测电源失败.");
            X6MBTestResultDir["电源"] = "不通过";
            updateControlText(X6skinLabel_MB_POWER_RESULT, "不通过", Color.Red);
            if (6 <= (GetCurrentTimeStamp() - X6MBPowerTimeTicks))
            {
                X6MBPowerTimeTicks = GetCurrentTimeStamp();
                updateTableSelectedIndex(X6skinTabControl_MB, ++X6MBTabSelectIndex);
            }
        }

        //主板电源测试跳过
        private void X6skinButton_MB_Power_Over_Click(object sender, EventArgs e)
        {
            LOG("跳过主板检测电源.");
            X6MBTestResultDir["电源"] = "跳过";
            updateControlText(X6skinLabel_MB_POWER_RESULT, "跳过", Color.Green);
            if (6 <= (GetCurrentTimeStamp() - X6MBPowerTimeTicks))
            {
                X6MBPowerTimeTicks = GetCurrentTimeStamp();
                updateTableSelectedIndex(X6skinTabControl_MB, ++X6MBTabSelectIndex);
            }
        }

        //主板电源重新测试
        private void X6skinButton_MB_Power_rTest_Click(object sender, EventArgs e)
        {
            ItemTestTime = GetCurrentTimeStamp();
            X6countDownTime_MB.PowerSource = X6countdownTime;
        }

        //主板测试项索引更新监听
        private void X6skinTabControl_MB_SelectedIndexChanged(object sender, EventArgs e)
        {
            X6MBTabSelectIndex = X6skinTabControl_MB.SelectedIndex;
            if (X6skinTabControl_MB.SelectedIndex == 0)
            {
                X6textBox_MB_QRCode.Focus();
            }
        }

        private void skinSplitContainer11_Panel1_Paint(object sender, PaintEventArgs e)
        {

        }

        private void skinSplitContainer11_Panel2_Paint(object sender, PaintEventArgs e)
        {

        }

        private void skinSplitContainer4_Panel1_Paint(object sender, PaintEventArgs e)
        {

        }

        private void skinSplitContainer4_Panel2_Paint(object sender, PaintEventArgs e)
        {

        }

        private void skinSplitContainer4_SplitterMoved(object sender, SplitterEventArgs e)
        {

        }

        private void skinSplitContainer8_Panel2_Paint(object sender, PaintEventArgs e)
        {

        }

        private void X6textBoxDebug_TextChanged(object sender, EventArgs e)
        {

        }

        private void skinLabel6_Click(object sender, EventArgs e)
        {

        }

        private void skinLabel8_Click(object sender, EventArgs e)
        {

        }

        private void X6skinButton_MB_LED_SUCCESS_Click(object sender, EventArgs e)
        {
            LOG("主板检测指示灯成功.");
            X6MBTestResultDir["指示灯"] = "通过";
            updateControlText(X6skinLabel_MB_LED_RESULT, "通过", Color.Green);
            updateTableSelectedIndex(X6skinTabControl_MB, ++X6MBTabSelectIndex);
        }

        private void X6skinButton_MB_LED_FALI_Click(object sender, EventArgs e)
        {
            LOG("主板检测指示灯失败.");
            X6MBTestResultDir["指示灯"] = "不通过";
            updateControlText(X6skinLabel_MB_LED_RESULT, "不通过", Color.Red);
            updateTableSelectedIndex(X6skinTabControl_MB, ++X6MBTabSelectIndex);
        }

        private void X6skinButton_MB_LED_OVER_Click(object sender, EventArgs e)
        {
            LOG("主板跳过检测指示灯.");
            //MBTestResultDir["指示灯"] = "跳过";
            updateControlText(X6skinLabel_MB_LED_RESULT, "跳过", Color.Green);
            updateTableSelectedIndex(X6skinTabControl_MB, ++X6MBTabSelectIndex);
        }

        private void X6skinButton_MB_LED_RTEST_Click(object sender, EventArgs e)
        {
            ItemTestTime = GetCurrentTimeStamp();
            X6countDownTime_MB.lcd = X6countdownTime;
            X6MBTestResultDir["指示灯"] = "";
            updateControlText(X6skinLabel_MB_LED_RESULT, "");
            LOG("主板指示灯重新测试.");
            //发送指示灯测试指令
            SendLedTestReq();
        }

        private void X6skinLabel_MB_LED_RESULT_Click(object sender, EventArgs e)
        {

        }

        private void skinLabel21_Click(object sender, EventArgs e)
        {

        }

        private void skinSplitContainer5_Panel2_Paint(object sender, PaintEventArgs e)
        {

        }

        private void skinLabel23_Click(object sender, EventArgs e)
        {

        }

        private void X6skinButton_MB_TRUMPT_SUCCESS_Click(object sender, EventArgs e)
        {
            LOG("主板喇叭测试成功.");
            X6MBTestResultDir["喇叭"] = "通过";
            updateControlText(X6skinLabel_MB_TRUMPT_RESULT, "通过", Color.Green);
            updateTableSelectedIndex(X6skinTabControl_MB, ++X6MBTabSelectIndex);
        }

        private void X6skinButton_MB_TRUMPT_FAIL_Click(object sender, EventArgs e)
        {
            LOG("主板喇叭测试失败.");
            X6MBTestResultDir["喇叭"] = "不通过";
            updateControlText(X6skinLabel_MB_TRUMPT_RESULT, "不通过", Color.Red);
            updateTableSelectedIndex(X6skinTabControl_MB, ++X6MBTabSelectIndex);
        }

        private void X6skinButton_MB_TRUMPT_SKIP_Click(object sender, EventArgs e)
        {
            LOG("跳过主板喇叭测试.");
            //MBTestResultDir["喇叭"] = "跳过";
            updateControlText(X6skinLabel_MB_TRUMPT_RESULT, "跳过", Color.Green);
            updateTableSelectedIndex(X6skinTabControl_MB, ++X6MBTabSelectIndex);
        }

        private void X6skinButton_MB_TRUMPT_RTEST_Click(object sender, EventArgs e)
        {
            ItemTestTime = GetCurrentTimeStamp();
            X6countDownTime_MB.trumpet = X6countdownTime;
            X6MBTestResultDir["喇叭"] = "";
            updateControlText(X6skinLabel_MB_TRUMPT_RESULT, "");
            LOG("主板喇叭重新测试.");
            //发送喇叭测试指令
            SendTrumptTestReq();
        }

        private void skinSplitContainer6_Panel1_Paint(object sender, PaintEventArgs e)
        {

        }

        private void skinLabel65_Click(object sender, EventArgs e)
        {

        }

        private void skinSplitContainer6_Panel2_Paint(object sender, PaintEventArgs e)
        {

        }

        private void X6skinButtonRelaySkip_Click(object sender, EventArgs e)
        {
            LOG("跳过主板继电器测试.");
            //MBTestResultDir["继电器"] = "跳过";
            updateControlText(X6skinLabel_MB_RELAY_RESULT, "跳过", Color.Green);
            updateTableSelectedIndex(X6skinTabControl_MB, ++X6MBTabSelectIndex);
        }

        private void X6skinButtonRelayReTest_Click(object sender, EventArgs e)
        {
            ItemTestTime = GetCurrentTimeStamp();
            X6countDownTime_MB.relay = X6countdownTime;
            X6MBTestResultDir["继电器"] = "";
            updateControlText(X6skinLabel_MB_RELAY_RESULT, "");
            LOG("主板继电器重新测试.");
            //发送继电器测试指令
            SendRelayTestReq(0x00, 0x00);
        }

        private void skinLabel70_Click(object sender, EventArgs e)
        {

        }

        private void skinLabel69_Click(object sender, EventArgs e)
        {

        }

        private void skinLabel_MB_FLASH_TIME_Click(object sender, EventArgs e)
        {

        }

        private void skinSplitContainer7_Panel1_Paint(object sender, PaintEventArgs e)
        {

        }

        private void skinSplitContainer7_Panel2_Paint(object sender, PaintEventArgs e)
        {

        }

        private void X6skinButton_MB_FLASH_RTEST_Click(object sender, EventArgs e)
        {
            ItemTestTime = GetCurrentTimeStamp();
            X6countDownTime_MB.flash = X6countdownTime;
            X6MBTestResultDir["FLASH"] = "";
            updateControlText(X6skinLabel_MB_FLASH_RESULT, "");
            LOG("主板FLASH重新测试.");
            //发送FLASH测试指令
            SendFlashTestReq();
        }

        private void X6skinButton_MB_FLASH_SKIP_Click(object sender, EventArgs e)
        {
            LOG("跳过主板FLASH测试.");
            //MBTestResultDir["Flash"] = "跳过";
            updateControlText(X6skinLabel_MB_FLASH_RESULT, "跳过", Color.Green);
            updateTableSelectedIndex(X6skinTabControl_MB, ++X6MBTabSelectIndex);
        }

        private void X6skinButton_MB_SETRTC_SKIP_Click(object sender, EventArgs e)
        {
            LOG("跳过主板设置RTC时间.");
            //MBTestResultDir["setRtc"] = "跳过";
            updateControlText(X6skinLabel_MB_SETRTC_RESULT, "跳过", Color.Green);
            updateTableSelectedIndex(X6skinTabControl_MB, ++X6MBTabSelectIndex);
        }

        private void X6skinButton_MB_SETRTC_RTEST_Click(object sender, EventArgs e)
        {
            ItemTestTime = GetCurrentTimeStamp();
            X6countDownTime_MB.setRtc = X6countdownTime;
            X6MBTestResultDir["SETRTC"] = "";
            updateControlText(X6skinLabel_MB_SETRTC_RESULT, "");
            LOG("主板重新设置RTC时间.");
            //发送设置RTC时间指令
            SendSetRtcTestReq();
        }

        private void skinSplitContainer9_Panel1_Paint(object sender, PaintEventArgs e)
        {

        }

        private void skinSplitContainer9_Panel2_Paint_1(object sender, PaintEventArgs e)
        {

        }

        private void X6skinButton_MB_GETRTC_SKIP_Click(object sender, EventArgs e)
        {
            LOG("跳过主板读取RTC时间.");
            //MBTestResultDir["GETRTC"] = "跳过";
            updateControlText(X6skinLabel_MB_GETRTC_RESULT, "跳过", Color.Green);
            updateTableSelectedIndex(X6skinTabControl_MB, ++X6MBTabSelectIndex);
        }

        private void X6skinButton_MB_GETRTC_RTEST_Click(object sender, EventArgs e)
        {
            ItemTestTime = GetCurrentTimeStamp();
            X6countDownTime_MB.getRtc = X6countdownTime;
            X6MBTestResultDir["GETRTC"] = "";
            updateControlText(X6skinLabel_MB_GETRTC_RESULT, "");
            LOG("主板重新读取RTC时间.");
            //发送读取RTC时间指令
            SendGetRtcTestReq();
        }

        private void skinSplitContainer10_Panel1_Paint(object sender, PaintEventArgs e)
        {

        }

        private void skinSplitContainer10_Panel2_Paint(object sender, PaintEventArgs e)
        {

        }

        private void X6skinButtoQQQQQQQQQQn_MB_Power_Success_Click(object sender, EventArgs e)
        {

        }

        private void X6skinButtSSSSSon_MB_TRUMPT_SUCCESS_Click(object sender, EventArgs e)
        {

        }

        private void skinSplitContainer11_SplitterMoved(object sender, SplitterEventArgs e)
        {

        }

        private void skinLabel18_Click(object sender, EventArgs e)
        {

        }

        private void skinSplitContainer1_Panel2_Paint_1(object sender, PaintEventArgs e)
        {

        }

        private void skinTabPage_X6WholeChgTest_Click(object sender, EventArgs e)
        {

        }

        private void X6skinButton_WholeChg_ClearLog_Click(object sender, EventArgs e)
        {
            X6textBoxDebugInfo.Text = "";
        }

        private void X6skinButton_WholeChg_ReportDir_Click(object sender, EventArgs e)
        {
            X6skinButtonPCBA_REPORTDIR_Click(sender, e);
        }

        private void X6skinButton_WholeChg_StartTest_Click(object sender, EventArgs e)
        {
            string TextStr = "";
            if (X6textBox_WholeChg_SN_QR.Text == "")
            {
                MessageBox.Show("桩号不能为空", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                X6textBox_WholeChg_SN_QR.Text = "";
                return;
            }

            if (X6textBox_WholeChg_SN_QR.Text.IndexOf(ProcTestData.X6StationIdQrcodeUrl) == 0)
            {
                //LOG("ProcTestData.X10QrCodeUrl.Length:0x"+ ProcTestData.X10QrCodeUrl.Length.ToString("X2"));
                //LOG("X6textBox_WholeChg_SN_QR.Text:" + X6textBox_WholeChg_SN_QR.Text);
                X6textBox_WholeChg_SN_QR.Text = X6textBox_WholeChg_SN_QR.Text.Remove(0, ProcTestData.X6StationIdQrcodeUrl.Length);
                //LOG("X6textBox_WholeChg_SN_QR.TextA:" + X6textBox_WholeChg_SN_QR.Text);
                System.Text.RegularExpressions.Regex rex = new System.Text.RegularExpressions.Regex(@"^\d+$");

                if (rex.IsMatch(X6textBox_WholeChg_SN_QR.Text) == false)
                {
                    MessageBox.Show("桩号包含非数字！", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    X6textBox_WholeChg_SN_QR.Text = "";
                    return;
                }
            }
            else
            {
                MessageBox.Show("二维码不正确！", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                X6textBox_WholeChg_SN_QR.Text = "";
                return;
            }

            X6TestSettingInfo["ChargerModel"] = X6skinComboBox_ChgType.SelectedItem;

            //Thread.Sleep(500);
            //X6textBoxGateWayAddr.Text = "389FE343C0";
            //SendSetGwAddr(X6textBoxGateWayAddr.Text.Trim());
            //Thread.Sleep(500);

            TextStr = "389FE343C1";
            //X6textBoxLocal2_4GAddr.Text = "389FE343C1"
            SendSetLocal2_4GAddr(TextStr.Trim());
            Thread.Sleep(500);
            if (X6ChargerTestingFlag == false)
            {
                LOG("整机请求开始测试.");
                SendTestModeReq((byte)X6TEST_MODE.TEST_MODE_START);
            }
            else
            {
                LOG("整机请求结束测试.");
                SendTestModeReq((byte)X6TEST_MODE.TEST_MODE_STOP);
            }
        }

        private void X6skinTabPage_WholeChg_SN_Click(object sender, EventArgs e)
        {

        }

        private void skinSplitContainer12_Panel2_Paint(object sender, PaintEventArgs e)
        {

        }

        private void skinSplitContainer15_SplitterMoved(object sender, SplitterEventArgs e)
        {

        }

        private void skinSplitContainer13_SplitterMoved(object sender, SplitterEventArgs e)
        {

        }

        private void X6textBox_WholeChg_SN_QR_TextChanged(object sender, EventArgs e)
        {

        }

        private void X6skinButton_WholeChg_SN_Confirm_Click(object sender, EventArgs e)
        {
            X6skinButton_WholeChg_StartTest_Click(sender, e);
        }

        private void X6skinButton_WholeChg_Led_Success_Click(object sender, EventArgs e)
        {

        }
        
        //整机测试项索引更新监听
        private void X6skinTabControl_WholeChg_SelectedIndexChanged(object sender, EventArgs e)
        {
            X6chargerTestSelectIndex = X6skinTabControl_WholeChg.SelectedIndex;
            if (X6chargerTestSelectIndex == 0)
            {
                X6textBox_WholeChg_SN_QR.Focus();
            }
        }

        private void X6skinButton_WholeChg_Led_Fail_Click(object sender, EventArgs e)
        {
            LOG("整机指示灯测试失败.");
            X6ChargerTestResultDir["指示灯"] = "不通过";
            updateControlText(X6skinLabel_CHG_LED_RESULT, "不通过", Color.Red);
            updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
        }

        private void X6skinButton_WholeChg_Led_Over_Click(object sender, EventArgs e)
        {
            LOG("跳过整机指示灯测试.");
            updateControlText(X6skinLabel_CHG_LED_RESULT, "跳过", Color.Green);
            updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
        }

        private void X6skinButton_WholeChg_Led_RTest_Click(object sender, EventArgs e)
        {
            ItemTestTime = GetCurrentTimeStamp();
            X6countDownTimeCharger.lcd = X6countdownTime;
            X6ChargerTestResultDir["指示灯"] = "";
            updateControlText(X6skinLabel_CHG_LED_RESULT, "");
            LOG("整机指示灯重新测试.");

            SendLedTestReq();
        }

        private void X6skinButton_CHG_TRUMPT_SUCCESS_Click(object sender, EventArgs e)
        {
            LOG("整机喇叭测试成功.");
            X6ChargerTestResultDir["喇叭"] = "通过";
            updateControlText(X6skinLabel_CHG_TRUMPT_RESULT, "通过", Color.Green);
            updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
        }

        private void X6skinButton_CHG_TRUMPT_FAIL_Click(object sender, EventArgs e)
        {
            LOG("整机喇叭测试失败.");
            X6ChargerTestResultDir["喇叭"] = "不通过";
            updateControlText(X6skinLabel_CHG_TRUMPT_RESULT, "不通过", Color.Red);
            updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
        }

        private void X6skinButton_CHG_TRUMPT_SKIP_Click(object sender, EventArgs e)
        {
            LOG("跳过整机喇叭测试.");
            updateControlText(X6skinLabel_CHG_TRUMPT_RESULT, "跳过", Color.Green);
            updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
        }

        private void X6skinButton_CHG_TRUMPT_RTEST_Click(object sender, EventArgs e)
        {
            ItemTestTime = GetCurrentTimeStamp();
            X6countDownTimeCharger.trumpet = X6countdownTime;
            X6ChargerTestResultDir["喇叭"] = "";
            updateControlText(X6skinLabel_CHG_TRUMPT_RESULT, "");
            LOG("整机喇叭重新测试.");

            SendTrumptTestReq();
        }

        private void skinSplitContainer15_Panel1_Paint(object sender, PaintEventArgs e)
        {

        }

        private void X6skinButton_CHG_RELAY_SKIP_Click(object sender, EventArgs e)
        {
            LOG("跳过整机继电器测试.");
            updateControlText(X6skinLabel_CHG_RELAY_RESULT, "跳过", Color.Green);
            updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
        }

        private void X6skinButton_CHG_RELAY_RTEST_Click(object sender, EventArgs e)
        {
            ItemTestTime = GetCurrentTimeStamp();
            X6countDownTimeCharger.relay = X6countdownTime;
            X6ChargerTestResultDir["继电器"] = "";
            updateControlText(X6skinLabel_CHG_RELAY_RESULT, "");
            LOG("整机继电器测试.");

            SendRelayTestReq(0x00, 0x00);
        }

        private void X6skinButtoQQQQQQQn_WholeChg_Led_Success_Click(object sender, EventArgs e)
        {

        }

        private void X6skinButton_WholeChg_Led_Success_Click_1(object sender, EventArgs e)
        {
            try
            {
                LOG("整机指示灯测试成功.");
                X6ChargerTestResultDir["指示灯"] = "通过";
                updateControlText(X6skinLabel_CHG_LED_RESULT, "通过", Color.Green);
                updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
            }
            catch (Exception ex)
            {
                //LOG(ex.Message);
            }
        }
        

        private void skinTabPageConfig_Click(object sender, EventArgs e)
        {

        }

        private void X6skinButtonDevReboot_Click(object sender, EventArgs e)
        {
            SendDevReboot();
        }

        private void X6ButtonSetChargerID_Click(object sender, EventArgs e)
        {
            SendSetID(X6textBoxChargerID.Text.Trim());
        }

        private void X6skinButtonGateWayAddr_Click(object sender, EventArgs e)
        {
            SendSetGwAddr(X6textBoxGateWayAddr.Text.Trim());
        }

        private void X6skinButtonLocal2_4GAddr_Click(object sender, EventArgs e)
        {
            SendSetLocal2_4GAddr(X6textBoxLocal2_4GAddr.Text.Trim());
        }

       

        private void X6skinButtonCleanConfigLog_Click(object sender, EventArgs e)
        {
            X6textBoxConfigPrint.Text = "";
        }

        //设置RTC时间消息处理
        private void X6MessageSetRtcHandle(byte[] pkt)
        {
            if (X6TestMeunSelectIndex == (1))//PCBA测试
            {
                if (X6PCBATestSelectIndex == 0)//主板测试
                {
                    if (pkt[17] == 0x00)//成功
                    {
                        LOG("设置RTC时间成功.");
                        X6MBTestResultDir["SETRTC"] = "通过";
                        updateControlText(X6skinLabel_MB_SETRTC_RESULT, "通过", Color.Green);
                        updateTableSelectedIndex(X6skinTabControl_MB, ++X6MBTabSelectIndex);
                    }
                    else
                    {
                        LOG("设置RTC时间失败.");
                        X6MBTestResultDir["SETRTC"] = "不通过";
                        updateControlText(X6skinLabel_MB_SETRTC_RESULT, "不通过", Color.Red);
                        //updateTableSelectedIndex(X6skinTabControl_MB, ++X6MBTabSelectIndex);
                    }
                }
            }
            else if (X6TestMeunSelectIndex == 2)//整机测试
            {
                if (pkt[17] == 0x00)//成功
                {
                    LOG("设置RTC时间成功.");
                    X6ChargerTestResultDir["SETRTC"] = "通过";
                    updateControlText(X6WholeskinLabel_SETRTC_RESULT, "通过", Color.Green);
                    updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
                }
                else
                {
                    LOG("设置RTC时间失败.");
                    X6ChargerTestResultDir["SETRTC"] = "不通过";
                    updateControlText(X6WholeskinLabel_SETRTC_RESULT, "不通过", Color.Red);
                }
            }
        }

        //读取RTC时间消息处理
        private void X6MessageGetRtctHandle(byte[] pkt)
        {
            if (X6TestMeunSelectIndex == 1)//PCBA测试
            {
                if (X6PCBATestSelectIndex == 0)//主板测试
                {
                    UInt32 stationRtcCount = (UInt32)((pkt[17] << 24) | (pkt[18] << 16) | (pkt[19] << 8) | pkt[20]);
                    UInt32 currentCount = GetCurrentTimeStamp();
                    UInt32 tmpCount = 0;

                    LOG("读取RTC时间成功," + stationRtcCount.ToString());
                    if (currentCount > stationRtcCount)
                    {
                        tmpCount = (currentCount - stationRtcCount) % 60;
                        LOG("RTC差值:" + tmpCount.ToString());
                        if (tmpCount < 20)
                        {
                            LOG("RTC校验OK.");
                            X6MBTestResultDir["GETRTC"] = "通过";
                            updateControlText(X6skinLabel_MB_GETRTC_RESULT, "通过", Color.Green);
                            updateTableSelectedIndex(X6skinTabControl_MB, ++X6MBTabSelectIndex);
                        }
                        else
                        {
                            LOG("RTC校验err.");
                            X6MBTestResultDir["GETRTC"] = "不通过";
                            updateControlText(X6skinLabel_MB_GETRTC_RESULT, "不通过", Color.Red);
                        }
                    }
                    else
                    {
                        tmpCount = (stationRtcCount - currentCount) % 60;
                        LOG("RTC差值:" + tmpCount.ToString());
                        if (tmpCount < 20)
                        {
                            LOG("RTC校验OK.");
                            X6MBTestResultDir["GETRTC"] = "通过";
                            updateControlText(X6skinLabel_MB_GETRTC_RESULT, "通过", Color.Green);
                            updateTableSelectedIndex(X6skinTabControl_MB, ++X6MBTabSelectIndex);
                        }
                        else
                        {
                            LOG("RTC校验err.");
                            X6MBTestResultDir["GETRTC"] = "不通过";
                            updateControlText(X6skinLabel_MB_GETRTC_RESULT, "不通过", Color.Red);
                        }
                    }
                }
            }
            else if (X6TestMeunSelectIndex == 2)//整机测试
            {
                UInt32 stationRtcCount = (UInt32)((pkt[17] << 24) | (pkt[18] << 16) | (pkt[19] << 8) | pkt[20]);
                UInt32 currentCount = GetCurrentTimeStamp();
                UInt32 tmpCount = 0;

                LOG("读取RTC时间成功," + stationRtcCount.ToString());
                if (currentCount > stationRtcCount)
                {
                    tmpCount = (currentCount - stationRtcCount) % 60;
                    LOG("RTC差值:" + tmpCount.ToString());
                    if (tmpCount < 20)
                    {
                        LOG("RTC校验OK.");
                        X6ChargerTestResultDir["GETRTC"] = "通过";
                        updateControlText(X6skinLabel_WholeTest_GETRTC_RESULT, "通过", Color.Green);
                        updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
                    }
                    else
                    {
                        LOG("RTC校验err.");
                        X6ChargerTestResultDir["GETRTC"] = "不通过";
                        updateControlText(X6skinLabel_WholeTest_GETRTC_RESULT, "不通过", Color.Red);
                    }
                }
                else
                {
                    tmpCount = (stationRtcCount - currentCount) % 60;
                    LOG("RTC差值:" + tmpCount.ToString());
                    if (tmpCount < 20)
                    {
                        LOG("RTC校验OK.");
                        X6ChargerTestResultDir["GETRTC"] = "通过";
                        updateControlText(X6skinLabel_WholeTest_GETRTC_RESULT, "通过", Color.Green);
                        updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
                    }
                    else
                    {
                        LOG("RTC校验err.");
                        X6ChargerTestResultDir["GETRTC"] = "不通过";
                        updateControlText(X6skinLabel_WholeTest_GETRTC_RESULT, "不通过", Color.Red);
                    }
                }
            }
        }

        //FLASH测试消息处理
        private void X6MessageFlashTestHandle(byte[] pkt)
        {
            if (X6TestMeunSelectIndex == 1)//PCBA测试
            {
                if (X6PCBATestSelectIndex == 0)//主板测试
                {
                    if (pkt[17] == 0x00)//成功
                    {
                        LOG("FLASH测试成功.");
                        X6MBTestResultDir["FLASH"] = "通过";
                        updateControlText(X6skinLabel_MB_FLASH_RESULT, "通过", Color.Green);
                        updateTableSelectedIndex(X6skinTabControl_MB, ++X6MBTabSelectIndex);
                    }
                    else
                    {
                        LOG("FLASH测试失败.");
                        X6MBTestResultDir["FLASH"] = "不通过";
                        updateControlText(X6skinLabel_MB_FLASH_RESULT, "不通过", Color.Red);
                        //updateTableSelectedIndex(skinTabControl_MB, ++MBTabSelectIndex);
                    }
                }
            }
            else if (X6TestMeunSelectIndex == 2)//整机测试
            {

            }
        }

        //设置终端信息
        bool setTerminalInfoFlag = false;
        private void X6SendSetTerminalInfo()
        {
            string sn = "1111222233";
            byte count = 1;

            try
            {
                List<byte> list = new List<byte> { };

                list.Add((byte)count);
                list.AddRange(ProcTestData.stringToBCD(sn));
                setTerminalInfoFlag = false;
                SendSerialData(MakeSendArray((byte)X6Command.CMD_SET_TERMINAL_INFO, list.ToArray()));

                int wait = 0, n = 0;
                while (setTerminalInfoFlag == false)
                {
                    Thread.Sleep(200);
                    if (wait++ > 10)
                    {
                        wait = 0;
                        SendSerialData(MakeSendArray((byte)X6Command.CMD_GET_CHARGER_SN, null));
                        n++;
                    }
                    if (n > 3)
                    {
                        break;
                    }
                }

                if (n > 3)
                {
                    if (MessageBox.Show("设置终端信息失败！\r\n是否重试", "提示", MessageBoxButtons.RetryCancel, MessageBoxIcon.Warning) == DialogResult.Retry)
                    {
                        X6SendSetTerminalInfo();
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        public Color X6decideColor(string text)
        {

            switch (text)
            {
                case "通过":
                    return Color.Green;

                case "不通过":
                    return Color.Red;

                default:
                    return Color.Black;

            }
        }
        // q[PCB编号, 21]
        //  q[测试员, Admin]
        //  q[软件版本, 5.1]
        //  q[测试结果, ]
        //  q[卡号, E00005235]
        //  q[蓝牙, 通过]
        //  q[2.4G, 通过]
        //   q[测试时间, 2018 - 08 - 31 11:35:04]
        //  q[测试用时, 0]
        //  get软件版本.
        //  1535686512
        //   q[刷卡, 通过]
        public Dictionary<string, string> X6SBModifyResultData(Dictionary<string, string> inputDic)
        {
            string resKey = "", resValue = "";
            //int kgm = 0;
            //int b = 0;
            Dictionary<string, string> dictionary = new Dictionary<string, string>();

            foreach (var item in inputDic)
            {
                //LOG_qwe("KKKKKKKKK" + kgm);
                //kgm++;
                dictionary.Add(item.Key, item.Value);
            }

            foreach (var item in dictionary)
            {
                if (item.Value == "" && item.Key != "测试结果")
                {
                    inputDic[item.Key] = "未测试";
                }
                else if (item.Value == "不通过" && item.Value != "无")
                {
                    LOG_qwe("hhh " + item.ToString());
                    inputDic["测试结果"] = "不通过";
                }

                if (inputDic[item.Key] == "未测试")
                {
                    LOG_qwe("fff " + item.ToString());
                    LOG_qwe("ggg " + item.Value);
                    LOG_qwe("lll " + item.Key);
                    inputDic["测试结果"] = "不通过";
                }
                //LOG_qwe("w " + item.ToString());
            }
            if (inputDic["测试结果"] == "")
            {
                inputDic["测试结果"] = "通过";
            }

            foreach (var item in inputDic)
            {
                resKey += item.Key + " :\r\n";
                resValue += item.Value + "\r\n";
            }

            return inputDic;
        }

        private Dictionary<string, string> X6ModifyResultData(Dictionary<string, string> inputDic)
        {
            string resKey = "", resValue = "";
            //int kgm = 0;
            //int b = 0;
            Dictionary<string, string> dictionary = new Dictionary<string, string>();

            foreach (var item in inputDic)
            {
                //LOG_qwe("KKKKKKKKK" + kgm);
                //kgm++;
                //LOG_qwe(" " + item.ToString());

                //LOG_qwe("item.Value " + item.Value);
                //LOG_qwe("item.Key " + item.Key);
                dictionary.Add(item.Key, item.Value);
            }

            foreach (var item in dictionary)
            {
                if (item.Value == "" && item.Key != "测试结果")
                {
                    inputDic[item.Key] = "未测试";
                }
                else if (item.Value == "不通过" && item.Value != "无")
                {
                    inputDic["测试结果"] = "不通过";
                }

                if (inputDic[item.Key] == "未测试")
                {
                    inputDic["测试结果"] = "不通过";
                }
            }
            if (inputDic["测试结果"] == "")
            {
                inputDic["测试结果"] = "通过";
            }

            foreach (var item in inputDic)
            {
                resKey += item.Key + " :\r\n";
                resValue += item.Value + "\r\n";
            }

            return inputDic;
        }

        //主板测试线程
        private void X6MainBoardTestProcess()
        {
            bool selectIndexUpgradeFlag = false;
            X6countdownTime = Convert.ToInt32(X6TestSettingInfo["CountDown"]);

            while (X6MBTestingFlag == true)
            {
                if (X6PreMBTabSelectIndex != X6MBTabSelectIndex)
                {
                    X6PreMBTabSelectIndex = X6MBTabSelectIndex;

                    selectIndexUpgradeFlag = true;
                }
                Thread.Sleep(200);
                switch (X6MBTabSelectIndex)
                {
                    case 0x00:
                        LOG("扫描主板二维码.");
                        break;
                    case 0x01://电源
                        if (selectIndexUpgradeFlag == true)
                        {
                            selectIndexUpgradeFlag = false;
                            //TestTime = GetRtcCount();
                            ItemTestTime = GetCurrentTimeStamp();
                            X6countDownTime_MB.PowerSource = X6countdownTime;
                            X6MBTestResultDir["电源"] = "";
                            updateControlText(X6skinLabel_MB_POWER_RESULT, "");
                            LOG("检测电源是否正常.");
                        }
                        if ((GetCurrentTimeStamp() - ItemTestTime) >= 30)//超时
                        {
                            LOG("检测电源超时.");
                            X6MBTestResultDir["电源"] = "不通过";
                            updateControlText(X6skinLabel_MB_POWER_RESULT, "不通过", Color.Red);
                            updateTableSelectedIndex(X6skinTabControl_MB, ++X6MBTabSelectIndex);
                        }
                        break;
                    case 0x02://指示灯
                        if (selectIndexUpgradeFlag == true)
                        {
                            selectIndexUpgradeFlag = false;
                            //TestTime = GetRtcCount();
                            ItemTestTime = GetCurrentTimeStamp();
                            X6countDownTime_MB.lcd = X6countdownTime;
                            X6MBTestResultDir["指示灯"] = "";
                            updateControlText(X6skinLabel_MB_LED_RESULT, "");
                            LOG("指示灯测试.");
                            //发送指示灯测试指令
                            SendLedTestReq();
                        }
                        if ((GetCurrentTimeStamp() - ItemTestTime) >= 30)//超时
                        {
                            LOG("指示灯测试超时.");
                            X6MBTestResultDir["指示灯"] = "不通过";
                            updateControlText(X6skinLabel_MB_LED_RESULT, "不通过", Color.Red);
                            updateTableSelectedIndex(X6skinTabControl_MB, ++X6MBTabSelectIndex);
                        }

                        break;
                    case 0x03://喇叭
                        if (selectIndexUpgradeFlag == true)
                        {
                            selectIndexUpgradeFlag = false;
                            //TestTime = GetRtcCount();
                            ItemTestTime = GetCurrentTimeStamp();
                            X6countDownTime_MB.trumpet = X6countdownTime;
                            X6MBTestResultDir["喇叭"] = "";
                            updateControlText(X6skinLabel_MB_TRUMPT_RESULT, "");
                            LOG("喇叭测试.");
                            //发送喇叭测试指令
                            SendTrumptTestReq();
                        }
                        if ((GetCurrentTimeStamp() - ItemTestTime) >= 30)//超时
                        {
                            LOG("喇叭测试超时.");
                            X6MBTestResultDir["喇叭"] = "不通过";
                            updateControlText(X6skinLabel_MB_TRUMPT_RESULT, "不通过", Color.Red);
                            updateTableSelectedIndex(X6skinTabControl_MB, ++X6MBTabSelectIndex);
                        }
                        break;
                    case 0x04://继电器
                        if (selectIndexUpgradeFlag == true)
                        {
                            selectIndexUpgradeFlag = false;
                            //TestTime = GetRtcCount();
                            ItemTestTime = GetCurrentTimeStamp();
                            X6countDownTime_MB.relay = X6countdownTime;
                            X6MBTestResultDir["继电器"] = "";
                            updateControlText(X6skinLabel_MB_RELAY_RESULT, "");
                            updateControlText(X6skinLabelRelayResult1, "");
                            updateControlText(X6skinLabelRelayResult2, "");
                            LOG("继电器测试.");
                            //发送继电器测试指令
                            SendRelayTestReq(0x00, 0x00);
                        }
                        if ((GetCurrentTimeStamp() - ItemTestTime) >= 30)//超时
                        {
                            LOG("继电器测试超时.");
                            X6MBTestResultDir["继电器"] = "不通过";
                            updateControlText(X6skinLabel_MB_RELAY_RESULT, "不通过", Color.Red);
                            updateTableSelectedIndex(X6skinTabControl_MB, ++X6MBTabSelectIndex);
                        }
                        break;
                    case 0x05://flash
                        if (selectIndexUpgradeFlag == true)
                        {
                            selectIndexUpgradeFlag = false;
                            //TestTime = GetRtcCount();
                            ItemTestTime = GetCurrentTimeStamp();
                            X6countDownTime_MB.flash = X6countdownTime;
                            X6MBTestResultDir["FLASH"] = "";
                            updateControlText(X6skinLabel_MB_FLASH_RESULT, "");
                            LOG("FLASH测试.");
                            //发送FLASH测试指令
                            SendFlashTestReq();
                        }
                        if ((GetCurrentTimeStamp() - ItemTestTime) >= 30)//超时
                        {
                            LOG("FLASH测试超时.");
                            X6MBTestResultDir["FLASH"] = "不通过";
                            updateControlText(X6skinLabel_MB_FLASH_RESULT, "不通过", Color.Red);
                            updateTableSelectedIndex(X6skinTabControl_MB, ++X6MBTabSelectIndex);
                        }
                        //if ((1 == MainBoardTimesDelayFlag.MainBoardFlashDelayFlag) && ((GetCurrentTimeStamp() - ItemTestTime) >= 2))
                        //{
                        //    updateTableSelectedIndex(skinTabControl_MB, ++MBTabSelectIndex);
                        //}
                        break;
                    case 0x06://setRTC
                        if (selectIndexUpgradeFlag == true)
                        {
                            selectIndexUpgradeFlag = false;
                            //TestTime = GetRtcCount();
                            ItemTestTime = GetCurrentTimeStamp();
                            X6countDownTime_MB.setRtc = X6countdownTime;
                            X6MBTestResultDir["SETRTC"] = "";
                            updateControlText(X6skinLabel_MB_SETRTC_RESULT, "");
                            LOG("设置RTC时间.");
                            //发送设置RTC时间指令
                            SendSetRtcTestReq();
                        }
                        if ((GetCurrentTimeStamp() - ItemTestTime) >= 30)//超时
                        {
                            LOG("设置RTC时间超时.");
                            X6MBTestResultDir["SETRTC"] = "不通过";
                            updateControlText(X6skinLabel_MB_SETRTC_RESULT, "不通过", Color.Red);
                            updateTableSelectedIndex(X6skinTabControl_MB, ++X6MBTabSelectIndex);
                        }
                        break;
                    case 0x07://getRTC
                        if (selectIndexUpgradeFlag == true)
                        {
                            selectIndexUpgradeFlag = false;
                            //TestTime = GetRtcCount();
                            ItemTestTime = GetCurrentTimeStamp();
                            X6countDownTime_MB.getRtc = X6countdownTime;
                            X6MBTestResultDir["GETRTC"] = "";
                            updateControlText(X6skinLabel_MB_GETRTC_RESULT, "");
                            LOG("读取RTC时间.");
                            //发送读取RTC时间指令
                            SendGetRtcTestReq();
                        }
                        if ((GetCurrentTimeStamp() - ItemTestTime) >= 30)//超时
                        {
                            LOG("读取RTC时间超时.");
                            X6MBTestResultDir["GETRTC"] = "不通过";
                            updateControlText(X6skinLabel_MB_GETRTC_RESULT, "不通过", Color.Red);
                            updateTableSelectedIndex(X6skinTabControl_MB, ++X6MBTabSelectIndex);
                        }
                        break;
                    case 0x08://结束测试
                        LOG("主板结束测试1.\n" + GetCurrentTimeStamp());
                        X6SendGetFwVersionReq(0x00);
                        Thread.Sleep(100);
                        X6SendSetPcbCodeReq(0x00, X6textBox_MB_QRCode.Text.Trim());
                        X6GetResultObj.UsedTime_main = GetCurrentTimeStamp() - X6GetResultObj.UsedTime_main;
                        X6MBTestResultDir["测试用时"] = (X6GetResultObj.UsedTime_main / 60) + "分 " + ((X6GetResultObj.UsedTime_main) % 60) + "秒";
                        Thread.Sleep(2000);
                        
                        X6MBTestResultDir = X6ModifyResultData(X6MBTestResultDir);
                        LOG("结束测试\r\n用时:" + X6MBTestResultDir["测试用时"]);

                        X6ShowMainboardResult();
                        LOG("主板结束测试2.\n" + GetCurrentTimeStamp());
                        SendTestModeReq(0x01);

                        //写入excel表
                        ProcTestData.WriteReport(X6TestSettingInfo["ChargerModel"] + "_PCBA_主板.xlsx", X6TestSettingInfo["ChargerModel"] + "_PCBA_主板", X6MBTestResultDir);


                        //string mysqlCmd = ProcTestData.X6MainboardTestMysqlCommand(
                        //    X6TestSettingInfo["ChargerModel"].ToString(),
                        //    X6MBTestResultDir["PCB编号"],
                        //    X6MBTestResultDir["测试员"],
                       //     X6MBTestResultDir["软件版本"],
                        //    X6MBTestResultDir["测试结果"] == "通过" ? "Pass" : "Fail",
                       //     X6MBTestResultDir["电源"] == "通过" ? "Pass" : "Fail",
                       //     X6MBTestResultDir["指示灯"] == "通过" ? "Pass" : "Fail",
                       //     X6MBTestResultDir["喇叭"] == "通过" ? "Pass" : "Fail",
                       //     X6MBTestResultDir["继电器"] == "通过" ? "Pass" : "Fail",
                       //     X6MBTestResultDir["FLASH"] == "通过" ? "Pass" : "Fail",
                       //     X6MBTestResultDir["SETRTC"] == "通过" ? "Pass" : "Fail",
                       //     X6MBTestResultDir["GETRTC"] == "通过" ? "Pass" : "Fail",
                      //      X6MBTestResultDir["测试时间"],
                      //      X6GetResultObj.UsedTime_main
                     //       );

                     //   if (ProcTestData.SendMysqlCommand(X6mysqlCmd, true) == true)
                     //   {
                     //       LOG("主板测试记录添加数据库成功");
                     //       ProcTestData.DealBackUpData(ProcTestData.backupMysqlCmdFile);
                    //    }

                        updateControlText(X6textBox_MB_QRCode, "");
                        X6MBTestingFlag = false;

                        Thread.Sleep(100);
                        SendDevReboot();
                        break;
                    default:
                        break;
                }
                Thread.Sleep(300);
            }
        }

        //副板测试线程
        private void X6SubBoardTestProcess()
        {
            bool selectIndexUpgradeFlag = false;
            X6countdownTime = Convert.ToInt32(X6TestSettingInfo["CountDown"]);

            X6TestTimeTicks = 0;
            X6cardNumTimeTicks = 0;
            X6BlueToothTimeTicks = 0;
            X624GTimeTicks = 0;
            X6MBPowerTimeTicks = 0;
            X6SBGetfwTicks = 0;

            while (X6SBTestingFlag == true)
            {
                if (X6PreSBTabSelectIndex != X6SBTabSelectIndex)
                {
                    X6PreSBTabSelectIndex = X6SBTabSelectIndex;
                    selectIndexUpgradeFlag = true;
                }
                Thread.Sleep(200);
                switch (X6SBTabSelectIndex)
                {
                    case 0x00:
                        if (selectIndexUpgradeFlag == true)
                        {
                            LOG("扫描副板二维码.");
                            selectIndexUpgradeFlag = false;
                        }
                        break;
                    case 0x01:  //刷卡
                        if (selectIndexUpgradeFlag == true)
                        {
                            LOG("副板刷卡测试.");
                            selectIndexUpgradeFlag = false;
                            ItemTestTime = GetCurrentTimeStamp();
                            X6countDownTime_SB.tapCard = X6countdownTime;
                            X6SBTestResultDir["刷卡"] = "";
                            updateControlText(X6skinLabelChargerTapCardResult, "");
                            X6SendTapCard();
                        }
                        if ((GetCurrentTimeStamp() - ItemTestTime) >= 30)//超时
                        {
                            LOG("副板蓝牙测试超时.");
                            X6SBTestResultDir["刷卡"] = "不通过";
                            updateControlText(X6skinLabelChargerTapCardResult, "不通过", Color.Red);
                            updateTableSelectedIndex(X6skinTabControl_SB, ++X6SBTabSelectIndex);
                        }
                        break;
                    case 0x02:      //蓝牙
                        if (selectIndexUpgradeFlag == true)
                        {
                            LOG("副板蓝牙测试.");
                            selectIndexUpgradeFlag = false;
                            ItemTestTime = GetCurrentTimeStamp();
                            X6countDownTime_SB.BLE = X6countdownTime;
                            X6SBTestResultDir["蓝牙"] = "";
                            updateControlText(X6skinLabel_SB_BT_RESULT, "");
                            //X6SendGetFwVersionReq(0x00);
                        }
                        if ((GetCurrentTimeStamp() - ItemTestTime) >= 30)//超时
                        {
                            LOG("副板蓝牙测试超时.");
                            X6SBTestResultDir["蓝牙"] = "不通过";
                            updateControlText(X6skinLabel_SB_BT_RESULT, "不通过", Color.Red);
                            updateTableSelectedIndex(X6skinTabControl_SB, ++X6SBTabSelectIndex);
                        }
                        break;
                    case 0x03:      //2.4
                        if (selectIndexUpgradeFlag == true)
                        {
                            selectIndexUpgradeFlag = false;

                            LOG("副板2.4G测试.");
                            ItemTestTime = GetCurrentTimeStamp();
                            X6countDownTime_SB._2_4G = X6countdownTime;
                            X6SBTestResultDir["2.4G"] = "";
                            updateControlText(X6skinLabel_SB_24G_RESULT, "");
                        }
                        if ((GetCurrentTimeStamp() - ItemTestTime) >= 30)//超时
                        {
                            LOG("副板2.4G测试超时.");
                            X6SBTestResultDir["2.4G"] = "不通过";
                            updateControlText(X6skinLabel_SB_24G_RESULT, "不通过", Color.Red);
                            updateTableSelectedIndex(X6skinTabControl_SB, ++X6SBTabSelectIndex);
                        }
                        break;
                    case 0x04://结束测试
                        LOG("副板结束测试.\n" + GetCurrentTimeStamp());
                        X6SendGetFwVersionReq(0x00);
                        X6GetResultObj.UsedTime_interface = GetCurrentTimeStamp() - X6GetResultObj.UsedTime_interface;
                        X6SBTestResultDir["测试用时"] = (X6GetResultObj.UsedTime_interface / 60) + "分 " + ((X6GetResultObj.UsedTime_interface) % 60) + "秒";
                        Thread.Sleep(2000);
                        
                        
                        X6SBTestResultDir = X6SBModifyResultData(X6SBTestResultDir);

                        
                        
                        LOG("结束测试\r\n用时:" + X6SBTestResultDir["测试用时"]);
                        Thread.Sleep(100);
                        X6ShowSubBoardResult();
                        SendTestModeReq(0x01);

                        //写入excel表
                        ProcTestData.WriteReport(X6TestSettingInfo["ChargerModel"] + "_PCBA_副板.xlsx", X6TestSettingInfo["ChargerModel"] + "_PCBA_副板", X6SBTestResultDir);

                        //string mysqlCmd = ProcTestData.SubBoardTestMysqlCommand(
                       //     X6TestSettingInfo["ChargerModel"].ToString(),
                        //    X6SBTestResultDir["PCB编号"],
                        //    X6SBTestResultDir["测试员"],
                        //    X6SBTestResultDir["软件版本"],
                        //    X6SBTestResultDir["测试结果"] == "通过" ? "Pass" : "Fail",
                         //   X6SBTestResultDir["蓝牙"] == "通过" ? "Pass" : (X6SBTestResultDir["蓝牙"] == "无" ? "Without" : "Fail"),
                        //    X6SBTestResultDir["2G"] == "通过" ? "Pass" : (X6SBTestResultDir["2G"] == "无" ? "Without" : "Fail"),
                         //   X6SBTestResultDir["测试时间"],
                         //   X6GetResultObj.UsedTime_interface
                        //    );

                     //   if (ProcTestData.SendMysqlCommand(mysqlCmd, true) == true)
                    //    {
                     //       LOG("副板测试记录添加数据库成功");
                     //       ProcTestData.DealBackUpData(ProcTestData.backupMysqlCmdFile);
                     //   }

                        updateControlText(X6textBox_SB_QR, "");
                        X6SBTestingFlag = false;
                        break;

                    default:
                        break;
                }

                Thread.Sleep(200);
            }
        }

        private int X6Wholesend2GTestCnt = 0;
        //整机测试线程
        private void X6ChargerTestProcess()
        {
            bool selectIndexUpgradeFlag = false;
            X6countdownTime = Convert.ToInt32(X6TestSettingInfo["CountDown"]);

            X6TestTimeTicks = 0;
            X6cardNumTimeTicks = 0;
            X6BlueToothTimeTicks = 0;
            X624GTimeTicks = 0;
            X6MBPowerTimeTicks = 0;
            X6SBGetfwTicks = 0;
            X6WholeCardNumTimeTicks = 0;

            while (X6ChargerTestingFlag == true)
            {
                if (X6PrechargerTestSelectIndex != X6chargerTestSelectIndex)
                {
                    X6PrechargerTestSelectIndex = X6chargerTestSelectIndex;
                    selectIndexUpgradeFlag = true;
                }
                Thread.Sleep(200);
                switch (X6chargerTestSelectIndex)
                {
                    case 0x00:
                        LOG("扫描电桩二维码.");
                        break;
                    case 0x01:  //指示灯
                        if (selectIndexUpgradeFlag == true)
                        {
                            selectIndexUpgradeFlag = false;
                            ItemTestTime = GetCurrentTimeStamp();
                            X6countDownTimeCharger.lcd = X6countdownTime;
                            X6ChargerTestResultDir["指示灯"] = "";
                            updateControlText(X6skinLabel_CHG_LED_RESULT, "");
                            LOG("整机指示灯测试.");

                            SendLedTestReq();
                        }
                        if ((GetCurrentTimeStamp() - ItemTestTime) >= 30)//超时
                        {
                            LOG("整机指示灯测试超时.");
                            X6ChargerTestResultDir["指示灯"] = "不通过";
                            updateControlText(X6skinLabel_CHG_LED_RESULT, "不通过", Color.Red);
                            updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
                        }
                        break;
                    case 0x02:  //喇叭
                        if (selectIndexUpgradeFlag == true)
                        {
                            selectIndexUpgradeFlag = false;
                            ItemTestTime = GetCurrentTimeStamp();
                            X6countDownTimeCharger.trumpet = X6countdownTime;
                            X6ChargerTestResultDir["喇叭"] = "";
                            updateControlText(X6skinLabel_CHG_TRUMPT_RESULT, "");
                            LOG("整机喇叭测试.");

                            SendTrumptTestReq();
                        }
                        if ((GetCurrentTimeStamp() - ItemTestTime) >= 30)//超时
                        {
                            LOG("整机喇叭测试超时.");
                            X6ChargerTestResultDir["喇叭"] = "不通过";
                            updateControlText(X6skinLabel_CHG_TRUMPT_RESULT, "不通过", Color.Red);
                            updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
                        }
                        break;
                    case 0x03:  //蓝牙
                        if (selectIndexUpgradeFlag == true)
                        {
                            selectIndexUpgradeFlag = false;
                            ItemTestTime = GetCurrentTimeStamp();
                            //X6countDownTimeCharger.BLE = X6countdownTime;
                            X6countDownTimeCharger.BLE = 60;
                            X6ChargerTestResultDir["蓝牙"] = "";
                            updateControlText(X6skinLabel_CHG_BT_RESULT, "");
                            X6SendBlueToothTestReq();
                            LOG("整机蓝牙测试.");
                        }
                        //X6skinButton_WholeChg_BT_RTest_Click(object sender, EventArgs e)
                            
                        if ((GetCurrentTimeStamp() - ItemTestTime) >= 10)
                        {
                            ItemTestTime = GetCurrentTimeStamp();

                            X6Wholesend2GTestCnt++;

                            if (X6Wholesend2GTestCnt < 4)
                            {
                                X6SendBlueToothTestReq();
                            }
                        }

                        //3次过后超时处理
                        if (X6Wholesend2GTestCnt > 4)
                        {
                            X6Wholesend2GTestCnt = 0;
                            LOG("整机蓝牙测试超时.");
                            X6ChargerTestResultDir["蓝牙"] = "不通过";
                            updateControlText(X6skinLabel_CHG_BT_RESULT, "不通过", Color.Red);
                            updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
                        }
                        break;
                    case 0x04:  //刷卡
                        if (selectIndexUpgradeFlag == true)
                        {
                            LOG("副板刷卡测试.");
                            selectIndexUpgradeFlag = false;
                            ItemTestTime = GetCurrentTimeStamp();
                            X6countDownTimeCharger.tapCard = X6countdownTime;
                            X6ChargerTestResultDir["刷卡"] = "";
                            updateControlText(X6WholeX6skinLabelChargerTapCardResult, "");
                            X6SendTapCard();
                        }

                        if ((GetCurrentTimeStamp() - ItemTestTime) >= 10)
                        {
                            ItemTestTime = GetCurrentTimeStamp();

                            X6Wholesend2GTestCnt++;

                            if (X6Wholesend2GTestCnt < 3)
                            {
                                X6SendTapCard();
                            }
                        }

                        //3次过后超时处理
                        if (X6Wholesend2GTestCnt > 3)
                        {
                            X6Wholesend2GTestCnt = 0;
                            LOG("副板蓝牙测试超时.");
                            X6ChargerTestResultDir["刷卡"] = "不通过";
                            updateControlText(X6WholeX6skinLabelChargerTapCardResult, "不通过", Color.Red);
                            updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
                        }
                        break;
                    case 0x05:  //2.4G
                        if (selectIndexUpgradeFlag == true)
                        {
                            selectIndexUpgradeFlag = false;
                            ItemTestTime = GetCurrentTimeStamp();
                            X6countDownTimeCharger._2_4G = X6countdownTime;
                            X6ChargerTestResultDir["2.4G"] = "";
                            updateControlText(X6skinLabel_CHG_24G_RESULT, "");
                            X6Send24G_COMMUNICATION_TestReq();
                            LOG("整机2.4G测试.");
                        }
                        
                        if ((GetCurrentTimeStamp() - ItemTestTime) >= 10)
                        {
                            ItemTestTime = GetCurrentTimeStamp();

                            X6Wholesend2GTestCnt++;

                            if (X6Wholesend2GTestCnt < 3)
                            {
                                X6Send24G_COMMUNICATION_TestReq();
                            }
                        }

                        //3次过后超时处理
                        if (X6Wholesend2GTestCnt > 3)
                        {
                            X6Wholesend2GTestCnt = 0;
                            LOG("整机2.4G测试超时.");
                            X6ChargerTestResultDir["2.4G"] = "不通过";
                            updateControlText(X6skinLabel_CHG_24G_RESULT, "不通过", Color.Red);
                            updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
                        }
                        break;
                    case 0x06://继电器测试
                        if (selectIndexUpgradeFlag == true)
                        {
                            selectIndexUpgradeFlag = false;
                            ItemTestTime = GetCurrentTimeStamp();
                            X6countDownTimeCharger.relay = X6countdownTime;
                            X6ChargerTestResultDir["继电器"] = "";
                            updateControlText(X6skinLabel_CHG_RELAY1_POWER, "");
                            updateControlText(X6skinLabel_CHG_RELAY2_POWER, "");
                            updateControlText(X6skinLabel_CHG_RELAY_RESULT, "");
                            LOG("整机继电器测试.");
                            SendRelayTestReq(0x00, 0x00);
                        }
                        if ((GetCurrentTimeStamp() - ItemTestTime) >= 30)//超时
                        {
                            LOG("整机继电器测试超时.");
                            X6ChargerTestResultDir["继电器"] = "不通过";
                            updateControlText(X6skinLabel_CHG_RELAY_RESULT, "不通过", Color.Red);
                            updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
                        }
                        break;
                    case 0x07://setRTC
                        if (selectIndexUpgradeFlag == true)
                        {
                            selectIndexUpgradeFlag = false;
                            //TestTime = GetRtcCount();
                            ItemTestTime = GetCurrentTimeStamp();
                            X6countDownTimeCharger.setRtc = X6countdownTime;
                            X6ChargerTestResultDir["SETRTC"] = "";
                            updateControlText(X6WholeskinLabel_SETRTC_RESULT, "");
                            LOG("设置RTC时间.");
                            //发送设置RTC时间指令
                            SendSetRtcTestReq();
                        }
                        if ((GetCurrentTimeStamp() - ItemTestTime) >= 30)//超时
                        {
                            LOG("设置RTC时间超时.");
                            X6ChargerTestResultDir["SETRTC"] = "不通过";
                            updateControlText(X6WholeskinLabel_SETRTC_RESULT, "不通过", Color.Red);
                            updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
                        }
                        break;
                    case 0x08://getRTC
                        if (selectIndexUpgradeFlag == true)
                        {
                            selectIndexUpgradeFlag = false;
                            //TestTime = GetRtcCount();
                            ItemTestTime = GetCurrentTimeStamp();
                            X6countDownTimeCharger.getRtc = X6countdownTime;
                            X6ChargerTestResultDir["GETRTC"] = "";
                            updateControlText(X6skinLabel_WholeTest_GETRTC_RESULT, "");
                            LOG("读取RTC时间.");
                            //发送读取RTC时间指令
                            SendGetRtcTestReq();
                        }
                        if ((GetCurrentTimeStamp() - ItemTestTime) >= 30)//超时
                        {
                            LOG("读取RTC时间超时.");
                            X6ChargerTestResultDir["GETRTC"] = "不通过";
                            updateControlText(X6skinLabel_WholeTest_GETRTC_RESULT, "不通过", Color.Red);
                            updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
                        }
                        break;
                    case 0x09://结束测试
                        X6SendGetFwVersionReq(0x00);
                        Thread.Sleep(100);
                        X6SendGetPcdCode(0x00);
                        X6GetResultObj.UsedTime_Charger = GetCurrentTimeStamp() - X6GetResultObj.UsedTime_Charger;
                        X6ChargerTestResultDir["测试用时"] = (X6GetResultObj.UsedTime_Charger / 60) + "分 " + ((X6GetResultObj.UsedTime_Charger) % 60) + "秒";
                        Thread.Sleep(2000);

                        X6ChargerTestResultDir = X6ModifyResultData(X6ChargerTestResultDir);
                        LOG("结束测试\r\n用时:" + X6ChargerTestResultDir["测试用时"]);

                        X6ShowChgBoardResult();
                        if ((X6ChargerTestResultDir["测试结果"] == "通过") || (X6ChargerTestResultDir["测试结果"] == "Pass"))
                        {
                            SendSetID(X6textBox_WholeChg_SN_QR.Text.Trim());
                        }
                        SendTestModeReq(0x01);

                        //写入excel表
                        ProcTestData.WriteReport(X6TestSettingInfo["ChargerModel"] + "_整机测试.xlsx", X6TestSettingInfo["ChargerModel"] + "_整机测试", X6ChargerTestResultDir);

                        //ProcTestData.WriteReport(TestSettingInfo["ChargerModel"] + "_整机测试.xlsx", TestSettingInfo["ChargerModel"] + "_整机测试", chargerTestData);

                        //string cmd = ProcTestData.ChargerTestMysqlCommand(
                        //        X6TestSettingInfo["ChargerModel"].ToString(),
                        //        X6ChargerTestResultDir["电桩号"],
                        //        X6ChargerTestResultDir["测试员"],
                        ///        X6ChargerTestResultDir["软件版本"],
                        //        X6ChargerTestResultDir["主板编号"],
                        //        X6ChargerTestResultDir["测试结果"] == "通过" ? "Pass" : "Fail",
                         //       X6ChargerTestResultDir["指示灯"] == "通过" ? "Pass" : "Fail",
                        //        X6ChargerTestResultDir["喇叭"] == "通过" ? "Pass" : "Fail",
                        //        X6ChargerTestResultDir["蓝牙"] == "通过" ? "Pass" : (X6ChargerTestResultDir["蓝牙"] == "无" ? "Without" : "Fail"),
                        //        X6ChargerTestResultDir["2G"] == "通过" ? "Pass" : (X6ChargerTestResultDir["2G"] == "无" ? "Without" : "Fail"),
                        //        X6ChargerTestResultDir["继电器"] == "通过" ? "Pass" : "Fail",
                         //       X6ChargerTestResultDir["测试时间"],
                         //       X6GetResultObj.UsedTime_Charger
                         //   );

                       // if (ProcTestData.SendMysqlCommand(cmd, true) == true)
                      //  {
                      //      LOG("整机测试记录添加数据库成功");
                      //      ProcTestData.DealBackUpData(ProcTestData.backupMysqlCmdFile);
                       // }
                        updateControlText(X6textBox_WholeChg_SN_QR, "");
                        X6ChargerTestingFlag = false;

                        Thread.Sleep(100);
                        SendDevReboot();
                        break;

                    default:
                        break;
                }
                
                Thread.Sleep(300);
            }
        }

        public UInt32 ItemTestTime = 0;
        //测试模式命令消息处理
        private void X6MessageTestModeHandle(byte[] pkt)
        {
            X6GetResultObj.testMode = pkt[17];
            X6GetResultObj.testModeAllow = pkt[18];

            X6MBTestingFlag = false;
            X6SBTestingFlag = false;
            X6ChargerTestingFlag = false;

            if (X6TestMeunSelectIndex == 1)
            { //PCBA测试
                if (X6PCBATestSelectIndex == 0)
                { //主板测试
                    if (X6GetResultObj.testMode == 0x00)//开始测试ack
                    {
                        if (X6GetResultObj.testModeAllow == 0x00)//成功
                        {
                            LOG("主板请求开始测试成功.");
                            updateControlText(X6skinButton_PCBA_STARTTEST, "结束测试");
                            X6MBTestingFlag = true;
                            X6MBTabSelectIndex = 1;
                            updateTableSelectedIndex(X6skinTabControl_MB, X6MBTabSelectIndex);

                            DateTime now = DateTime.Now;
                            X6MBTestResultDir.Clear();
                            X6MBTestResultDir.Add("PCB编号", X6textBox_MB_QRCode.Text.Trim());
                            X6MBTestResultDir.Add("测试员", ProcTestData.PresentAccount);
                            X6MBTestResultDir.Add("软件版本", "");
                            X6MBTestResultDir.Add("测试结果", "");
                            X6MBTestResultDir.Add("电源", "");
                            X6MBTestResultDir.Add("指示灯", "");
                            //MBTestResultDir.Add("卡号", "00000000");
                            X6MBTestResultDir.Add("喇叭", "");
                            X6MBTestResultDir.Add("继电器", "");
                            X6MBTestResultDir.Add("FLASH", "");
                            X6MBTestResultDir.Add("SETRTC", "");
                            X6MBTestResultDir.Add("GETRTC", "");
                            //X6MBTestResultDir.Add("2G", "");
                            X6MBTestResultDir.Add("测试时间", DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss"));
                            X6MBTestResultDir.Add("测试用时", "0");

                            X6GetResultObj.UsedTime_main = GetCurrentTimeStamp();


                            if (X6MBTestThread != null)
                            {
                                X6MBTestThread.Abort();
                                X6MBTestThread = null;
                            }
                            X6MBTestThread = new Thread(X6MainBoardTestProcess);
                            X6MBTestThread.Start();
                        }
                        else
                        {
                            LOG("主板请求开始测试失败.");
                            updateControlText(X6skinButton_PCBA_STARTTEST, "开始测试");
                            X6MBTestingFlag = false;
                        }
                    }
                    else
                    {//结束测试ack
                        LOG("主板请求结束测试成功.");
                        X6MBTestingFlag = false;
                        updateControlText(X6skinButton_PCBA_STARTTEST, "开始测试");
                    }
                }
                else if (X6PCBATestSelectIndex == 1)                                      //副板测试
                {
                    if (X6GetResultObj.testMode == 0x00)        //副板开始测试请求回复
                    {
                        if (X6GetResultObj.testModeAllow == 0x00)//成功
                        {
                            LOG("副板请求开始测试成功.");
                            updateControlText(X6skinButton_PCBA_STARTTEST, "结束测试");
                            X6SBTestingFlag = true;

                            DateTime now = DateTime.Now;
                            X6SBTestResultDir.Clear();
                            X6SBTestResultDir.Add("PCB编号", X6textBox_SB_QR.Text.Trim());
                            X6SBTestResultDir.Add("测试员", ProcTestData.PresentAccount);
                            X6SBTestResultDir.Add("软件版本", "");
                            X6SBTestResultDir.Add("测试结果", "");
                            X6SBTestResultDir.Add("卡号", "00000000");
                            X6SBTestResultDir.Add("蓝牙", "");
                            X6SBTestResultDir.Add("2.4G", "");
                            X6SBTestResultDir.Add("测试时间", DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss"));
                            X6SBTestResultDir.Add("测试用时", "0");

                            X6GetResultObj.UsedTime_interface = GetCurrentTimeStamp();
                            X6SBTabSelectIndex = 1;
                            updateTableSelectedIndex(X6skinTabControl_SB, X6SBTabSelectIndex);

                            if (X6SBTestThread != null)
                            {
                                X6SBTestThread.Abort();
                                X6SBTestThread = null;
                            }

                            X6SBTestThread = new Thread(X6SubBoardTestProcess);
                            X6SBTestThread.Start();
                        }
                        else
                        {
                            LOG("副板请求开始测试失败.");
                            updateControlText(X6skinButton_PCBA_STARTTEST, "开始测试");
                            X6SBTestingFlag = false;
                        }
                    }
                    else//副板结束测试请求回复
                    {
                        LOG("副板请求结束测试成功!");
                        X6SBTestingFlag = false;
                        updateControlText(X6skinButton_PCBA_STARTTEST, "开始测试");
                    }
                }
            }
            else if (X6TestMeunSelectIndex == 2)        //整机测试
            {
                if (X6GetResultObj.testMode == 0x00)//整机开始测试请求回复
                {
                    if (X6GetResultObj.testModeAllow == 0x00)//成功
                    {
                        LOG("整机请求开始测试成功.");
                        updateControlText(X6skinButton_WholeChg_StartTest, "结束测试");
                        X6ChargerTestingFlag = true;

                        DateTime now = DateTime.Now;
                        X6ChargerTestResultDir.Clear();
                        X6ChargerTestResultDir.Add("电桩号", X6textBox_WholeChg_SN_QR.Text.Trim());
                        X6ChargerTestResultDir.Add("主板编号", "");
                        X6ChargerTestResultDir.Add("软件版本", "");
                        X6ChargerTestResultDir.Add("测试员", ProcTestData.PresentAccount);
                        X6ChargerTestResultDir.Add("测试结果", "");
                        X6ChargerTestResultDir.Add("指示灯", "");
                        X6ChargerTestResultDir.Add("喇叭", "");
                        X6ChargerTestResultDir.Add("蓝牙", "");
                        X6ChargerTestResultDir.Add("卡号", "00000000");
                        X6ChargerTestResultDir.Add("2.4G", "");
                        X6ChargerTestResultDir.Add("继电器", "");
                        X6ChargerTestResultDir.Add("SETRTC", "");
                        X6ChargerTestResultDir.Add("GETRTC", "");
                        X6ChargerTestResultDir.Add("测试时间", DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss"));
                        X6ChargerTestResultDir.Add("测试用时", "0");

                        X6GetResultObj.UsedTime_Charger = GetCurrentTimeStamp();
                        X6chargerTestSelectIndex = 1;
                        updateTableSelectedIndex(X6skinTabControl_WholeChg, X6chargerTestSelectIndex);

                        if (X6ChargerTestThread != null)
                        {
                            X6ChargerTestThread.Abort();
                            X6ChargerTestThread = null;
                        }
                        X6ChargerTestThread = new Thread(X6ChargerTestProcess);
                        X6ChargerTestThread.Start();
                    }
                    else
                    {
                        LOG("整机请求开始测试失败.");
                        updateControlText(X6skinButton_WholeChg_StartTest, "开始测试");
                        X6ChargerTestingFlag = false;
                    }
                }
                else//整机结束测试请求回复
                {
                    LOG("整机请求结束测试成功...");
                    X6ChargerTestingFlag = false;
                    updateControlText(X6skinButton_WholeChg_StartTest, "开始测试");
                }

            }
        }

        //继电器测试消息处理
        private void X6MessageRelayTestHandle(byte[] pkt)
        {
            X6GetResultObj.relay = pkt[17];

            int operate = pkt[17];//继电器动作
            byte ch = pkt[18];    //第N路 
            string power1 = "";
            string power2 = "";
            string resultStr = "";
            bool pass = false;

            int Relay1_Power = ((pkt[19] << 8) | (pkt[20])) / 10;
            int Relay2_Power = ((pkt[21] << 8) | (pkt[22])) / 10;

            X6GetResultObj.getPower[ch] = Relay1_Power;
            X6GetResultObj.getPower[ch + 1] = Relay2_Power;

            power1 += X6GetResultObj.getPower[ch] + "W";
            power2 += X6GetResultObj.getPower[ch + 1] + "W";

            updateControlText(X6skinLabelRelayResult1, power1);
            updateControlText(X6skinLabelRelayResult2, power2);

            updateControlText(X6skinLabel_CHG_RELAY1_POWER, power1);
            updateControlText(X6skinLabel_CHG_RELAY2_POWER, power2);

            resultStr = "第";
            for (int j = 0; j < 2; j++)
            {
                //LOG("jjjjj:" + j);
                //LOG("GetResultObj.getPower[j]:" + GetResultObj.getPower[j]);
                //LOG("PowerLowerLimit:" + Convert.ToInt32(TestSettingInfo["PowerLowerLimit"]));
                //LOG("PowerUpperLimi:" + Convert.ToInt32(TestSettingInfo["PowerUpperLimit"]));
                if (X6GetResultObj.getPower[j] < Convert.ToInt32(X6TestSettingInfo["PowerLowerLimit"]) || X6GetResultObj.getPower[j] > Convert.ToInt32(X6TestSettingInfo["PowerUpperLimit"]))
                {
                    resultStr += (j + 1) + " ";
                    pass = false;
                }
                else
                {
                    pass = true;
                }
            }
            resultStr += "路功率不正常";

            if (X6TestMeunSelectIndex == 1)//PCBA测试
            {
                if (X6PCBATestSelectIndex == 0)//主板测试
                {
                    if (pass)
                    {
                        LOG("继电器测试pass,功率正常.");
                        X6MBTestResultDir["继电器"] = "通过";
                        updateControlText(X6skinLabel_MB_RELAY_RESULT, "测试通过", Color.Green);
                        updateTableSelectedIndex(X6skinTabControl_MB, ++X6MBTabSelectIndex);
                    }
                    else
                    {
                        LOG("继电器测试失败,功率不正常.");
                        X6MBTestResultDir["继电器"] = "不通过";
                        updateControlText(X6skinLabel_MB_RELAY_RESULT, "测试不通过," + resultStr, Color.Red);
                    }
                }
            }
            else if (X6TestMeunSelectIndex == 2)//整机测试
            {
                if (pass)
                {
                    LOG("整机测试,继电器测试成功,功率正常.");
                    X6ChargerTestResultDir["继电器"] = "通过";
                    updateControlText(X6skinLabel_CHG_RELAY_RESULT, "测试通过", Color.Green);
                    updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
                }
                else
                {
                    LOG("整机测试,继电器测试失败,功率不正常.");
                    X6ChargerTestResultDir["继电器"] = "不通过";
                    updateControlText(X6skinLabel_CHG_RELAY_RESULT, "测试不通过," + resultStr, Color.Red);
                }
            }
        }

        int length;
        private void X6TestDataProc(byte[] data)
        {
            byte[] buf = new byte[data.Length];
            Array.Copy(data, buf, data.Length);

            try
            {
                if (buf.Length > 17)
                {
                    for (int i = 0; i < buf.Length; i++)
                    {
                        if (buf[i] == 0xAA && buf[i + 1] == 0x55)
                        {
                            length = (buf[i + 12]) + (buf[i + 13] << 8) + 14;
                            if (buf.Length >= (length + i))
                            {
                                byte checkSum = buf[length + i - 1];
                                byte[] validFrame = new byte[length];

                                Array.Copy(buf, i, validFrame, 0, validFrame.Length);

                                byte calcCRC = ProcTestData.caculatedCRC(validFrame, validFrame.Length - 1);

                                if (X6MsgDebug)
                                {
                                    string receive = "";
                                    for (int j = 0; j < validFrame.Length; j++)
                                    {
                                        receive += validFrame[j].ToString("X2") + " ";
                                    }
                                    LOG("Receive: " + receive);

                                }

                                if (calcCRC == checkSum)
                                {
                                    arraybuffer.Clear();

                                    byte cmd = validFrame[16];

                                    switch (cmd)
                                    {
                                        case (byte)X6Command.TestMode://测试模式请求
                                            if (4 <= (GetCurrentTimeStamp() - X6TestTimeTicks))
                                            {
                                                X6TestTimeTicks = GetCurrentTimeStamp();
                                                X6MessageTestModeHandle(validFrame);
                                            }
                                            break;

                                        case (byte)X6Command.CMD_KEY_TEST:
                                            break;

                                        case (byte)X6Command.CMD_CARD_TEST://刷卡
                                            string pStr = "";
                                            X6GetResultObj.tapCard = 0XA5;
                                            //int ik = 0;
                                          //  for (ik = 0; ik < 16; ik++)
                                          //  {
                                           //     LOG("aaaa:" + ik);
                                          //      LOG("qqqqqqq:" + validFrame[16 + ik] + "\r\n");
                                          //  }
                                            
                                            X6GetResultObj.cardNum = Encoding.ASCII.GetString(validFrame, 18, 16).ToUpper();
                                            X6GetResultObj.cardNum = X6GetResultObj.cardNum.Remove(X6GetResultObj.cardNum.IndexOf('\0'));
                                            
                                            if (48 != validFrame[17])
                                            {
                                                pStr += validFrame[17].ToString("X2");
                                               // LOG("kkk:" + pStr + "\r\n");
                                            }
                                            //LOG("PPPPP:" + validFrame[17] + "\r\n");
                                            pStr += X6GetResultObj.cardNum;
                                            //LOG("HHH:" + pStr + "\r\n");
                                            LOG("卡号:" + X6GetResultObj.cardNum + "\r\n");
                                            if (X6TestMeunSelectIndex == 1)//PCBA测试
                                            {
                                                if (X6PCBATestSelectIndex == 0)//主板测试
                                                {
                                                }
                                                else if (X6PCBATestSelectIndex == 1)//副板测试
                                                {
                                                    //LOG("卡号qqqqqqq " + X6TestSettingInfo["CardNum"].ToString() + "\r\n");
                                                   // X6textBox_TestCardNum.Text
                                                    if (X6TestSettingInfo["CardNum"].ToString() == X6GetResultObj.cardNum)
                                                    {
                                                        X6SBTestResultDir["刷卡"] = "通过";
                                                        X6SBTestResultDir["卡号"] = X6GetResultObj.cardNum;
                                                        updateControlText(X6skinLabelChargerTapCardResult, "测试通过", Color.Green);
                                                        if (6 <= (GetCurrentTimeStamp() - X6cardNumTimeTicks))
                                                        {
                                                            X6cardNumTimeTicks = GetCurrentTimeStamp();
                                                            updateTableSelectedIndex(X6skinTabControl_SB, ++X6SBTabSelectIndex);
                                                        }
                                                    }
                                                    else
                                                    {
                                                        LOG("卡号与设置的不一致,请点击重新测试按钮!");
                                                        updateControlText(X6skinLabelChargerTapCardResult, "测试不通过", Color.Red);
                                                        X6SBTestResultDir["刷卡"] = "不通过";
                                                    }
                                                }
                                            }
                                            else if (X6TestMeunSelectIndex == 2)//整机测试
                                            {
                                                if (X6TestSettingInfo["CardNum"].ToString() == X6GetResultObj.cardNum)
                                                {
                                                    X6ChargerTestResultDir["刷卡"] = "通过";
                                                    X6ChargerTestResultDir["卡号"] = X6GetResultObj.cardNum;
                                                    updateControlText(X6WholeX6skinLabelChargerTapCardResult, "测试通过", Color.Green);
                                                    if (3 <= (GetCurrentTimeStamp() - X6WholeCardNumTimeTicks))
                                                    {
                                                        X6WholeCardNumTimeTicks = GetCurrentTimeStamp();
                                                        updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
                                                    }
                                                }
                                                else
                                                {
                                                    LOG("卡号与设置的不一致,请点击重新测试按钮!");
                                                    updateControlText(X6WholeX6skinLabelChargerTapCardResult, "测试不通过", Color.Red);
                                                    X6ChargerTestResultDir["刷卡"] = "不通过";
                                                }
                                            }
                                            break;

                                        case (byte)X6Command.CMD_LCD_TEST://指示灯
                                            X6GetResultObj.lcd = validFrame[17];
                                            break;

                                        case (byte)X6Command.CMD_2G_TEST://2G模块
                                            
                                            break;

                                        case (byte)X6Command.CMD_TRUMPET_TEST://喇叭
                                            X6GetResultObj.trumpet = validFrame[17];
                                            break;

                                        case (byte)X6Command.CMD_RELAY_TEST://继电器
                                            X6MessageRelayTestHandle(validFrame);
                                            Thread.Sleep(500);
                                            break;

                                        case (byte)X6Command.CMD_SET_PCB://设置PCB
                                            X6GetResultObj.SetPcbCode = validFrame[18];
                                            if (validFrame[17] == 0x00)
                                            {
                                                if (X6GetResultObj.SetPcbCode == 0x00)
                                                {
                                                    LOG("主板编码设置成功");
                                                }
                                                else
                                                {
                                                    LOG("主板编码设置失败");
                                                }
                                            }
                                            else if (validFrame[18] == 0x01)
                                            {
                                                if (X6GetResultObj.SetPcbCode == 0x00)
                                                {
                                                    LOG("副板编码设置成功");
                                                }
                                                else
                                                {
                                                    LOG("副板编码设置失败");
                                                }
                                            }
                                            break;

                                        case (byte)X6Command.CMD_SET_SN:
                                            X6GetResultObj.SetCID = 1;
                                            if (validFrame[17] == 0x00)
                                            {
                                                //设置成功     
                                                LOG("桩号设置成功");
                                            }
                                            else
                                            {
                                                LOG("桩号设置失败");
                                            }
                                            break;

                                        case (byte)X6Command.CMD_BT_TEST://蓝牙测试
                                            getBlueToothFlag = 1;
                                            X6GetResultObj.BLE = validFrame[17];

                                            LOG("蓝牙接收数据:" + X6GetResultObj.BLE.ToString("D2"));
                                            if (X6TestMeunSelectIndex == 1)//PCBA测试
                                            {
                                                if (X6PCBATestSelectIndex == 0)//主板测试
                                                {
                                                }
                                                else if (X6PCBATestSelectIndex == 1)//副板测试
                                                {
                                                    switch (X6GetResultObj.BLE)
                                                    {
                                                        case 0x01:
                                                            X6SBTestResultDir["蓝牙"] = "通过";
                                                            updateControlText(X6skinLabel_SB_BT_RESULT, "测试通过", Color.Green);
                                                            updateTableSelectedIndex(X6skinTabControl_SB, ++X6SBTabSelectIndex);
                                                            break;
                                                        case 0x00:
                                                            updateControlText(X6skinLabel_SB_BT_RESULT, "测试不通过", Color.Red);
                                                            X6SBTestResultDir["蓝牙"] = "不通过";
                                                            break;
                                                        case 0x02:
                                                            X6SBTestResultDir["蓝牙"] = "无";
                                                            updateControlText(X6skinLabel_SB_BT_RESULT, "此PCB不带蓝牙模块", Color.Black);
                                                            updateTableSelectedIndex(X6skinTabControl_SB, ++X6SBTabSelectIndex);
                                                            break;
                                                    }
                                                }
                                            }
                                            else if (X6TestMeunSelectIndex == 2)//整机测试
                                            {
                                                switch (X6GetResultObj.BLE)
                                                {
                                                    case 0x01:
                                                        X6ChargerTestResultDir["蓝牙"] = "通过";
                                                        updateControlText(X6skinLabel_CHG_BT_RESULT, "测试通过", Color.Green);
                                                        updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
                                                        break;
                                                    case 0x00:
                                                        updateControlText(X6skinLabel_CHG_BT_RESULT, "测试不通过", Color.Red);
                                                        X6ChargerTestResultDir["蓝牙"] = "不通过";
                                                        break;
                                                    case 0x02:
                                                        X6ChargerTestResultDir["蓝牙"] = "无";
                                                        updateControlText(X6skinLabel_CHG_BT_RESULT, "此PCB不带蓝牙模块", Color.Black);
                                                        updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
                                                        break;
                                                }
                                            }
                                            break;
                                        case (byte)X6Command.CMD_24G_COMMUNICATION_TEST://24g通信测试
                                            X6GetResultObj.BLE_24G = validFrame[17];
                                            getX6_24G_Flag = 1;

                                            LOG("24G接收数据:" + X6GetResultObj.BLE_24G.ToString("D2"));
                                            if (X6TestMeunSelectIndex == 1)//PCBA测试
                                            {
                                                if (X6PCBATestSelectIndex == 0)//主板测试
                                                {
                                                }
                                                else if (X6PCBATestSelectIndex == 1)//副板测试
                                                {

                                                }
                                            }
                                            else if (X6TestMeunSelectIndex == 2)//整机测试
                                            {
                                                switch (X6GetResultObj.BLE_24G)
                                                {
                                                    case 0x01:
                                                        X6ChargerTestResultDir["2.4G"] = "通过";
                                                        updateControlText(X6skinLabel_CHG_24G_RESULT, "测试通过", Color.Green);
                                                        updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
                                                        break;
                                                    case 0x00:
                                                        updateControlText(X6skinLabel_CHG_24G_RESULT, "测试不通过", Color.Red);
                                                        X6ChargerTestResultDir["2.4G"] = "不通过";
                                                        break;
                                                    case 0x02:
                                                        X6ChargerTestResultDir["2.4G"] = "无";
                                                        updateControlText(X6skinLabel_CHG_24G_RESULT, "此PCB不带蓝牙模块", Color.Black);
                                                        updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
                                                        break;
                                                }
                                            }
                                            break;
                                        case (byte)X6Command.CMD_GET_FW:
                                            if (6 <= (GetCurrentTimeStamp() - X6SBGetfwTicks))
                                            {
                                                X6SBGetfwTicks = GetCurrentTimeStamp();
                                                break;
                                            }
                                            if (validFrame[17] == 0x00)
                                            {
                                                int fwVer = (int)((validFrame[18] << 8) | (validFrame[19]));
                                                int subver = validFrame[20];
                                                X6GetResultObj.FwVersion = fwVer + "." + subver;
                                                LOG("get软件版本.\n" + GetCurrentTimeStamp());
                                            }
                                            else if (validFrame[17] == 0x01)
                                            {
                                                X6GetResultObj.FwVersion = validFrame[18].ToString("D3");
                                            }

                                            if (X6MBTestingFlag)
                                            {
                                                X6MBTestResultDir["软件版本"] = X6GetResultObj.FwVersion;
                                            }
                                            else if (X6SBTestingFlag)
                                            {
                                                X6SBTestResultDir["软件版本"] = X6GetResultObj.FwVersion;
                                            }
                                            else if (X6ChargerTestingFlag)
                                            {
                                                if (validFrame[17] == 0)
                                                {
                                                    X6ChargerTestResultDir["软件版本"] = X6GetResultObj.FwVersion;
                                                }
                                                else
                                                {
                                                    X6ChargerTestResultDir["副板软件版本"] = X6GetResultObj.FwVersion;
                                                }
                                            }
                                            LOG("软件版本:" + X6GetResultObj.FwVersion);
                                            break;

                                        case (byte)X6Command.CMD_READ_PCB_CODE://读取PCB编号
                                            string str = "";
                                            bool isZero = true;
                                            for (int m = 0; m < 8; m++)
                                            {
                                                if (validFrame[18 + m] != 0x00 && isZero == true)
                                                {
                                                    isZero = false;
                                                }

                                                if (isZero == false)
                                                {
                                                    str += validFrame[18 + m].ToString("X2");
                                                }

                                            }
                                            if (validFrame[17] == 0)
                                            {

                                                X6GetResultObj.MainBoardCode = str;
                                                X6ChargerTestResultDir["主板编号"] = X6GetResultObj.MainBoardCode;
                                            }
                                            else if (validFrame[17] == 1)
                                            {
                                                X6GetResultObj.InterfaceBoardCode = str;
                                                X6ChargerTestResultDir["按键板编号"] = X6GetResultObj.InterfaceBoardCode;
                                            }
                                            break;
                                        case (byte)X6Command.CMD_SET_REGISTER_CODE:
                                            X6GetResultObj.SetRegisterCode = validFrame[17];
                                            break;
                                        case (byte)X6Command.CMD_SET_DEV_TYPE:
                                            break;
                                        case (byte)X6Command.CMD_SET_2_4G_GW_ADD:
                                            if (validFrame[17] == 0)
                                            {
                                                X6SetGwAddrFlag = 0XA5;
                                                LOG("2.4G网关地址成功");
                                                //X6setTerminalInfoFlag = true;
                                            }
                                            else
                                            {
                                                LOG("2.4G网关地址失败");
                                                //X6setTerminalInfoFlag = false;
                                            }
                                            break;
                                        case (byte)X6Command.CMD_SET_TERMINAL_INFO:
                                            if (validFrame[17] == 0)
                                            {
                                                X6SetLocal2_4GAddrFlag = 0xa5;
                                                LOG("设置终端信息成功");
                                                //X6setTerminalInfoFlag = true;
                                            }
                                            else
                                            {
                                                LOG("设置终端信息失败");
                                                //X6setTerminalInfoFlag = false;
                                            }
                                            break;
                                        case (byte)X6Command.CMD_SET_SERVER_ADDR:
                                            break;
                                        case (byte)X6Command.CMD_SET_SERVER_PORT:
                                            break;
                                        case (byte)X6Command.CMD_SET_PRINT_SWITCH:
                                            break;
                                        case (byte)X6Command.CMD_REBOOT:
                                            if (validFrame[17] == 0x00)
                                            {
                                                //设置成功     
                                                LOG("设备重启成功");
                                            }
                                            else
                                            {
                                                LOG("设备重启失败");
                                            }
                                            break;
                                        case (byte)X6Command.CMD_SET_DEVICE_ID:
                                            if (validFrame[17] == 0x00)
                                            {
                                                //设置成功     
                                                LOG("设置识别码成功");
                                            }
                                            else
                                            {
                                                LOG("设置识别码失败");
                                            }
                                            break;
                                        case (byte)X6Command.CMD_SET_RTC:
                                            X6MessageSetRtcHandle(validFrame);
                                            Thread.Sleep(500);
                                            break;
                                        case (byte)X6Command.CMD_GET_RTC:
                                            X6MessageGetRtctHandle(validFrame);
                                            Thread.Sleep(500);
                                            break;
                                        case (byte)X6Command.CMD_FLASH_TEST:
                                            X6MessageFlashTestHandle(validFrame);
                                            Thread.Sleep(500);
                                            break;
                                        case (byte)X6Command.CMD_LED_TEST:
                                            if (validFrame[17] == 0)
                                            {
                                                LOG("指示灯测试回复OK.");
                                            }
                                            break;

                                        case (byte)X6Command.CMD_485_TEST:
                                            
                                            break;
                                        case (byte)X6Command.CMD_ONLINE_TEST:
                                            
                                            break;
                                        case (byte)X6Command.CMD_GET_CHARGER_SN://获取桩号
                                            
                                        case (byte)X6Command.CMD_GET_DEVICE_CODE://获取桩号
                                            
                                            break;
                                        case (byte)X6Command.CMD_START_AGING_TEST:
                                            if (validFrame[17] == 0)
                                            {
                                                X6SendOldTestFlag = 0xa5;
                                                LOG("老机测试请求成功.");
                                            }
                                            else
                                            {
                                                LOG("老机测试请求失败.");
                                            }
                                            break;
                                        case (byte)X6Command.CMD_GET_AGING_TEST_RESULT:
                                            
                                            break;
                                        default:
                                            break;
                                    }
                                }
                                else
                                {
                                    LOG("recv data err");
                                    if (X6MsgDebug)
                                    {
                                        string receive = "";
                                        for (int j = 0; j < validFrame.Length; j++)
                                        {
                                            receive += validFrame[j].ToString("X2") + " ";
                                        }
                                        LOG("Receive: " + receive);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                //MessageBox.Show(ex.Message);
            }
        }

        private void X6serialPort_DataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            int n = X6serialPort.BytesToRead;
            byte[] buf = new byte[n];
            X6serialPort.Read(buf, 0, n);

            arraybuffer.AddRange(buf);
            X6TestDataProc(arraybuffer.ToArray());
        }

        //倒计时显示
        public int X6ItemCountDown(int time, Label label, TabControl tabControl, TabPage tabPage)
        {
            if (time > 0)
            {
                time--;
                updateControlText(label, time.ToString("D2"));
                if (time == 0)
                {
                    if (tabControl.SelectedTab == tabPage)
                    {
                        tabControl.SelectedIndex++;
                    }

                }
            }
            return time;
        }

        private void X6timer_Tick(object sender, EventArgs e)
        {
            if (X6tick++ >= 10)
            {
                X6RtcCount++;
                X6tick = 0;

                if (X6MBTestingFlag)    //主板测试倒计时
                {
                    X6countDownTime_MB.PowerSource = X6ItemCountDown(X6countDownTime_MB.PowerSource, X6skinLabel_MB_PowerTimeCountDown, X6skinTabControl_MB, X6skinTabPage_MainBoard_Power);
                    X6countDownTime_MB.lcd = X6ItemCountDown(X6countDownTime_MB.lcd, X6skinLabel_MB_LED_TIMECOUNTDOWN, X6skinTabControl_MB, X6skinTabPage_MainBoard_Led);
                    X6countDownTime_MB.trumpet = X6ItemCountDown(X6countDownTime_MB.trumpet, X6skinLabel_MB_TRUMPT_TIME, X6skinTabControl_MB, X6skinTabPage_MB_TRUMPT);
                    X6countDownTime_MB.relay = X6ItemCountDown(X6countDownTime_MB.relay, X6skinLabel_MB_Relay_TIME, X6skinTabControl_MB, X6skinTabPage_MB_RELAY);

                    X6countDownTime_MB.flash = X6ItemCountDown(X6countDownTime_MB.flash, X6skinLabel_MB_FLASH_TIME, X6skinTabControl_MB, X6skinTabPage_MB_FLASH);
                    X6countDownTime_MB.setRtc = X6ItemCountDown(X6countDownTime_MB.setRtc, X6skinLabel_MB_SETRTC_TIME, X6skinTabControl_MB, X6skinTabPage_MB_SET_RTC);
                    X6countDownTime_MB.getRtc = X6ItemCountDown(X6countDownTime_MB.getRtc, X6skinLabel_MB_GETRTC_TIME, X6skinTabControl_MB, X6skinTabPage_MB_GET_RTC);
                }
                else if (X6SBTestingFlag)   //副板测试倒计时
                {
                    X6countDownTime_SB.tapCard = X6ItemCountDown(X6countDownTime_SB.tapCard, X6skinLabel_SB_CARD_TIME, X6skinTabControl_SB, X6skinTabPageChargerTapCard);
                    X6countDownTime_SB.BLE = X6ItemCountDown(X6countDownTime_SB.BLE, X6skinLabel_SB_BT_TIME, X6skinTabControl_SB, X6skinTabPage_SB_BLUETOOTH);
                    X6countDownTime_SB._2_4G = X6ItemCountDown(X6countDownTime_SB._2_4G, X6skinLabel_SB_24G_TIME, X6skinTabControl_SB, X6skinTabPage_SB_BLUETOOTH);
                }
                else if (X6ChargerTestingFlag)  //整机测试倒计时
                {
                    X6countDownTimeCharger.lcd = X6ItemCountDown(X6countDownTimeCharger.lcd, X6skinLabel_WholeChg_Led_Time, X6skinTabControl_WholeChg, X6skinTabPage_WholeChg_Led);
                    X6countDownTimeCharger.trumpet = X6ItemCountDown(X6countDownTimeCharger.trumpet, X6skinLabel_CHG_TRUMPT_TIME, X6skinTabControl_WholeChg, X6skinTabPage_CHG_TRUMPT);
                    X6countDownTimeCharger.BLE = X6ItemCountDown(X6countDownTimeCharger.BLE, X6skinLabel_WholeChg_BT_Time, X6skinTabControl_WholeChg, X6skinTabPage_WholeChg_Bt);
                    X6countDownTimeCharger.tapCard = X6ItemCountDown(X6countDownTimeCharger.tapCard, X6WholeskinLabel_CARD_TIME, X6skinTabControl_WholeChg, X6WholeskinTabPageChargerTapCard);
                    X6countDownTimeCharger._2_4G = X6ItemCountDown(X6countDownTimeCharger._2_4G, X6skinLabel_WholeChg_2POINT4_Time, X6skinTabControl_WholeChg, X6skinTabPage_WholeChg_2POINT4);
                    X6countDownTimeCharger.relay = X6ItemCountDown(X6countDownTimeCharger.relay, X6skinLabel_CHG_RELAY_TIME, X6skinTabControl_WholeChg, X6skinTabPage_CHG_RELAY);
                    X6countDownTimeCharger.setRtc = X6ItemCountDown(X6countDownTimeCharger.setRtc, X6skinLabel_Whole_SETRTC_TIME, X6skinTabControl_WholeChg, X6skinTabPage_WHOLE_TEST_SET_RTC);
                    X6countDownTimeCharger.getRtc = X6ItemCountDown(X6countDownTimeCharger.getRtc, X6skinLabel_Whole_GETRTC_TIME, X6skinTabControl_WholeChg, X6skinTabPage_WHOLE_TEST_GET_RTC);
                }
            }
        }

        private void numericUpDown1_ValueChanged(object sender, EventArgs e)
        {

        }

        private void X6skinComboBox_ChgType_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void X6skinTabPage_CurrentUser_Click(object sender, EventArgs e)
        {

        }

        private void skinNumericUpDown1_ValueChanged(object sender, EventArgs e)
        {
                    }

        private void skinNumericUpDown1_ValueChanged_1(object sender, EventArgs e)
        {

        }

        private void skinNumericUpDown1_ValueChanged_2(object sender, EventArgs e)
        {

        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {

        }

        private void textBox1_TextChanged_1(object sender, EventArgs e)
        {

        }

        private void skinNumericUpDown_PowerLowerLimit_ValueChanged(object sender, EventArgs e)
        {

        }

        private void skinLabel64_Click(object sender, EventArgs e)
        {

        }

        private void label11_Click(object sender, EventArgs e)
        {

        }

        private void X6skinButton_SaveSettings_Click(object sender, EventArgs e)
        {
            try
            {
                X6TestSettingInfo["ChargerModel"] = X6skinComboBox_ChgType.SelectedItem;
                X6TestSettingInfo["CountDown"] = X6skinNumericUpDown_TestOverTime.Value;
                X6TestSettingInfo["CardNum"] = X6textBox_TestCardNum.Text;
                X6TestSettingInfo["PowerLowerLimit"] = X6skinNumericUpDown_PowerLowerLimit.Value;
                X6TestSettingInfo["PowerUpperLimit"] = X6skinNumericUpDown_PowerUpperLimit.Value;
                ProcTestData.WriteConfig(ProcTestData.X6testConfigFile, X6TestSettingInfo);
                MessageBox.Show("保存成功", "温馨提示", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "异常提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);

            }

        }

        //升级按钮监听
        string X6filepath = @".\X6_Upgrade_Tools_V1.1\release\";
        private void X6skinButton_upGrade_Click(object sender, EventArgs e)
        {
            try
            {
                //选择烧录软件安装路径
                System.Diagnostics.Process.Start(X6filepath + "X6_Upgrade.exe");
            }
            catch (Exception ex)
            {
                LOG("升级工具打开出错!");
                LOG(ex.Message);
            }
        }

        //获取当前tab控件的页
        private TabPage X6getPresentTabPage(TabControl tabControl)
        {
            TabPage tabPage = null;
            try
            {
                tabControl.Invoke(
                new MethodInvoker(delegate
                {
                    tabPage = tabControl.SelectedTab;
                }));
            }
            catch (Exception ex)
            {
                //LOG(ex.Message);
            }
            return tabPage;
        }

        private void X6skinTabControl_X6TestMenu_Selecting(object sender, TabControlCancelEventArgs e)
        {
            if ((X6getPresentTabPage(X6skinTabControl_X6TestMenu) == X6skinTabPage_PCBATest)
                || (X6getPresentTabPage(X6skinTabControl_X6TestMenu) == X6skinTabPage_X6WholeChgTest)
                || (X6getPresentTabPage(X6skinTabControl_X6TestMenu) == X6skinTabPageConfig))
            {
                if (X6serialPort.IsOpen == false)
                {
                    MessageBox.Show("请先打开串口", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    e.Cancel = true;
                }
            }
        }

        private void X6skinButton_WholeChg_BT_Over_Click(object sender, EventArgs e)
        {
            LOG("跳过整机蓝牙测试.");
            updateControlText(X6skinLabel_CHG_BT_RESULT, "跳过", Color.Green);
            updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
        }

        private void X6skinButton_WholeChg_BT_RTest_Click(object sender, EventArgs e)
        {
            ItemTestTime = GetCurrentTimeStamp();
            //X6countDownTimeCharger.BLE = X6countdownTime;
            X6countDownTimeCharger.BLE = 60;
            X6ChargerTestResultDir["蓝牙"] = "";
            updateControlText(X6skinLabel_CHG_BT_RESULT, "");
            LOG("蓝牙重新测试.");

            X6SendBlueToothTestReq();
        }
        
        //选择串口端口号
        private void skinComboBox_X6SerialPortSelect_DropDown(object sender, EventArgs e)
        {
            try
            {
                skinComboBox_X6SerialPortSelect.Items.Clear();

                foreach (string s in System.IO.Ports.SerialPort.GetPortNames())
                {//获取多少串口
                    skinComboBox_X6SerialPortSelect.Items.Add(s);
                }

                if (skinComboBox_X6SerialPortSelect.Items.Count > 0)
                {
                    skinComboBox_X6SerialPortSelect.SelectedIndex = 0;
                    skinComboBox_X6SerialBuateSelect.SelectedIndex = 0;
                }
                else
                {
                    skinComboBox_X6SerialPortSelect.Text = "";
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "提示");
            }
        }

        //波特率索引更新
        private void skinComboBox_X6SerialBuateSelect_SelectedIndexChanged(object sender, EventArgs e)
        {
            X6serialPort.BaudRate = int.Parse(skinComboBox_X6SerialBuateSelect.SelectedItem.ToString());
        }

        private void splitContainer1_Panel2_Paint(object sender, PaintEventArgs e)
        {

        }

        private void X6skinButtonChargerTapCardSkip_Click(object sender, EventArgs e)
        {
            LOG("副板跳过刷卡测试.");
            updateControlText(X6skinLabelChargerTapCardResult, "跳过", Color.Green);
            updateTableSelectedIndex(X6skinTabControl_SB, ++X6SBTabSelectIndex);
        }
        

        private void X6skinButtonChargerTapCardReTest_Click(object sender, EventArgs e)
        {
            ItemTestTime = GetCurrentTimeStamp();
            X6countDownTime_SB.tapCard = X6countdownTime;
            //X6MBTestResultDir["指示灯"] = "";
            //updateControlText(X6skinLabel_MB_LED_RESULT, "");
            LOG("副板刷卡重新测试.");
            //发送指示灯测试指令
            X6SendTapCard();
        }

        private void splitContainer2_Panel2_Paint(object sender, PaintEventArgs e)
        {

        }

        private void X6skinButton_SB_BT_SUCCESS_Click(object sender, EventArgs e)
        {
            LOG("副板蓝牙测试成功.");
            X6SBTestResultDir["蓝牙"] = "通过";
            updateControlText(X6skinLabel_SB_BT_RESULT, "通过", Color.Green);
            if (6 <= (GetCurrentTimeStamp() - X6BlueToothTimeTicks))
            {
                X6BlueToothTimeTicks = GetCurrentTimeStamp();
                updateTableSelectedIndex(X6skinTabControl_SB, ++X6SBTabSelectIndex);
            }
        }

        private void X6skinButton_SB_BT_FAIL_Click(object sender, EventArgs e)
        {
            LOG("副板蓝牙测试失败.");
            X6SBTestResultDir["蓝牙"] = "不通过";
            updateControlText(X6skinLabel_SB_BT_RESULT, "不通过", Color.Red);
            if (6 <= (GetCurrentTimeStamp() - X6BlueToothTimeTicks))
            {
                X6BlueToothTimeTicks = GetCurrentTimeStamp();
                updateTableSelectedIndex(X6skinTabControl_SB, ++X6SBTabSelectIndex);
            }
        }

        private void X6skinButton_SB_BT_SKIP_Click(object sender, EventArgs e)
        {
            LOG("副板跳过蓝牙测试.");
            updateControlText(X6skinLabel_SB_BT_RESULT, "跳过", Color.Green);
            if (6 <= (GetCurrentTimeStamp() - X6BlueToothTimeTicks))
            {
                X6BlueToothTimeTicks = GetCurrentTimeStamp();
                updateTableSelectedIndex(X6skinTabControl_SB, ++X6SBTabSelectIndex);
            }
        }

        private void X6skinButton_SB_BT_RTEST_Click(object sender, EventArgs e)
        {
            LOG("副板蓝牙重新测试.");
            ItemTestTime = GetCurrentTimeStamp();
            X6countDownTime_SB.BLE = X6countdownTime;
        }

        private void skinLabel91_Click(object sender, EventArgs e)
        {

        }

        private void skinLabel88_Click(object sender, EventArgs e)
        {

        }

        private void X6skinLabel_SB_TEST_USETIME_VAL_Click(object sender, EventArgs e)
        {

        }

        private void X6skinLabel_SB_TEST_START_TIME_Click(object sender, EventArgs e)
        {

        }

        private void skinLabel98_Click(object sender, EventArgs e)
        {

        }

        private void X6skinLabel_SB_CARD_TIME_Click(object sender, EventArgs e)
        {

        }

        private void splitContainer3_Panel2_Paint(object sender, PaintEventArgs e)
        {

        }

        private void X6skinButton_SB_24G_RTEST_Click(object sender, EventArgs e)
        {
            LOG("副板2.4G重新测试");
            ItemTestTime = GetCurrentTimeStamp();
            X6countDownTime_SB._2_4G = X6countdownTime;
        }

        private void X6skinButton_SB_24G_SUCCESS_Click(object sender, EventArgs e)
        {
            LOG("副板2.4G测试成功.");
            X6SBTestResultDir["2.4G"] = "通过";
            updateControlText(X6skinLabel_SB_24G_RESULT, "通过", Color.Green);
            if (6 <= (GetCurrentTimeStamp() - X624GTimeTicks))
            {
                X624GTimeTicks = GetCurrentTimeStamp();
                updateTableSelectedIndex(X6skinTabControl_SB, ++X6SBTabSelectIndex);
            }
        }

        private void X6skinButton_SB_24G_FAIL_Click(object sender, EventArgs e)
        {
            LOG("副板2.4G测试失败.");
            X6SBTestResultDir["2.4G"] = "不通过";
            updateControlText(X6skinLabel_SB_24G_RESULT, "不通过", Color.Red);
            if (6 <= (GetCurrentTimeStamp() - X624GTimeTicks))
            {
                X624GTimeTicks = GetCurrentTimeStamp();
                updateTableSelectedIndex(X6skinTabControl_SB, ++X6SBTabSelectIndex);
            }
        }

        private void X6skinButton_SB_24G_SKIP_Click(object sender, EventArgs e)
        {
            LOG("跳过副板2.4G测试.");
            updateControlText(X6skinLabel_SB_24G_RESULT, "跳过", Color.Green);
            if (6 <= (GetCurrentTimeStamp() - X624GTimeTicks))
            {
                X624GTimeTicks = GetCurrentTimeStamp();
                updateTableSelectedIndex(X6skinTabControl_SB, ++X6SBTabSelectIndex);
            }
        }

        private void skinSplitContainer17_Panel2_Paint(object sender, PaintEventArgs e)
        {

        }

        private void X6skinButton_WholeChg_2POINT4_Over_Click(object sender, EventArgs e)
        {
            LOG("跳过整机2.4G测试.");
            updateControlText(X6skinLabel_CHG_24G_RESULT, "跳过", Color.Green);
            updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
        }

        private void X6skinButton_WholeChg_2POINT4_RTest_Click(object sender, EventArgs e)
        {
            ItemTestTime = GetCurrentTimeStamp();
            X6countDownTimeCharger._2_4G = X6countdownTime;
            X6ChargerTestResultDir["2.4G"] = "";
            updateControlText(X6skinLabel_CHG_24G_RESULT, "");

            X6Send24G_COMMUNICATION_TestReq();
            LOG("整机2.4G重新测试.");
        }

        private void skinSplitContainer16_Panel2_Paint(object sender, PaintEventArgs e)
        {

        }

        private void X6button_OldTest_Click(object sender, EventArgs e)
        {
            X6SendOldTest();
        }

        private void X6textBox_WholeChg_SN_QR_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (e.KeyChar == '\r')
            {
                X6skinButton_WholeChg_StartTest_Click(sender, e);
            }
        }

        private void X6textBox_MB_QRCode_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (e.KeyChar == '\r')
            {
                X6skinButton_PCBA_STARTTEST_Click(sender, e);
            }
        }

        private void X6textBox_SB_QR_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (e.KeyChar == '\r')
            {
                X6skinButton_SB_CONFIRM_Click(sender, e);
            }
        }

        private void X6WholeskinTabPageChargerTapCard_Click(object sender, EventArgs e)
        {

        }

        private void X6WholeskinButtonChargerTapCardReTest_Click(object sender, EventArgs e)
        {
            ItemTestTime = GetCurrentTimeStamp();
            X6countDownTimeCharger.tapCard = X6countdownTime;
            LOG("副板刷卡重新测试.");
            //
            X6SendTapCard();
        }

        private void X6WholeskinButtonChargerTapCardSkip_Click(object sender, EventArgs e)
        {
            LOG("跳过刷卡测试.");
            updateControlText(X6WholeX6skinLabelChargerTapCardResult, "跳过", Color.Green);
            updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
        }

        private void splitContainer6_Panel2_Paint(object sender, PaintEventArgs e)
        {

        }

        private void X6skinButton_Whole_SETRTC_SKIP_Click(object sender, EventArgs e)
        {
            LOG("跳过设置RTC时间.");
            updateControlText(X6WholeskinLabel_SETRTC_RESULT, "跳过", Color.Green);
            updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
        }

        private void X6skinButton_Whole_SETRTC_RTEST_Click(object sender, EventArgs e)
        {
            ItemTestTime = GetCurrentTimeStamp();
            X6countDownTimeCharger.setRtc = X6countdownTime;
            X6ChargerTestResultDir["SETRTC"] = "";
            updateControlText(X6WholeskinLabel_SETRTC_RESULT, "");
            LOG("重新设置RTC时间.");
            //发送设置RTC时间指令
            SendSetRtcTestReq();
        }

        private void X6skinButton_WholeTest_GETRTC_SKIP_Click(object sender, EventArgs e)
        {
            LOG("跳过读取RTC时间.");
            updateControlText(X6skinLabel_WholeTest_GETRTC_RESULT, "跳过", Color.Green);
            updateTableSelectedIndex(X6skinTabControl_WholeChg, ++X6chargerTestSelectIndex);
        }

        private void X6skinButton_WholeTest_GETRTC_RTEST_Click(object sender, EventArgs e)
        {
            ItemTestTime = GetCurrentTimeStamp();
            X6countDownTimeCharger.getRtc = X6countdownTime;
            X6ChargerTestResultDir["GETRTC"] = "";
            updateControlText(X6skinLabel_WholeTest_GETRTC_RESULT, "");
            LOG("重新读取RTC时间.");
            //发送读取RTC时间指令
            SendGetRtcTestReq();
        }

        string X6HexToBinfilepath = @".\changer_tools\";
        private void X6_skinButton_HexToBin_Click(object sender, EventArgs e)
        {
            try
            {
                //选择转换软件安装路径
                System.Diagnostics.Process.Start(X6HexToBinfilepath + "Tools.exe");
            }
            catch (Exception ex)
            {
                LOG("升级工具打开出错!");
                LOG(ex.Message);
            }
        }
    }
}



