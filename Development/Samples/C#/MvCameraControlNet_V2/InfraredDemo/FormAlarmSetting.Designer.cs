namespace InfraredDemo
{
    partial class FormAlarmSetting
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FormAlarmSetting));
            this.lbCurrentRegion = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.cbSetAlarmEnableCheck = new System.Windows.Forms.CheckBox();
            this.lbSetAlarmEnableCheck = new System.Windows.Forms.Label();
            this.cbSetAlarmSource = new System.Windows.Forms.ComboBox();
            this.lbSetAlarmSource = new System.Windows.Forms.Label();
            this.cbSetAlarmCondition = new System.Windows.Forms.ComboBox();
            this.lbSetAlarmCondition = new System.Windows.Forms.Label();
            this.teSetAlarmAbs = new System.Windows.Forms.TextBox();
            this.lbSetAlarmAbs = new System.Windows.Forms.Label();
            this.teSetAlarmReference = new System.Windows.Forms.TextBox();
            this.lbSetAlarmReference = new System.Windows.Forms.Label();
            this.bnOK = new System.Windows.Forms.Button();
            this.bnCancel = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // lbCurrentRegion
            // 
            resources.ApplyResources(this.lbCurrentRegion, "lbCurrentRegion");
            this.lbCurrentRegion.Name = "lbCurrentRegion";
            // 
            // label1
            // 
            resources.ApplyResources(this.label1, "label1");
            this.label1.Name = "label1";
            // 
            // cbSetAlarmEnableCheck
            // 
            resources.ApplyResources(this.cbSetAlarmEnableCheck, "cbSetAlarmEnableCheck");
            this.cbSetAlarmEnableCheck.Name = "cbSetAlarmEnableCheck";
            this.cbSetAlarmEnableCheck.UseVisualStyleBackColor = true;
            // 
            // lbSetAlarmEnableCheck
            // 
            resources.ApplyResources(this.lbSetAlarmEnableCheck, "lbSetAlarmEnableCheck");
            this.lbSetAlarmEnableCheck.Name = "lbSetAlarmEnableCheck";
            // 
            // cbSetAlarmSource
            // 
            this.cbSetAlarmSource.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbSetAlarmSource.FormattingEnabled = true;
            resources.ApplyResources(this.cbSetAlarmSource, "cbSetAlarmSource");
            this.cbSetAlarmSource.Name = "cbSetAlarmSource";
            // 
            // lbSetAlarmSource
            // 
            resources.ApplyResources(this.lbSetAlarmSource, "lbSetAlarmSource");
            this.lbSetAlarmSource.Name = "lbSetAlarmSource";
            // 
            // cbSetAlarmCondition
            // 
            this.cbSetAlarmCondition.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbSetAlarmCondition.FormattingEnabled = true;
            resources.ApplyResources(this.cbSetAlarmCondition, "cbSetAlarmCondition");
            this.cbSetAlarmCondition.Name = "cbSetAlarmCondition";
            // 
            // lbSetAlarmCondition
            // 
            resources.ApplyResources(this.lbSetAlarmCondition, "lbSetAlarmCondition");
            this.lbSetAlarmCondition.Name = "lbSetAlarmCondition";
            // 
            // teSetAlarmAbs
            // 
            resources.ApplyResources(this.teSetAlarmAbs, "teSetAlarmAbs");
            this.teSetAlarmAbs.Name = "teSetAlarmAbs";
            // 
            // lbSetAlarmAbs
            // 
            resources.ApplyResources(this.lbSetAlarmAbs, "lbSetAlarmAbs");
            this.lbSetAlarmAbs.Name = "lbSetAlarmAbs";
            // 
            // teSetAlarmReference
            // 
            resources.ApplyResources(this.teSetAlarmReference, "teSetAlarmReference");
            this.teSetAlarmReference.Name = "teSetAlarmReference";
            // 
            // lbSetAlarmReference
            // 
            resources.ApplyResources(this.lbSetAlarmReference, "lbSetAlarmReference");
            this.lbSetAlarmReference.Name = "lbSetAlarmReference";
            // 
            // bnOK
            // 
            resources.ApplyResources(this.bnOK, "bnOK");
            this.bnOK.Name = "bnOK";
            this.bnOK.UseVisualStyleBackColor = true;
            this.bnOK.Click += new System.EventHandler(this.bnOK_Click);
            // 
            // bnCancel
            // 
            resources.ApplyResources(this.bnCancel, "bnCancel");
            this.bnCancel.Name = "bnCancel";
            this.bnCancel.UseVisualStyleBackColor = true;
            this.bnCancel.Click += new System.EventHandler(this.bnCancel_Click);
            // 
            // FormAlarmSetting
            // 
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.bnCancel);
            this.Controls.Add(this.bnOK);
            this.Controls.Add(this.teSetAlarmAbs);
            this.Controls.Add(this.lbSetAlarmAbs);
            this.Controls.Add(this.teSetAlarmReference);
            this.Controls.Add(this.lbSetAlarmReference);
            this.Controls.Add(this.cbSetAlarmCondition);
            this.Controls.Add(this.lbSetAlarmCondition);
            this.Controls.Add(this.cbSetAlarmSource);
            this.Controls.Add(this.lbSetAlarmSource);
            this.Controls.Add(this.cbSetAlarmEnableCheck);
            this.Controls.Add(this.lbSetAlarmEnableCheck);
            this.Controls.Add(this.lbCurrentRegion);
            this.Controls.Add(this.label1);
            this.Name = "FormAlarmSetting";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label lbCurrentRegion;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.CheckBox cbSetAlarmEnableCheck;
        private System.Windows.Forms.Label lbSetAlarmEnableCheck;
        private System.Windows.Forms.ComboBox cbSetAlarmSource;
        private System.Windows.Forms.Label lbSetAlarmSource;
        private System.Windows.Forms.ComboBox cbSetAlarmCondition;
        private System.Windows.Forms.Label lbSetAlarmCondition;
        private System.Windows.Forms.TextBox teSetAlarmAbs;
        private System.Windows.Forms.Label lbSetAlarmAbs;
        private System.Windows.Forms.TextBox teSetAlarmReference;
        private System.Windows.Forms.Label lbSetAlarmReference;
        private System.Windows.Forms.Button bnOK;
        private System.Windows.Forms.Button bnCancel;
    }
}