# -*- mode: python ; coding: utf-8 -*-

import os

# 只使用64位DLL路径
dll_path = r'C:\Program Files (x86)\Common Files\MVS\Runtime\Win64_x64'

# 收集DLL文件
runtime_dlls = []
if os.path.exists(dll_path):
    for file in os.listdir(dll_path):
        if file.lower().endswith('.dll'):
            full_path = os.path.join(dll_path, file)
            runtime_dlls.append((full_path, '.'))  # 复制到主目录

a = Analysis(
    ['main.py'],
    pathex=[
        'Development/Libraries/win64',
        'Development/Samples/Python/MvImport',
        dll_path,  # 只添加64位路径
    ],
    binaries=runtime_dlls,  # 添加收集到的DLL
    datas=[
        ('Tools/camera_controller.py', 'Tools'),
        ('Tools/camera_thread.py', 'Tools'),
        ('Tools/settings_manager.py', 'Tools'),
        ('Tools/log_manager.py', 'Tools'),
        ('Tools/measurement_manager.py', 'Tools'),
        ('Tools/drawing_manager.py', 'Tools'),
        ('Tools/grid_container.py', 'Tools'),
        ('Development/MvCameraControl_class.py', 'Development'),
        ('Development/MvErrorDefine_const.py', 'Development'),
        ('Development/CameraParams_header.py', 'Development'),
        ('Development/PixelType_header.py', 'Development'),
        ('Development/CameraParams_const.py', 'Development'),
        ('Settings', 'Settings'),
        ('Logs', 'Logs'),
    ],
    hiddenimports=[
        'MvCameraControl_class',
        'MvErrorDefine_const',
        'CameraParams_header',
        'PixelType_header',
        'CameraParams_const',
        'cv2',
        'numpy',
        'PyQt5',
        'PyQt5.QtCore',
        'PyQt5.QtGui',
        'PyQt5.QtWidgets',
        'PyQt5.sip',
    ],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    win_no_prefer_redirects=False,
    win_private_assemblies=False,
    cipher=None,
    noarchive=False,
)
pyz = PYZ(a.pure, a.zipped_data)

exe = EXE(
    pyz,
    a.scripts,
    [],
    exclude_binaries=True,
    name='MutiCamApp',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    console=False,  # 改为False以隐藏控制台窗口
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
    #icon='app.ico',  # 如果有图标文件的话
)

coll = COLLECT(
    exe,
    a.binaries,
    a.datas,
    strip=False,
    upx=True,
    upx_exclude=[],
    name='MutiCamApp',
)
