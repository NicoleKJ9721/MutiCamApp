namespace InterfaceBasicDemo
{
    partial class XOFConfigForm
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
            this.teCurrentStreamDevice = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.cbStreamSelector = new System.Windows.Forms.ComboBox();
            this.cbMinFrameDelay = new System.Windows.Forms.CheckBox();
            this.label4 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.cbCameraType = new System.Windows.Forms.ComboBox();
            this.teImageHeight = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.teFrameTimeoutTime = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.cbPartialImageOutputMode = new System.Windows.Forms.ComboBox();
            this.bnSetParameter = new System.Windows.Forms.Button();
            this.bnGetParameter = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // teCurrentStreamDevice
            // 
            this.teCurrentStreamDevice.Enabled = false;
            this.teCurrentStreamDevice.Location = new System.Drawing.Point(172, 45);
            this.teCurrentStreamDevice.Name = "teCurrentStreamDevice";
            this.teCurrentStreamDevice.Size = new System.Drawing.Size(172, 21);
            this.teCurrentStreamDevice.TabIndex = 13;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(15, 48);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(131, 12);
            this.label1.TabIndex = 12;
            this.label1.Text = "CurrentStreamDevice：";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(15, 15);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(101, 12);
            this.label2.TabIndex = 11;
            this.label2.Text = "StreamSelector：";
            // 
            // cbStreamSelector
            // 
            this.cbStreamSelector.FormattingEnabled = true;
            this.cbStreamSelector.Location = new System.Drawing.Point(172, 12);
            this.cbStreamSelector.Name = "cbStreamSelector";
            this.cbStreamSelector.Size = new System.Drawing.Size(172, 20);
            this.cbStreamSelector.TabIndex = 10;
            this.cbStreamSelector.SelectedIndexChanged += new System.EventHandler(this.cbStreamSelector_SelectedIndexChanged);
            // 
            // cbMinFrameDelay
            // 
            this.cbMinFrameDelay.AutoSize = true;
            this.cbMinFrameDelay.Location = new System.Drawing.Point(172, 83);
            this.cbMinFrameDelay.Name = "cbMinFrameDelay";
            this.cbMinFrameDelay.Size = new System.Drawing.Size(15, 14);
            this.cbMinFrameDelay.TabIndex = 15;
            this.cbMinFrameDelay.UseVisualStyleBackColor = true;
            this.cbMinFrameDelay.CheckedChanged += new System.EventHandler(this.cbMinFrameDelay_CheckedChanged);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(15, 83);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(95, 12);
            this.label4.TabIndex = 14;
            this.label4.Text = "MinFrameDelay：";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(15, 119);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(77, 12);
            this.label3.TabIndex = 17;
            this.label3.Text = "CameraType：";
            // 
            // cbCameraType
            // 
            this.cbCameraType.FormattingEnabled = true;
            this.cbCameraType.Location = new System.Drawing.Point(172, 116);
            this.cbCameraType.Name = "cbCameraType";
            this.cbCameraType.Size = new System.Drawing.Size(172, 20);
            this.cbCameraType.TabIndex = 16;
            this.cbCameraType.SelectedIndexChanged += new System.EventHandler(this.cbCameraType_SelectedIndexChanged);
            // 
            // teImageHeight
            // 
            this.teImageHeight.Enabled = false;
            this.teImageHeight.Location = new System.Drawing.Point(172, 156);
            this.teImageHeight.Name = "teImageHeight";
            this.teImageHeight.Size = new System.Drawing.Size(172, 21);
            this.teImageHeight.TabIndex = 21;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(15, 159);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(83, 12);
            this.label6.TabIndex = 20;
            this.label6.Text = "ImageHeight：";
            // 
            // teFrameTimeoutTime
            // 
            this.teFrameTimeoutTime.Enabled = false;
            this.teFrameTimeoutTime.Location = new System.Drawing.Point(172, 201);
            this.teFrameTimeoutTime.Name = "teFrameTimeoutTime";
            this.teFrameTimeoutTime.Size = new System.Drawing.Size(172, 21);
            this.teFrameTimeoutTime.TabIndex = 23;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(15, 204);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(113, 12);
            this.label5.TabIndex = 22;
            this.label5.Text = "FrameTimeoutTime：";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(15, 242);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(149, 12);
            this.label7.TabIndex = 25;
            this.label7.Text = "PartialImageOutputMode：";
            // 
            // cbPartialImageOutputMode
            // 
            this.cbPartialImageOutputMode.FormattingEnabled = true;
            this.cbPartialImageOutputMode.Location = new System.Drawing.Point(172, 239);
            this.cbPartialImageOutputMode.Name = "cbPartialImageOutputMode";
            this.cbPartialImageOutputMode.Size = new System.Drawing.Size(172, 20);
            this.cbPartialImageOutputMode.TabIndex = 24;
            this.cbPartialImageOutputMode.SelectedIndexChanged += new System.EventHandler(this.cbPartialImageOutputMode_SelectedIndexChanged);
            // 
            // bnSetParameter
            // 
            this.bnSetParameter.ImeMode = System.Windows.Forms.ImeMode.NoControl;
            this.bnSetParameter.Location = new System.Drawing.Point(208, 279);
            this.bnSetParameter.Name = "bnSetParameter";
            this.bnSetParameter.Size = new System.Drawing.Size(88, 23);
            this.bnSetParameter.TabIndex = 27;
            this.bnSetParameter.Text = "设置参数";
            this.bnSetParameter.UseVisualStyleBackColor = true;
            this.bnSetParameter.Click += new System.EventHandler(this.bnSetParameter_Click);
            // 
            // bnGetParameter
            // 
            this.bnGetParameter.ImeMode = System.Windows.Forms.ImeMode.NoControl;
            this.bnGetParameter.Location = new System.Drawing.Point(63, 279);
            this.bnGetParameter.Name = "bnGetParameter";
            this.bnGetParameter.Size = new System.Drawing.Size(88, 23);
            this.bnGetParameter.TabIndex = 26;
            this.bnGetParameter.Text = "获取参数";
            this.bnGetParameter.UseVisualStyleBackColor = true;
            this.bnGetParameter.Click += new System.EventHandler(this.bnGetParameter_Click);
            // 
            // XOFConfigForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(374, 311);
            this.Controls.Add(this.bnSetParameter);
            this.Controls.Add(this.bnGetParameter);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.cbPartialImageOutputMode);
            this.Controls.Add(this.teFrameTimeoutTime);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.teImageHeight);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.cbCameraType);
            this.Controls.Add(this.cbMinFrameDelay);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.teCurrentStreamDevice);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.cbStreamSelector);
            this.Name = "XOFConfigForm";
            this.Text = "XOFConfigForm";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox teCurrentStreamDevice;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ComboBox cbStreamSelector;
        private System.Windows.Forms.CheckBox cbMinFrameDelay;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.ComboBox cbCameraType;
        private System.Windows.Forms.TextBox teImageHeight;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.TextBox teFrameTimeoutTime;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.ComboBox cbPartialImageOutputMode;
        private System.Windows.Forms.Button bnSetParameter;
        private System.Windows.Forms.Button bnGetParameter;
    }
}