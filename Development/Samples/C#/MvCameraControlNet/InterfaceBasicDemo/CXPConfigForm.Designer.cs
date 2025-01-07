namespace InterfaceBasicDemo
{
    partial class CXPConfigForm
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
            this.teCurrentStreamDevice = new System.Windows.Forms.TextBox();
            this.teStreamEnableStatus = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.cbBayerCFAEnable = new System.Windows.Forms.CheckBox();
            this.cbIspGammaEnable = new System.Windows.Forms.CheckBox();
            this.label5 = new System.Windows.Forms.Label();
            this.teIspGamma = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.bnGetParameter = new System.Windows.Forms.Button();
            this.bnSetParameter = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(9, 15);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(101, 12);
            this.label2.TabIndex = 7;
            this.label2.Text = "StreamSelector：";
            // 
            // cbStreamSelector
            // 
            this.cbStreamSelector.FormattingEnabled = true;
            this.cbStreamSelector.Location = new System.Drawing.Point(166, 12);
            this.cbStreamSelector.Name = "cbStreamSelector";
            this.cbStreamSelector.Size = new System.Drawing.Size(172, 20);
            this.cbStreamSelector.TabIndex = 6;
            this.cbStreamSelector.SelectedIndexChanged += new System.EventHandler(this.cbStreamSelector_SelectedIndexChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(9, 48);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(131, 12);
            this.label1.TabIndex = 8;
            this.label1.Text = "CurrentStreamDevice：";
            // 
            // teCurrentStreamDevice
            // 
            this.teCurrentStreamDevice.Enabled = false;
            this.teCurrentStreamDevice.Location = new System.Drawing.Point(166, 45);
            this.teCurrentStreamDevice.Name = "teCurrentStreamDevice";
            this.teCurrentStreamDevice.Size = new System.Drawing.Size(172, 21);
            this.teCurrentStreamDevice.TabIndex = 9;
            // 
            // teStreamEnableStatus
            // 
            this.teStreamEnableStatus.Enabled = false;
            this.teStreamEnableStatus.Location = new System.Drawing.Point(166, 81);
            this.teStreamEnableStatus.Name = "teStreamEnableStatus";
            this.teStreamEnableStatus.Size = new System.Drawing.Size(172, 21);
            this.teStreamEnableStatus.TabIndex = 11;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(9, 84);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(125, 12);
            this.label3.TabIndex = 10;
            this.label3.Text = "StreamEnableStatus：";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(9, 120);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(101, 12);
            this.label4.TabIndex = 12;
            this.label4.Text = "BayerCFAEnable：";
            // 
            // cbBayerCFAEnable
            // 
            this.cbBayerCFAEnable.AutoSize = true;
            this.cbBayerCFAEnable.Location = new System.Drawing.Point(166, 120);
            this.cbBayerCFAEnable.Name = "cbBayerCFAEnable";
            this.cbBayerCFAEnable.Size = new System.Drawing.Size(15, 14);
            this.cbBayerCFAEnable.TabIndex = 13;
            this.cbBayerCFAEnable.UseVisualStyleBackColor = true;
            this.cbBayerCFAEnable.CheckedChanged += new System.EventHandler(this.cbBayerCFAEnable_CheckedChanged);
            // 
            // cbIspGammaEnable
            // 
            this.cbIspGammaEnable.AutoSize = true;
            this.cbIspGammaEnable.Location = new System.Drawing.Point(166, 152);
            this.cbIspGammaEnable.Name = "cbIspGammaEnable";
            this.cbIspGammaEnable.Size = new System.Drawing.Size(15, 14);
            this.cbIspGammaEnable.TabIndex = 15;
            this.cbIspGammaEnable.UseVisualStyleBackColor = true;
            this.cbIspGammaEnable.CheckedChanged += new System.EventHandler(this.cbIspGammaEnable_CheckedChanged);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(9, 152);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(101, 12);
            this.label5.TabIndex = 14;
            this.label5.Text = "IspGammaEnable：";
            // 
            // teIspGamma
            // 
            this.teIspGamma.Enabled = false;
            this.teIspGamma.Location = new System.Drawing.Point(166, 181);
            this.teIspGamma.Name = "teIspGamma";
            this.teIspGamma.Size = new System.Drawing.Size(172, 21);
            this.teIspGamma.TabIndex = 17;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(9, 184);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(65, 12);
            this.label6.TabIndex = 16;
            this.label6.Text = "IspGamma：";
            // 
            // bnGetParameter
            // 
            this.bnGetParameter.ImeMode = System.Windows.Forms.ImeMode.NoControl;
            this.bnGetParameter.Location = new System.Drawing.Point(70, 238);
            this.bnGetParameter.Name = "bnGetParameter";
            this.bnGetParameter.Size = new System.Drawing.Size(88, 23);
            this.bnGetParameter.TabIndex = 18;
            this.bnGetParameter.Text = "获取参数";
            this.bnGetParameter.UseVisualStyleBackColor = true;
            this.bnGetParameter.Click += new System.EventHandler(this.bnGetParameter_Click);
            // 
            // bnSetParameter
            // 
            this.bnSetParameter.ImeMode = System.Windows.Forms.ImeMode.NoControl;
            this.bnSetParameter.Location = new System.Drawing.Point(215, 238);
            this.bnSetParameter.Name = "bnSetParameter";
            this.bnSetParameter.Size = new System.Drawing.Size(88, 23);
            this.bnSetParameter.TabIndex = 19;
            this.bnSetParameter.Text = "设置参数";
            this.bnSetParameter.UseVisualStyleBackColor = true;
            this.bnSetParameter.Click += new System.EventHandler(this.bnSetParameter_Click);
            // 
            // CXPConfigForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(379, 269);
            this.Controls.Add(this.bnSetParameter);
            this.Controls.Add(this.bnGetParameter);
            this.Controls.Add(this.teIspGamma);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.cbIspGammaEnable);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.cbBayerCFAEnable);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.teStreamEnableStatus);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.teCurrentStreamDevice);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.cbStreamSelector);
            this.Name = "CXPConfigForm";
            this.Text = "ConfigForm";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ComboBox cbStreamSelector;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox teCurrentStreamDevice;
        private System.Windows.Forms.TextBox teStreamEnableStatus;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.CheckBox cbBayerCFAEnable;
        private System.Windows.Forms.CheckBox cbIspGammaEnable;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox teIspGamma;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Button bnGetParameter;
        private System.Windows.Forms.Button bnSetParameter;

    }
}