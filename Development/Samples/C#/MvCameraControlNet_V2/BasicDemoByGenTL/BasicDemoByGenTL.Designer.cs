namespace BasicDemoByGenTL
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
            this.bnClose = new System.Windows.Forms.Button();
            this.bnOpen = new System.Windows.Forms.Button();
            this.btnEnumDevice = new System.Windows.Forms.Button();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.bnTriggerExec = new System.Windows.Forms.Button();
            this.cbSoftTrigger = new System.Windows.Forms.CheckBox();
            this.bnStopGrab = new System.Windows.Forms.Button();
            this.bnStartGrab = new System.Windows.Forms.Button();
            this.bnTriggerMode = new System.Windows.Forms.RadioButton();
            this.bnContinuesMode = new System.Windows.Forms.RadioButton();
            this.btnEnumInterface = new System.Windows.Forms.Button();
            this.lblInterface = new System.Windows.Forms.Label();
            this.lblDevice = new System.Windows.Forms.Label();
            this.cmbInterfaceList = new System.Windows.Forms.ComboBox();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.SuspendLayout();
            // 
            // cmbDeviceList
            // 
            resources.ApplyResources(this.cmbDeviceList, "cmbDeviceList");
            this.cmbDeviceList.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbDeviceList.FormattingEnabled = true;
            this.cmbDeviceList.Name = "cmbDeviceList";
            // 
            // pictureBox1
            // 
            resources.ApplyResources(this.pictureBox1, "pictureBox1");
            this.pictureBox1.BackColor = System.Drawing.SystemColors.ControlDarkDark;
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.TabStop = false;
            // 
            // groupBox1
            // 
            resources.ApplyResources(this.groupBox1, "groupBox1");
            this.groupBox1.Controls.Add(this.bnClose);
            this.groupBox1.Controls.Add(this.bnOpen);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.TabStop = false;
            // 
            // bnClose
            // 
            resources.ApplyResources(this.bnClose, "bnClose");
            this.bnClose.Name = "bnClose";
            this.bnClose.UseVisualStyleBackColor = true;
            this.bnClose.Click += new System.EventHandler(this.bnClose_Click);
            // 
            // bnOpen
            // 
            resources.ApplyResources(this.bnOpen, "bnOpen");
            this.bnOpen.Name = "bnOpen";
            this.bnOpen.UseVisualStyleBackColor = true;
            this.bnOpen.Click += new System.EventHandler(this.bnOpen_Click);
            // 
            // btnEnumDevice
            // 
            resources.ApplyResources(this.btnEnumDevice, "btnEnumDevice");
            this.btnEnumDevice.Name = "btnEnumDevice";
            this.btnEnumDevice.UseVisualStyleBackColor = true;
            this.btnEnumDevice.Click += new System.EventHandler(this.btnEnumDevice_Click);
            // 
            // groupBox2
            // 
            resources.ApplyResources(this.groupBox2, "groupBox2");
            this.groupBox2.Controls.Add(this.bnTriggerExec);
            this.groupBox2.Controls.Add(this.cbSoftTrigger);
            this.groupBox2.Controls.Add(this.bnStopGrab);
            this.groupBox2.Controls.Add(this.bnStartGrab);
            this.groupBox2.Controls.Add(this.bnTriggerMode);
            this.groupBox2.Controls.Add(this.bnContinuesMode);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.TabStop = false;
            // 
            // bnTriggerExec
            // 
            resources.ApplyResources(this.bnTriggerExec, "bnTriggerExec");
            this.bnTriggerExec.Name = "bnTriggerExec";
            this.bnTriggerExec.UseVisualStyleBackColor = true;
            this.bnTriggerExec.Click += new System.EventHandler(this.bnTriggerExec_Click);
            // 
            // cbSoftTrigger
            // 
            resources.ApplyResources(this.cbSoftTrigger, "cbSoftTrigger");
            this.cbSoftTrigger.Name = "cbSoftTrigger";
            this.cbSoftTrigger.UseVisualStyleBackColor = true;
            this.cbSoftTrigger.CheckedChanged += new System.EventHandler(this.cbSoftTrigger_CheckedChanged);
            // 
            // bnStopGrab
            // 
            resources.ApplyResources(this.bnStopGrab, "bnStopGrab");
            this.bnStopGrab.Name = "bnStopGrab";
            this.bnStopGrab.UseVisualStyleBackColor = true;
            this.bnStopGrab.Click += new System.EventHandler(this.bnStopGrab_Click);
            // 
            // bnStartGrab
            // 
            resources.ApplyResources(this.bnStartGrab, "bnStartGrab");
            this.bnStartGrab.Name = "bnStartGrab";
            this.bnStartGrab.UseVisualStyleBackColor = true;
            this.bnStartGrab.Click += new System.EventHandler(this.bnStartGrab_Click);
            // 
            // bnTriggerMode
            // 
            resources.ApplyResources(this.bnTriggerMode, "bnTriggerMode");
            this.bnTriggerMode.Name = "bnTriggerMode";
            this.bnTriggerMode.TabStop = true;
            this.bnTriggerMode.UseMnemonic = false;
            this.bnTriggerMode.UseVisualStyleBackColor = true;
            this.bnTriggerMode.CheckedChanged += new System.EventHandler(this.bnTriggerMode_CheckedChanged);
            // 
            // bnContinuesMode
            // 
            resources.ApplyResources(this.bnContinuesMode, "bnContinuesMode");
            this.bnContinuesMode.Name = "bnContinuesMode";
            this.bnContinuesMode.TabStop = true;
            this.bnContinuesMode.UseVisualStyleBackColor = true;
            this.bnContinuesMode.CheckedChanged += new System.EventHandler(this.bnContinuesMode_CheckedChanged);
            // 
            // btnEnumInterface
            // 
            resources.ApplyResources(this.btnEnumInterface, "btnEnumInterface");
            this.btnEnumInterface.Name = "btnEnumInterface";
            this.btnEnumInterface.UseVisualStyleBackColor = true;
            this.btnEnumInterface.Click += new System.EventHandler(this.btnEnumInterface_Click);
            // 
            // lblInterface
            // 
            resources.ApplyResources(this.lblInterface, "lblInterface");
            this.lblInterface.Name = "lblInterface";
            // 
            // lblDevice
            // 
            resources.ApplyResources(this.lblDevice, "lblDevice");
            this.lblDevice.Name = "lblDevice";
            // 
            // cmbInterfaceList
            // 
            resources.ApplyResources(this.cmbInterfaceList, "cmbInterfaceList");
            this.cmbInterfaceList.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbInterfaceList.FormattingEnabled = true;
            this.cmbInterfaceList.Name = "cmbInterfaceList";
            // 
            // groupBox3
            // 
            resources.ApplyResources(this.groupBox3, "groupBox3");
            this.groupBox3.Controls.Add(this.btnEnumDevice);
            this.groupBox3.Controls.Add(this.btnEnumInterface);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.TabStop = false;
            // 
            // Form1
            // 
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.groupBox3);
            this.Controls.Add(this.lblDevice);
            this.Controls.Add(this.lblInterface);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.cmbInterfaceList);
            this.Controls.Add(this.cmbDeviceList);
            this.MaximizeBox = false;
            this.Name = "Form1";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox cmbDeviceList;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button bnClose;
        private System.Windows.Forms.Button bnOpen;
        private System.Windows.Forms.Button btnEnumDevice;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.RadioButton bnTriggerMode;
        private System.Windows.Forms.RadioButton bnContinuesMode;
        private System.Windows.Forms.CheckBox cbSoftTrigger;
        private System.Windows.Forms.Button bnStopGrab;
        private System.Windows.Forms.Button bnStartGrab;
        private System.Windows.Forms.Button bnTriggerExec;
        private System.Windows.Forms.Button btnEnumInterface;
        private System.Windows.Forms.Label lblInterface;
        private System.Windows.Forms.Label lblDevice;
        private System.Windows.Forms.ComboBox cmbInterfaceList;
        private System.Windows.Forms.GroupBox groupBox3;
    }
}

