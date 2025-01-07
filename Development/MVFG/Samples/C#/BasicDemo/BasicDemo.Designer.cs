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
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.btnSaveJpg = new System.Windows.Forms.Button();
            this.btnSaveBmp = new System.Windows.Forms.Button();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.cmbInterfaceList = new System.Windows.Forms.ComboBox();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox3.SuspendLayout();
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
            // groupBox2
            // 
            this.groupBox2.AccessibleDescription = null;
            this.groupBox2.AccessibleName = null;
            resources.ApplyResources(this.groupBox2, "groupBox2");
            this.groupBox2.BackgroundImage = null;
            this.groupBox2.Controls.Add(this.btnTriggerExec);
            this.groupBox2.Controls.Add(this.cbSoftTrigger);
            this.groupBox2.Controls.Add(this.btnStopGrab);
            this.groupBox2.Controls.Add(this.btnStartGrab);
            this.groupBox2.Controls.Add(this.bnTriggerMode);
            this.groupBox2.Controls.Add(this.bnContinuesMode);
            this.groupBox2.Font = null;
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.TabStop = false;
            // 
            // btnTriggerExec
            // 
            this.btnTriggerExec.AccessibleDescription = null;
            this.btnTriggerExec.AccessibleName = null;
            resources.ApplyResources(this.btnTriggerExec, "btnTriggerExec");
            this.btnTriggerExec.BackgroundImage = null;
            this.btnTriggerExec.Font = null;
            this.btnTriggerExec.Name = "btnTriggerExec";
            this.btnTriggerExec.UseVisualStyleBackColor = true;
            this.btnTriggerExec.Click += new System.EventHandler(this.bnTriggerExec_Click);
            // 
            // cbSoftTrigger
            // 
            this.cbSoftTrigger.AccessibleDescription = null;
            this.cbSoftTrigger.AccessibleName = null;
            resources.ApplyResources(this.cbSoftTrigger, "cbSoftTrigger");
            this.cbSoftTrigger.BackgroundImage = null;
            this.cbSoftTrigger.Font = null;
            this.cbSoftTrigger.Name = "cbSoftTrigger";
            this.cbSoftTrigger.UseVisualStyleBackColor = true;
            this.cbSoftTrigger.CheckedChanged += new System.EventHandler(this.cbSoftTrigger_CheckedChanged);
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
            // bnTriggerMode
            // 
            this.bnTriggerMode.AccessibleDescription = null;
            this.bnTriggerMode.AccessibleName = null;
            resources.ApplyResources(this.bnTriggerMode, "bnTriggerMode");
            this.bnTriggerMode.BackgroundImage = null;
            this.bnTriggerMode.Font = null;
            this.bnTriggerMode.Name = "bnTriggerMode";
            this.bnTriggerMode.TabStop = true;
            this.bnTriggerMode.UseMnemonic = false;
            this.bnTriggerMode.UseVisualStyleBackColor = true;
            this.bnTriggerMode.CheckedChanged += new System.EventHandler(this.bnTriggerMode_CheckedChanged);
            // 
            // bnContinuesMode
            // 
            this.bnContinuesMode.AccessibleDescription = null;
            this.bnContinuesMode.AccessibleName = null;
            resources.ApplyResources(this.bnContinuesMode, "bnContinuesMode");
            this.bnContinuesMode.BackgroundImage = null;
            this.bnContinuesMode.Font = null;
            this.bnContinuesMode.Name = "bnContinuesMode";
            this.bnContinuesMode.TabStop = true;
            this.bnContinuesMode.UseVisualStyleBackColor = true;
            this.bnContinuesMode.CheckedChanged += new System.EventHandler(this.bnContinuesMode_CheckedChanged);
            // 
            // groupBox3
            // 
            this.groupBox3.AccessibleDescription = null;
            this.groupBox3.AccessibleName = null;
            resources.ApplyResources(this.groupBox3, "groupBox3");
            this.groupBox3.BackgroundImage = null;
            this.groupBox3.Controls.Add(this.btnSaveJpg);
            this.groupBox3.Controls.Add(this.btnSaveBmp);
            this.groupBox3.Font = null;
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.TabStop = false;
            // 
            // btnSaveJpg
            // 
            this.btnSaveJpg.AccessibleDescription = null;
            this.btnSaveJpg.AccessibleName = null;
            resources.ApplyResources(this.btnSaveJpg, "btnSaveJpg");
            this.btnSaveJpg.BackgroundImage = null;
            this.btnSaveJpg.Font = null;
            this.btnSaveJpg.Name = "btnSaveJpg";
            this.btnSaveJpg.UseVisualStyleBackColor = true;
            this.btnSaveJpg.Click += new System.EventHandler(this.btnSaveJpg_Click);
            // 
            // btnSaveBmp
            // 
            this.btnSaveBmp.AccessibleDescription = null;
            this.btnSaveBmp.AccessibleName = null;
            resources.ApplyResources(this.btnSaveBmp, "btnSaveBmp");
            this.btnSaveBmp.BackgroundImage = null;
            this.btnSaveBmp.Font = null;
            this.btnSaveBmp.Name = "btnSaveBmp";
            this.btnSaveBmp.UseVisualStyleBackColor = true;
            this.btnSaveBmp.Click += new System.EventHandler(this.btnSaveBmp_Click);
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
            this.Controls.Add(this.groupBox3);
            this.Controls.Add(this.groupBox2);
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
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.groupBox3.ResumeLayout(false);
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
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.RadioButton bnTriggerMode;
        private System.Windows.Forms.RadioButton bnContinuesMode;
        private System.Windows.Forms.CheckBox cbSoftTrigger;
        private System.Windows.Forms.Button btnStopGrab;
        private System.Windows.Forms.Button btnStartGrab;
        private System.Windows.Forms.Button btnTriggerExec;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.Button btnSaveJpg;
        private System.Windows.Forms.Button btnSaveBmp;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.ComboBox cmbInterfaceList;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.Button btnCloseInterface;
        private System.Windows.Forms.Button btnOpenInterface;
        private System.Windows.Forms.Button btnEnumInterface;
    }
}

