namespace InfraredDemo
{
    partial class InfraredDemo
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(InfraredDemo));
            this.cbDeviceList = new System.Windows.Forms.ComboBox();
            this.pbDisplay = new System.Windows.Forms.PictureBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.bnStopGrab = new System.Windows.Forms.Button();
            this.bnStartGrab = new System.Windows.Forms.Button();
            this.bnClose = new System.Windows.Forms.Button();
            this.bnOpen = new System.Windows.Forms.Button();
            this.bnEnum = new System.Windows.Forms.Button();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.bnWarningSetting = new System.Windows.Forms.Button();
            this.bnRegionSetting = new System.Windows.Forms.Button();
            this.cbRegionSelect = new System.Windows.Forms.ComboBox();
            this.label6 = new System.Windows.Forms.Label();
            this.cbExportModeCheck = new System.Windows.Forms.CheckBox();
            this.cbLegendCheck = new System.Windows.Forms.CheckBox();
            this.label5 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.cbPaletteMode = new System.Windows.Forms.ComboBox();
            this.label3 = new System.Windows.Forms.Label();
            this.cbDisplaySource = new System.Windows.Forms.ComboBox();
            this.label2 = new System.Windows.Forms.Label();
            this.cbPixelFormat = new System.Windows.Forms.ComboBox();
            this.label1 = new System.Windows.Forms.Label();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.bnSetParameter = new System.Windows.Forms.Button();
            this.bnGetParameter = new System.Windows.Forms.Button();
            this.teEmissivity = new System.Windows.Forms.TextBox();
            this.label10 = new System.Windows.Forms.Label();
            this.teTargetDistance = new System.Windows.Forms.TextBox();
            this.label9 = new System.Windows.Forms.Label();
            this.cbMeasureRange = new System.Windows.Forms.ComboBox();
            this.label8 = new System.Windows.Forms.Label();
            this.teTransmissivity = new System.Windows.Forms.TextBox();
            this.label7 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.pbDisplay)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox3.SuspendLayout();
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
            // pbDisplay
            // 
            this.pbDisplay.AccessibleDescription = null;
            this.pbDisplay.AccessibleName = null;
            resources.ApplyResources(this.pbDisplay, "pbDisplay");
            this.pbDisplay.BackColor = System.Drawing.SystemColors.ControlDarkDark;
            this.pbDisplay.BackgroundImage = null;
            this.pbDisplay.Font = null;
            this.pbDisplay.ImageLocation = null;
            this.pbDisplay.Name = "pbDisplay";
            this.pbDisplay.TabStop = false;
            // 
            // groupBox1
            // 
            this.groupBox1.AccessibleDescription = null;
            this.groupBox1.AccessibleName = null;
            resources.ApplyResources(this.groupBox1, "groupBox1");
            this.groupBox1.BackgroundImage = null;
            this.groupBox1.Controls.Add(this.bnStopGrab);
            this.groupBox1.Controls.Add(this.bnStartGrab);
            this.groupBox1.Controls.Add(this.bnClose);
            this.groupBox1.Controls.Add(this.bnOpen);
            this.groupBox1.Controls.Add(this.bnEnum);
            this.groupBox1.Font = null;
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.TabStop = false;
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
            this.groupBox2.Controls.Add(this.bnWarningSetting);
            this.groupBox2.Controls.Add(this.bnRegionSetting);
            this.groupBox2.Controls.Add(this.cbRegionSelect);
            this.groupBox2.Controls.Add(this.label6);
            this.groupBox2.Controls.Add(this.cbExportModeCheck);
            this.groupBox2.Controls.Add(this.cbLegendCheck);
            this.groupBox2.Controls.Add(this.label5);
            this.groupBox2.Controls.Add(this.label4);
            this.groupBox2.Controls.Add(this.cbPaletteMode);
            this.groupBox2.Controls.Add(this.label3);
            this.groupBox2.Controls.Add(this.cbDisplaySource);
            this.groupBox2.Controls.Add(this.label2);
            this.groupBox2.Controls.Add(this.cbPixelFormat);
            this.groupBox2.Controls.Add(this.label1);
            this.groupBox2.Font = null;
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.TabStop = false;
            // 
            // bnWarningSetting
            // 
            this.bnWarningSetting.AccessibleDescription = null;
            this.bnWarningSetting.AccessibleName = null;
            resources.ApplyResources(this.bnWarningSetting, "bnWarningSetting");
            this.bnWarningSetting.BackgroundImage = null;
            this.bnWarningSetting.Font = null;
            this.bnWarningSetting.Name = "bnWarningSetting";
            this.bnWarningSetting.UseVisualStyleBackColor = true;
            this.bnWarningSetting.Click += new System.EventHandler(this.bnWarningSetting_Click);
            // 
            // bnRegionSetting
            // 
            this.bnRegionSetting.AccessibleDescription = null;
            this.bnRegionSetting.AccessibleName = null;
            resources.ApplyResources(this.bnRegionSetting, "bnRegionSetting");
            this.bnRegionSetting.BackgroundImage = null;
            this.bnRegionSetting.Font = null;
            this.bnRegionSetting.Name = "bnRegionSetting";
            this.bnRegionSetting.UseVisualStyleBackColor = true;
            this.bnRegionSetting.Click += new System.EventHandler(this.bnRegionSetting_Click);
            // 
            // cbRegionSelect
            // 
            this.cbRegionSelect.AccessibleDescription = null;
            this.cbRegionSelect.AccessibleName = null;
            resources.ApplyResources(this.cbRegionSelect, "cbRegionSelect");
            this.cbRegionSelect.BackgroundImage = null;
            this.cbRegionSelect.Font = null;
            this.cbRegionSelect.FormattingEnabled = true;
            this.cbRegionSelect.Name = "cbRegionSelect";
            this.cbRegionSelect.SelectedIndexChanged += new System.EventHandler(this.cbRegionSelect_SelectedIndexChanged);
            // 
            // label6
            // 
            this.label6.AccessibleDescription = null;
            this.label6.AccessibleName = null;
            resources.ApplyResources(this.label6, "label6");
            this.label6.Font = null;
            this.label6.Name = "label6";
            // 
            // cbExportModeCheck
            // 
            this.cbExportModeCheck.AccessibleDescription = null;
            this.cbExportModeCheck.AccessibleName = null;
            resources.ApplyResources(this.cbExportModeCheck, "cbExportModeCheck");
            this.cbExportModeCheck.BackgroundImage = null;
            this.cbExportModeCheck.Font = null;
            this.cbExportModeCheck.Name = "cbExportModeCheck";
            this.cbExportModeCheck.UseVisualStyleBackColor = true;
            this.cbExportModeCheck.CheckedChanged += new System.EventHandler(this.cbExportModeCheck_CheckedChanged);
            // 
            // cbLegendCheck
            // 
            this.cbLegendCheck.AccessibleDescription = null;
            this.cbLegendCheck.AccessibleName = null;
            resources.ApplyResources(this.cbLegendCheck, "cbLegendCheck");
            this.cbLegendCheck.BackgroundImage = null;
            this.cbLegendCheck.Font = null;
            this.cbLegendCheck.Name = "cbLegendCheck";
            this.cbLegendCheck.UseVisualStyleBackColor = true;
            this.cbLegendCheck.CheckedChanged += new System.EventHandler(this.cbLegendCheck_CheckedChanged);
            // 
            // label5
            // 
            this.label5.AccessibleDescription = null;
            this.label5.AccessibleName = null;
            resources.ApplyResources(this.label5, "label5");
            this.label5.Font = null;
            this.label5.Name = "label5";
            // 
            // label4
            // 
            this.label4.AccessibleDescription = null;
            this.label4.AccessibleName = null;
            resources.ApplyResources(this.label4, "label4");
            this.label4.Font = null;
            this.label4.Name = "label4";
            // 
            // cbPaletteMode
            // 
            this.cbPaletteMode.AccessibleDescription = null;
            this.cbPaletteMode.AccessibleName = null;
            resources.ApplyResources(this.cbPaletteMode, "cbPaletteMode");
            this.cbPaletteMode.BackgroundImage = null;
            this.cbPaletteMode.Font = null;
            this.cbPaletteMode.FormattingEnabled = true;
            this.cbPaletteMode.Name = "cbPaletteMode";
            this.cbPaletteMode.SelectedIndexChanged += new System.EventHandler(this.cbPaletteMode_SelectedIndexChanged);
            // 
            // label3
            // 
            this.label3.AccessibleDescription = null;
            this.label3.AccessibleName = null;
            resources.ApplyResources(this.label3, "label3");
            this.label3.Font = null;
            this.label3.Name = "label3";
            // 
            // cbDisplaySource
            // 
            this.cbDisplaySource.AccessibleDescription = null;
            this.cbDisplaySource.AccessibleName = null;
            resources.ApplyResources(this.cbDisplaySource, "cbDisplaySource");
            this.cbDisplaySource.BackgroundImage = null;
            this.cbDisplaySource.Font = null;
            this.cbDisplaySource.FormattingEnabled = true;
            this.cbDisplaySource.Name = "cbDisplaySource";
            this.cbDisplaySource.SelectedIndexChanged += new System.EventHandler(this.cbDisplaySource_SelectedIndexChanged);
            // 
            // label2
            // 
            this.label2.AccessibleDescription = null;
            this.label2.AccessibleName = null;
            resources.ApplyResources(this.label2, "label2");
            this.label2.Font = null;
            this.label2.Name = "label2";
            // 
            // cbPixelFormat
            // 
            this.cbPixelFormat.AccessibleDescription = null;
            this.cbPixelFormat.AccessibleName = null;
            resources.ApplyResources(this.cbPixelFormat, "cbPixelFormat");
            this.cbPixelFormat.BackgroundImage = null;
            this.cbPixelFormat.Font = null;
            this.cbPixelFormat.FormattingEnabled = true;
            this.cbPixelFormat.Name = "cbPixelFormat";
            this.cbPixelFormat.SelectedIndexChanged += new System.EventHandler(this.cbPixelFormat_SelectedIndexChanged);
            // 
            // label1
            // 
            this.label1.AccessibleDescription = null;
            this.label1.AccessibleName = null;
            resources.ApplyResources(this.label1, "label1");
            this.label1.Font = null;
            this.label1.Name = "label1";
            // 
            // groupBox3
            // 
            this.groupBox3.AccessibleDescription = null;
            this.groupBox3.AccessibleName = null;
            resources.ApplyResources(this.groupBox3, "groupBox3");
            this.groupBox3.BackgroundImage = null;
            this.groupBox3.Controls.Add(this.bnSetParameter);
            this.groupBox3.Controls.Add(this.bnGetParameter);
            this.groupBox3.Controls.Add(this.teEmissivity);
            this.groupBox3.Controls.Add(this.label10);
            this.groupBox3.Controls.Add(this.teTargetDistance);
            this.groupBox3.Controls.Add(this.label9);
            this.groupBox3.Controls.Add(this.cbMeasureRange);
            this.groupBox3.Controls.Add(this.label8);
            this.groupBox3.Controls.Add(this.teTransmissivity);
            this.groupBox3.Controls.Add(this.label7);
            this.groupBox3.Font = null;
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.TabStop = false;
            // 
            // bnSetParameter
            // 
            this.bnSetParameter.AccessibleDescription = null;
            this.bnSetParameter.AccessibleName = null;
            resources.ApplyResources(this.bnSetParameter, "bnSetParameter");
            this.bnSetParameter.BackgroundImage = null;
            this.bnSetParameter.Font = null;
            this.bnSetParameter.Name = "bnSetParameter";
            this.bnSetParameter.UseVisualStyleBackColor = true;
            this.bnSetParameter.Click += new System.EventHandler(this.bnSetParameter_Click);
            // 
            // bnGetParameter
            // 
            this.bnGetParameter.AccessibleDescription = null;
            this.bnGetParameter.AccessibleName = null;
            resources.ApplyResources(this.bnGetParameter, "bnGetParameter");
            this.bnGetParameter.BackgroundImage = null;
            this.bnGetParameter.Font = null;
            this.bnGetParameter.Name = "bnGetParameter";
            this.bnGetParameter.UseVisualStyleBackColor = true;
            this.bnGetParameter.Click += new System.EventHandler(this.bnGetParameter_Click);
            // 
            // teEmissivity
            // 
            this.teEmissivity.AccessibleDescription = null;
            this.teEmissivity.AccessibleName = null;
            resources.ApplyResources(this.teEmissivity, "teEmissivity");
            this.teEmissivity.BackgroundImage = null;
            this.teEmissivity.Font = null;
            this.teEmissivity.Name = "teEmissivity";
            // 
            // label10
            // 
            this.label10.AccessibleDescription = null;
            this.label10.AccessibleName = null;
            resources.ApplyResources(this.label10, "label10");
            this.label10.Font = null;
            this.label10.Name = "label10";
            // 
            // teTargetDistance
            // 
            this.teTargetDistance.AccessibleDescription = null;
            this.teTargetDistance.AccessibleName = null;
            resources.ApplyResources(this.teTargetDistance, "teTargetDistance");
            this.teTargetDistance.BackgroundImage = null;
            this.teTargetDistance.Font = null;
            this.teTargetDistance.Name = "teTargetDistance";
            // 
            // label9
            // 
            this.label9.AccessibleDescription = null;
            this.label9.AccessibleName = null;
            resources.ApplyResources(this.label9, "label9");
            this.label9.Font = null;
            this.label9.Name = "label9";
            // 
            // cbMeasureRange
            // 
            this.cbMeasureRange.AccessibleDescription = null;
            this.cbMeasureRange.AccessibleName = null;
            resources.ApplyResources(this.cbMeasureRange, "cbMeasureRange");
            this.cbMeasureRange.BackgroundImage = null;
            this.cbMeasureRange.Font = null;
            this.cbMeasureRange.FormattingEnabled = true;
            this.cbMeasureRange.Name = "cbMeasureRange";
            this.cbMeasureRange.SelectedIndexChanged += new System.EventHandler(this.cbMeasureRange_SelectedIndexChanged);
            // 
            // label8
            // 
            this.label8.AccessibleDescription = null;
            this.label8.AccessibleName = null;
            resources.ApplyResources(this.label8, "label8");
            this.label8.Font = null;
            this.label8.Name = "label8";
            // 
            // teTransmissivity
            // 
            this.teTransmissivity.AccessibleDescription = null;
            this.teTransmissivity.AccessibleName = null;
            resources.ApplyResources(this.teTransmissivity, "teTransmissivity");
            this.teTransmissivity.BackgroundImage = null;
            this.teTransmissivity.Font = null;
            this.teTransmissivity.Name = "teTransmissivity";
            // 
            // label7
            // 
            this.label7.AccessibleDescription = null;
            this.label7.AccessibleName = null;
            resources.ApplyResources(this.label7, "label7");
            this.label7.Font = null;
            this.label7.Name = "label7";
            // 
            // InfraredDemo
            // 
            this.AccessibleDescription = null;
            this.AccessibleName = null;
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackgroundImage = null;
            this.Controls.Add(this.groupBox3);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.pbDisplay);
            this.Controls.Add(this.cbDeviceList);
            this.Font = null;
            this.Icon = null;
            this.Name = "InfraredDemo";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.InfraredDemo_Closing);
            ((System.ComponentModel.ISupportInitialize)(this.pbDisplay)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ComboBox cbDeviceList;
        private System.Windows.Forms.PictureBox pbDisplay;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button bnStopGrab;
        private System.Windows.Forms.Button bnStartGrab;
        private System.Windows.Forms.Button bnClose;
        private System.Windows.Forms.Button bnOpen;
        private System.Windows.Forms.Button bnEnum;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.ComboBox cbPaletteMode;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.ComboBox cbDisplaySource;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ComboBox cbPixelFormat;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.CheckBox cbExportModeCheck;
        private System.Windows.Forms.CheckBox cbLegendCheck;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.ComboBox cbRegionSelect;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Button bnWarningSetting;
        private System.Windows.Forms.Button bnRegionSetting;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.TextBox teTransmissivity;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Button bnSetParameter;
        private System.Windows.Forms.Button bnGetParameter;
        private System.Windows.Forms.TextBox teEmissivity;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.TextBox teTargetDistance;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.ComboBox cbMeasureRange;
        private System.Windows.Forms.Label label8;
    }
}

