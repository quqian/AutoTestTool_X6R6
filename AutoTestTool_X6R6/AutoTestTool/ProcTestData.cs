using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.IO;

using SpreadsheetLight;
using DocumentFormat.OpenXml.Spreadsheet;
using System.Text.RegularExpressions;
using System.Security.Cryptography;
using MySql.Data.MySqlClient;
using System.Data;
using System.Net.NetworkInformation;


namespace AutoTestTool
{
    class ProcTestData
    {

        public static List<string> Account = new List<string> { };
        public static List<string> Password = new List<string> { };

        public static string PresentAccount = "";
        public static string StationIdQrcodeUrl = "http://www.chargerlink.com/alicp/";
        public static string X6StationIdQrcodeUrl = "http://www.chargerlink.com/alicn/";
        public static string savePath = @".\智能报表\";
        public static string testConfigFile = "TestInfo.conf";
        public static string encryptAccountFile = "private.bin";
        public static string lastLoginUserFile = "lastUser.rd";
        public static string backupMysqlCmdFile = "MysqlCmd.bak";

        //public static string X6savePath = @".\X6智能报表\";
        public static string X6testConfigFile = "X6TestInfo.conf";

        // http://www.chargerlink.com/alicp/3900009999
        // http://www.chargerlink.com/alicp/3900008888
        // http://www.chargerlink.com/alicp/3800008888


        // http://www.chargerlink.com/alicn/3800008888


        static string encryptKey = ".*w@";    //定义密钥  

        #region 加密字符串  
        /// <summary> /// 加密字符串   
                /// </summary>  
                /// <param name="str">要加密的字符串</param>  
                /// <returns>加密后的字符串</returns>  
        public static string Encrypt(string str)
        {
            DESCryptoServiceProvider descsp = new DESCryptoServiceProvider();   //实例化加/解密类对象   

            byte[] key = Encoding.Unicode.GetBytes(encryptKey); //定义字节数组，用来存储密钥    

            byte[] data = Encoding.Unicode.GetBytes(str);//定义字节数组，用来存储要加密的字符串  

            MemoryStream MStream = new MemoryStream(); //实例化内存流对象      

            //使用内存流实例化加密流对象   
            CryptoStream CStream = new CryptoStream(MStream, descsp.CreateEncryptor(key, key), CryptoStreamMode.Write);

            CStream.Write(data, 0, data.Length);  //向加密流中写入数据      

            CStream.FlushFinalBlock();              //释放加密流      

            return Convert.ToBase64String(MStream.ToArray());//返回加密后的字符串  
        }
        #endregion

        #region 解密字符串   
        /// <summary>  
                /// 解密字符串   
                /// </summary>  
                /// <param name="str">要解密的字符串</param>  
                /// <returns>解密后的字符串</returns>  
        public static string Decrypt(string str)
        {
            DESCryptoServiceProvider descsp = new DESCryptoServiceProvider();   //实例化加/解密类对象    

            byte[] key = Encoding.Unicode.GetBytes(encryptKey); //定义字节数组，用来存储密钥    

            byte[] data = Convert.FromBase64String(str);//定义字节数组，用来存储要解密的字符串  

            MemoryStream MStream = new MemoryStream(); //实例化内存流对象      

            //使用内存流实例化解密流对象       
            CryptoStream CStream = new CryptoStream(MStream, descsp.CreateDecryptor(key, key), CryptoStreamMode.Write);

            CStream.Write(data, 0, data.Length);      //向解密流中写入数据     

            CStream.FlushFinalBlock();               //释放解密流      

            return Encoding.Unicode.GetString(MStream.ToArray());       //返回解密后的字符串  
        }
        #endregion

        public static byte caculatedCRC(byte[] buffer, int length)
        {
            byte crc = 0;
            int sum = 0;

            for (int i = 14; i < length; i++)
            {
                sum += buffer[i];
            }

            crc = (byte)(sum % 256);

            return crc;
        }

        public static string fillString(string srcstr, int destLen, char fillChar, int index)
        {

            string str;
            str = srcstr;
            if (srcstr.Length < destLen)
            {
                for (int i = 0; i < destLen - srcstr.Length; i++)
                {
                    str = str.Insert(index, fillChar.ToString());
                }
            }
            return str;
        }

        public static byte[] stringToBCD(string text)
        {

            if (text.Length % 2 != 0)
            {
                text = text.Insert(0, "0");
            }
            byte[] array = new byte[text.Length / 2];
            string str = "";
            int j = 0;
            try
            {
                for (int i = 0; i < text.Length; i += 2)
                {
                    str = text[i].ToString() + text[i + 1].ToString();

                    array[j] = byte.Parse(str, System.Globalization.NumberStyles.HexNumber);

                    j++;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }

            return array;

        }

        [DllImport("wininet.dll")]
        private extern static bool InternetGetConnectedState(int Description, int ReservedValue);

        #region 
        /// <summary>
        /// 用于检查网络是否可以连接互联网,true表示连接成功,false表示连接失败 
        /// </summary>
        /// <returns></returns>
        public static bool IsConnectInternet()
        {
            int Description = 0;
            return InternetGetConnectedState(Description, 0);
        }
        #endregion

       

        public static string mysqlConnectionString = "Server=factorytest.chargerlink.com;Port=3307;Database=X6SQL;Uid=liutao;Pwd=fatorytest@liutao;Charset=utf8";
        public static bool SendMysqlCommand(string mysqlCmd, bool backupIfFail)
        {
            bool state = false;
            MySqlConnection conn = new MySqlConnection(mysqlConnectionString);
            MySqlCommand command;
            try
            {
                conn.Open();
                command = conn.CreateCommand();
                command.CommandText = mysqlCmd;
                command.ExecuteNonQuery();
                state = true;
            }
            catch (Exception ex)
            {
                state = false;
                if (backupIfFail)
                {
                    BackupUnconnectData(backupMysqlCmdFile, mysqlCmd);//上传失败则备份
                }
                MessageBox.Show(ex.Message, "异常提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
            finally
            {
                if (conn.State == ConnectionState.Open)
                {
                    conn.Close();
                }

            }
            return state;
        }

        /// <summary>
        /// 用执行的数据库连接执行一个返回数据集的sql命令
        /// </summary>
        /// <remarks>
        /// 举例:
        ///  MySqlDataReader r = ExecuteReader(connString, CommandType.StoredProcedure, "PublishOrders", new MySqlParameter("@prodid", 24));
        /// </remarks>
        /// <param name="connectionString">一个有效的连接字符串</param>
        /// <param name="cmdType">命令类型(存储过程, 文本, 等等)</param>
        /// <param name="cmdText">存储过程名称或者sql命令语句</param>
        /// <param name="commandParameters">执行命令所用参数的集合</param>
        /// <returns>包含结果的读取器</returns>
        public static MySqlDataReader ExecuteReader(string connectionString, string mysqlCmd)
        {
            //创建一个MySqlCommand对象
            MySqlCommand cmd = new MySqlCommand();
            //创建一个MySqlConnection对象
            MySqlConnection conn = new MySqlConnection(connectionString);

            //在这里我们用一个try/catch结构执行sql文本命令/存储过程，因为如果这个方法产生一个异常我们要关闭连接，因为没有读取器存在，
            //因此commandBehaviour.CloseConnection 就不会执行
            try
            {
                ////调用 PrepareCommand 方法，对 MySqlCommand 对象设置参数
                //PrepareCommand(cmd, conn, null, cmdType, cmdText, commandParameters);

                conn.Open();
                cmd = conn.CreateCommand();
                cmd.CommandText = mysqlCmd;
                //cmd.ExecuteNonQuery();

                //调用 MySqlCommand  的 ExecuteReader 方法
                MySqlDataReader reader = cmd.ExecuteReader(CommandBehavior.CloseConnection);
                //清除参数
                // cmd.Parameters.Clear();
                //while (reader.Read())
                //{
                //    Console.WriteLine( reader.GetString(1)+"  "+reader.GetString(2));
                //}

                return reader;
            }
            catch
            {
                //关闭连接，抛出异常
                conn.Close();
                throw;
            }
        }

        public static List<string> GetMysqlUserInfo(string item)
        {
            try
            {
                string cmd = "select " + item + " from login_table";
                MySqlDataReader reader = ExecuteReader(mysqlConnectionString, cmd);

                List<string> list = new List<string> { };

                while (reader.Read())
                {
                    list.Add(reader.GetString(item));
                }
                return list;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "异常提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return null;
            }

        }

        public static void SaveUserInfo(string filePath, List<string> username, List<string> password)
        {
            using (StreamWriter sw = new StreamWriter(filePath))
            {
                string context = "";
                for (int i = 0; i < username.Count; i++)
                {
                    context += username[i] + ":" + password[i] + "\n";
                }
                //Console.WriteLine(context);
                string enContext = Encrypt(context);
                sw.Write(enContext);
                sw.Flush();
                sw.Close();

            }
        }

        public static Dictionary<string, string> ReadUserInfo(string filePath)
        {
            Dictionary<string, string> dictionary = new Dictionary<string, string> { };
            using (StreamReader sr = new StreamReader(filePath))
            {
                string enContext = sr.ReadToEnd();
                string deContext = Decrypt(enContext);
                string[] context = deContext.Split('\n');

                for (int i = 0; i < context.Length; i++)
                {
                    // Console.WriteLine(context[i]);
                    if (context[i] != "")
                    {
                        string[] info = context[i].Split(':');
                        dictionary.Add(info[0], info[1]);
                    }
                }
                sr.Close();
            }
            return dictionary;

        }

        public static void WriteLastUserName(string filePath, string userName)
        {
            File.WriteAllText(filePath, userName);
        }

        public static string ReadLastUserName(string filePath)
        {
            try
            {
                return File.ReadAllText(filePath);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                return null;
            }
        }

        public static void ModifyUserPassword(string username, string password)
        {

            string mysqlCmd = "UPDATE login_table SET  password='" + password + "' WHERE user='" + username + "';";

            MySqlConnection conn = new MySqlConnection(mysqlConnectionString);
            MySqlCommand command;
            try
            {
                conn.Open();
                command = conn.CreateCommand();
                command.CommandText = mysqlCmd;
                command.ExecuteNonQuery();

            }
            catch (Exception ex)
            {


                MessageBox.Show(ex.Message, "异常提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
            finally
            {
                if (conn.State == ConnectionState.Open)
                {
                    conn.Close();
                }

            }
        }

        public static void AddUserAndPassword(string username, string password)
        {
            string mysqlCmd = "INSERT INTO login_table(user,password) VALUES('" + username + "','" + password + "');";

            MySqlConnection conn = new MySqlConnection(mysqlConnectionString);
            MySqlCommand command;
            try
            {
                conn.Open();
                command = conn.CreateCommand();
                command.CommandText = mysqlCmd;
                command.ExecuteNonQuery();

            }
            catch (Exception ex)
            {


                MessageBox.Show(ex.Message, "异常提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
            finally
            {
                if (conn.State == ConnectionState.Open)
                {
                    conn.Close();
                }

            }
        }

        public static void DeleteUser(string username)
        {
            string mysqlCmd = "DELETE FROM login_table WHERE user='" + username + "';";

            MySqlConnection conn = new MySqlConnection(mysqlConnectionString);
            MySqlCommand command;
            try
            {
                conn.Open();
                command = conn.CreateCommand();
                command.CommandText = mysqlCmd;
                command.ExecuteNonQuery();

            }
            catch (Exception ex)
            {

                MessageBox.Show(ex.Message, "异常提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
            finally
            {
                if (conn.State == ConnectionState.Open)
                {
                    conn.Close();
                }

            }
        }

        public static bool CheckChgIdInSql(string presentId)
        {
            bool result = false;
            string cmd = "select CHG_ID from CHG_TEST_TABLE where CHG_ID = " + presentId;

            try
            {
                MySqlDataReader reader = ExecuteReader(mysqlConnectionString, cmd);
                if (reader.Read() == false)
                {
                    result = true;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);

            }

            return result;

        }


        public static void DealBackUpData(string filePath)
        {
            bool state = true;
            try
            {
                string context = ReadBackupMysqlCmd(filePath);
                if (context == "")
                {
                    return;
                }

                string[] line = context.Split('\n');
                for (int i = 0; i < line.Length; i++)
                {
                    if (line[i] != "")
                    {
                        if (SendMysqlCommand(line[i].Trim(), false) == false)
                        {
                            state = false;
                            break;

                        }
                    }
                }
                if (state == true)
                {
                    File.Delete(filePath);
                }

            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "异常提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
        }

        public static string ReadBackupMysqlCmd(string filePath)
        {
            string context = "";
            try
            {
                using (StreamReader sr = new StreamReader(filePath))
                {
                    context = sr.ReadToEnd();
                    sr.Close();
                }
            }
            catch (Exception ex)
            {

                Console.WriteLine(ex.Message);
            }

            return context;
        }

        public static void BackupUnconnectData(string filePath, string cmd)
        {
            try
            {
                string savedContext = ReadBackupMysqlCmd(filePath);
                using (StreamWriter sw = new StreamWriter(filePath))
                {
                    string context = savedContext + cmd + "\n";
                    sw.Write(context);
                    sw.Flush();
                    sw.Close();
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        public static void renameFile(string filename)
        {
            if (File.Exists(savePath + filename) == true)
            {
                FileInfo fileInfo = new FileInfo(savePath + filename);
                if (fileInfo.Length >= 50*1024)
                {
                    string newName = savePath + DateTime.Now.ToString("yyyyMMddHHmm") + "_" + filename;
                    fileInfo.MoveTo(newName);
                }
            }
        }

        //去掉字符串中的非数字
        public static string RemoveNotNumber(string key)
        {
            return Regex.Replace(key, @"[^\d]*", "");
        }

        //liutao
        public static string MainboardTestMysqlCommand( string chargerType, string mainboardSN, string testor,
                                                        string mainboardFw, string totalResult, string powersource,
                                                        string lcd,  string flash, string testTime, UInt32 totalTime)
        {
            return String.Format(
            "INSERT INTO R_MB_TEST_TABLE(" +
                    "MB_TYPE," +
                    "MB_PCB," +
                    "MB_TESTOR," +
                    "MB_FW," +
                    "MB_TEST_RESULT," +
                    "MB_POWER_SOURCE," +
                    "MB_LED," +
                    "MB_FLASH," +
                    "MB_TEST_TIME,"  +
                    "MB_USE_TIME) VALUES('{0}','{1}','{2}','{3}','{4}','{5}','{6}','{7}','{8}','{9}');",
            chargerType, mainboardSN, testor, mainboardFw, totalResult, powersource, lcd, flash, testTime, totalTime);
        }

        public static string SubBoardTestMysqlCommand( string chargerType, string mainboardSN, string testor,
                                                       string interfaceFw, string totalResult, string ble, string _2_4G,
                                                       string _2g, string csq, string testTime, UInt32 totalTime)
        {
            return String.Format(
            "INSERT INTO R_SB_TEST_TABLE(" +
                    "SB_TYPE," +
                    "SB_PCB," +
                    "SB_TESTOR," +
                    "SB_FW," +
                    "SB_TEST_RESULT," +
                    "SB_BT," +
                    "SB_2_4G," +
                    "SB_2G," +
                    "SB_CSQ," +
                    "SB_TEST_TIME," +
                    "SB_USE_TIME) VALUES('{0}','{1}','{2}','{3}','{4}','{5}','{6}','{7}','{8}','{9}','{10}');",
            chargerType, mainboardSN, testor, interfaceFw, totalResult, ble, _2_4G, _2g, csq, testTime, totalTime);
        }

        public static string ChargerTestMysqlCommand( string chargerType, string chargerId, string testor,
                                                      string mainboardFw, string mainboardSN, string totalResult,
                                                      string lcd, string ble, string _2_4G, string _2g, string csq,
                                                      string iccid, string testTime, UInt32 totalTime)
        {
            return String.Format(
            "INSERT INTO R_CHG_TEST_TABLE(" +
                    "R_CHG_TYPE," +
                    "R_CHG_ID," +
                    "R_CHG_TESTOR," +
                    "R_CHG_FW," +
                    "R_CHG_MB_PCB," +
                    "R_CHG_TEST_RESULT," +
                    "R_CHG_LED," +
                    "R_CHG_BT," +
                    "R_CHG_24G," +
                    "R_CHG_2G," +
                    "R_CHG_CSQ," +
                    "R_CHG_ICCID," +
                    "R_CHG_TEST_TIME," +
                    "R_CHG_USE_TIME) VALUES('{0}','{1}','{2}','{3}','{4}','{5}','{6}','{7}','{8}','{9}','{10}','{11}','{12}','{13}');",
            chargerType, chargerId, testor, mainboardFw, mainboardSN, totalResult, lcd, ble, _2_4G, _2g, csq, iccid, testTime, totalTime);
        }

        public static string R9PCBATestMysqlCommand(string chargerType, string pcbCode, string testor,
                                                       string mainboardFw, string totalResult, string powersource, string trumpt,
                                                       string _2g, string signal, string gpio, string testTime, UInt32 totalTime)
        {
            return String.Format(
            "INSERT INTO R9_PCBA_TEST_TABLE(" +
                    "PCBA_TYPE," +
                    "R9_PCB," +
                    "R9_TESTOR," +
                    "R9_FW," +
                    "R9_TEST_RESULT," +
                    "R9_POWER," +
                    "R9_TRUMPT," +
                    "R9_2G," +
                    "R9_SIGNAL," +
                    "R9_GPIO," +
                    "R9_TEST_TIME," +
                    "R9_TEST_USE_TIME) VALUES('{0}','{1}','{2}','{3}','{4}','{5}','{6}','{7}','{8}','{9}','{10}','{11}');",
            chargerType, pcbCode, testor, mainboardFw, totalResult, powersource, trumpt, _2g, signal, gpio, testTime, totalTime);
        }

        public static string R9ChargerTestMysqlCommand(string chargerType, string chargerId, string mainboardSN, string testor,
                                                      string mainboardFw,  string totalResult, string _2g, string csq,
                                                      string iccid, string trumpt, string gpio, string online, string testTime, UInt32 totalTime)

        {
            return String.Format(
            "INSERT INTO R9_CHG_TEST_TABLE(" +
                    "R9_CHG_TYPE," +
                    "R9_CHG_SN," +
                    "R9_CHG_PCB," +
                    "R9_CHG_TESTOR," +
                    "R9_CHG_FW," +
                    "R9_CHG_TEST_RESULT," +
                    "R9_CHG_2G," +
                    "R9_CHG_SIGNAL," +
                    "R9_CHG_ICCID," +
                    "R9_CHG_TRUMPT," +
                    "R9_CHG_GPIO," +
                    "R9_CHG_ONLINE," +
                    "R9_CHG_TEST_TIME," +
                    "R9_CHG_TEST_USE_TIME) VALUES('{0}','{1}','{2}','{3}','{4}','{5}','{6}','{7}','{8}','{9}','{10}','{11}','{12}','13');",
            chargerType, chargerId, testor, mainboardFw, mainboardSN, totalResult, _2g, csq, iccid, trumpt, gpio, online, testTime, totalTime);
        }
        public static int Asc(string character)
        {
            if (character.Length == 1)
            {
                System.Text.ASCIIEncoding asciiEncoding = new System.Text.ASCIIEncoding();
                int intAsciiCode = (int)asciiEncoding.GetBytes(character)[0];
                return (intAsciiCode);
            }
            else
            {
                throw new Exception("Character is not valid.");
            }

        }

        public static string Chr(int asciiCode)
        {
            if (asciiCode >= 0 && asciiCode <= 255)
            {
                System.Text.ASCIIEncoding asciiEncoding = new System.Text.ASCIIEncoding();
                byte[] byteArray = new byte[] { (byte)asciiCode };
                string strCharacter = asciiEncoding.GetString(byteArray);
                return (strCharacter);
            }
            else
            {
                throw new Exception("ASCII Code is not valid.");
            }
        }

        static string getNextColumnCellName(string lastCol)
        {
            if (lastCol.Length == 1)
            {
                int last = Asc(lastCol);
                last++;
                if (last >= 91)
                {
                    return "AA";
                }
                else
                {
                    return Chr(last);
                }
            }
            else if (lastCol.Length == 2)
            {
                string laststr = lastCol.Substring(1);
                int last = Asc(laststr);
                last++;
                if (last >= 91)
                {
                    return "AAA";
                }
                else
                {
                    return "A" + Chr(last);
                }
            }
            else
            {
                return "";
            }
        }
        public static void WriteReportHeader(string fileName, string testMode, string[] header)
        {
            try
            {
                if (File.Exists(fileName) == false)
                {
                    using (SLDocument s = new SLDocument())
                    {
                        s.SaveAs(fileName);
                    }
                }

                using (SLDocument sl = new SLDocument(fileName))
                {
                    SLStyle headerstyle = sl.CreateStyle();
                    headerstyle.Alignment.Horizontal = DocumentFormat.OpenXml.Spreadsheet.HorizontalAlignmentValues.Center;
                    headerstyle.Font.FontSize = 18;
                    headerstyle.Font.FontName = "宋体";
                    headerstyle.Font.Bold = true;

                    SLStyle secondrow_header = sl.CreateStyle();
                    secondrow_header.Alignment.Horizontal = DocumentFormat.OpenXml.Spreadsheet.HorizontalAlignmentValues.Center;
                    secondrow_header.SetWrapText(true);
                    secondrow_header.SetVerticalAlignment(VerticalAlignmentValues.Center);
                    secondrow_header.Font.FontSize = 12;
                    secondrow_header.Font.FontName = "宋体";


                    string rowindex = "1";
                    string lastCol = "A";

                    sl.SetCellValue("A1", testMode);
                    sl.MergeWorksheetCells("A1", "M1");
                    sl.SetCellStyle("A1", headerstyle);

                    rowindex = "2";
                    lastCol = "A";

                    sl.SetCellStyle("A2", "M2", secondrow_header);

                    for (int i = 0; i < header.Length; i++)
                    {
                        sl.SetCellValue(lastCol + rowindex, header[i]);
                        lastCol = getNextColumnCellName(lastCol);
                    }
                    sl.Save();
                }


            }
            catch (Exception ex)
            {

                System.Windows.Forms.MessageBox.Show(ex.Message);
            }
        }

        public static void WriteReportHeader(string fileName, string testMode, Dictionary<string, string> dictionary)
        {
            try
            {
                if (File.Exists(fileName) == false)
                {
                    using (SLDocument s = new SLDocument())
                    {
                        s.SaveAs(fileName);
                    }
                }

                using (SLDocument sl = new SLDocument(fileName))
                {
                    SLStyle headerstyle = sl.CreateStyle();
                    headerstyle.Alignment.Horizontal = DocumentFormat.OpenXml.Spreadsheet.HorizontalAlignmentValues.Center;
                    headerstyle.Font.FontSize = 18;
                    headerstyle.Font.FontName = "宋体";
                    headerstyle.Font.Bold = true;

                    SLStyle secondrow_header = sl.CreateStyle();
                    secondrow_header.Alignment.Horizontal = DocumentFormat.OpenXml.Spreadsheet.HorizontalAlignmentValues.Center;
                    secondrow_header.SetWrapText(true);
                    secondrow_header.SetVerticalAlignment(VerticalAlignmentValues.Center);
                    secondrow_header.Font.FontSize = 12;
                    secondrow_header.Font.FontName = "宋体";


                    string rowindex = "1";
                    string lastCol = "A";

                    char endRow = Convert.ToChar('A' + dictionary.Keys.Count);

                    sl.SetCellValue("A1", testMode);
                    sl.MergeWorksheetCells("A1", endRow.ToString() + '1');
                    sl.SetCellStyle("A1", headerstyle);

                    rowindex = "2";
                    lastCol = "A";

                    sl.SetCellStyle("A2", endRow.ToString() + '2', secondrow_header);

                    foreach (var item in dictionary)
                    {
                        sl.SetCellValue(lastCol + rowindex, item.Key);
                        lastCol = getNextColumnCellName(lastCol);
                    }

                    sl.Save();
                }


            }
            catch (Exception ex)
            {

                System.Windows.Forms.MessageBox.Show(ex.Message);
            }
        }

        public static void WriteReportContext(string filename, string[] context)
        {
            try
            {
                using (SLDocument sl = new SLDocument(filename))
                {
                    string lastCol = "A";
                    int row = 3;

                    while (sl.GetCellValueAsString(lastCol + row.ToString()) != "")
                    {
                        row++;
                    }

                    for (int i = 0; i < context.Length; i++)
                    {
                        sl.SetCellValue(lastCol + row.ToString(), context[i]);
                        lastCol = getNextColumnCellName(lastCol);
                    }
                    sl.Save();
                }
            }
            catch (Exception ex)
            {

                System.Windows.Forms.MessageBox.Show(ex.Message);
            }
        }

        public static void WriteReportContext(string filename, Dictionary<string, string> dictionary)
        {
            try
            {
                using (SLDocument sl = new SLDocument(filename))
                {
                    string lastCol = "A";
                    int row = 3;

                    while (sl.GetCellValueAsString(lastCol + row.ToString()) != "")
                    {
                        row++;
                    }



                    foreach (var item in dictionary)
                    {
                        sl.SetCellValue(lastCol + row.ToString(), item.Value);
                        lastCol = getNextColumnCellName(lastCol);
                    }

                    sl.Save();
                }
            }
            catch (Exception ex)
            {

                MessageBox.Show(ex.Message);
            }
        }
        public static void WriteReport(string fileName, string testMode, string[] header, string[] context)
        {
            renameFile(fileName);
            WriteReportHeader(savePath + fileName, testMode, header);
            WriteReportContext(savePath + fileName, context);
        }

        public static void WriteReport(string fileName, string testMode, Dictionary<string, string> dictionary)
        {
            renameFile(fileName);
            WriteReportHeader(savePath + fileName, testMode, dictionary);
            WriteReportContext(savePath + fileName, dictionary);
        }


        /// <summary>
        /// 获得字符串中开始和结束字符串中间得值
        /// </summary>
        /// <param name="str">字符串</param>
        /// <param name="s">开始</param>
        /// <param name="e">结束</param>
        /// <returns></returns> 
        public static string GetValue(string str, string s, string e)
        {
            Regex rg = new Regex("(?<=(" + s + "))[.\\s\\S]*?(?=(" + e + "))", RegexOptions.Multiline | RegexOptions.Singleline);
            return rg.Match(str).Value;
        }

        public static Dictionary<string, object> ReadConfig(string filePath, Dictionary<string, object> dictionary)
        {
            if (File.Exists(filePath) == false)
            {
                WriteConfig(filePath, dictionary);     
            }
            else
            {
                using (StreamReader sr = new StreamReader(filePath))
                {
                    string line;
                    string context = "";
                    while (null != (line = sr.ReadLine()))
                    {
                        context += line + "\r\n";
                    }

                    Console.WriteLine(context);

                    dictionary["ChargerModel"] = GetValue(context, "ChargerModel:", "\r\n");
                    dictionary["CountDown"] = GetValue(context, "CountDown:", "\r\n");
                    dictionary["CardNum"] = GetValue(context, "CardNum:", "\r\n");
                    dictionary["CsqLowerLimit"] = GetValue(context, "CsqLowerLimit:", "\r\n");
                    dictionary["CsqUpperLimit"] = GetValue(context, "CsqUpperLimit:", "\r\n");
                    dictionary["PowerLowerLimit"] = GetValue(context, "PowerLowerLimit:", "\r\n");
                    dictionary["PowerUpperLimit"] = GetValue(context, "PowerUpperLimit:", "\r\n");
                }
            }
            return dictionary;
        }

        public static void WriteConfig(string filePath, Dictionary<string, object> dictionary)
        {

            using (StreamWriter sw = new StreamWriter(filePath))
            {
                foreach (var item in dictionary)
                {
                    sw.WriteLine(item.Key + ":" + item.Value);
                }
                sw.Flush();
                sw.Close();
            }
        }

        public static void setArrayValue(Array array, object setValue, int startindex, int length)
        {
            try
            {
                for (int i = startindex; i < length; i++)
                {
                    array.SetValue(setValue, i);
                }
            }
            catch (Exception)
            {

            }
        }
    }
}
