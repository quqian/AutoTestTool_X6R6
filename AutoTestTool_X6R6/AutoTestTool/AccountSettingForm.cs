using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace AutoTestTool
{
    public partial class AccountSettingForm : Form
    {
        public AccountSettingForm()
        {
            InitializeComponent();
        }

        private void skinButtonAddUser_Click(object sender, EventArgs e)
        {
            if (textBoxUserName.Text != "" && textBoxPassword.Text != "")
            {
                if (ProcTestData.Account.Contains(textBoxUserName.Text) == false)
                {
                    ProcTestData.Account.Add(textBoxUserName.Text);
                    ProcTestData.Password.Add(textBoxPassword.Text);

                    ProcTestData.AddUserAndPassword(textBoxUserName.Text, textBoxPassword.Text);
                    MessageBox.Show("已添加" + textBoxUserName.Text, "温馨提示", MessageBoxButtons.OK, MessageBoxIcon.Information);
                    comboBoxAllUsers.Items.Add(textBoxUserName.Text);
                }
                else
                {
                    MessageBox.Show("账号已存在", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    textBoxUserName.Text = "";
                }

            }
            else
            {
                MessageBox.Show("账号或密码不能为空", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
        }

        private void skinButtonComfirmModifyPwd_Click(object sender, EventArgs e)
        {
            if (textBoxModifiedPassword.Text == "")
            {
                MessageBox.Show("密码不能为空", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }
            ProcTestData.Password[comboBoxAllUsers.SelectedIndex] = textBoxModifiedPassword.Text;
            ProcTestData.ModifyUserPassword(ProcTestData.Account[comboBoxAllUsers.SelectedIndex], ProcTestData.Password[comboBoxAllUsers.SelectedIndex]);
            MessageBox.Show(comboBoxAllUsers.SelectedItem.ToString() + " 密码已设置成功", "温馨提示", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        private void skinButtonDeleteUser_Click(object sender, EventArgs e)
        {
            if (comboBoxAllUsers.SelectedItem.ToString() == "Admin")
            {
                MessageBox.Show("此用户为管理用户\r\n不可删除!", "警告", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }
            if (MessageBox.Show("确认删除用户:" + comboBoxAllUsers.SelectedItem.ToString() + "\r\n此操作不能撤回！", "提示", MessageBoxButtons.YesNo, MessageBoxIcon.Warning) == DialogResult.Yes)
            {
                ProcTestData.DeleteUser(comboBoxAllUsers.SelectedItem.ToString());
                ProcTestData.Account.RemoveAt(comboBoxAllUsers.SelectedIndex);
                ProcTestData.Password.RemoveAt(comboBoxAllUsers.SelectedIndex);
                comboBoxAllUsers.Items.RemoveAt(comboBoxAllUsers.SelectedIndex);
                textBoxModifiedPassword.Text = "";
            }
        }

        private void AccountSettingForm_Load(object sender, EventArgs e)
        {
            comboBoxAllUsers.Items.AddRange(ProcTestData.Account.ToArray());
        }

        private void comboBoxAllUsers_SelectedIndexChanged(object sender, EventArgs e)
        {
            textBoxModifiedPassword.Text = ProcTestData.Password[comboBoxAllUsers.SelectedIndex];
        }
    }
}
