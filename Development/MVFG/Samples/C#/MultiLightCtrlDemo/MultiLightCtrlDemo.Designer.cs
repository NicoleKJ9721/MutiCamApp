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
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.btnTriggerExec = new System.Windows.Forms.Button();
            this.cbSoftTrigger = new System.Windows.Forms.CheckBox();
            this.btnStopGrab = new System.Windows.Forms.Button();
            this.btnStartGrab = new System.Windows.Forms.Button();
            this.bnTriggerMode = new System.Windows.Forms.RadioButton();
            this.bnContinuesMode = new System.Windows.Forms.RadioButton();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.cmbInterfaceList = new System.Windows.Forms.ComboBox();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.labelMultiLightControl = new System.Windows.Forms.Label();
            this.comboBoxMultiLightControl = new System.Windows.Forms.ComboBox();
            this.pictureBox2 = new System.Windows.Forms.PictureBox();
            this.pictureBox3 = new System.Windows.Forms.PictureBox();
            this.pictureBox4 = new System.Windows.Forms.PictureBox();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox4.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox2)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox3)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox4)).BeginInit();
            this.SuspendLayout();
            // 
            // cmbDeviceList
            // 
            this.cmbDeviceList.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbDeviceList.FormattingEnabled = true;
            resources.ApplyResources(this.cmbDeviceList, "cmbDeviceList");
            this.cmbDeviceList.Name = "cmbDeviceList";
            // 
            // pictureBox1
            // 
            this.pictureBox1.BackColor = System.Drawing.SystemColors.ControlDarkDark;
            resources.ApplyResources(this.pictureBox1, "pictureBox1");
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.TabStop = false;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.btnCloseInterface);
            this.groupBox1.Controls.Add(this.btnOpenInterface);
            this.groupBox1.Controls.Add(this.btnEnumInterface);
            resources.ApplyResources(this.groupBox1, "groupBox1");
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.TabStop = false;
            // 
            // btnCloseInterface
            // 
            resources.ApplyResources(this.btnCloseInterface, "btnCloseInterface");
            this.btnCloseInterface.Name = "btnCloseInterface";
            this.btnCloseInterface.UseVisualStyleBackColor = true;
            this.btnCloseInterface.Click += new System.EventHandler(this.btnCloseInterface_Click);
            // 
            // btnOpenInterface
            // 
            resources.ApplyResources(this.btnOpenInterface, "btnOpenInterface");
            this.btnOpenInterface.Name = "btnOpenInterface";
            this.btnOpenInterface.UseVisualStyleBackColor = true;
            this.btnOpenInterface.Click += new System.EventHandler(this.btnOpenInterface_Click);
            // 
            // btnEnumInterface
            // 
            resources.ApplyResources(this.btnEnumInterface, "btnEnumInterface");
            this.btnEnumInterface.Name = "btnEnumInterface";
            this.btnEnumInterface.UseVisualStyleBackColor = true;
            this.btnEnumInterface.Click += new System.EventHandler(this.btnEnumInterface_Click);
            // 
            // btnCloseDevice
            // 
            resources.ApplyResources(this.btnCloseDevice, "btnCloseDevice");
            this.btnCloseDevice.Name = "btnCloseDevice";
            this.btnCloseDevice.UseVisualStyleBackColor = true;
            this.btnCloseDevice.Click += new System.EventHandler(this.btnCloseDevice_Click);
            // 
            // btnOpenDevice
            // 
            resources.ApplyResources(this.btnOpenDevice, "btnOpenDevice");
            this.btnOpenDevice.Name = "btnOpenDevice";
            this.btnOpenDevice.UseVisualStyleBackColor = true;
            this.btnOpenDevice.Click += new System.EventHandler(this.btnOpenDevice_Click);
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
            this.groupBox2.Controls.Add(this.btnTriggerExec);
            this.groupBox2.Controls.Add(this.cbSoftTrigger);
            this.groupBox2.Controls.Add(this.btnStopGrab);
            this.groupBox2.Controls.Add(this.btnStartGrab);
            this.groupBox2.Controls.Add(this.bnTriggerMode);
            this.groupBox2.Controls.Add(this.bnContinuesMode);
            resources.ApplyResources(this.groupBox2, "groupBox2");
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.TabStop = false;
            // 
            // btnTriggerExec
            // 
            resources.ApplyResources(this.btnTriggerExec, "btnTriggerExec");
            this.btnTriggerExec.Name = "btnTriggerExec";
            this.btnTriggerExec.UseVisualStyleBackColor = true;
            this.btnTriggerExec.Click += new System.EventHandler(this.bnTriggerExec_Click);
            // 
            // cbSoftTrigger
            // 
            resources.ApplyResources(this.cbSoftTrigger, "cbSoftTrigger");
            this.cbSoftTrigger.Name = "cbSoftTrigger";
            this.cbSoftTrigger.UseVisualStyleBackColor = true;
            this.cbSoftTrigger.CheckedChanged += new System.EventHandler(this.cbSoftTrigger_CheckedChanged);
            // 
            // btnStopGrab
            // 
            resources.ApplyResources(this.btnStopGrab, "btnStopGrab");
            this.btnStopGrab.Name = "btnStopGrab";
            this.btnStopGrab.UseVisualStyleBackColor = true;
            this.btnStopGrab.Click += new System.EventHandler(this.btnStopGrab_Click);
            // 
            // btnStartGrab
            // 
            resources.ApplyResources(this.btnStartGrab, "btnStartGrab");
            this.btnStartGrab.Name = "btnStartGrab";
            this.btnStartGrab.UseVisualStyleBackColor = true;
            this.btnStartGrab.Click += new System.EventHandler(this.btnStartGrab_Click);
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
            // label4
            // 
            resources.ApplyResources(this.label4, "label4");
            this.label4.Name = "label4";
            // 
            // label5
            // 
            resources.ApplyResources(this.label5, "label5");
            this.label5.Name = "label5";
            // 
            // cmbInterfaceList
            // 
            this.cmbInterfaceList.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbInterfaceList.FormattingEnabled = true;
            resources.ApplyResources(this.cmbInterfaceList, "cmbInterfaceList");
            this.cmbInterfaceList.Name = "cmbInterfaceList";
            // 
            // groupBox4
            // 
            this.groupBox4.Controls.Add(this.labelMultiLightControl);
            this.groupBox4.Controls.Add(this.comboBoxMultiLightControl);
            this.groupBox4.Controls.Add(this.btnEnumDevice);
            this.groupBox4.Controls.Add(this.btnCloseDevice);
            this.groupBox4.Controls.Add(this.btnOpenDevice);
            resources.ApplyResources(this.groupBox4, "groupBox4");
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.TabStop = false;
            // 
            // labelMultiLightControl
            // 
            resources.ApplyResources(this.labelMultiLightControl, "labelMultiLightControl");
            this.labelMultiLightControl.Name = "labelMultiLightControl";
            // 
            // comboBoxMultiLightControl
            // 
            this.comboBoxMultiLightControl.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxMultiLightControl.FormattingEnabled = true;
            resources.ApplyResources(this.comboBoxMultiLightControl, "comboBoxMultiLightControl");
            this.comboBoxMultiLightControl.Name = "comboBoxMultiLightControl";
            this.comboBoxMultiLightControl.SelectionChangeCommitted += new System.EventHandler(this.ParamsChanged);
            // 
            // comboBoxMultiLightControl
            // 
            this.comboBoxMultiLightControl.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxMultiLightControl.FormattingEnabled = true;
            resources.ApplyResources(this.comboBoxMultiLightControl, "comboBoxMultiLightControl");
            this.comboBoxMultiLightControl.Name = "comboBoxMultiLightControl";
            // 
            // pictureBox2
            // 
            this.pictureBox2.BackColor = System.Drawing.SystemColors.ControlDarkDark;
            resources.ApplyResources(this.pictureBox2, "pictureBox2");
            this.pictureBox2.Name = "pictureBox2";
            this.pictureBox2.TabStop = false;
            // 
            // pictureBox3
            // 
            this.pictureBox3.BackColor = System.Drawing.SystemColors.ControlDarkDark;
            resources.ApplyResources(this.pictureBox3, "pictureBox3");
            this.pictureBox3.Name = "pictureBox3";
            this.pictureBox3.TabStop = false;
            // 
            // pictureBox4
            // 
            this.pictureBox4.BackColor = System.Drawing.SystemColors.ControlDarkDark;
            resources.ApplyResources(this.pictureBox4, "pictureBox4");
            this.pictureBox4.Name = "pictureBox4";
            this.pictureBox4.TabStop = false;
            // 
            // Form1
            // 
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.pictureBox4);
            this.Controls.Add(this.pictureBox3);
            this.Controls.Add(this.groupBox4);
            this.Controls.Add(this.cmbInterfaceList);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.pictureBox2);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.cmbDeviceList);
            this.MaximizeBox = false;
            this.Name = "Form1";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.groupBox4.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox2)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox3)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox4)).EndInit();
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
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.RadioButton bnTriggerMode;
        private System.Windows.Forms.RadioButton bnContinuesMode;
        private System.Windows.Forms.CheckBox cbSoftTrigger;
        private System.Windows.Forms.Button btnStopGrab;
        private System.Windows.Forms.Button btnStartGrab;
        private System.Windows.Forms.Button btnTriggerExec;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.ComboBox cmbInterfaceList;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.Button btnCloseInterface;
        private System.Windows.Forms.Button btnOpenInterface;
        private System.Windows.Forms.Button btnEnumInterface;
        private System.Windows.Forms.PictureBox pictureBox2;
        private System.Windows.Forms.PictureBox pictureBox3;
        private System.Windows.Forms.PictureBox pictureBox4;
        private System.Windows.Forms.ComboBox comboBoxMultiLightControl;
        private System.Windows.Forms.Label labelMultiLightControl;
    }
}

