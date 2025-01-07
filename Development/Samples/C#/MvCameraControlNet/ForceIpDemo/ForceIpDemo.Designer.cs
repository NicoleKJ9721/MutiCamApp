namespace MvCamera_SDK_CS_Demo
{
    partial class ForceIpDemo
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ForceIpDemo));
            this.cbDeviceList = new System.Windows.Forms.ComboBox();
            this.bnEnum = new System.Windows.Forms.Button();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.label4 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.tbDefaultWay = new System.Windows.Forms.TextBox();
            this.tbMask = new System.Windows.Forms.TextBox();
            this.bnSetIp = new System.Windows.Forms.Button();
            this.tbIP = new System.Windows.Forms.TextBox();
            this.lbTip = new System.Windows.Forms.Label();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.SuspendLayout();
            // 
            // cbDeviceList
            // 
            this.cbDeviceList.FormattingEnabled = true;
            resources.ApplyResources(this.cbDeviceList, "cbDeviceList");
            this.cbDeviceList.Name = "cbDeviceList";
            this.cbDeviceList.SelectedIndexChanged += new System.EventHandler(this.cbDeviceList_SelectedIndexChanged);
            // 
            // bnEnum
            // 
            resources.ApplyResources(this.bnEnum, "bnEnum");
            this.bnEnum.Name = "bnEnum";
            this.bnEnum.UseVisualStyleBackColor = true;
            this.bnEnum.Click += new System.EventHandler(this.bnEnum_Click);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.bnEnum);
            resources.ApplyResources(this.groupBox1, "groupBox1");
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.TabStop = false;
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.label4);
            this.groupBox2.Controls.Add(this.label3);
            this.groupBox2.Controls.Add(this.label2);
            this.groupBox2.Controls.Add(this.label1);
            this.groupBox2.Controls.Add(this.tbDefaultWay);
            this.groupBox2.Controls.Add(this.tbMask);
            this.groupBox2.Controls.Add(this.bnSetIp);
            this.groupBox2.Controls.Add(this.tbIP);
            this.groupBox2.Controls.Add(this.lbTip);
            resources.ApplyResources(this.groupBox2, "groupBox2");
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.TabStop = false;
            // 
            // label4
            // 
            resources.ApplyResources(this.label4, "label4");
            this.label4.Name = "label4";
            // 
            // label3
            // 
            resources.ApplyResources(this.label3, "label3");
            this.label3.Name = "label3";
            // 
            // label2
            // 
            resources.ApplyResources(this.label2, "label2");
            this.label2.Name = "label2";
            // 
            // label1
            // 
            resources.ApplyResources(this.label1, "label1");
            this.label1.Name = "label1";
            // 
            // tbDefaultWay
            // 
            resources.ApplyResources(this.tbDefaultWay, "tbDefaultWay");
            this.tbDefaultWay.Name = "tbDefaultWay";
            // 
            // tbMask
            // 
            resources.ApplyResources(this.tbMask, "tbMask");
            this.tbMask.Name = "tbMask";
            // 
            // bnSetIp
            // 
            resources.ApplyResources(this.bnSetIp, "bnSetIp");
            this.bnSetIp.Name = "bnSetIp";
            this.bnSetIp.UseVisualStyleBackColor = true;
            this.bnSetIp.Click += new System.EventHandler(this.bnSetIp_Click);
            // 
            // tbIP
            // 
            resources.ApplyResources(this.tbIP, "tbIP");
            this.tbIP.Name = "tbIP";
            // 
            // lbTip
            // 
            resources.ApplyResources(this.lbTip, "lbTip");
            this.lbTip.Name = "lbTip";
            // 
            // ForceIpDemo
            // 
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.cbDeviceList);
            this.Controls.Add(this.groupBox1);
            this.Name = "ForceIpDemo";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.ForceIpDemo_Closing);
            this.groupBox1.ResumeLayout(false);
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ComboBox cbDeviceList;
        private System.Windows.Forms.Button bnEnum;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Label lbTip;
        private System.Windows.Forms.Button bnSetIp;
        private System.Windows.Forms.TextBox tbIP;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox tbDefaultWay;
        private System.Windows.Forms.TextBox tbMask;
        private System.Windows.Forms.Label label4;
    }
}

