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
            this.groupBoxInterface = new System.Windows.Forms.GroupBox();
            this.btnCloseInterface = new System.Windows.Forms.Button();
            this.btnOpenInterface = new System.Windows.Forms.Button();
            this.btnEnumInterface = new System.Windows.Forms.Button();
            this.btnCloseDevice = new System.Windows.Forms.Button();
            this.btnOpenDevice = new System.Windows.Forms.Button();
            this.btnEnumDevice = new System.Windows.Forms.Button();
            this.groupBoxImageAcq = new System.Windows.Forms.GroupBox();
            this.btnStopGrab = new System.Windows.Forms.Button();
            this.btnStartGrab = new System.Windows.Forms.Button();
            this.btnTriggerExec = new System.Windows.Forms.Button();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.cmbInterfaceList = new System.Windows.Forms.ComboBox();
            this.groupBoxDevice = new System.Windows.Forms.GroupBox();
            this.groupBoxInterfaceParams = new System.Windows.Forms.GroupBox();
            this.textBoxImageHeight = new System.Windows.Forms.TextBox();
            this.labelCCSource = new System.Windows.Forms.Label();
            this.labelCCSelector = new System.Windows.Forms.Label();
            this.labelEncoderTriggerMode = new System.Windows.Forms.Label();
            this.labelEncoderSourceB = new System.Windows.Forms.Label();
            this.labelEncoderSourceA = new System.Windows.Forms.Label();
            this.labelEncoderSelector = new System.Windows.Forms.Label();
            this.labelLineInputPolarity = new System.Windows.Forms.Label();
            this.labelLineMode = new System.Windows.Forms.Label();
            this.labelLineSelector = new System.Windows.Forms.Label();
            this.labelImageHeight = new System.Windows.Forms.Label();
            this.labelCameraType = new System.Windows.Forms.Label();
            this.comboBoxCCSource = new System.Windows.Forms.ComboBox();
            this.comboBoxCCSelector = new System.Windows.Forms.ComboBox();
            this.comboBoxEncoderTrigger = new System.Windows.Forms.ComboBox();
            this.comboBoxEncoderSourceB = new System.Windows.Forms.ComboBox();
            this.comboBoxEncoderSourceA = new System.Windows.Forms.ComboBox();
            this.comboBoxEncoderSelector = new System.Windows.Forms.ComboBox();
            this.comboBoxLineInputPolarity = new System.Windows.Forms.ComboBox();
            this.comboBoxLineMode = new System.Windows.Forms.ComboBox();
            this.comboBoxLineSelector = new System.Windows.Forms.ComboBox();
            this.comboBoxCameraType = new System.Windows.Forms.ComboBox();
            this.groupBoxDeviceParams = new System.Windows.Forms.GroupBox();
            this.textBoxHeight = new System.Windows.Forms.TextBox();
            this.labelTriggerActivation = new System.Windows.Forms.Label();
            this.labelTriggerSource = new System.Windows.Forms.Label();
            this.labelTriggerMode = new System.Windows.Forms.Label();
            this.labelScanMode = new System.Windows.Forms.Label();
            this.labelTriggerSelector = new System.Windows.Forms.Label();
            this.labelPixelFormat = new System.Windows.Forms.Label();
            this.labelHeight = new System.Windows.Forms.Label();
            this.textBoxWidth = new System.Windows.Forms.TextBox();
            this.labelWidth = new System.Windows.Forms.Label();
            this.comboBoxTriggerActivation = new System.Windows.Forms.ComboBox();
            this.comboBoxTriggerSource = new System.Windows.Forms.ComboBox();
            this.comboBoxTriggerMode = new System.Windows.Forms.ComboBox();
            this.comboBoxScanMode = new System.Windows.Forms.ComboBox();
            this.comboBoxTriggerSelector = new System.Windows.Forms.ComboBox();
            this.comboBoxPixelFormat = new System.Windows.Forms.ComboBox();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.groupBoxInterface.SuspendLayout();
            this.groupBoxImageAcq.SuspendLayout();
            this.groupBoxDevice.SuspendLayout();
            this.groupBoxInterfaceParams.SuspendLayout();
            this.groupBoxDeviceParams.SuspendLayout();
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
            // groupBoxInterface
            // 
            this.groupBoxInterface.Controls.Add(this.btnCloseInterface);
            this.groupBoxInterface.Controls.Add(this.btnOpenInterface);
            this.groupBoxInterface.Controls.Add(this.btnEnumInterface);
            resources.ApplyResources(this.groupBoxInterface, "groupBoxInterface");
            this.groupBoxInterface.Name = "groupBoxInterface";
            this.groupBoxInterface.TabStop = false;
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
            // groupBoxImageAcq
            // 
            this.groupBoxImageAcq.Controls.Add(this.btnStopGrab);
            this.groupBoxImageAcq.Controls.Add(this.btnStartGrab);
            resources.ApplyResources(this.groupBoxImageAcq, "groupBoxImageAcq");
            this.groupBoxImageAcq.Name = "groupBoxImageAcq";
            this.groupBoxImageAcq.TabStop = false;
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
            // btnTriggerExec
            // 
            resources.ApplyResources(this.btnTriggerExec, "btnTriggerExec");
            this.btnTriggerExec.Name = "btnTriggerExec";
            this.btnTriggerExec.UseVisualStyleBackColor = true;
            this.btnTriggerExec.Click += new System.EventHandler(this.bnTriggerExec_Click);
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
            // groupBoxDevice
            // 
            this.groupBoxDevice.Controls.Add(this.btnEnumDevice);
            this.groupBoxDevice.Controls.Add(this.btnCloseDevice);
            this.groupBoxDevice.Controls.Add(this.btnOpenDevice);
            resources.ApplyResources(this.groupBoxDevice, "groupBoxDevice");
            this.groupBoxDevice.Name = "groupBoxDevice";
            this.groupBoxDevice.TabStop = false;
            // 
            // groupBoxInterfaceParams
            // 
            this.groupBoxInterfaceParams.Controls.Add(this.textBoxImageHeight);
            this.groupBoxInterfaceParams.Controls.Add(this.labelCCSource);
            this.groupBoxInterfaceParams.Controls.Add(this.labelCCSelector);
            this.groupBoxInterfaceParams.Controls.Add(this.labelEncoderTriggerMode);
            this.groupBoxInterfaceParams.Controls.Add(this.labelEncoderSourceB);
            this.groupBoxInterfaceParams.Controls.Add(this.labelEncoderSourceA);
            this.groupBoxInterfaceParams.Controls.Add(this.labelEncoderSelector);
            this.groupBoxInterfaceParams.Controls.Add(this.labelLineInputPolarity);
            this.groupBoxInterfaceParams.Controls.Add(this.labelLineMode);
            this.groupBoxInterfaceParams.Controls.Add(this.labelLineSelector);
            this.groupBoxInterfaceParams.Controls.Add(this.labelImageHeight);
            this.groupBoxInterfaceParams.Controls.Add(this.labelCameraType);
            this.groupBoxInterfaceParams.Controls.Add(this.comboBoxCCSource);
            this.groupBoxInterfaceParams.Controls.Add(this.comboBoxCCSelector);
            this.groupBoxInterfaceParams.Controls.Add(this.comboBoxEncoderTrigger);
            this.groupBoxInterfaceParams.Controls.Add(this.comboBoxEncoderSourceB);
            this.groupBoxInterfaceParams.Controls.Add(this.comboBoxEncoderSourceA);
            this.groupBoxInterfaceParams.Controls.Add(this.comboBoxEncoderSelector);
            this.groupBoxInterfaceParams.Controls.Add(this.comboBoxLineInputPolarity);
            this.groupBoxInterfaceParams.Controls.Add(this.comboBoxLineMode);
            this.groupBoxInterfaceParams.Controls.Add(this.comboBoxLineSelector);
            this.groupBoxInterfaceParams.Controls.Add(this.comboBoxCameraType);
            resources.ApplyResources(this.groupBoxInterfaceParams, "groupBoxInterfaceParams");
            this.groupBoxInterfaceParams.Name = "groupBoxInterfaceParams";
            this.groupBoxInterfaceParams.TabStop = false;
            // 
            // textBoxImageHeight
            // 
            resources.ApplyResources(this.textBoxImageHeight, "textBoxImageHeight");
            this.textBoxImageHeight.Name = "textBoxImageHeight";
            this.textBoxImageHeight.Validated += new System.EventHandler(this.ParamsChanged);
            // 
            // labelCCSource
            // 
            resources.ApplyResources(this.labelCCSource, "labelCCSource");
            this.labelCCSource.Name = "labelCCSource";
            // 
            // labelCCSelector
            // 
            resources.ApplyResources(this.labelCCSelector, "labelCCSelector");
            this.labelCCSelector.Name = "labelCCSelector";
            // 
            // labelEncoderTriggerMode
            // 
            resources.ApplyResources(this.labelEncoderTriggerMode, "labelEncoderTriggerMode");
            this.labelEncoderTriggerMode.Name = "labelEncoderTriggerMode";
            // 
            // labelEncoderSourceB
            // 
            resources.ApplyResources(this.labelEncoderSourceB, "labelEncoderSourceB");
            this.labelEncoderSourceB.Name = "labelEncoderSourceB";
            // 
            // labelEncoderSourceA
            // 
            resources.ApplyResources(this.labelEncoderSourceA, "labelEncoderSourceA");
            this.labelEncoderSourceA.Name = "labelEncoderSourceA";
            // 
            // labelEncoderSelector
            // 
            resources.ApplyResources(this.labelEncoderSelector, "labelEncoderSelector");
            this.labelEncoderSelector.Name = "labelEncoderSelector";
            // 
            // labelLineInputPolarity
            // 
            resources.ApplyResources(this.labelLineInputPolarity, "labelLineInputPolarity");
            this.labelLineInputPolarity.Name = "labelLineInputPolarity";
            // 
            // labelLineMode
            // 
            resources.ApplyResources(this.labelLineMode, "labelLineMode");
            this.labelLineMode.Name = "labelLineMode";
            // 
            // labelLineSelector
            // 
            resources.ApplyResources(this.labelLineSelector, "labelLineSelector");
            this.labelLineSelector.Name = "labelLineSelector";
            // 
            // labelImageHeight
            // 
            resources.ApplyResources(this.labelImageHeight, "labelImageHeight");
            this.labelImageHeight.Name = "labelImageHeight";
            // 
            // labelCameraType
            // 
            resources.ApplyResources(this.labelCameraType, "labelCameraType");
            this.labelCameraType.Name = "labelCameraType";
            // 
            // comboBoxCCSource
            // 
            this.comboBoxCCSource.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxCCSource.FormattingEnabled = true;
            resources.ApplyResources(this.comboBoxCCSource, "comboBoxCCSource");
            this.comboBoxCCSource.Name = "comboBoxCCSource";
            this.comboBoxCCSource.SelectionChangeCommitted += new System.EventHandler(this.ParamsChanged);
            // 
            // comboBoxCCSelector
            // 
            this.comboBoxCCSelector.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxCCSelector.FormattingEnabled = true;
            resources.ApplyResources(this.comboBoxCCSelector, "comboBoxCCSelector");
            this.comboBoxCCSelector.Name = "comboBoxCCSelector";
            this.comboBoxCCSelector.SelectionChangeCommitted += new System.EventHandler(this.ParamsChanged);
            // 
            // comboBoxEncoderTrigger
            // 
            this.comboBoxEncoderTrigger.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxEncoderTrigger.FormattingEnabled = true;
            resources.ApplyResources(this.comboBoxEncoderTrigger, "comboBoxEncoderTrigger");
            this.comboBoxEncoderTrigger.Name = "comboBoxEncoderTrigger";
            this.comboBoxEncoderTrigger.SelectionChangeCommitted += new System.EventHandler(this.ParamsChanged);
            // 
            // comboBoxEncoderSourceB
            // 
            this.comboBoxEncoderSourceB.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxEncoderSourceB.FormattingEnabled = true;
            resources.ApplyResources(this.comboBoxEncoderSourceB, "comboBoxEncoderSourceB");
            this.comboBoxEncoderSourceB.Name = "comboBoxEncoderSourceB";
            this.comboBoxEncoderSourceB.SelectionChangeCommitted += new System.EventHandler(this.ParamsChanged);
            // 
            // comboBoxEncoderSourceA
            // 
            this.comboBoxEncoderSourceA.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxEncoderSourceA.FormattingEnabled = true;
            resources.ApplyResources(this.comboBoxEncoderSourceA, "comboBoxEncoderSourceA");
            this.comboBoxEncoderSourceA.Name = "comboBoxEncoderSourceA";
            this.comboBoxEncoderSourceA.SelectionChangeCommitted += new System.EventHandler(this.ParamsChanged);
            // 
            // comboBoxEncoderSelector
            // 
            this.comboBoxEncoderSelector.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxEncoderSelector.FormattingEnabled = true;
            resources.ApplyResources(this.comboBoxEncoderSelector, "comboBoxEncoderSelector");
            this.comboBoxEncoderSelector.Name = "comboBoxEncoderSelector";
            this.comboBoxEncoderSelector.SelectionChangeCommitted += new System.EventHandler(this.ParamsChanged);
            // 
            // comboBoxLineInputPolarity
            // 
            this.comboBoxLineInputPolarity.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxLineInputPolarity.FormattingEnabled = true;
            resources.ApplyResources(this.comboBoxLineInputPolarity, "comboBoxLineInputPolarity");
            this.comboBoxLineInputPolarity.Name = "comboBoxLineInputPolarity";
            this.comboBoxLineInputPolarity.SelectionChangeCommitted += new System.EventHandler(this.ParamsChanged);
            // 
            // comboBoxLineMode
            // 
            this.comboBoxLineMode.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxLineMode.FormattingEnabled = true;
            resources.ApplyResources(this.comboBoxLineMode, "comboBoxLineMode");
            this.comboBoxLineMode.Name = "comboBoxLineMode";
            this.comboBoxLineMode.SelectionChangeCommitted += new System.EventHandler(this.ParamsChanged);
            // 
            // comboBoxLineSelector
            // 
            this.comboBoxLineSelector.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxLineSelector.FormattingEnabled = true;
            resources.ApplyResources(this.comboBoxLineSelector, "comboBoxLineSelector");
            this.comboBoxLineSelector.Name = "comboBoxLineSelector";
            this.comboBoxLineSelector.SelectionChangeCommitted += new System.EventHandler(this.ParamsChanged);
            // 
            // comboBoxCameraType
            // 
            this.comboBoxCameraType.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxCameraType.FormattingEnabled = true;
            resources.ApplyResources(this.comboBoxCameraType, "comboBoxCameraType");
            this.comboBoxCameraType.Name = "comboBoxCameraType";
            this.comboBoxCameraType.SelectionChangeCommitted += new System.EventHandler(this.ParamsChanged);
            // 
            // groupBoxDeviceParams
            // 
            this.groupBoxDeviceParams.Controls.Add(this.btnTriggerExec);
            this.groupBoxDeviceParams.Controls.Add(this.textBoxHeight);
            this.groupBoxDeviceParams.Controls.Add(this.labelTriggerActivation);
            this.groupBoxDeviceParams.Controls.Add(this.labelTriggerSource);
            this.groupBoxDeviceParams.Controls.Add(this.labelTriggerMode);
            this.groupBoxDeviceParams.Controls.Add(this.labelScanMode);
            this.groupBoxDeviceParams.Controls.Add(this.labelTriggerSelector);
            this.groupBoxDeviceParams.Controls.Add(this.labelPixelFormat);
            this.groupBoxDeviceParams.Controls.Add(this.labelHeight);
            this.groupBoxDeviceParams.Controls.Add(this.textBoxWidth);
            this.groupBoxDeviceParams.Controls.Add(this.labelWidth);
            this.groupBoxDeviceParams.Controls.Add(this.comboBoxTriggerActivation);
            this.groupBoxDeviceParams.Controls.Add(this.comboBoxTriggerSource);
            this.groupBoxDeviceParams.Controls.Add(this.comboBoxTriggerMode);
            this.groupBoxDeviceParams.Controls.Add(this.comboBoxScanMode);
            this.groupBoxDeviceParams.Controls.Add(this.comboBoxTriggerSelector);
            this.groupBoxDeviceParams.Controls.Add(this.comboBoxPixelFormat);
            resources.ApplyResources(this.groupBoxDeviceParams, "groupBoxDeviceParams");
            this.groupBoxDeviceParams.Name = "groupBoxDeviceParams";
            this.groupBoxDeviceParams.TabStop = false;
            // 
            // textBoxHeight
            // 
            resources.ApplyResources(this.textBoxHeight, "textBoxHeight");
            this.textBoxHeight.Name = "textBoxHeight";
            this.textBoxHeight.Validated += new System.EventHandler(this.ParamsChanged);
            // 
            // labelTriggerActivation
            // 
            resources.ApplyResources(this.labelTriggerActivation, "labelTriggerActivation");
            this.labelTriggerActivation.Name = "labelTriggerActivation";
            // 
            // labelTriggerSource
            // 
            resources.ApplyResources(this.labelTriggerSource, "labelTriggerSource");
            this.labelTriggerSource.Name = "labelTriggerSource";
            // 
            // labelTriggerMode
            // 
            resources.ApplyResources(this.labelTriggerMode, "labelTriggerMode");
            this.labelTriggerMode.Name = "labelTriggerMode";
            // 
            // labelScanMode
            // 
            resources.ApplyResources(this.labelScanMode, "labelScanMode");
            this.labelScanMode.Name = "labelScanMode";
            // 
            // labelTriggerSelector
            // 
            resources.ApplyResources(this.labelTriggerSelector, "labelTriggerSelector");
            this.labelTriggerSelector.Name = "labelTriggerSelector";
            // 
            // labelPixelFormat
            // 
            resources.ApplyResources(this.labelPixelFormat, "labelPixelFormat");
            this.labelPixelFormat.Name = "labelPixelFormat";
            // 
            // labelHeight
            // 
            resources.ApplyResources(this.labelHeight, "labelHeight");
            this.labelHeight.Name = "labelHeight";
            // 
            // textBoxWidth
            // 
            resources.ApplyResources(this.textBoxWidth, "textBoxWidth");
            this.textBoxWidth.Name = "textBoxWidth";
            this.textBoxWidth.Validated += new System.EventHandler(this.ParamsChanged);
            // 
            // labelWidth
            // 
            resources.ApplyResources(this.labelWidth, "labelWidth");
            this.labelWidth.Name = "labelWidth";
            // 
            // comboBoxTriggerActivation
            // 
            this.comboBoxTriggerActivation.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxTriggerActivation.FormattingEnabled = true;
            resources.ApplyResources(this.comboBoxTriggerActivation, "comboBoxTriggerActivation");
            this.comboBoxTriggerActivation.Name = "comboBoxTriggerActivation";
            this.comboBoxTriggerActivation.SelectionChangeCommitted += new System.EventHandler(this.ParamsChanged);
            // 
            // comboBoxTriggerSource
            // 
            this.comboBoxTriggerSource.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxTriggerSource.FormattingEnabled = true;
            resources.ApplyResources(this.comboBoxTriggerSource, "comboBoxTriggerSource");
            this.comboBoxTriggerSource.Name = "comboBoxTriggerSource";
            this.comboBoxTriggerSource.SelectionChangeCommitted += new System.EventHandler(this.ParamsChanged);
            // 
            // comboBoxTriggerMode
            // 
            this.comboBoxTriggerMode.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxTriggerMode.FormattingEnabled = true;
            resources.ApplyResources(this.comboBoxTriggerMode, "comboBoxTriggerMode");
            this.comboBoxTriggerMode.Name = "comboBoxTriggerMode";
            this.comboBoxTriggerMode.SelectionChangeCommitted += new System.EventHandler(this.ParamsChanged);
            // 
            // comboBoxScanMode
            // 
            this.comboBoxScanMode.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxScanMode.FormattingEnabled = true;
            resources.ApplyResources(this.comboBoxScanMode, "comboBoxScanMode");
            this.comboBoxScanMode.Name = "comboBoxScanMode";
            this.comboBoxScanMode.SelectionChangeCommitted += new System.EventHandler(this.ParamsChanged);
            // 
            // comboBoxTriggerSelector
            // 
            this.comboBoxTriggerSelector.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxTriggerSelector.FormattingEnabled = true;
            resources.ApplyResources(this.comboBoxTriggerSelector, "comboBoxTriggerSelector");
            this.comboBoxTriggerSelector.Name = "comboBoxTriggerSelector";
            this.comboBoxTriggerSelector.SelectionChangeCommitted += new System.EventHandler(this.ParamsChanged);
            // 
            // comboBoxPixelFormat
            // 
            this.comboBoxPixelFormat.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxPixelFormat.FormattingEnabled = true;
            resources.ApplyResources(this.comboBoxPixelFormat, "comboBoxPixelFormat");
            this.comboBoxPixelFormat.Name = "comboBoxPixelFormat";
            this.comboBoxPixelFormat.SelectionChangeCommitted += new System.EventHandler(this.ParamsChanged);
            // 
            // Form1
            // 
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.groupBoxDeviceParams);
            this.Controls.Add(this.groupBoxInterfaceParams);
            this.Controls.Add(this.groupBoxDevice);
            this.Controls.Add(this.cmbInterfaceList);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.groupBoxImageAcq);
            this.Controls.Add(this.groupBoxInterface);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.cmbDeviceList);
            this.MaximizeBox = false;
            this.Name = "Form1";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.groupBoxInterface.ResumeLayout(false);
            this.groupBoxImageAcq.ResumeLayout(false);
            this.groupBoxDevice.ResumeLayout(false);
            this.groupBoxInterfaceParams.ResumeLayout(false);
            this.groupBoxInterfaceParams.PerformLayout();
            this.groupBoxDeviceParams.ResumeLayout(false);
            this.groupBoxDeviceParams.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox cmbDeviceList;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.GroupBox groupBoxInterface;
        private System.Windows.Forms.Button btnCloseDevice;
        private System.Windows.Forms.Button btnOpenDevice;
        private System.Windows.Forms.Button btnEnumDevice;
        private System.Windows.Forms.GroupBox groupBoxImageAcq;
        private System.Windows.Forms.Button btnStopGrab;
        private System.Windows.Forms.Button btnStartGrab;
        private System.Windows.Forms.Button btnTriggerExec;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.ComboBox cmbInterfaceList;
        private System.Windows.Forms.GroupBox groupBoxDevice;
        private System.Windows.Forms.Button btnCloseInterface;
        private System.Windows.Forms.Button btnOpenInterface;
        private System.Windows.Forms.Button btnEnumInterface;
        private System.Windows.Forms.GroupBox groupBoxInterfaceParams;
        private System.Windows.Forms.ComboBox comboBoxCameraType;
        private System.Windows.Forms.Label labelCameraType;
        private System.Windows.Forms.Label labelImageHeight;
        private System.Windows.Forms.TextBox textBoxImageHeight;
        private System.Windows.Forms.Label labelLineSelector;
        private System.Windows.Forms.ComboBox comboBoxLineSelector;
        private System.Windows.Forms.Label labelLineMode;
        private System.Windows.Forms.ComboBox comboBoxLineMode;
        private System.Windows.Forms.Label labelLineInputPolarity;
        private System.Windows.Forms.ComboBox comboBoxLineInputPolarity;
        private System.Windows.Forms.Label labelEncoderSelector;
        private System.Windows.Forms.ComboBox comboBoxEncoderSelector;
        private System.Windows.Forms.Label labelEncoderSourceA;
        private System.Windows.Forms.ComboBox comboBoxEncoderSourceA;
        private System.Windows.Forms.Label labelEncoderSourceB;
        private System.Windows.Forms.ComboBox comboBoxEncoderSourceB;
        private System.Windows.Forms.Label labelEncoderTriggerMode;
        private System.Windows.Forms.ComboBox comboBoxEncoderTrigger;
        private System.Windows.Forms.Label labelCCSelector;
        private System.Windows.Forms.ComboBox comboBoxCCSelector;
        private System.Windows.Forms.Label labelCCSource;
        private System.Windows.Forms.ComboBox comboBoxCCSource;
        private System.Windows.Forms.GroupBox groupBoxDeviceParams;
        private System.Windows.Forms.TextBox textBoxWidth;
        private System.Windows.Forms.Label labelWidth;
        private System.Windows.Forms.TextBox textBoxHeight;
        private System.Windows.Forms.Label labelHeight;
        private System.Windows.Forms.Label labelPixelFormat;
        private System.Windows.Forms.ComboBox comboBoxPixelFormat;
        private System.Windows.Forms.Label labelTriggerSelector;
        private System.Windows.Forms.ComboBox comboBoxTriggerSelector;
        private System.Windows.Forms.Label labelTriggerMode;
        private System.Windows.Forms.ComboBox comboBoxTriggerMode;
        private System.Windows.Forms.Label labelTriggerSource;
        private System.Windows.Forms.ComboBox comboBoxTriggerSource;
        private System.Windows.Forms.Label labelTriggerActivation;
        private System.Windows.Forms.ComboBox comboBoxTriggerActivation;
        private System.Windows.Forms.Label labelScanMode;
        private System.Windows.Forms.ComboBox comboBoxScanMode;
    }
}

