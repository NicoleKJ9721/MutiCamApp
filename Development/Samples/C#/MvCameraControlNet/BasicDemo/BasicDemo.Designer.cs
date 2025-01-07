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
            this.cbDeviceList = new System.Windows.Forms.ComboBox();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.bnClose = new System.Windows.Forms.Button();
            this.bnOpen = new System.Windows.Forms.Button();
            this.bnEnum = new System.Windows.Forms.Button();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.bnTriggerExec = new System.Windows.Forms.Button();
            this.cbSoftTrigger = new System.Windows.Forms.CheckBox();
            this.bnStopGrab = new System.Windows.Forms.Button();
            this.bnStartGrab = new System.Windows.Forms.Button();
            this.bnTriggerMode = new System.Windows.Forms.RadioButton();
            this.bnContinuesMode = new System.Windows.Forms.RadioButton();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.bnSavePng = new System.Windows.Forms.Button();
            this.bnSaveTiff = new System.Windows.Forms.Button();
            this.bnSaveJpg = new System.Windows.Forms.Button();
            this.bnSaveBmp = new System.Windows.Forms.Button();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.bnSetParam = new System.Windows.Forms.Button();
            this.bnGetParam = new System.Windows.Forms.Button();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.tbFrameRate = new System.Windows.Forms.TextBox();
            this.tbGain = new System.Windows.Forms.TextBox();
            this.tbExposure = new System.Windows.Forms.TextBox();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.groupBox4.SuspendLayout();
            this.SuspendLayout();
            // 
            // cbDeviceList
            // 
            this.cbDeviceList.AccessibleDescription = null;
            this.cbDeviceList.AccessibleName = null;
            resources.ApplyResources(this.cbDeviceList, "cbDeviceList");
            this.cbDeviceList.BackgroundImage = null;
            this.cbDeviceList.Font = null;
            this.cbDeviceList.FormattingEnabled = true;
            this.cbDeviceList.Name = "cbDeviceList";
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
            this.groupBox1.Controls.Add(this.bnClose);
            this.groupBox1.Controls.Add(this.bnOpen);
            this.groupBox1.Controls.Add(this.bnEnum);
            this.groupBox1.Font = null;
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.TabStop = false;
            // 
            // bnClose
            // 
            this.bnClose.AccessibleDescription = null;
            this.bnClose.AccessibleName = null;
            resources.ApplyResources(this.bnClose, "bnClose");
            this.bnClose.BackgroundImage = null;
            this.bnClose.Font = null;
            this.bnClose.Name = "bnClose";
            this.bnClose.UseVisualStyleBackColor = true;
            this.bnClose.Click += new System.EventHandler(this.bnClose_Click);
            // 
            // bnOpen
            // 
            this.bnOpen.AccessibleDescription = null;
            this.bnOpen.AccessibleName = null;
            resources.ApplyResources(this.bnOpen, "bnOpen");
            this.bnOpen.BackgroundImage = null;
            this.bnOpen.Font = null;
            this.bnOpen.Name = "bnOpen";
            this.bnOpen.UseVisualStyleBackColor = true;
            this.bnOpen.Click += new System.EventHandler(this.bnOpen_Click);
            // 
            // bnEnum
            // 
            this.bnEnum.AccessibleDescription = null;
            this.bnEnum.AccessibleName = null;
            resources.ApplyResources(this.bnEnum, "bnEnum");
            this.bnEnum.BackgroundImage = null;
            this.bnEnum.Font = null;
            this.bnEnum.Name = "bnEnum";
            this.bnEnum.UseVisualStyleBackColor = true;
            this.bnEnum.Click += new System.EventHandler(this.bnEnum_Click);
            // 
            // groupBox2
            // 
            this.groupBox2.AccessibleDescription = null;
            this.groupBox2.AccessibleName = null;
            resources.ApplyResources(this.groupBox2, "groupBox2");
            this.groupBox2.BackgroundImage = null;
            this.groupBox2.Controls.Add(this.bnTriggerExec);
            this.groupBox2.Controls.Add(this.cbSoftTrigger);
            this.groupBox2.Controls.Add(this.bnStopGrab);
            this.groupBox2.Controls.Add(this.bnStartGrab);
            this.groupBox2.Controls.Add(this.bnTriggerMode);
            this.groupBox2.Controls.Add(this.bnContinuesMode);
            this.groupBox2.Font = null;
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.TabStop = false;
            // 
            // bnTriggerExec
            // 
            this.bnTriggerExec.AccessibleDescription = null;
            this.bnTriggerExec.AccessibleName = null;
            resources.ApplyResources(this.bnTriggerExec, "bnTriggerExec");
            this.bnTriggerExec.BackgroundImage = null;
            this.bnTriggerExec.Font = null;
            this.bnTriggerExec.Name = "bnTriggerExec";
            this.bnTriggerExec.UseVisualStyleBackColor = true;
            this.bnTriggerExec.Click += new System.EventHandler(this.bnTriggerExec_Click);
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
            // bnStopGrab
            // 
            this.bnStopGrab.AccessibleDescription = null;
            this.bnStopGrab.AccessibleName = null;
            resources.ApplyResources(this.bnStopGrab, "bnStopGrab");
            this.bnStopGrab.BackgroundImage = null;
            this.bnStopGrab.Font = null;
            this.bnStopGrab.Name = "bnStopGrab";
            this.bnStopGrab.UseVisualStyleBackColor = true;
            this.bnStopGrab.Click += new System.EventHandler(this.bnStopGrab_Click);
            // 
            // bnStartGrab
            // 
            this.bnStartGrab.AccessibleDescription = null;
            this.bnStartGrab.AccessibleName = null;
            resources.ApplyResources(this.bnStartGrab, "bnStartGrab");
            this.bnStartGrab.BackgroundImage = null;
            this.bnStartGrab.Font = null;
            this.bnStartGrab.Name = "bnStartGrab";
            this.bnStartGrab.UseVisualStyleBackColor = true;
            this.bnStartGrab.Click += new System.EventHandler(this.bnStartGrab_Click);
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
            this.groupBox3.Controls.Add(this.bnSavePng);
            this.groupBox3.Controls.Add(this.bnSaveTiff);
            this.groupBox3.Controls.Add(this.bnSaveJpg);
            this.groupBox3.Controls.Add(this.bnSaveBmp);
            this.groupBox3.Font = null;
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.TabStop = false;
            // 
            // bnSavePng
            // 
            this.bnSavePng.AccessibleDescription = null;
            this.bnSavePng.AccessibleName = null;
            resources.ApplyResources(this.bnSavePng, "bnSavePng");
            this.bnSavePng.BackgroundImage = null;
            this.bnSavePng.Font = null;
            this.bnSavePng.Name = "bnSavePng";
            this.bnSavePng.UseVisualStyleBackColor = true;
            this.bnSavePng.Click += new System.EventHandler(this.bnSavePng_Click);
            // 
            // bnSaveTiff
            // 
            this.bnSaveTiff.AccessibleDescription = null;
            this.bnSaveTiff.AccessibleName = null;
            resources.ApplyResources(this.bnSaveTiff, "bnSaveTiff");
            this.bnSaveTiff.BackgroundImage = null;
            this.bnSaveTiff.Font = null;
            this.bnSaveTiff.Name = "bnSaveTiff";
            this.bnSaveTiff.UseVisualStyleBackColor = true;
            this.bnSaveTiff.Click += new System.EventHandler(this.bnSaveTiff_Click);
            // 
            // bnSaveJpg
            // 
            this.bnSaveJpg.AccessibleDescription = null;
            this.bnSaveJpg.AccessibleName = null;
            resources.ApplyResources(this.bnSaveJpg, "bnSaveJpg");
            this.bnSaveJpg.BackgroundImage = null;
            this.bnSaveJpg.Font = null;
            this.bnSaveJpg.Name = "bnSaveJpg";
            this.bnSaveJpg.UseVisualStyleBackColor = true;
            this.bnSaveJpg.Click += new System.EventHandler(this.bnSaveJpg_Click);
            // 
            // bnSaveBmp
            // 
            this.bnSaveBmp.AccessibleDescription = null;
            this.bnSaveBmp.AccessibleName = null;
            resources.ApplyResources(this.bnSaveBmp, "bnSaveBmp");
            this.bnSaveBmp.BackgroundImage = null;
            this.bnSaveBmp.Font = null;
            this.bnSaveBmp.Name = "bnSaveBmp";
            this.bnSaveBmp.UseVisualStyleBackColor = true;
            this.bnSaveBmp.Click += new System.EventHandler(this.bnSaveBmp_Click);
            // 
            // groupBox4
            // 
            this.groupBox4.AccessibleDescription = null;
            this.groupBox4.AccessibleName = null;
            resources.ApplyResources(this.groupBox4, "groupBox4");
            this.groupBox4.BackgroundImage = null;
            this.groupBox4.Controls.Add(this.bnSetParam);
            this.groupBox4.Controls.Add(this.bnGetParam);
            this.groupBox4.Controls.Add(this.label3);
            this.groupBox4.Controls.Add(this.label2);
            this.groupBox4.Controls.Add(this.label1);
            this.groupBox4.Controls.Add(this.tbFrameRate);
            this.groupBox4.Controls.Add(this.tbGain);
            this.groupBox4.Controls.Add(this.tbExposure);
            this.groupBox4.Font = null;
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.TabStop = false;
            // 
            // bnSetParam
            // 
            this.bnSetParam.AccessibleDescription = null;
            this.bnSetParam.AccessibleName = null;
            resources.ApplyResources(this.bnSetParam, "bnSetParam");
            this.bnSetParam.BackgroundImage = null;
            this.bnSetParam.Font = null;
            this.bnSetParam.Name = "bnSetParam";
            this.bnSetParam.UseVisualStyleBackColor = true;
            this.bnSetParam.Click += new System.EventHandler(this.bnSetParam_Click);
            // 
            // bnGetParam
            // 
            this.bnGetParam.AccessibleDescription = null;
            this.bnGetParam.AccessibleName = null;
            resources.ApplyResources(this.bnGetParam, "bnGetParam");
            this.bnGetParam.BackgroundImage = null;
            this.bnGetParam.Font = null;
            this.bnGetParam.Name = "bnGetParam";
            this.bnGetParam.UseVisualStyleBackColor = true;
            this.bnGetParam.Click += new System.EventHandler(this.bnGetParam_Click);
            // 
            // label3
            // 
            this.label3.AccessibleDescription = null;
            this.label3.AccessibleName = null;
            resources.ApplyResources(this.label3, "label3");
            this.label3.Font = null;
            this.label3.Name = "label3";
            // 
            // label2
            // 
            this.label2.AccessibleDescription = null;
            this.label2.AccessibleName = null;
            resources.ApplyResources(this.label2, "label2");
            this.label2.Font = null;
            this.label2.Name = "label2";
            // 
            // label1
            // 
            this.label1.AccessibleDescription = null;
            this.label1.AccessibleName = null;
            resources.ApplyResources(this.label1, "label1");
            this.label1.Font = null;
            this.label1.Name = "label1";
            // 
            // tbFrameRate
            // 
            this.tbFrameRate.AccessibleDescription = null;
            this.tbFrameRate.AccessibleName = null;
            resources.ApplyResources(this.tbFrameRate, "tbFrameRate");
            this.tbFrameRate.BackgroundImage = null;
            this.tbFrameRate.Font = null;
            this.tbFrameRate.Name = "tbFrameRate";
            // 
            // tbGain
            // 
            this.tbGain.AccessibleDescription = null;
            this.tbGain.AccessibleName = null;
            resources.ApplyResources(this.tbGain, "tbGain");
            this.tbGain.BackgroundImage = null;
            this.tbGain.Font = null;
            this.tbGain.Name = "tbGain";
            // 
            // tbExposure
            // 
            this.tbExposure.AccessibleDescription = null;
            this.tbExposure.AccessibleName = null;
            resources.ApplyResources(this.tbExposure, "tbExposure");
            this.tbExposure.BackgroundImage = null;
            this.tbExposure.Font = null;
            this.tbExposure.Name = "tbExposure";
            // 
            // Form1
            // 
            this.AccessibleDescription = null;
            this.AccessibleName = null;
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackgroundImage = null;
            this.Controls.Add(this.groupBox4);
            this.Controls.Add(this.groupBox3);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.cbDeviceList);
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
            this.groupBox4.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ComboBox cbDeviceList;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button bnClose;
        private System.Windows.Forms.Button bnOpen;
        private System.Windows.Forms.Button bnEnum;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.RadioButton bnTriggerMode;
        private System.Windows.Forms.RadioButton bnContinuesMode;
        private System.Windows.Forms.CheckBox cbSoftTrigger;
        private System.Windows.Forms.Button bnStopGrab;
        private System.Windows.Forms.Button bnStartGrab;
        private System.Windows.Forms.Button bnTriggerExec;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.Button bnSaveJpg;
        private System.Windows.Forms.Button bnSaveBmp;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.TextBox tbFrameRate;
        private System.Windows.Forms.TextBox tbGain;
        private System.Windows.Forms.TextBox tbExposure;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button bnSetParam;
        private System.Windows.Forms.Button bnGetParam;
        private System.Windows.Forms.Button bnSavePng;
        private System.Windows.Forms.Button bnSaveTiff;
    }
}

