<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> _
Partial Class SetIO
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
        Dim resources As System.ComponentModel.ComponentResourceManager = New System.ComponentModel.ComponentResourceManager(GetType(SetIO))
        Me.ButtonCloseDevice = New System.Windows.Forms.Button()
        Me.GroupBoxInit = New System.Windows.Forms.GroupBox()
        Me.ButtonOpenDevice = New System.Windows.Forms.Button()
        Me.ButtonEnumDevice = New System.Windows.Forms.Button()
        Me.ComboBoxDeviceList = New System.Windows.Forms.ComboBox()
        Me.GroupBoxSetIO = New System.Windows.Forms.GroupBox()
        Me.ButtonSetMode = New System.Windows.Forms.Button()
        Me.ButtonGetMode = New System.Windows.Forms.Button()
        Me.ButtonSetSelector = New System.Windows.Forms.Button()
        Me.ButtonGetSelector = New System.Windows.Forms.Button()
        Me.ComboBoxLineMode = New System.Windows.Forms.ComboBox()
        Me.ComboBoxLineSelector = New System.Windows.Forms.ComboBox()
        Me.LabelLineMode = New System.Windows.Forms.Label()
        Me.LabelLineSelector = New System.Windows.Forms.Label()
        Me.GroupBoxInit.SuspendLayout()
        Me.GroupBoxSetIO.SuspendLayout()
        Me.SuspendLayout()
        '
        'ButtonCloseDevice
        '
        resources.ApplyResources(Me.ButtonCloseDevice, "ButtonCloseDevice")
        Me.ButtonCloseDevice.Name = "ButtonCloseDevice"
        Me.ButtonCloseDevice.UseVisualStyleBackColor = True
        '
        'GroupBoxInit
        '
        resources.ApplyResources(Me.GroupBoxInit, "GroupBoxInit")
        Me.GroupBoxInit.Controls.Add(Me.ButtonCloseDevice)
        Me.GroupBoxInit.Controls.Add(Me.ButtonOpenDevice)
        Me.GroupBoxInit.Controls.Add(Me.ButtonEnumDevice)
        Me.GroupBoxInit.Name = "GroupBoxInit"
        Me.GroupBoxInit.TabStop = False
        '
        'ButtonOpenDevice
        '
        resources.ApplyResources(Me.ButtonOpenDevice, "ButtonOpenDevice")
        Me.ButtonOpenDevice.Name = "ButtonOpenDevice"
        Me.ButtonOpenDevice.UseVisualStyleBackColor = True
        '
        'ButtonEnumDevice
        '
        resources.ApplyResources(Me.ButtonEnumDevice, "ButtonEnumDevice")
        Me.ButtonEnumDevice.Name = "ButtonEnumDevice"
        Me.ButtonEnumDevice.UseVisualStyleBackColor = True
        '
        'ComboBoxDeviceList
        '
        resources.ApplyResources(Me.ComboBoxDeviceList, "ComboBoxDeviceList")
        Me.ComboBoxDeviceList.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList
        Me.ComboBoxDeviceList.FormattingEnabled = True
        Me.ComboBoxDeviceList.Name = "ComboBoxDeviceList"
        '
        'GroupBoxSetIO
        '
        resources.ApplyResources(Me.GroupBoxSetIO, "GroupBoxSetIO")
        Me.GroupBoxSetIO.Controls.Add(Me.ButtonSetMode)
        Me.GroupBoxSetIO.Controls.Add(Me.ButtonGetMode)
        Me.GroupBoxSetIO.Controls.Add(Me.ButtonSetSelector)
        Me.GroupBoxSetIO.Controls.Add(Me.ButtonGetSelector)
        Me.GroupBoxSetIO.Controls.Add(Me.ComboBoxLineMode)
        Me.GroupBoxSetIO.Controls.Add(Me.ComboBoxLineSelector)
        Me.GroupBoxSetIO.Controls.Add(Me.LabelLineMode)
        Me.GroupBoxSetIO.Controls.Add(Me.LabelLineSelector)
        Me.GroupBoxSetIO.Name = "GroupBoxSetIO"
        Me.GroupBoxSetIO.TabStop = False
        '
        'ButtonSetMode
        '
        resources.ApplyResources(Me.ButtonSetMode, "ButtonSetMode")
        Me.ButtonSetMode.Name = "ButtonSetMode"
        Me.ButtonSetMode.UseVisualStyleBackColor = True
        '
        'ButtonGetMode
        '
        resources.ApplyResources(Me.ButtonGetMode, "ButtonGetMode")
        Me.ButtonGetMode.Name = "ButtonGetMode"
        Me.ButtonGetMode.UseVisualStyleBackColor = True
        '
        'ButtonSetSelector
        '
        resources.ApplyResources(Me.ButtonSetSelector, "ButtonSetSelector")
        Me.ButtonSetSelector.Name = "ButtonSetSelector"
        Me.ButtonSetSelector.UseVisualStyleBackColor = True
        '
        'ButtonGetSelector
        '
        resources.ApplyResources(Me.ButtonGetSelector, "ButtonGetSelector")
        Me.ButtonGetSelector.Name = "ButtonGetSelector"
        Me.ButtonGetSelector.UseVisualStyleBackColor = True
        '
        'ComboBoxLineMode
        '
        resources.ApplyResources(Me.ComboBoxLineMode, "ComboBoxLineMode")
        Me.ComboBoxLineMode.FormattingEnabled = True
        Me.ComboBoxLineMode.Name = "ComboBoxLineMode"
        '
        'ComboBoxLineSelector
        '
        resources.ApplyResources(Me.ComboBoxLineSelector, "ComboBoxLineSelector")
        Me.ComboBoxLineSelector.FormattingEnabled = True
        Me.ComboBoxLineSelector.Name = "ComboBoxLineSelector"
        '
        'LabelLineMode
        '
        resources.ApplyResources(Me.LabelLineMode, "LabelLineMode")
        Me.LabelLineMode.Name = "LabelLineMode"
        '
        'LabelLineSelector
        '
        resources.ApplyResources(Me.LabelLineSelector, "LabelLineSelector")
        Me.LabelLineSelector.Name = "LabelLineSelector"
        '
        'SetIO
        '
        resources.ApplyResources(Me, "$this")
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.Controls.Add(Me.GroupBoxSetIO)
        Me.Controls.Add(Me.GroupBoxInit)
        Me.Controls.Add(Me.ComboBoxDeviceList)
        Me.Name = "SetIO"
        Me.GroupBoxInit.ResumeLayout(False)
        Me.GroupBoxSetIO.ResumeLayout(False)
        Me.GroupBoxSetIO.PerformLayout()
        Me.ResumeLayout(False)

    End Sub
    Friend WithEvents ButtonCloseDevice As System.Windows.Forms.Button
    Friend WithEvents GroupBoxInit As System.Windows.Forms.GroupBox
    Friend WithEvents ButtonOpenDevice As System.Windows.Forms.Button
    Friend WithEvents ButtonEnumDevice As System.Windows.Forms.Button
    Friend WithEvents ComboBoxDeviceList As System.Windows.Forms.ComboBox
    Friend WithEvents GroupBoxSetIO As System.Windows.Forms.GroupBox
    Friend WithEvents ComboBoxLineMode As System.Windows.Forms.ComboBox
    Friend WithEvents ComboBoxLineSelector As System.Windows.Forms.ComboBox
    Friend WithEvents LabelLineMode As System.Windows.Forms.Label
    Friend WithEvents LabelLineSelector As System.Windows.Forms.Label
    Friend WithEvents ButtonSetMode As System.Windows.Forms.Button
    Friend WithEvents ButtonGetMode As System.Windows.Forms.Button
    Friend WithEvents ButtonSetSelector As System.Windows.Forms.Button
    Friend WithEvents ButtonGetSelector As System.Windows.Forms.Button
End Class
