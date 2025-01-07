namespace BasicDemoLineScan
{
    partial class Form1
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
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.cmbDeviceList = new System.Windows.Forms.ComboBox();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.btnClose = new System.Windows.Forms.Button();
            this.btnOpen = new System.Windows.Forms.Button();
            this.btnEnum = new System.Windows.Forms.Button();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.cmbTriggerSource = new System.Windows.Forms.ComboBox();
            this.btnTriggerExec = new System.Windows.Forms.Button();
            this.labTriggerSource = new System.Windows.Forms.Label();
            this.cmbTriggerMode = new System.Windows.Forms.ComboBox();
            this.labTriggerSwith = new System.Windows.Forms.Label();
            this.cmbTriggerSelector = new System.Windows.Forms.ComboBox();
            this.labTriggerOpt = new System.Windows.Forms.Label();
            this.btnStopGrab = new System.Windows.Forms.Button();
            this.btnStartGrab = new System.Windows.Forms.Button();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.btnSavePng = new System.Windows.Forms.Button();
            this.btnSaveTiff = new System.Windows.Forms.Button();
            this.btnSaveJpg = new System.Windows.Forms.Button();
            this.btnSaveBmp = new System.Windows.Forms.Button();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.chkLineRateEnable = new System.Windows.Forms.CheckBox();
            this.cmbPreampGain = new System.Windows.Forms.ComboBox();
            this.tbResLineRate = new System.Windows.Forms.TextBox();
            this.label10 = new System.Windows.Forms.Label();
            this.btnGetParam = new System.Windows.Forms.Button();
            this.btnSetParam = new System.Windows.Forms.Button();
            this.tbAcqLineRate = new System.Windows.Forms.TextBox();
            this.label9 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.tbDigitalShift = new System.Windows.Forms.TextBox();
            this.label7 = new System.Windows.Forms.Label();
            this.tbExposure = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.cmbHBMode = new System.Windows.Forms.ComboBox();
            this.label5 = new System.Windows.Forms.Label();
            this.cmbPixelFormat = new System.Windows.Forms.ComboBox();
            this.label4 = new System.Windows.Forms.Label();
            this.mVCCENUMVALUEBindingSource = new System.Windows.Forms.BindingSource(this.components);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.groupBox4.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.mVCCENUMVALUEBindingSource)).BeginInit();
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
            this.groupBox1.Controls.Add(this.btnClose);
            this.groupBox1.Controls.Add(this.btnOpen);
            this.groupBox1.Controls.Add(this.btnEnum);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.TabStop = false;
            // 
            // btnClose
            // 
            resources.ApplyResources(this.btnClose, "btnClose");
            this.btnClose.Name = "btnClose";
            this.btnClose.UseVisualStyleBackColor = true;
            this.btnClose.Click += new System.EventHandler(this.bnClose_Click);
            // 
            // btnOpen
            // 
            resources.ApplyResources(this.btnOpen, "btnOpen");
            this.btnOpen.Name = "btnOpen";
            this.btnOpen.UseVisualStyleBackColor = true;
            this.btnOpen.Click += new System.EventHandler(this.bnOpen_Click);
            // 
            // btnEnum
            // 
            resources.ApplyResources(this.btnEnum, "btnEnum");
            this.btnEnum.Name = "btnEnum";
            this.btnEnum.UseVisualStyleBackColor = true;
            this.btnEnum.Click += new System.EventHandler(this.bnEnum_Click);
            // 
            // groupBox2
            // 
            resources.ApplyResources(this.groupBox2, "groupBox2");
            this.groupBox2.Controls.Add(this.cmbTriggerSource);
            this.groupBox2.Controls.Add(this.btnTriggerExec);
            this.groupBox2.Controls.Add(this.labTriggerSource);
            this.groupBox2.Controls.Add(this.cmbTriggerMode);
            this.groupBox2.Controls.Add(this.labTriggerSwith);
            this.groupBox2.Controls.Add(this.cmbTriggerSelector);
            this.groupBox2.Controls.Add(this.labTriggerOpt);
            this.groupBox2.Controls.Add(this.btnStopGrab);
            this.groupBox2.Controls.Add(this.btnStartGrab);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.TabStop = false;
            // 
            // cmbTriggerSource
            // 
            resources.ApplyResources(this.cmbTriggerSource, "cmbTriggerSource");
            this.cmbTriggerSource.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbTriggerSource.FormattingEnabled = true;
            this.cmbTriggerSource.Name = "cmbTriggerSource";
            this.cmbTriggerSource.SelectedIndexChanged += new System.EventHandler(this.cbTriggerSource_SelectedIndexChanged);
            // 
            // btnTriggerExec
            // 
            resources.ApplyResources(this.btnTriggerExec, "btnTriggerExec");
            this.btnTriggerExec.Name = "btnTriggerExec";
            this.btnTriggerExec.UseVisualStyleBackColor = true;
            this.btnTriggerExec.Click += new System.EventHandler(this.bnTriggerExec_Click);
            // 
            // labTriggerSource
            // 
            resources.ApplyResources(this.labTriggerSource, "labTriggerSource");
            this.labTriggerSource.Name = "labTriggerSource";
            // 
            // cmbTriggerMode
            // 
            resources.ApplyResources(this.cmbTriggerMode, "cmbTriggerMode");
            this.cmbTriggerMode.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbTriggerMode.FormattingEnabled = true;
            this.cmbTriggerMode.Name = "cmbTriggerMode";
            this.cmbTriggerMode.SelectedIndexChanged += new System.EventHandler(this.cbTiggerMode_SelectedIndexChanged);
            // 
            // labTriggerSwith
            // 
            resources.ApplyResources(this.labTriggerSwith, "labTriggerSwith");
            this.labTriggerSwith.Name = "labTriggerSwith";
            // 
            // cmbTriggerSelector
            // 
            resources.ApplyResources(this.cmbTriggerSelector, "cmbTriggerSelector");
            this.cmbTriggerSelector.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbTriggerSelector.ForeColor = System.Drawing.SystemColors.WindowText;
            this.cmbTriggerSelector.FormattingEnabled = true;
            this.cmbTriggerSelector.Name = "cmbTriggerSelector";
            this.cmbTriggerSelector.SelectedIndexChanged += new System.EventHandler(this.cbTriggerSelector_SelectedIndexChanged);
            // 
            // labTriggerOpt
            // 
            resources.ApplyResources(this.labTriggerOpt, "labTriggerOpt");
            this.labTriggerOpt.Name = "labTriggerOpt";
            // 
            // btnStopGrab
            // 
            resources.ApplyResources(this.btnStopGrab, "btnStopGrab");
            this.btnStopGrab.Name = "btnStopGrab";
            this.btnStopGrab.UseVisualStyleBackColor = true;
            this.btnStopGrab.Click += new System.EventHandler(this.bnStopGrab_Click);
            // 
            // btnStartGrab
            // 
            resources.ApplyResources(this.btnStartGrab, "btnStartGrab");
            this.btnStartGrab.Name = "btnStartGrab";
            this.btnStartGrab.UseVisualStyleBackColor = true;
            this.btnStartGrab.Click += new System.EventHandler(this.bnStartGrab_Click);
            // 
            // groupBox3
            // 
            resources.ApplyResources(this.groupBox3, "groupBox3");
            this.groupBox3.Controls.Add(this.btnSavePng);
            this.groupBox3.Controls.Add(this.btnSaveTiff);
            this.groupBox3.Controls.Add(this.btnSaveJpg);
            this.groupBox3.Controls.Add(this.btnSaveBmp);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.TabStop = false;
            // 
            // btnSavePng
            // 
            resources.ApplyResources(this.btnSavePng, "btnSavePng");
            this.btnSavePng.Name = "btnSavePng";
            this.btnSavePng.UseVisualStyleBackColor = true;
            this.btnSavePng.Click += new System.EventHandler(this.bnSavePng_Click);
            // 
            // btnSaveTiff
            // 
            resources.ApplyResources(this.btnSaveTiff, "btnSaveTiff");
            this.btnSaveTiff.Name = "btnSaveTiff";
            this.btnSaveTiff.UseVisualStyleBackColor = true;
            this.btnSaveTiff.Click += new System.EventHandler(this.bnSaveTiff_Click);
            // 
            // btnSaveJpg
            // 
            resources.ApplyResources(this.btnSaveJpg, "btnSaveJpg");
            this.btnSaveJpg.Name = "btnSaveJpg";
            this.btnSaveJpg.UseVisualStyleBackColor = true;
            this.btnSaveJpg.Click += new System.EventHandler(this.bnSaveJpg_Click);
            // 
            // btnSaveBmp
            // 
            resources.ApplyResources(this.btnSaveBmp, "btnSaveBmp");
            this.btnSaveBmp.Name = "btnSaveBmp";
            this.btnSaveBmp.UseVisualStyleBackColor = true;
            this.btnSaveBmp.Click += new System.EventHandler(this.bnSaveBmp_Click);
            // 
            // groupBox4
            // 
            resources.ApplyResources(this.groupBox4, "groupBox4");
            this.groupBox4.Controls.Add(this.chkLineRateEnable);
            this.groupBox4.Controls.Add(this.cmbPreampGain);
            this.groupBox4.Controls.Add(this.tbResLineRate);
            this.groupBox4.Controls.Add(this.label10);
            this.groupBox4.Controls.Add(this.btnGetParam);
            this.groupBox4.Controls.Add(this.btnSetParam);
            this.groupBox4.Controls.Add(this.tbAcqLineRate);
            this.groupBox4.Controls.Add(this.label9);
            this.groupBox4.Controls.Add(this.label8);
            this.groupBox4.Controls.Add(this.tbDigitalShift);
            this.groupBox4.Controls.Add(this.label7);
            this.groupBox4.Controls.Add(this.tbExposure);
            this.groupBox4.Controls.Add(this.label6);
            this.groupBox4.Controls.Add(this.cmbHBMode);
            this.groupBox4.Controls.Add(this.label5);
            this.groupBox4.Controls.Add(this.cmbPixelFormat);
            this.groupBox4.Controls.Add(this.label4);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.TabStop = false;
            // 
            // chkLineRateEnable
            // 
            resources.ApplyResources(this.chkLineRateEnable, "chkLineRateEnable");
            this.chkLineRateEnable.Name = "chkLineRateEnable";
            this.chkLineRateEnable.UseVisualStyleBackColor = true;
            this.chkLineRateEnable.CheckedChanged += new System.EventHandler(this.chkLineRateEnable_CheckedChanged);
            // 
            // cmbPreampGain
            // 
            resources.ApplyResources(this.cmbPreampGain, "cmbPreampGain");
            this.cmbPreampGain.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbPreampGain.FormattingEnabled = true;
            this.cmbPreampGain.Name = "cmbPreampGain";
            this.cmbPreampGain.SelectedIndexChanged += new System.EventHandler(this.cbPreampGain_SelectedIndexChanged);
            // 
            // tbResLineRate
            // 
            resources.ApplyResources(this.tbResLineRate, "tbResLineRate");
            this.tbResLineRate.Name = "tbResLineRate";
            this.tbResLineRate.ReadOnly = true;
            // 
            // label10
            // 
            resources.ApplyResources(this.label10, "label10");
            this.label10.Name = "label10";
            // 
            // btnGetParam
            // 
            resources.ApplyResources(this.btnGetParam, "btnGetParam");
            this.btnGetParam.Name = "btnGetParam";
            this.btnGetParam.UseVisualStyleBackColor = true;
            this.btnGetParam.Click += new System.EventHandler(this.bnGetParam_Click);
            // 
            // btnSetParam
            // 
            resources.ApplyResources(this.btnSetParam, "btnSetParam");
            this.btnSetParam.Name = "btnSetParam";
            this.btnSetParam.UseVisualStyleBackColor = true;
            this.btnSetParam.Click += new System.EventHandler(this.bnSetParam_Click);
            // 
            // tbAcqLineRate
            // 
            resources.ApplyResources(this.tbAcqLineRate, "tbAcqLineRate");
            this.tbAcqLineRate.Name = "tbAcqLineRate";
            // 
            // label9
            // 
            resources.ApplyResources(this.label9, "label9");
            this.label9.Name = "label9";
            // 
            // label8
            // 
            resources.ApplyResources(this.label8, "label8");
            this.label8.Name = "label8";
            // 
            // tbDigitalShift
            // 
            resources.ApplyResources(this.tbDigitalShift, "tbDigitalShift");
            this.tbDigitalShift.Name = "tbDigitalShift";
            // 
            // label7
            // 
            resources.ApplyResources(this.label7, "label7");
            this.label7.Name = "label7";
            // 
            // tbExposure
            // 
            resources.ApplyResources(this.tbExposure, "tbExposure");
            this.tbExposure.Name = "tbExposure";
            // 
            // label6
            // 
            resources.ApplyResources(this.label6, "label6");
            this.label6.Name = "label6";
            // 
            // cmbHBMode
            // 
            resources.ApplyResources(this.cmbHBMode, "cmbHBMode");
            this.cmbHBMode.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbHBMode.FormattingEnabled = true;
            this.cmbHBMode.Name = "cmbHBMode";
            this.cmbHBMode.SelectedIndexChanged += new System.EventHandler(this.cbHBMode_SelectedIndexChanged);
            // 
            // label5
            // 
            resources.ApplyResources(this.label5, "label5");
            this.label5.Name = "label5";
            // 
            // cmbPixelFormat
            // 
            resources.ApplyResources(this.cmbPixelFormat, "cmbPixelFormat");
            this.cmbPixelFormat.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbPixelFormat.FormattingEnabled = true;
            this.cmbPixelFormat.Name = "cmbPixelFormat";
            this.cmbPixelFormat.SelectedIndexChanged += new System.EventHandler(this.cbPixelFormat_SelectedIndexChanged);
            // 
            // label4
            // 
            resources.ApplyResources(this.label4, "label4");
            this.label4.Name = "label4";
            // 
            // mVCCENUMVALUEBindingSource
            // 
            this.mVCCENUMVALUEBindingSource.DataSource = typeof(MvCameraControl.IEnumValue);
            // 
            // Form1
            // 
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.groupBox4);
            this.Controls.Add(this.groupBox3);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.cmbDeviceList);
            this.Name = "Form1";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            this.groupBox4.ResumeLayout(false);
            this.groupBox4.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.mVCCENUMVALUEBindingSource)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ComboBox cmbDeviceList;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button btnClose;
        private System.Windows.Forms.Button btnOpen;
        private System.Windows.Forms.Button btnEnum;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Button btnTriggerExec;
        private System.Windows.Forms.Label labTriggerSource;
        private System.Windows.Forms.ComboBox cmbTriggerMode;
        private System.Windows.Forms.Label labTriggerSwith;
        private System.Windows.Forms.ComboBox cmbTriggerSelector;
        private System.Windows.Forms.Label labTriggerOpt;
        private System.Windows.Forms.Button btnStopGrab;
        private System.Windows.Forms.Button btnStartGrab;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.Button btnSavePng;
        private System.Windows.Forms.Button btnSaveTiff;
        private System.Windows.Forms.Button btnSaveJpg;
        private System.Windows.Forms.Button btnSaveBmp;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.Button btnGetParam;
        private System.Windows.Forms.Button btnSetParam;
        private System.Windows.Forms.TextBox tbAcqLineRate;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.TextBox tbDigitalShift;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.TextBox tbExposure;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.ComboBox cmbHBMode;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.ComboBox cmbPixelFormat;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox tbResLineRate;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.BindingSource mVCCENUMVALUEBindingSource;
        private System.Windows.Forms.ComboBox cmbPreampGain;
        private System.Windows.Forms.CheckBox chkLineRateEnable;
        private System.Windows.Forms.ComboBox cmbTriggerSource;
        private System.Windows.Forms.PictureBox pictureBox1;
    }
}

