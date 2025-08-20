# 串口测试程序

一个跨平台的串口通信测试程序，支持 Windows 和 Linux 系统。

## 功能特性

- **串口通信**: 支持串口数据的读取和监听
- **触发机制**: 接收到字符 'T' 或 't' 时自动触发拍照操作
- **跨平台**: 同时支持 Windows 和 Linux 系统
- **线程安全**: 使用多线程设计，确保数据处理的安全性
- **错误处理**: 完善的错误处理机制，提供详细的错误信息

## 快速开始

### 编译程序
```bash
build_and_run.bat
```

### 运行程序
```bash
run_test.bat
```

或者直接运行编译后的可执行文件：
```bash
build_serial\bin\Release\serial_test.exe
```

## 配置参数

- **串口**: COM5
- **波特率**: 9600
- **数据位**: 8位
- **奇偶校验**: 无
- **停止位**: 1位

## 程序工作流程

1. 打开指定的串口（COM5）
2. 配置串口参数（9600波特率等）
3. 启动监听线程，持续接收串口数据
4. 当接收到触发字符（'T' 或 't'）时，执行拍照操作
5. 显示接收到的所有数据（包括非可见字符的十六进制表示）
6. 按 Enter 键退出程序

## 项目文件

- `serial_test.cpp`: 主程序源文件
- `CMakeLists.txt`: CMake 构建配置文件
- `build_and_run.bat`: 编译和运行脚本
- `run_test.bat`: 简化的运行脚本
- `build_serial/`: 构建输出目录

## 系统要求

### Windows
- Windows 7 或更高版本
- Visual Studio 2017 或更高版本（或 Visual Studio Build Tools）
- CMake 3.10 或更高版本

### Linux
- 支持 C++11 的编译器（GCC 4.8+ 或 Clang 3.3+）
- CMake 3.10 或更高版本
- pthread 库

## 注意事项

1. **权限要求**: 程序可能需要管理员权限才能访问串口
2. **端口可用性**: 确保 COM5 端口可用且未被其他程序占用
3. **Linux 用户组**: 在 Linux 系统下，可能需要将用户添加到 `dialout` 组：
   ```bash
   sudo usermod -a -G dialout $USER
   ```
4. **串口设备**: 如需使用其他串口，请修改源代码中的端口名称

## 故障排除

### 编译错误
- 确保已安装 Visual Studio 和 CMake
- 检查路径中是否包含中文字符
- 尝试使用管理员权限运行

### 运行时错误
- 检查串口是否被其他程序占用
- 确认串口设备是否正确连接
- 在 Linux 下检查用户权限设置

## 技术实现

- **语言**: C++11
- **构建系统**: CMake
- **线程库**: std::thread
- **同步机制**: std::mutex, std::atomic
- **串口API**: Windows API (Windows) / termios (Linux) 