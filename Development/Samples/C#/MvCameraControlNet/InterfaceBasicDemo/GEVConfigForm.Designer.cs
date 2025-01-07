namespace InterfaceBasicDemo
{
    partial class GEVConfigForm
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
            this.cbHBDecompression = new System.Windows.Forms.CheckBox();
            this.label4 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.cbTimerSelector = new System.Windows.Forms.ComboBox();
            this.teTimerDuration = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.teTimerDelay = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.teTimerFrequency = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.bnTimerReset = new System.Windows.Forms.Button();
            this.label8 = new System.Windows.Forms.Label();
            this.cbTimerTriggerSource = new System.Windows.Forms.ComboBox();
            this.bnTimerTriggerSoftware = new System.Windows.Forms.Button();
            this.label9 = new System.Windows.Forms.Label();
            this.label10 = new System.Windows.Forms.Label();
            this.cbTimerTriggerActivation = new System.Windows.Forms.ComboBox();
            this.bnSetParameter = new System.Windows.Forms.Button();
            this.bnGetParameter = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(7, 28);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(101, 12);
            this.label2.TabIndex = 9;
            this.label2.Text = "StreamSelector：";
            // 
            // cbStreamSelector
            // 
            this.cbStreamSelector.FormattingEnabled = true;
            this.cbStreamSelector.Location = new System.Drawing.Point(164, 25);
            this.cbStreamSelector.Name = "cbStreamSelector";
            this.cbStreamSelector.Size = new System.Drawing.Size(172, 20);
            this.cbStreamSelector.TabIndex = 8;
            this.cbStreamSelector.SelectedIndexChanged += new System.EventHandler(this.cbStreamSelector_SelectedIndexChanged);
            // 
            // cbHBDecompression
            // 
            this.cbHBDecompression.AutoSize = true;
            this.cbHBDecompression.Location = new System.Drawing.Point(164, 65);
            this.cbHBDecompression.Name = "cbHBDecompression";
            this.cbHBDecompression.Size = new System.Drawing.Size(15, 14);
            this.cbHBDecompression.TabIndex = 15;
            this.cbHBDecompression.UseVisualStyleBackColor = true;
            this.cbHBDecompression.CheckedChanged += new System.EventHandler(this.cbHBDecompression_CheckedChanged);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(7, 65);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(107, 12);
            this.label4.TabIndex = 14;
            this.label4.Text = "HBDecompression：";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(7, 99);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(95, 12);
            this.label1.TabIndex = 17;
            this.label1.Text = "TimerSelector：";
            // 
            // cbTimerSelector
            // 
            this.cbTimerSelector.FormattingEnabled = true;
            this.cbTimerSelector.Location = new System.Drawing.Point(164, 96);
            this.cbTimerSelector.Name = "cbTimerSelector";
            this.cbTimerSelector.Size = new System.Drawing.Size(172, 20);
            this.cbTimerSelector.TabIndex = 16;
            this.cbTimerSelector.SelectedIndexChanged += new System.EventHandler(this.cbTimerSelector_SelectedIndexChanged);
            // 
            // teTimerDuration
            // 
            this.teTimerDuration.Enabled = false;
            this.teTimerDuration.Location = new System.Drawing.Point(164, 132);
            this.teTimerDuration.Name = "teTimerDuration";
            this.teTimerDuration.Size = new System.Drawing.Size(172, 21);
            this.teTimerDuration.TabIndex = 19;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(7, 135);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(95, 12);
            this.label3.TabIndex = 18;
            this.label3.Text = "TimerDuration：";
            // 
            // teTimerDelay
            // 
            this.teTimerDelay.Enabled = false;
            this.teTimerDelay.Location = new System.Drawing.Point(164, 172);
            this.teTimerDelay.Name = "teTimerDelay";
            this.teTimerDelay.Size = new System.Drawing.Size(172, 21);
            this.teTimerDelay.TabIndex = 21;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(7, 175);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(77, 12);
            this.label5.TabIndex = 20;
            this.label5.Text = "TimerDelay：";
            // 
            // teTimerFrequency
            // 
            this.teTimerFrequency.Enabled = false;
            this.teTimerFrequency.Location = new System.Drawing.Point(164, 209);
            this.teTimerFrequency.Name = "teTimerFrequency";
            this.teTimerFrequency.Size = new System.Drawing.Size(172, 21);
            this.teTimerFrequency.TabIndex = 23;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(7, 212);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(101, 12);
            this.label6.TabIndex = 22;
            this.label6.Text = "TimerFrequency：";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(7, 255);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(77, 12);
            this.label7.TabIndex = 24;
            this.label7.Text = "TimerReset：";
            // 
            // bnTimerReset
            // 
            this.bnTimerReset.Location = new System.Drawing.Point(164, 250);
            this.bnTimerReset.Name = "bnTimerReset";
            this.bnTimerReset.Size = new System.Drawing.Size(172, 23);
            this.bnTimerReset.TabIndex = 25;
            this.bnTimerReset.Text = "TimerReset";
            this.bnTimerReset.UseVisualStyleBackColor = true;
            this.bnTimerReset.Click += new System.EventHandler(this.bnTimerReset_Click);
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(7, 294);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(125, 12);
            this.label8.TabIndex = 27;
            this.label8.Text = "TimerTriggerSource：";
            // 
            // cbTimerTriggerSource
            // 
            this.cbTimerTriggerSource.FormattingEnabled = true;
            this.cbTimerTriggerSource.Location = new System.Drawing.Point(164, 291);
            this.cbTimerTriggerSource.Name = "cbTimerTriggerSource";
            this.cbTimerTriggerSource.Size = new System.Drawing.Size(172, 20);
            this.cbTimerTriggerSource.TabIndex = 26;
            this.cbTimerTriggerSource.SelectedIndexChanged += new System.EventHandler(this.cbTimerTriggerSource_SelectedIndexChanged);
            // 
            // bnTimerTriggerSoftware
            // 
            this.bnTimerTriggerSoftware.Location = new System.Drawing.Point(164, 328);
            this.bnTimerTriggerSoftware.Name = "bnTimerTriggerSoftware";
            this.bnTimerTriggerSoftware.Size = new System.Drawing.Size(172, 23);
            this.bnTimerTriggerSoftware.TabIndex = 29;
            this.bnTimerTriggerSoftware.Text = "TimerTriggerSoftware";
            this.bnTimerTriggerSoftware.UseVisualStyleBackColor = true;
            this.bnTimerTriggerSoftware.Click += new System.EventHandler(this.bnTimerTriggerSoftware_Click);
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(7, 333);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(137, 12);
            this.label9.TabIndex = 28;
            this.label9.Text = "TimerTriggerSoftware：";
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(7, 377);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(149, 12);
            this.label10.TabIndex = 31;
            this.label10.Text = "TimerTriggerActivation：";
            // 
            // cbTimerTriggerActivation
            // 
            this.cbTimerTriggerActivation.FormattingEnabled = true;
            this.cbTimerTriggerActivation.Location = new System.Drawing.Point(164, 374);
            this.cbTimerTriggerActivation.Name = "cbTimerTriggerActivation";
            this.cbTimerTriggerActivation.Size = new System.Drawing.Size(172, 20);
            this.cbTimerTriggerActivation.TabIndex = 30;
            this.cbTimerTriggerActivation.SelectedIndexChanged += new System.EventHandler(this.cbTimerTriggerActivation_SelectedIndexChanged);
            // 
            // bnSetParameter
            // 
            this.bnSetParameter.ImeMode = System.Windows.Forms.ImeMode.NoControl;
            this.bnSetParameter.Location = new System.Drawing.Point(196, 423);
            this.bnSetParameter.Name = "bnSetParameter";
            this.bnSetParameter.Size = new System.Drawing.Size(88, 23);
            this.bnSetParameter.TabIndex = 33;
            this.bnSetParameter.Text = "设置参数";
            this.bnSetParameter.UseVisualStyleBackColor = true;
            this.bnSetParameter.Click += new System.EventHandler(this.bnSetParameter_Click);
            // 
            // bnGetParameter
            // 
            this.bnGetParameter.ImeMode = System.Windows.Forms.ImeMode.NoControl;
            this.bnGetParameter.Location = new System.Drawing.Point(55, 423);
            this.bnGetParameter.Name = "bnGetParameter";
            this.bnGetParameter.Size = new System.Drawing.Size(88, 23);
            this.bnGetParameter.TabIndex = 32;
            this.bnGetParameter.Text = "获取参数";
            this.bnGetParameter.UseVisualStyleBackColor = true;
            this.bnGetParameter.Click += new System.EventHandler(this.bnGetParameter_Click);
            // 
            // GEVConfigForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(351, 458);
            this.Controls.Add(this.bnSetParameter);
            this.Controls.Add(this.bnGetParameter);
            this.Controls.Add(this.label10);
            this.Controls.Add(this.cbTimerTriggerActivation);
            this.Controls.Add(this.bnTimerTriggerSoftware);
            this.Controls.Add(this.label9);
            this.Controls.Add(this.label8);
            this.Controls.Add(this.cbTimerTriggerSource);
            this.Controls.Add(this.bnTimerReset);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.teTimerFrequency);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.teTimerDelay);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.teTimerDuration);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.cbTimerSelector);
            this.Controls.Add(this.cbHBDecompression);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.cbStreamSelector);
            this.Name = "GEVConfigForm";
            this.Text = "GEVConfigForm";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ComboBox cbStreamSelector;
        private System.Windows.Forms.CheckBox cbHBDecompression;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox cbTimerSelector;
        private System.Windows.Forms.TextBox teTimerDuration;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox teTimerDelay;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox teTimerFrequency;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Button bnTimerReset;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.ComboBox cbTimerTriggerSource;
        private System.Windows.Forms.Button bnTimerTriggerSoftware;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.ComboBox cbTimerTriggerActivation;
        private System.Windows.Forms.Button bnSetParameter;
        private System.Windows.Forms.Button bnGetParameter;
    }
}