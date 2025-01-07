namespace BasicDemo
{
    partial class Form1
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.cmbDeviceList = new System.Windows.Forms.ComboBox();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.btnCloseInterface = new System.Windows.Forms.Button();
            this.btnOpenInterface = new System.Windows.Forms.Button();
            this.btnEnumInterface = new System.Windows.Forms.Button();
            this.btnCloseDevice = new System.Windows.Forms.Button();
            this.btnOpenDevice = new System.Windows.Forms.Button();
            this.btnEnumDevice = new System.Windows.Forms.Button();
            this.groupBoxIFParams = new System.Windows.Forms.GroupBox();
            this.comboBoxClConfig = new System.Windows.Forms.ComboBox();
            this.labelClConfiguration = new System.Windows.Forms.Label();
            this.comboBoxTap = new System.Windows.Forms.ComboBox();
            this.labelTap = new System.Windows.Forms.Label();
            this.comboBoxPixelSize = new System.Windows.Forms.ComboBox();
            this.labelPixelSize = new System.Windows.Forms.Label();
            this.comboBoxPixelFormat = new System.Windows.Forms.ComboBox();
            this.labelPixelFormat = new System.Windows.Forms.Label();
            this.textBoxHeight = new System.Windows.Forms.TextBox();
            this.labelHeight = new System.Windows.Forms.Label();
            this.textBoxWidth = new System.Windows.Forms.TextBox();
            this.labelWidth = new System.Windows.Forms.Label();
            this.btnStopGrab = new System.Windows.Forms.Button();
            this.btnStartGrab = new System.Windows.Forms.Button();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.cmbInterfaceList = new System.Windows.Forms.ComboBox();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.groupBoxIFParams.SuspendLayout();
            this.groupBox4.SuspendLayout();
            this.SuspendLayout();
            // 
            // cmbDeviceList
            // 
            this.cmbDeviceList.AccessibleDescription = null;
            this.cmbDeviceList.AccessibleName = null;
            resources.ApplyResources(this.cmbDeviceList, "cmbDeviceList");
            this.cmbDeviceList.BackgroundImage = null;
            this.cmbDeviceList.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbDeviceList.Font = null;
            this.cmbDeviceList.FormattingEnabled = true;
            this.cmbDeviceList.Name = "cmbDeviceList";
            // 
            // pictureBox1
            // 
            this.pictureBox1.AccessibleDescription = null;
            this.pictureBox1.AccessibleName = null;
            resources.ApplyResources(this.pictureBox1, "pictureBox1");
            this.pictureBox1.BackColor = System.Drawing.SystemColors.ControlDarkDark;
            this.pictureBox1.BackgroundImage = null;
            this.pictureBox1.Font = null;
            this.pictureBox1.ImageLocation = null;
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.TabStop = false;
            // 
            // groupBox1
            // 
            this.groupBox1.AccessibleDescription = null;
            this.groupBox1.AccessibleName = null;
            resources.ApplyResources(this.groupBox1, "groupBox1");
            this.groupBox1.BackgroundImage = null;
            this.groupBox1.Controls.Add(this.btnCloseInterface);
            this.groupBox1.Controls.Add(this.btnOpenInterface);
            this.groupBox1.Controls.Add(this.btnEnumInterface);
            this.groupBox1.Font = null;
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.TabStop = false;
            // 
            // btnCloseInterface
            // 
            this.btnCloseInterface.AccessibleDescription = null;
            this.btnCloseInterface.AccessibleName = null;
            resources.ApplyResources(this.btnCloseInterface, "btnCloseInterface");
            this.btnCloseInterface.BackgroundImage = null;
            this.btnCloseInterface.Font = null;
            this.btnCloseInterface.Name = "btnCloseInterface";
            this.btnCloseInterface.UseVisualStyleBackColor = true;
            this.btnCloseInterface.Click += new System.EventHandler(this.btnCloseInterface_Click);
            // 
            // btnOpenInterface
            // 
            this.btnOpenInterface.AccessibleDescription = null;
            this.btnOpenInterface.AccessibleName = null;
            resources.ApplyResources(this.btnOpenInterface, "btnOpenInterface");
            this.btnOpenInterface.BackgroundImage = null;
            this.btnOpenInterface.Font = null;
            this.btnOpenInterface.Name = "btnOpenInterface";
            this.btnOpenInterface.UseVisualStyleBackColor = true;
            this.btnOpenInterface.Click += new System.EventHandler(this.btnOpenInterface_Click);
            // 
            // btnEnumInterface
            // 
            this.btnEnumInterface.AccessibleDescription = null;
            this.btnEnumInterface.AccessibleName = null;
            resources.ApplyResources(this.btnEnumInterface, "btnEnumInterface");
            this.btnEnumInterface.BackgroundImage = null;
            this.btnEnumInterface.Font = null;
            this.btnEnumInterface.Name = "btnEnumInterface";
            this.btnEnumInterface.UseVisualStyleBackColor = true;
            this.btnEnumInterface.Click += new System.EventHandler(this.btnEnumInterface_Click);
            // 
            // btnCloseDevice
            // 
            this.btnCloseDevice.AccessibleDescription = null;
            this.btnCloseDevice.AccessibleName = null;
            resources.ApplyResources(this.btnCloseDevice, "btnCloseDevice");
            this.btnCloseDevice.BackgroundImage = null;
            this.btnCloseDevice.Font = null;
            this.btnCloseDevice.Name = "btnCloseDevice";
            this.btnCloseDevice.UseVisualStyleBackColor = true;
            this.btnCloseDevice.Click += new System.EventHandler(this.btnCloseDevice_Click);
            // 
            // btnOpenDevice
            // 
            this.btnOpenDevice.AccessibleDescription = null;
            this.btnOpenDevice.AccessibleName = null;
            resources.ApplyResources(this.btnOpenDevice, "btnOpenDevice");
            this.btnOpenDevice.BackgroundImage = null;
            this.btnOpenDevice.Font = null;
            this.btnOpenDevice.Name = "btnOpenDevice";
            this.btnOpenDevice.UseVisualStyleBackColor = true;
            this.btnOpenDevice.Click += new System.EventHandler(this.btnOpenDevice_Click);
            // 
            // btnEnumDevice
            // 
            this.btnEnumDevice.AccessibleDescription = null;
            this.btnEnumDevice.AccessibleName = null;
            resources.ApplyResources(this.btnEnumDevice, "btnEnumDevice");
            this.btnEnumDevice.BackgroundImage = null;
            this.btnEnumDevice.Font = null;
            this.btnEnumDevice.Name = "btnEnumDevice";
            this.btnEnumDevice.UseVisualStyleBackColor = true;
            this.btnEnumDevice.Click += new System.EventHandler(this.btnEnumDevice_Click);
            // 
            // groupBoxIFParams
            // 
            this.groupBoxIFParams.AccessibleDescription = null;
            this.groupBoxIFParams.AccessibleName = null;
            resources.ApplyResources(this.groupBoxIFParams, "groupBoxIFParams");
            this.groupBoxIFParams.BackgroundImage = null;
            this.groupBoxIFParams.Controls.Add(this.comboBoxClConfig);
            this.groupBoxIFParams.Controls.Add(this.labelClConfiguration);
            this.groupBoxIFParams.Controls.Add(this.comboBoxTap);
            this.groupBoxIFParams.Controls.Add(this.labelTap);
            this.groupBoxIFParams.Controls.Add(this.comboBoxPixelSize);
            this.groupBoxIFParams.Controls.Add(this.labelPixelSize);
            this.groupBoxIFParams.Controls.Add(this.comboBoxPixelFormat);
            this.groupBoxIFParams.Controls.Add(this.labelPixelFormat);
            this.groupBoxIFParams.Controls.Add(this.textBoxHeight);
            this.groupBoxIFParams.Controls.Add(this.labelHeight);
            this.groupBoxIFParams.Controls.Add(this.textBoxWidth);
            this.groupBoxIFParams.Controls.Add(this.labelWidth);
            this.groupBoxIFParams.Font = null;
            this.groupBoxIFParams.Name = "groupBoxIFParams";
            this.groupBoxIFParams.TabStop = false;
            // 
            // comboBoxClConfig
            // 
            this.comboBoxClConfig.AccessibleDescription = null;
            this.comboBoxClConfig.AccessibleName = null;
            resources.ApplyResources(this.comboBoxClConfig, "comboBoxClConfig");
            this.comboBoxClConfig.BackgroundImage = null;
            this.comboBoxClConfig.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxClConfig.Font = null;
            this.comboBoxClConfig.FormattingEnabled = true;
            this.comboBoxClConfig.Name = "comboBoxClConfig";
            this.comboBoxClConfig.SelectionChangeCommitted += new System.EventHandler(this.comboBoxClConfig_SelectionChangeCommitted);
            // 
            // labelClConfiguration
            // 
            this.labelClConfiguration.AccessibleDescription = null;
            this.labelClConfiguration.AccessibleName = null;
            resources.ApplyResources(this.labelClConfiguration, "labelClConfiguration");
            this.labelClConfiguration.Font = null;
            this.labelClConfiguration.Name = "labelClConfiguration";
            // 
            // comboBoxTap
            // 
            this.comboBoxTap.AccessibleDescription = null;
            this.comboBoxTap.AccessibleName = null;
            resources.ApplyResources(this.comboBoxTap, "comboBoxTap");
            this.comboBoxTap.BackgroundImage = null;
            this.comboBoxTap.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxTap.Font = null;
            this.comboBoxTap.FormattingEnabled = true;
            this.comboBoxTap.Name = "comboBoxTap";
            this.comboBoxTap.SelectionChangeCommitted += new System.EventHandler(this.comboBoxTap_SelectionChangeCommitted);
            // 
            // labelTap
            // 
            this.labelTap.AccessibleDescription = null;
            this.labelTap.AccessibleName = null;
            resources.ApplyResources(this.labelTap, "labelTap");
            this.labelTap.Font = null;
            this.labelTap.Name = "labelTap";
            // 
            // comboBoxPixelSize
            // 
            this.comboBoxPixelSize.AccessibleDescription = null;
            this.comboBoxPixelSize.AccessibleName = null;
            resources.ApplyResources(this.comboBoxPixelSize, "comboBoxPixelSize");
            this.comboBoxPixelSize.BackgroundImage = null;
            this.comboBoxPixelSize.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxPixelSize.Font = null;
            this.comboBoxPixelSize.FormattingEnabled = true;
            this.comboBoxPixelSize.Name = "comboBoxPixelSize";
            this.comboBoxPixelSize.SelectionChangeCommitted += new System.EventHandler(this.comboBoxPixelSize_SelectionChangeCommitted);
            // 
            // labelPixelSize
            // 
            this.labelPixelSize.AccessibleDescription = null;
            this.labelPixelSize.AccessibleName = null;
            resources.ApplyResources(this.labelPixelSize, "labelPixelSize");
            this.labelPixelSize.Font = null;
            this.labelPixelSize.Name = "labelPixelSize";
            // 
            // comboBoxPixelFormat
            // 
            this.comboBoxPixelFormat.AccessibleDescription = null;
            this.comboBoxPixelFormat.AccessibleName = null;
            resources.ApplyResources(this.comboBoxPixelFormat, "comboBoxPixelFormat");
            this.comboBoxPixelFormat.BackgroundImage = null;
            this.comboBoxPixelFormat.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxPixelFormat.Font = null;
            this.comboBoxPixelFormat.FormattingEnabled = true;
            this.comboBoxPixelFormat.Name = "comboBoxPixelFormat";
            this.comboBoxPixelFormat.SelectionChangeCommitted += new System.EventHandler(this.comboBoxPixelFormat_SelectionChangeCommitted);
            // 
            // labelPixelFormat
            // 
            this.labelPixelFormat.AccessibleDescription = null;
            this.labelPixelFormat.AccessibleName = null;
            resources.ApplyResources(this.labelPixelFormat, "labelPixelFormat");
            this.labelPixelFormat.Font = null;
            this.labelPixelFormat.Name = "labelPixelFormat";
            // 
            // textBoxHeight
            // 
            this.textBoxHeight.AccessibleDescription = null;
            this.textBoxHeight.AccessibleName = null;
            resources.ApplyResources(this.textBoxHeight, "textBoxHeight");
            this.textBoxHeight.BackgroundImage = null;
            this.textBoxHeight.Font = null;
            this.textBoxHeight.Name = "textBoxHeight";
            this.textBoxHeight.Validated += new System.EventHandler(this.textBoxHeight_Validated);
            // 
            // labelHeight
            // 
            this.labelHeight.AccessibleDescription = null;
            this.labelHeight.AccessibleName = null;
            resources.ApplyResources(this.labelHeight, "labelHeight");
            this.labelHeight.Font = null;
            this.labelHeight.Name = "labelHeight";
            // 
            // textBoxWidth
            // 
            this.textBoxWidth.AccessibleDescription = null;
            this.textBoxWidth.AccessibleName = null;
            resources.ApplyResources(this.textBoxWidth, "textBoxWidth");
            this.textBoxWidth.BackgroundImage = null;
            this.textBoxWidth.Font = null;
            this.textBoxWidth.Name = "textBoxWidth";
            this.textBoxWidth.Validated += new System.EventHandler(this.textBoxWidth_Validated);
            // 
            // labelWidth
            // 
            this.labelWidth.AccessibleDescription = null;
            this.labelWidth.AccessibleName = null;
            resources.ApplyResources(this.labelWidth, "labelWidth");
            this.labelWidth.Font = null;
            this.labelWidth.Name = "labelWidth";
            // 
            // btnStopGrab
            // 
            this.btnStopGrab.AccessibleDescription = null;
            this.btnStopGrab.AccessibleName = null;
            resources.ApplyResources(this.btnStopGrab, "btnStopGrab");
            this.btnStopGrab.BackgroundImage = null;
            this.btnStopGrab.Font = null;
            this.btnStopGrab.Name = "btnStopGrab";
            this.btnStopGrab.UseVisualStyleBackColor = true;
            this.btnStopGrab.Click += new System.EventHandler(this.btnStopGrab_Click);
            // 
            // btnStartGrab
            // 
            this.btnStartGrab.AccessibleDescription = null;
            this.btnStartGrab.AccessibleName = null;
            resources.ApplyResources(this.btnStartGrab, "btnStartGrab");
            this.btnStartGrab.BackgroundImage = null;
            this.btnStartGrab.Font = null;
            this.btnStartGrab.Name = "btnStartGrab";
            this.btnStartGrab.UseVisualStyleBackColor = true;
            this.btnStartGrab.Click += new System.EventHandler(this.btnStartGrab_Click);
            // 
            // label4
            // 
            this.label4.AccessibleDescription = null;
            this.label4.AccessibleName = null;
            resources.ApplyResources(this.label4, "label4");
            this.label4.Font = null;
            this.label4.Name = "label4";
            // 
            // label5
            // 
            this.label5.AccessibleDescription = null;
            this.label5.AccessibleName = null;
            resources.ApplyResources(this.label5, "label5");
            this.label5.Font = null;
            this.label5.Name = "label5";
            // 
            // cmbInterfaceList
            // 
            this.cmbInterfaceList.AccessibleDescription = null;
            this.cmbInterfaceList.AccessibleName = null;
            resources.ApplyResources(this.cmbInterfaceList, "cmbInterfaceList");
            this.cmbInterfaceList.BackgroundImage = null;
            this.cmbInterfaceList.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbInterfaceList.Font = null;
            this.cmbInterfaceList.FormattingEnabled = true;
            this.cmbInterfaceList.Name = "cmbInterfaceList";
            // 
            // groupBox4
            // 
            this.groupBox4.AccessibleDescription = null;
            this.groupBox4.AccessibleName = null;
            resources.ApplyResources(this.groupBox4, "groupBox4");
            this.groupBox4.BackgroundImage = null;
            this.groupBox4.Controls.Add(this.btnEnumDevice);
            this.groupBox4.Controls.Add(this.btnCloseDevice);
            this.groupBox4.Controls.Add(this.btnOpenDevice);
            this.groupBox4.Controls.Add(this.btnStartGrab);
            this.groupBox4.Controls.Add(this.btnStopGrab);
            this.groupBox4.Font = null;
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.TabStop = false;
            // 
            // Form1
            // 
            this.AccessibleDescription = null;
            this.AccessibleName = null;
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackgroundImage = null;
            this.Controls.Add(this.groupBox4);
            this.Controls.Add(this.cmbInterfaceList);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.groupBoxIFParams);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.cmbDeviceList);
            this.Font = null;
            this.Icon = null;
            this.MaximizeBox = false;
            this.Name = "Form1";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBoxIFParams.ResumeLayout(false);
            this.groupBoxIFParams.PerformLayout();
            this.groupBox4.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox cmbDeviceList;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button btnCloseDevice;
        private System.Windows.Forms.Button btnOpenDevice;
        private System.Windows.Forms.Button btnEnumDevice;
        private System.Windows.Forms.GroupBox groupBoxIFParams;
        private System.Windows.Forms.Button btnStopGrab;
        private System.Windows.Forms.Button btnStartGrab;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.ComboBox cmbInterfaceList;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.Button btnCloseInterface;
        private System.Windows.Forms.Button btnOpenInterface;
        private System.Windows.Forms.Button btnEnumInterface;
        private System.Windows.Forms.ComboBox comboBoxClConfig;
        private System.Windows.Forms.Label labelClConfiguration;
        private System.Windows.Forms.ComboBox comboBoxTap;
        private System.Windows.Forms.Label labelTap;
        private System.Windows.Forms.ComboBox comboBoxPixelSize;
        private System.Windows.Forms.Label labelPixelSize;
        private System.Windows.Forms.ComboBox comboBoxPixelFormat;
        private System.Windows.Forms.Label labelPixelFormat;
        private System.Windows.Forms.TextBox textBoxHeight;
        private System.Windows.Forms.Label labelHeight;
        private System.Windows.Forms.TextBox textBoxWidth;
        private System.Windows.Forms.Label labelWidth;
    }
}

