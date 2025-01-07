namespace InterfaceBasicDemo
{
    partial class CMLConfigForm
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
            this.label2 = new System.Windows.Forms.Label();
            this.cbStreamSelector = new System.Windows.Forms.ComboBox();
            this.label1 = new System.Windows.Forms.Label();
            this.cbCameraType = new System.Windows.Forms.ComboBox();
            this.teImageHeight = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.teFrameTimeoutTime = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.cbStreamPartialImageControl = new System.Windows.Forms.ComboBox();
            this.bnSetParameter = new System.Windows.Forms.Button();
            this.bnGetParameter = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(8, 32);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(101, 12);
            this.label2.TabIndex = 9;
            this.label2.Text = "StreamSelector：";
            // 
            // cbStreamSelector
            // 
            this.cbStreamSelector.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbStreamSelector.FormattingEnabled = true;
            this.cbStreamSelector.Location = new System.Drawing.Point(176, 29);
            this.cbStreamSelector.Name = "cbStreamSelector";
            this.cbStreamSelector.Size = new System.Drawing.Size(172, 20);
            this.cbStreamSelector.TabIndex = 8;
            this.cbStreamSelector.SelectedIndexChanged += new System.EventHandler(this.cbStreamSelector_SelectedIndexChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(8, 67);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(77, 12);
            this.label1.TabIndex = 11;
            this.label1.Text = "CameraType：";
            // 
            // cbCameraType
            // 
            this.cbCameraType.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbCameraType.FormattingEnabled = true;
            this.cbCameraType.Location = new System.Drawing.Point(176, 64);
            this.cbCameraType.Name = "cbCameraType";
            this.cbCameraType.Size = new System.Drawing.Size(172, 20);
            this.cbCameraType.TabIndex = 10;
            this.cbCameraType.SelectedIndexChanged += new System.EventHandler(this.cbCameraType_SelectedIndexChanged);
            // 
            // teImageHeight
            // 
            this.teImageHeight.Enabled = false;
            this.teImageHeight.Location = new System.Drawing.Point(176, 102);
            this.teImageHeight.Name = "teImageHeight";
            this.teImageHeight.Size = new System.Drawing.Size(172, 21);
            this.teImageHeight.TabIndex = 19;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(8, 105);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(83, 12);
            this.label6.TabIndex = 18;
            this.label6.Text = "ImageHeight：";
            // 
            // teFrameTimeoutTime
            // 
            this.teFrameTimeoutTime.Enabled = false;
            this.teFrameTimeoutTime.Location = new System.Drawing.Point(176, 138);
            this.teFrameTimeoutTime.Name = "teFrameTimeoutTime";
            this.teFrameTimeoutTime.Size = new System.Drawing.Size(172, 21);
            this.teFrameTimeoutTime.TabIndex = 21;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(8, 141);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(113, 12);
            this.label3.TabIndex = 20;
            this.label3.Text = "FrameTimeoutTime：";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(8, 178);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(167, 12);
            this.label4.TabIndex = 23;
            this.label4.Text = "StreamPartialImageControl：";
            // 
            // cbStreamPartialImageControl
            // 
            this.cbStreamPartialImageControl.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbStreamPartialImageControl.FormattingEnabled = true;
            this.cbStreamPartialImageControl.Location = new System.Drawing.Point(176, 175);
            this.cbStreamPartialImageControl.Name = "cbStreamPartialImageControl";
            this.cbStreamPartialImageControl.Size = new System.Drawing.Size(172, 20);
            this.cbStreamPartialImageControl.TabIndex = 22;
            this.cbStreamPartialImageControl.SelectedIndexChanged += new System.EventHandler(this.cbStreamPartialImageControl_SelectedIndexChanged);
            // 
            // bnSetParameter
            // 
            this.bnSetParameter.ImeMode = System.Windows.Forms.ImeMode.NoControl;
            this.bnSetParameter.Location = new System.Drawing.Point(204, 235);
            this.bnSetParameter.Name = "bnSetParameter";
            this.bnSetParameter.Size = new System.Drawing.Size(88, 23);
            this.bnSetParameter.TabIndex = 25;
            this.bnSetParameter.Text = "设置参数";
            this.bnSetParameter.UseVisualStyleBackColor = true;
            this.bnSetParameter.Click += new System.EventHandler(this.bnSetParameter_Click);
            // 
            // bnGetParameter
            // 
            this.bnGetParameter.ImeMode = System.Windows.Forms.ImeMode.NoControl;
            this.bnGetParameter.Location = new System.Drawing.Point(63, 235);
            this.bnGetParameter.Name = "bnGetParameter";
            this.bnGetParameter.Size = new System.Drawing.Size(88, 23);
            this.bnGetParameter.TabIndex = 24;
            this.bnGetParameter.Text = "获取参数";
            this.bnGetParameter.UseVisualStyleBackColor = true;
            this.bnGetParameter.Click += new System.EventHandler(this.bnGetParameter_Click);
            // 
            // CMLConfigForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(374, 273);
            this.Controls.Add(this.bnSetParameter);
            this.Controls.Add(this.bnGetParameter);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.cbStreamPartialImageControl);
            this.Controls.Add(this.teFrameTimeoutTime);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.teImageHeight);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.cbCameraType);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.cbStreamSelector);
            this.Name = "CMLConfigForm";
            this.Text = "CMLConfigForm";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ComboBox cbStreamSelector;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox cbCameraType;
        private System.Windows.Forms.TextBox teImageHeight;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.TextBox teFrameTimeoutTime;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.ComboBox cbStreamPartialImageControl;
        private System.Windows.Forms.Button bnSetParameter;
        private System.Windows.Forms.Button bnGetParameter;
    }
}