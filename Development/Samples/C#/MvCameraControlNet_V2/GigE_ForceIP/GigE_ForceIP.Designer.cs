namespace GigE_ForceIP
{
    partial class GigE_ForceIP
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(GigE_ForceIP));
            this.deviceListComboBox = new System.Windows.Forms.ComboBox();
            this.initGroupBox = new System.Windows.Forms.GroupBox();
            this.enumButton = new System.Windows.Forms.Button();
            this.forceIPGroupBox = new System.Windows.Forms.GroupBox();
            this.setButton = new System.Windows.Forms.Button();
            this.gatewayTextBox = new System.Windows.Forms.TextBox();
            this.gatewayLabel = new System.Windows.Forms.Label();
            this.subnetTextBox = new System.Windows.Forms.TextBox();
            this.subnetLabel = new System.Windows.Forms.Label();
            this.ipTextBox = new System.Windows.Forms.TextBox();
            this.ipLabel = new System.Windows.Forms.Label();
            this.rangeLabel = new System.Windows.Forms.Label();
            this.recommendLabel = new System.Windows.Forms.Label();
            this.initGroupBox.SuspendLayout();
            this.forceIPGroupBox.SuspendLayout();
            this.SuspendLayout();
            // 
            // deviceListComboBox
            // 
            this.deviceListComboBox.FormattingEnabled = true;
            resources.ApplyResources(this.deviceListComboBox, "deviceListComboBox");
            this.deviceListComboBox.Name = "deviceListComboBox";
            this.deviceListComboBox.SelectedIndexChanged += new System.EventHandler(this.deviceListComboBox_SelectedIndexChanged);
            // 
            // initGroupBox
            // 
            this.initGroupBox.Controls.Add(this.enumButton);
            resources.ApplyResources(this.initGroupBox, "initGroupBox");
            this.initGroupBox.Name = "initGroupBox";
            this.initGroupBox.TabStop = false;
            // 
            // enumButton
            // 
            resources.ApplyResources(this.enumButton, "enumButton");
            this.enumButton.Name = "enumButton";
            this.enumButton.UseVisualStyleBackColor = true;
            this.enumButton.Click += new System.EventHandler(this.enumButton_Click);
            // 
            // forceIPGroupBox
            // 
            this.forceIPGroupBox.Controls.Add(this.setButton);
            this.forceIPGroupBox.Controls.Add(this.gatewayTextBox);
            this.forceIPGroupBox.Controls.Add(this.gatewayLabel);
            this.forceIPGroupBox.Controls.Add(this.subnetTextBox);
            this.forceIPGroupBox.Controls.Add(this.subnetLabel);
            this.forceIPGroupBox.Controls.Add(this.ipTextBox);
            this.forceIPGroupBox.Controls.Add(this.ipLabel);
            this.forceIPGroupBox.Controls.Add(this.rangeLabel);
            this.forceIPGroupBox.Controls.Add(this.recommendLabel);
            resources.ApplyResources(this.forceIPGroupBox, "forceIPGroupBox");
            this.forceIPGroupBox.Name = "forceIPGroupBox";
            this.forceIPGroupBox.TabStop = false;
            // 
            // setButton
            // 
            resources.ApplyResources(this.setButton, "setButton");
            this.setButton.Name = "setButton";
            this.setButton.UseVisualStyleBackColor = true;
            this.setButton.Click += new System.EventHandler(this.setButton_Click);
            // 
            // gatewayTextBox
            // 
            resources.ApplyResources(this.gatewayTextBox, "gatewayTextBox");
            this.gatewayTextBox.Name = "gatewayTextBox";
            // 
            // gatewayLabel
            // 
            resources.ApplyResources(this.gatewayLabel, "gatewayLabel");
            this.gatewayLabel.Name = "gatewayLabel";
            // 
            // subnetTextBox
            // 
            resources.ApplyResources(this.subnetTextBox, "subnetTextBox");
            this.subnetTextBox.Name = "subnetTextBox";
            // 
            // subnetLabel
            // 
            resources.ApplyResources(this.subnetLabel, "subnetLabel");
            this.subnetLabel.Name = "subnetLabel";
            // 
            // ipTextBox
            // 
            resources.ApplyResources(this.ipTextBox, "ipTextBox");
            this.ipTextBox.Name = "ipTextBox";
            // 
            // ipLabel
            // 
            resources.ApplyResources(this.ipLabel, "ipLabel");
            this.ipLabel.Name = "ipLabel";
            // 
            // rangeLabel
            // 
            resources.ApplyResources(this.rangeLabel, "rangeLabel");
            this.rangeLabel.Name = "rangeLabel";
            // 
            // recommendLabel
            // 
            resources.ApplyResources(this.recommendLabel, "recommendLabel");
            this.recommendLabel.Name = "recommendLabel";
            // 
            // GigE_ForceIP
            // 
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.forceIPGroupBox);
            this.Controls.Add(this.initGroupBox);
            this.Controls.Add(this.deviceListComboBox);
            this.Name = "GigE_ForceIP";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.GigE_ForceIP_Closing);
            this.initGroupBox.ResumeLayout(false);
            this.forceIPGroupBox.ResumeLayout(false);
            this.forceIPGroupBox.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ComboBox deviceListComboBox;
        private System.Windows.Forms.GroupBox initGroupBox;
        private System.Windows.Forms.Button enumButton;
        private System.Windows.Forms.GroupBox forceIPGroupBox;
        private System.Windows.Forms.Button setButton;
        private System.Windows.Forms.TextBox gatewayTextBox;
        private System.Windows.Forms.Label gatewayLabel;
        private System.Windows.Forms.TextBox subnetTextBox;
        private System.Windows.Forms.Label subnetLabel;
        private System.Windows.Forms.TextBox ipTextBox;
        private System.Windows.Forms.Label ipLabel;
        private System.Windows.Forms.Label rangeLabel;
        private System.Windows.Forms.Label recommendLabel;
    }
}

