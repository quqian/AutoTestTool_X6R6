namespace AutoTestTool
{
    partial class AccountSettingForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(AccountSettingForm));
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.label1 = new System.Windows.Forms.Label();
            this.textBoxUserName = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.textBoxPassword = new System.Windows.Forms.TextBox();
            this.skinButtonAddUser = new CCWin.SkinControl.SkinButton();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.comboBoxAllUsers = new System.Windows.Forms.ComboBox();
            this.textBoxModifiedPassword = new System.Windows.Forms.TextBox();
            this.skinButtonComfirmModifyPwd = new CCWin.SkinControl.SkinButton();
            this.skinButtonDeleteUser = new CCWin.SkinControl.SkinButton();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).BeginInit();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.SuspendLayout();
            // 
            // splitContainer1
            // 
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.Location = new System.Drawing.Point(0, 0);
            this.splitContainer1.Name = "splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.groupBox1);
            this.splitContainer1.Panel1.Tag = "";
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.groupBox2);
            this.splitContainer1.Size = new System.Drawing.Size(692, 355);
            this.splitContainer1.SplitterDistance = 340;
            this.splitContainer1.TabIndex = 0;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.skinButtonAddUser);
            this.groupBox1.Controls.Add(this.textBoxPassword);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.textBoxUserName);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.groupBox1.Location = new System.Drawing.Point(0, 0);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(340, 355);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "添加用户";
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.skinButtonDeleteUser);
            this.groupBox2.Controls.Add(this.skinButtonComfirmModifyPwd);
            this.groupBox2.Controls.Add(this.textBoxModifiedPassword);
            this.groupBox2.Controls.Add(this.comboBoxAllUsers);
            this.groupBox2.Controls.Add(this.label4);
            this.groupBox2.Controls.Add(this.label3);
            this.groupBox2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.groupBox2.Location = new System.Drawing.Point(0, 0);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(348, 355);
            this.groupBox2.TabIndex = 0;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "修改密码";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.BackColor = System.Drawing.Color.Transparent;
            this.label1.Location = new System.Drawing.Point(26, 73);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(73, 19);
            this.label1.TabIndex = 33;
            this.label1.Text = "用户名：";
            // 
            // textBoxUserName
            // 
            this.textBoxUserName.Location = new System.Drawing.Point(105, 73);
            this.textBoxUserName.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
            this.textBoxUserName.Name = "textBoxUserName";
            this.textBoxUserName.Size = new System.Drawing.Size(180, 29);
            this.textBoxUserName.TabIndex = 34;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.BackColor = System.Drawing.Color.Transparent;
            this.label2.Location = new System.Drawing.Point(42, 115);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(57, 19);
            this.label2.TabIndex = 35;
            this.label2.Text = "密码：";
            // 
            // textBoxPassword
            // 
            this.textBoxPassword.Location = new System.Drawing.Point(105, 115);
            this.textBoxPassword.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
            this.textBoxPassword.Name = "textBoxPassword";
            this.textBoxPassword.PasswordChar = '*';
            this.textBoxPassword.Size = new System.Drawing.Size(180, 29);
            this.textBoxPassword.TabIndex = 36;
            // 
            // skinButtonAddUser
            // 
            this.skinButtonAddUser.BackColor = System.Drawing.Color.Transparent;
            this.skinButtonAddUser.BaseColor = System.Drawing.Color.DeepSkyBlue;
            this.skinButtonAddUser.BorderColor = System.Drawing.Color.DeepSkyBlue;
            this.skinButtonAddUser.ControlState = CCWin.SkinClass.ControlState.Normal;
            this.skinButtonAddUser.DownBack = null;
            this.skinButtonAddUser.ForeColor = System.Drawing.Color.Black;
            this.skinButtonAddUser.Location = new System.Drawing.Point(105, 240);
            this.skinButtonAddUser.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.skinButtonAddUser.MouseBack = null;
            this.skinButtonAddUser.Name = "skinButtonAddUser";
            this.skinButtonAddUser.NormlBack = null;
            this.skinButtonAddUser.Radius = 10;
            this.skinButtonAddUser.RoundStyle = CCWin.SkinClass.RoundStyle.All;
            this.skinButtonAddUser.Size = new System.Drawing.Size(146, 51);
            this.skinButtonAddUser.TabIndex = 37;
            this.skinButtonAddUser.Text = "添加测试账号";
            this.skinButtonAddUser.UseVisualStyleBackColor = false;
            this.skinButtonAddUser.Click += new System.EventHandler(this.skinButtonAddUser_Click);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.BackColor = System.Drawing.Color.Transparent;
            this.label3.Location = new System.Drawing.Point(22, 73);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(89, 19);
            this.label3.TabIndex = 35;
            this.label3.Text = "选择用户：";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.BackColor = System.Drawing.Color.Transparent;
            this.label4.Location = new System.Drawing.Point(54, 115);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(57, 19);
            this.label4.TabIndex = 37;
            this.label4.Text = "密码：";
            // 
            // comboBoxAllUsers
            // 
            this.comboBoxAllUsers.FormattingEnabled = true;
            this.comboBoxAllUsers.Location = new System.Drawing.Point(128, 75);
            this.comboBoxAllUsers.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
            this.comboBoxAllUsers.Name = "comboBoxAllUsers";
            this.comboBoxAllUsers.Size = new System.Drawing.Size(170, 27);
            this.comboBoxAllUsers.TabIndex = 38;
            this.comboBoxAllUsers.SelectedIndexChanged += new System.EventHandler(this.comboBoxAllUsers_SelectedIndexChanged);
            // 
            // textBoxModifiedPassword
            // 
            this.textBoxModifiedPassword.Location = new System.Drawing.Point(128, 118);
            this.textBoxModifiedPassword.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
            this.textBoxModifiedPassword.Name = "textBoxModifiedPassword";
            this.textBoxModifiedPassword.Size = new System.Drawing.Size(170, 29);
            this.textBoxModifiedPassword.TabIndex = 39;
            // 
            // skinButtonComfirmModifyPwd
            // 
            this.skinButtonComfirmModifyPwd.BackColor = System.Drawing.Color.Transparent;
            this.skinButtonComfirmModifyPwd.BaseColor = System.Drawing.Color.DeepSkyBlue;
            this.skinButtonComfirmModifyPwd.BorderColor = System.Drawing.Color.DeepSkyBlue;
            this.skinButtonComfirmModifyPwd.ControlState = CCWin.SkinClass.ControlState.Normal;
            this.skinButtonComfirmModifyPwd.DownBack = null;
            this.skinButtonComfirmModifyPwd.ForeColor = System.Drawing.Color.Black;
            this.skinButtonComfirmModifyPwd.Location = new System.Drawing.Point(26, 240);
            this.skinButtonComfirmModifyPwd.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.skinButtonComfirmModifyPwd.MouseBack = null;
            this.skinButtonComfirmModifyPwd.Name = "skinButtonComfirmModifyPwd";
            this.skinButtonComfirmModifyPwd.NormlBack = null;
            this.skinButtonComfirmModifyPwd.Radius = 10;
            this.skinButtonComfirmModifyPwd.RoundStyle = CCWin.SkinClass.RoundStyle.All;
            this.skinButtonComfirmModifyPwd.Size = new System.Drawing.Size(150, 51);
            this.skinButtonComfirmModifyPwd.TabIndex = 40;
            this.skinButtonComfirmModifyPwd.Text = "修改密码";
            this.skinButtonComfirmModifyPwd.UseVisualStyleBackColor = false;
            this.skinButtonComfirmModifyPwd.Click += new System.EventHandler(this.skinButtonComfirmModifyPwd_Click);
            // 
            // skinButtonDeleteUser
            // 
            this.skinButtonDeleteUser.BackColor = System.Drawing.Color.Transparent;
            this.skinButtonDeleteUser.BaseColor = System.Drawing.Color.DeepSkyBlue;
            this.skinButtonDeleteUser.BorderColor = System.Drawing.Color.DeepSkyBlue;
            this.skinButtonDeleteUser.ControlState = CCWin.SkinClass.ControlState.Normal;
            this.skinButtonDeleteUser.DownBack = null;
            this.skinButtonDeleteUser.ForeColor = System.Drawing.Color.Black;
            this.skinButtonDeleteUser.Location = new System.Drawing.Point(191, 240);
            this.skinButtonDeleteUser.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.skinButtonDeleteUser.MouseBack = null;
            this.skinButtonDeleteUser.Name = "skinButtonDeleteUser";
            this.skinButtonDeleteUser.NormlBack = null;
            this.skinButtonDeleteUser.Radius = 10;
            this.skinButtonDeleteUser.RoundStyle = CCWin.SkinClass.RoundStyle.All;
            this.skinButtonDeleteUser.Size = new System.Drawing.Size(150, 51);
            this.skinButtonDeleteUser.TabIndex = 41;
            this.skinButtonDeleteUser.Text = "删除用户";
            this.skinButtonDeleteUser.UseVisualStyleBackColor = false;
            this.skinButtonDeleteUser.Click += new System.EventHandler(this.skinButtonDeleteUser_Click);
            // 
            // AccountSettingForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(10F, 19F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(692, 355);
            this.Controls.Add(this.splitContainer1);
            this.Font = new System.Drawing.Font("华文中宋", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Margin = new System.Windows.Forms.Padding(5, 5, 5, 5);
            this.Name = "AccountSettingForm";
            this.Text = "账号管理";
            this.Load += new System.EventHandler(this.AccountSettingForm_Load);
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).EndInit();
            this.splitContainer1.ResumeLayout(false);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.GroupBox groupBox2;
        private CCWin.SkinControl.SkinButton skinButtonAddUser;
        private System.Windows.Forms.TextBox textBoxPassword;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox textBoxUserName;
        private System.Windows.Forms.Label label1;
        private CCWin.SkinControl.SkinButton skinButtonDeleteUser;
        private CCWin.SkinControl.SkinButton skinButtonComfirmModifyPwd;
        private System.Windows.Forms.TextBox textBoxModifiedPassword;
        private System.Windows.Forms.ComboBox comboBoxAllUsers;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label3;
    }
}