namespace InterfaceBasicDemo
{
    partial class InterfaceBasicDemo
    {
        /// <summary>
        /// 必需的设计器变量。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 清理所有正在使用的资源。
        /// </summary>
        /// <param name="disposing">如果应释放托管资源，为 true；否则为 false。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows 窗体设计器生成的代码

        /// <summary>
        /// 设计器支持所需的方法 - 不要
        /// 使用代码编辑器修改此方法的内容。
        /// </summary>
        private void InitializeComponent()
        {
            this.cbInterfaceType = new System.Windows.Forms.ComboBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.cbInterfaceList = new System.Windows.Forms.ComboBox();
            this.bnOpen = new System.Windows.Forms.Button();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.bnEnum = new System.Windows.Forms.Button();
            this.bnConfig = new System.Windows.Forms.Button();
            this.bnClose = new System.Windows.Forms.Button();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // cbInterfaceType
            // 
            this.cbInterfaceType.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbInterfaceType.FormattingEnabled = true;
            this.cbInterfaceType.Location = new System.Drawing.Point(140, 16);
            this.cbInterfaceType.Name = "cbInterfaceType";
            this.cbInterfaceType.Size = new System.Drawing.Size(489, 20);
            this.cbInterfaceType.TabIndex = 2;
            this.cbInterfaceType.SelectedIndexChanged += new System.EventHandler(this.cbInterfaceType_SelectedIndexChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(22, 19);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(95, 12);
            this.label1.TabIndex = 3;
            this.label1.Text = "选择采集卡类型:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(22, 54);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(77, 12);
            this.label2.TabIndex = 5;
            this.label2.Text = "采集卡列表：";
            // 
            // cbInterfaceList
            // 
            this.cbInterfaceList.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbInterfaceList.FormattingEnabled = true;
            this.cbInterfaceList.Location = new System.Drawing.Point(140, 51);
            this.cbInterfaceList.Name = "cbInterfaceList";
            this.cbInterfaceList.Size = new System.Drawing.Size(489, 20);
            this.cbInterfaceList.TabIndex = 4;
            this.cbInterfaceList.SelectedIndexChanged += new System.EventHandler(this.cbInterfaceList_SelectedIndexChanged);
            // 
            // bnOpen
            // 
            this.bnOpen.Location = new System.Drawing.Point(174, 33);
            this.bnOpen.Name = "bnOpen";
            this.bnOpen.Size = new System.Drawing.Size(75, 23);
            this.bnOpen.TabIndex = 6;
            this.bnOpen.Text = "打开采集卡";
            this.bnOpen.UseVisualStyleBackColor = true;
            this.bnOpen.Click += new System.EventHandler(this.bnOpen_Click);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.bnEnum);
            this.groupBox1.Controls.Add(this.bnConfig);
            this.groupBox1.Controls.Add(this.bnClose);
            this.groupBox1.Controls.Add(this.bnOpen);
            this.groupBox1.Location = new System.Drawing.Point(140, 94);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(489, 77);
            this.groupBox1.TabIndex = 7;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "采集卡操作";
            // 
            // bnEnum
            // 
            this.bnEnum.Location = new System.Drawing.Point(74, 33);
            this.bnEnum.Name = "bnEnum";
            this.bnEnum.Size = new System.Drawing.Size(75, 23);
            this.bnEnum.TabIndex = 9;
            this.bnEnum.Text = "枚举采集卡";
            this.bnEnum.UseVisualStyleBackColor = true;
            this.bnEnum.Click += new System.EventHandler(this.bnEnum_Click);
            // 
            // bnConfig
            // 
            this.bnConfig.Location = new System.Drawing.Point(369, 33);
            this.bnConfig.Name = "bnConfig";
            this.bnConfig.Size = new System.Drawing.Size(75, 23);
            this.bnConfig.TabIndex = 8;
            this.bnConfig.Text = "属性配置";
            this.bnConfig.UseVisualStyleBackColor = true;
            this.bnConfig.Click += new System.EventHandler(this.bnConfig_Click);
            // 
            // bnClose
            // 
            this.bnClose.Location = new System.Drawing.Point(269, 33);
            this.bnClose.Name = "bnClose";
            this.bnClose.Size = new System.Drawing.Size(75, 23);
            this.bnClose.TabIndex = 7;
            this.bnClose.Text = "关闭采集卡";
            this.bnClose.UseVisualStyleBackColor = true;
            this.bnClose.Click += new System.EventHandler(this.bnClose_Click);
            // 
            // InterfaceBasicDemo
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(652, 183);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.cbInterfaceList);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.cbInterfaceType);
            this.Name = "InterfaceBasicDemo";
            this.Text = "Form1";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            this.groupBox1.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox cbInterfaceType;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ComboBox cbInterfaceList;
        private System.Windows.Forms.Button bnOpen;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button bnClose;
        private System.Windows.Forms.Button bnEnum;
        private System.Windows.Forms.Button bnConfig;
    }
}

