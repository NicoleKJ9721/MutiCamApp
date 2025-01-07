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
            resources.ApplyResources(this.cbDeviceList, "cbDeviceList");
            this.cbDeviceList.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbDeviceList.FormattingEnabled = true;
            this.cbDeviceList.Name = "cbDeviceList";
            // 
            // pbDisplay
            // 
            resources.ApplyResources(this.pbDisplay, "pbDisplay");
            this.pbDisplay.BackColor = System.Drawing.SystemColors.ControlDarkDark;
            this.pbDisplay.Name = "pbDisplay";
            this.pbDisplay.TabStop = false;
            // 
            // groupBox1
            // 
            resources.ApplyResources(this.groupBox1, "groupBox1");
            this.groupBox1.Controls.Add(this.bnStopGrab);
            this.groupBox1.Controls.Add(this.bnStartGrab);
            this.groupBox1.Controls.Add(this.bnClose);
            this.groupBox1.Controls.Add(this.bnOpen);
            this.groupBox1.Controls.Add(this.bnEnum);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.TabStop = false;
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
            // bnEnum
            // 
            resources.ApplyResources(this.bnEnum, "bnEnum");
            this.bnEnum.Name = "bnEnum";
            this.bnEnum.UseVisualStyleBackColor = true;
            this.bnEnum.Click += new System.EventHandler(this.bnEnum_Click);
            // 
            // groupBox2
            // 
            resources.ApplyResources(this.groupBox2, "groupBox2");
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
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.TabStop = false;
            // 
            // bnWarningSetting
            // 
            resources.ApplyResources(this.bnWarningSetting, "bnWarningSetting");
            this.bnWarningSetting.Name = "bnWarningSetting";
            this.bnWarningSetting.UseVisualStyleBackColor = true;
            this.bnWarningSetting.Click += new System.EventHandler(this.bnWarningSetting_Click);
            // 
            // bnRegionSetting
            // 
            resources.ApplyResources(this.bnRegionSetting, "bnRegionSetting");
            this.bnRegionSetting.Name = "bnRegionSetting";
            this.bnRegionSetting.UseVisualStyleBackColor = true;
            this.bnRegionSetting.Click += new System.EventHandler(this.bnRegionSetting_Click);
            // 
            // cbRegionSelect
            // 
            resources.ApplyResources(this.cbRegionSelect, "cbRegionSelect");
            this.cbRegionSelect.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbRegionSelect.FormattingEnabled = true;
            this.cbRegionSelect.Name = "cbRegionSelect";
            this.cbRegionSelect.SelectedIndexChanged += new System.EventHandler(this.cbRegionSelect_SelectedIndexChanged);
            // 
            // label6
            // 
            resources.ApplyResources(this.label6, "label6");
            this.label6.Name = "label6";
            // 
            // cbExportModeCheck
            // 
            resources.ApplyResources(this.cbExportModeCheck, "cbExportModeCheck");
            this.cbExportModeCheck.Name = "cbExportModeCheck";
            this.cbExportModeCheck.UseVisualStyleBackColor = true;
            this.cbExportModeCheck.CheckedChanged += new System.EventHandler(this.cbExportModeCheck_CheckedChanged);
            // 
            // cbLegendCheck
            // 
            resources.ApplyResources(this.cbLegendCheck, "cbLegendCheck");
            this.cbLegendCheck.Name = "cbLegendCheck";
            this.cbLegendCheck.UseVisualStyleBackColor = true;
            this.cbLegendCheck.CheckedChanged += new System.EventHandler(this.cbLegendCheck_CheckedChanged);
            // 
            // label5
            // 
            resources.ApplyResources(this.label5, "label5");
            this.label5.Name = "label5";
            // 
            // label4
            // 
            resources.ApplyResources(this.label4, "label4");
            this.label4.Name = "label4";
            // 
            // cbPaletteMode
            // 
            resources.ApplyResources(this.cbPaletteMode, "cbPaletteMode");
            this.cbPaletteMode.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbPaletteMode.FormattingEnabled = true;
            this.cbPaletteMode.Name = "cbPaletteMode";
            this.cbPaletteMode.SelectedIndexChanged += new System.EventHandler(this.cbPaletteMode_SelectedIndexChanged);
            // 
            // label3
            // 
            resources.ApplyResources(this.label3, "label3");
            this.label3.Name = "label3";
            // 
            // cbDisplaySource
            // 
            resources.ApplyResources(this.cbDisplaySource, "cbDisplaySource");
            this.cbDisplaySource.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbDisplaySource.FormattingEnabled = true;
            this.cbDisplaySource.Name = "cbDisplaySource";
            this.cbDisplaySource.SelectedIndexChanged += new System.EventHandler(this.cbDisplaySource_SelectedIndexChanged);
            // 
            // label2
            // 
            resources.ApplyResources(this.label2, "label2");
            this.label2.Name = "label2";
            // 
            // cbPixelFormat
            // 
            resources.ApplyResources(this.cbPixelFormat, "cbPixelFormat");
            this.cbPixelFormat.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbPixelFormat.FormattingEnabled = true;
            this.cbPixelFormat.Name = "cbPixelFormat";
            this.cbPixelFormat.SelectedIndexChanged += new System.EventHandler(this.cbPixelFormat_SelectedIndexChanged);
            // 
            // label1
            // 
            resources.ApplyResources(this.label1, "label1");
            this.label1.Name = "label1";
            // 
            // groupBox3
            // 
            resources.ApplyResources(this.groupBox3, "groupBox3");
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
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.TabStop = false;
            // 
            // bnSetParameter
            // 
            resources.ApplyResources(this.bnSetParameter, "bnSetParameter");
            this.bnSetParameter.Name = "bnSetParameter";
            this.bnSetParameter.UseVisualStyleBackColor = true;
            this.bnSetParameter.Click += new System.EventHandler(this.bnSetParameter_Click);
            // 
            // bnGetParameter
            // 
            resources.ApplyResources(this.bnGetParameter, "bnGetParameter");
            this.bnGetParameter.Name = "bnGetParameter";
            this.bnGetParameter.UseVisualStyleBackColor = true;
            this.bnGetParameter.Click += new System.EventHandler(this.bnGetParameter_Click);
            // 
            // teEmissivity
            // 
            resources.ApplyResources(this.teEmissivity, "teEmissivity");
            this.teEmissivity.Name = "teEmissivity";
            // 
            // label10
            // 
            resources.ApplyResources(this.label10, "label10");
            this.label10.Name = "label10";
            // 
            // teTargetDistance
            // 
            resources.ApplyResources(this.teTargetDistance, "teTargetDistance");
            this.teTargetDistance.Name = "teTargetDistance";
            // 
            // label9
            // 
            resources.ApplyResources(this.label9, "label9");
            this.label9.Name = "label9";
            // 
            // cbMeasureRange
            // 
            resources.ApplyResources(this.cbMeasureRange, "cbMeasureRange");
            this.cbMeasureRange.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbMeasureRange.FormattingEnabled = true;
            this.cbMeasureRange.Name = "cbMeasureRange";
            this.cbMeasureRange.SelectedIndexChanged += new System.EventHandler(this.cbMeasureRange_SelectedIndexChanged);
            // 
            // label8
            // 
            resources.ApplyResources(this.label8, "label8");
            this.label8.Name = "label8";
            // 
            // teTransmissivity
            // 
            resources.ApplyResources(this.teTransmissivity, "teTransmissivity");
            this.teTransmissivity.Name = "teTransmissivity";
            // 
            // label7
            // 
            resources.ApplyResources(this.label7, "label7");
            this.label7.Name = "label7";
            // 
            // InfraredDemo
            // 
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.groupBox3);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.pbDisplay);
            this.Controls.Add(this.cbDeviceList);
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

