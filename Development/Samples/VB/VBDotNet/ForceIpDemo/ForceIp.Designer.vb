<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> _
Partial Class ForceIp
    Inherits System.Windows.Forms.Form

    'Form 重写 Dispose，以清理组件列表。
    <System.Diagnostics.DebuggerNonUserCode()> _
    Protected Overrides Sub Dispose(ByVal disposing As Boolean)
        Try
            If disposing AndAlso components IsNot Nothing Then
                components.Dispose()
            End If
        Finally
            MyBase.Dispose(disposing)
        End Try
    End Sub

    'Windows 窗体设计器所必需的
    Private components As System.ComponentModel.IContainer

    '注意: 以下过程是 Windows 窗体设计器所必需的
    '可以使用 Windows 窗体设计器修改它。
    '不要使用代码编辑器修改它。
    <System.Diagnostics.DebuggerStepThrough()> _
    Private Sub InitializeComponent()
        Dim resources As System.ComponentModel.ComponentResourceManager = New System.ComponentModel.ComponentResourceManager(GetType(ForceIp))
        Me.GroupBoxInit = New System.Windows.Forms.GroupBox()
        Me.ButtonEnumDevice = New System.Windows.Forms.Button()
        Me.ComboBoxDeviceList = New System.Windows.Forms.ComboBox()
        Me.GroupBoxSetIp = New System.Windows.Forms.GroupBox()
        Me.Label4 = New System.Windows.Forms.Label()
        Me.Label3 = New System.Windows.Forms.Label()
        Me.Label2 = New System.Windows.Forms.Label()
        Me.Label1 = New System.Windows.Forms.Label()
        Me.TextBoxSetDefaultWay = New System.Windows.Forms.TextBox()
        Me.TextBoxSetMask = New System.Windows.Forms.TextBox()
        Me.BtnSetIp = New System.Windows.Forms.Button()
        Me.TextBoxSetIp = New System.Windows.Forms.TextBox()
        Me.LabelIpInfo = New System.Windows.Forms.Label()
        Me.GroupBoxInit.SuspendLayout()
        Me.GroupBoxSetIp.SuspendLayout()
        Me.SuspendLayout()
        '
        'GroupBoxInit
        '
        Me.GroupBoxInit.Controls.Add(Me.ButtonEnumDevice)
        resources.ApplyResources(Me.GroupBoxInit, "GroupBoxInit")
        Me.GroupBoxInit.Name = "GroupBoxInit"
        Me.GroupBoxInit.TabStop = False
        '
        'ButtonEnumDevice
        '
        resources.ApplyResources(Me.ButtonEnumDevice, "ButtonEnumDevice")
        Me.ButtonEnumDevice.Name = "ButtonEnumDevice"
        Me.ButtonEnumDevice.UseVisualStyleBackColor = True
        '
        'ComboBoxDeviceList
        '
        Me.ComboBoxDeviceList.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList
        Me.ComboBoxDeviceList.FormattingEnabled = True
        resources.ApplyResources(Me.ComboBoxDeviceList, "ComboBoxDeviceList")
        Me.ComboBoxDeviceList.Name = "ComboBoxDeviceList"
        '
        'GroupBoxSetIp
        '
        Me.GroupBoxSetIp.Controls.Add(Me.Label4)
        Me.GroupBoxSetIp.Controls.Add(Me.Label3)
        Me.GroupBoxSetIp.Controls.Add(Me.Label2)
        Me.GroupBoxSetIp.Controls.Add(Me.Label1)
        Me.GroupBoxSetIp.Controls.Add(Me.TextBoxSetDefaultWay)
        Me.GroupBoxSetIp.Controls.Add(Me.TextBoxSetMask)
        Me.GroupBoxSetIp.Controls.Add(Me.BtnSetIp)
        Me.GroupBoxSetIp.Controls.Add(Me.TextBoxSetIp)
        Me.GroupBoxSetIp.Controls.Add(Me.LabelIpInfo)
        resources.ApplyResources(Me.GroupBoxSetIp, "GroupBoxSetIp")
        Me.GroupBoxSetIp.Name = "GroupBoxSetIp"
        Me.GroupBoxSetIp.TabStop = False
        '
        'Label4
        '
        resources.ApplyResources(Me.Label4, "Label4")
        Me.Label4.Name = "Label4"
        '
        'Label3
        '
        resources.ApplyResources(Me.Label3, "Label3")
        Me.Label3.Name = "Label3"
        '
        'Label2
        '
        resources.ApplyResources(Me.Label2, "Label2")
        Me.Label2.Name = "Label2"
        '
        'Label1
        '
        resources.ApplyResources(Me.Label1, "Label1")
        Me.Label1.Name = "Label1"
        '
        'TextBoxSetDefaultWay
        '
        resources.ApplyResources(Me.TextBoxSetDefaultWay, "TextBoxSetDefaultWay")
        Me.TextBoxSetDefaultWay.Name = "TextBoxSetDefaultWay"
        '
        'TextBoxSetMask
        '
        resources.ApplyResources(Me.TextBoxSetMask, "TextBoxSetMask")
        Me.TextBoxSetMask.Name = "TextBoxSetMask"
        '
        'BtnSetIp
        '
        resources.ApplyResources(Me.BtnSetIp, "BtnSetIp")
        Me.BtnSetIp.Name = "BtnSetIp"
        Me.BtnSetIp.UseVisualStyleBackColor = True
        '
        'TextBoxSetIp
        '
        resources.ApplyResources(Me.TextBoxSetIp, "TextBoxSetIp")
        Me.TextBoxSetIp.Name = "TextBoxSetIp"
        '
        'LabelIpInfo
        '
        resources.ApplyResources(Me.LabelIpInfo, "LabelIpInfo")
        Me.LabelIpInfo.Name = "LabelIpInfo"
        '
        'ForceIp
        '
        resources.ApplyResources(Me, "$this")
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.Controls.Add(Me.GroupBoxSetIp)
        Me.Controls.Add(Me.GroupBoxInit)
        Me.Controls.Add(Me.ComboBoxDeviceList)
        Me.Name = "ForceIp"
        Me.GroupBoxInit.ResumeLayout(False)
        Me.GroupBoxSetIp.ResumeLayout(False)
        Me.GroupBoxSetIp.PerformLayout()
        Me.ResumeLayout(False)

    End Sub
    Friend WithEvents GroupBoxInit As System.Windows.Forms.GroupBox
    Friend WithEvents ButtonEnumDevice As System.Windows.Forms.Button
    Friend WithEvents ComboBoxDeviceList As System.Windows.Forms.ComboBox
    Friend WithEvents GroupBoxSetIp As System.Windows.Forms.GroupBox
    Friend WithEvents BtnSetIp As System.Windows.Forms.Button
    Friend WithEvents TextBoxSetIp As System.Windows.Forms.TextBox
    Friend WithEvents LabelIpInfo As System.Windows.Forms.Label
    Friend WithEvents Label3 As System.Windows.Forms.Label
    Friend WithEvents Label2 As System.Windows.Forms.Label
    Friend WithEvents Label1 As System.Windows.Forms.Label
    Friend WithEvents TextBoxSetDefaultWay As System.Windows.Forms.TextBox
    Friend WithEvents TextBoxSetMask As System.Windows.Forms.TextBox
    Friend WithEvents Label4 As System.Windows.Forms.Label
End Class
