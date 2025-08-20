#include "SerialController.h"
#include <QDebug>
#include <QThread>

SerialController::SerialController(QObject *parent)
    : QObject(parent)
    , m_status(SerialStatus::Disconnected)
    , m_isListening(false)
    , m_baudRate(9600)
    , m_listenTimer(nullptr)
{
#ifdef _WIN32
    m_serialHandle = INVALID_HANDLE_VALUE;
#else
    m_serialFd = -1;
#endif

    // 创建监听定时器
    m_listenTimer = new QTimer(this);
    m_listenTimer->setInterval(LISTEN_INTERVAL_MS);
    connect(m_listenTimer, &QTimer::timeout, this, &SerialController::processSerialData);

    qDebug() << "SerialController initialized";
}

SerialController::~SerialController()
{
    closePort();
}

bool SerialController::openPort(const QString& portName, int baudRate)
{
    QMutexLocker locker(&m_mutex);

    // 如果已经打开，先关闭
    if (m_status != SerialStatus::Disconnected) {
        closePort();
    }

    m_portName = portName;
    m_baudRate = baudRate;

#ifdef _WIN32
    // Windows串口打开
    QString fullPortName = "\\\\.\\" + portName;
    m_serialHandle = CreateFileA(fullPortName.toLocal8Bit().constData(),
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                nullptr,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                nullptr);

    if (m_serialHandle == INVALID_HANDLE_VALUE) {
        setError(QString("无法打开串口 %1: %2").arg(portName).arg(GetLastError()));
        return false;
    }

    // 配置串口参数
    if (!configurePortWindows(baudRate)) {
        CloseHandle(m_serialHandle);
        m_serialHandle = INVALID_HANDLE_VALUE;
        return false;
    }

#else
    // Linux串口打开（非阻塞模式）
    m_serialFd = open(portName.toLocal8Bit().constData(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (m_serialFd < 0) {
        setError(QString("无法打开串口 %1: %2").arg(portName).arg(strerror(errno)));
        return false;
    }

    // 配置串口参数
    if (!configurePortLinux(baudRate)) {
        close(m_serialFd);
        m_serialFd = -1;
        return false;
    }
#endif

    setStatus(SerialStatus::Connected);
    qDebug() << "串口" << portName << "打开成功，波特率:" << baudRate;
    return true;
}

void SerialController::closePort()
{
    QMutexLocker locker(&m_mutex);

    // 停止监听
    stopListening();

#ifdef _WIN32
    if (m_serialHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_serialHandle);
        m_serialHandle = INVALID_HANDLE_VALUE;
    }
#else
    if (m_serialFd >= 0) {
        close(m_serialFd);
        m_serialFd = -1;
    }
#endif

    setStatus(SerialStatus::Disconnected);
    m_dataBuffer.clear();
    qDebug() << "串口已关闭";
}

void SerialController::startListening()
{
    if (m_status != SerialStatus::Connected) {
        setError("串口未连接，无法开始监听");
        return;
    }

    if (m_isListening) {
        qDebug() << "串口监听已在运行中";
        return;
    }

    m_isListening = true;
    m_listenTimer->start();
    setStatus(SerialStatus::Listening);
    qDebug() << "开始监听串口数据...";
}

void SerialController::stopListening()
{
    if (!m_isListening) {
        return;
    }

    m_isListening = false;
    if (m_listenTimer) {
        m_listenTimer->stop();
    }

    if (m_status == SerialStatus::Listening) {
        setStatus(SerialStatus::Connected);
    }
    qDebug() << "停止监听串口数据";
}

void SerialController::processSerialData()
{
    if (!m_isListening || m_status != SerialStatus::Listening) {
        return;
    }

    char buffer[BUFFER_SIZE];
    int bytesRead = 0;

#ifdef _WIN32
    DWORD dwBytesRead = 0;
    if (ReadFile(m_serialHandle, buffer, BUFFER_SIZE - 1, &dwBytesRead, nullptr)) {
        bytesRead = static_cast<int>(dwBytesRead);
    } else {
        DWORD error = GetLastError();
        if (error != ERROR_TIMEOUT) {
            setError(QString("读取串口数据失败: %1").arg(error));
            setStatus(SerialStatus::Error);
        }
    }
#else
    bytesRead = read(m_serialFd, buffer, BUFFER_SIZE - 1);
    if (bytesRead < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            setError(QString("读取串口数据失败: %1").arg(strerror(errno)));
            setStatus(SerialStatus::Error);
        }
        bytesRead = 0;
    }
#endif

    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        QByteArray data(buffer, bytesRead);
        
        // 发射原始数据信号
        emit dataReceived(data);
        
        // 解析按钮事件
        ButtonEvent event = parseButtonEvent(data);
        if (event != ButtonEvent::Unknown) {
            emit buttonEventReceived(event, data);
            qDebug() << "检测到按钮事件:" << static_cast<int>(event);
        }
    }
}

SerialController::ButtonEvent SerialController::parseButtonEvent(const QByteArray& data)
{
    // 解析按钮事件的逻辑
    // 基于serial_test的实现，检测特定字符
    for (char c : data) {
        switch (c) {
            case 'T':
            case 't':
                return ButtonEvent::Button1Pressed;  // 拍照按钮
            case '1':
                return ButtonEvent::Button1Pressed;
            case '2':
                return ButtonEvent::Button2Pressed;
            case '3':
                return ButtonEvent::Button3Pressed;
            case 'E':
            case 'e':
                return ButtonEvent::EmergencyStop;   // 急停按钮
            case 'R':
            case 'r':
                return ButtonEvent::Button1Released; // 释放事件
            default:
                break;
        }
    }
    return ButtonEvent::Unknown;
}

void SerialController::setStatus(SerialStatus status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged(status);
    }
}

void SerialController::setError(const QString& error)
{
    m_lastError = error;
    qDebug() << "SerialController错误:" << error;
    emit errorOccurred(error);
}

#ifdef _WIN32
bool SerialController::configurePortWindows(int baudRate)
{
    // 配置串口参数
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    
    if (!GetCommState(m_serialHandle, &dcbSerialParams)) {
        setError("无法获取串口状态");
        return false;
    }

    dcbSerialParams.BaudRate = baudRate;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(m_serialHandle, &dcbSerialParams)) {
        setError("无法设置串口参数");
        return false;
    }

    // 设置非阻塞超时，立即返回
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = MAXDWORD;  // 立即返回
    timeouts.ReadTotalTimeoutConstant = 0;    // 不等待
    timeouts.ReadTotalTimeoutMultiplier = 0;  // 不等待
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(m_serialHandle, &timeouts)) {
        setError("无法设置串口超时");
        return false;
    }

    return true;
}
#else
bool SerialController::configurePortLinux(int baudRate)
{
    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    
    if (tcgetattr(m_serialFd, &tty) != 0) {
        setError(QString("无法获取串口属性: %1").arg(strerror(errno)));
        return false;
    }

    // 设置波特率
    speed_t baud;
    switch (baudRate) {
        case 9600: baud = B9600; break;
        case 19200: baud = B19200; break;
        case 38400: baud = B38400; break;
        case 57600: baud = B57600; break;
        case 115200: baud = B115200; break;
        default: baud = B9600; break;
    }

    cfsetospeed(&tty, baud);
    cfsetispeed(&tty, baud);

    // 8数据位，无奇偶校验，1停止位
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag |= CREAD | CLOCAL;

    // 原始输入模式
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    // 原始输出模式
    tty.c_oflag &= ~OPOST;

    // 设置非阻塞读取
    tty.c_cc[VMIN] = 0;   // 不等待最小字符数
    tty.c_cc[VTIME] = 0;  // 立即返回

    if (tcsetattr(m_serialFd, TCSANOW, &tty) != 0) {
        setError(QString("无法设置串口属性: %1").arg(strerror(errno)));
        return false;
    }

    return true;
}
#endif
