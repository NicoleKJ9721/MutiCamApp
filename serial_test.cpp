#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <string>
#include <vector>
#include <mutex>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#endif

class SerialPort {
private:
#ifdef _WIN32
    HANDLE hSerial;
#else
    int fd;
#endif
    std::atomic<bool> isRunning;
    std::thread listenerThread;
    std::mutex printMutex;

public:
    SerialPort() : isRunning(false) {
#ifdef _WIN32
        hSerial = INVALID_HANDLE_VALUE;
#else
        fd = -1;
#endif
    }

    ~SerialPort() {
        closePort();
    }

    bool openPort(const std::string& portName, int baudRate) {
#ifdef _WIN32
        // Windows serial port open
        std::string fullPortName = "\\\\.\\" + portName;
        hSerial = CreateFileA(fullPortName.c_str(),
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            0,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            0);

        if (hSerial == INVALID_HANDLE_VALUE) {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "Error: Cannot open serial port " << portName << std::endl;
            return false;
        }

        // Configure serial port parameters
        DCB dcbSerialParams = {0};
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
        
        if (!GetCommState(hSerial, &dcbSerialParams)) {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "Error: Cannot get serial port state" << std::endl;
            CloseHandle(hSerial);
            hSerial = INVALID_HANDLE_VALUE;
            return false;
        }

        dcbSerialParams.BaudRate = baudRate;
        dcbSerialParams.ByteSize = 8;
        dcbSerialParams.StopBits = ONESTOPBIT;
        dcbSerialParams.Parity = NOPARITY;

        if (!SetCommState(hSerial, &dcbSerialParams)) {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "Error: Cannot set serial port parameters" << std::endl;
            CloseHandle(hSerial);
            hSerial = INVALID_HANDLE_VALUE;
            return false;
        }

        // Set timeouts
        COMMTIMEOUTS timeouts = {0};
        timeouts.ReadIntervalTimeout = 50;
        timeouts.ReadTotalTimeoutConstant = 50;
        timeouts.ReadTotalTimeoutMultiplier = 10;
        timeouts.WriteTotalTimeoutConstant = 50;
        timeouts.WriteTotalTimeoutMultiplier = 10;

        if (!SetCommTimeouts(hSerial, &timeouts)) {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "Error: Cannot set serial port timeouts" << std::endl;
            CloseHandle(hSerial);
            hSerial = INVALID_HANDLE_VALUE;
            return false;
        }

#else
        // Linux serial port open
        fd = open(portName.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
        if (fd < 0) {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "Error: Cannot open serial port " << portName << " (error: " << strerror(errno) << ")" << std::endl;
            return false;
        }

        // Configure serial port parameters
        struct termios tty;
        memset(&tty, 0, sizeof(tty));
        
        if (tcgetattr(fd, &tty) != 0) {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "Error: Cannot get serial port attributes (error: " << strerror(errno) << ")" << std::endl;
            close(fd);
            fd = -1;
            return false;
        }

        // Set baud rate
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

        // 8 data bits, no parity, 1 stop bit
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
        tty.c_cflag &= ~(PARENB | PARODD);
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;
        tty.c_cflag |= CREAD | CLOCAL;

        // Raw input mode
        tty.c_iflag &= ~(IXON | IXOFF | IXANY);
        tty.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);

        // Raw output mode
        tty.c_oflag &= ~OPOST;

        // Set timeouts
        tty.c_cc[VMIN] = 0;
        tty.c_cc[VTIME] = 5;

        if (tcsetattr(fd, TCSANOW, &tty) != 0) {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "Error: Cannot set serial port attributes (error: " << strerror(errno) << ")" << std::endl;
            close(fd);
            fd = -1;
            return false;
        }
#endif

        std::lock_guard<std::mutex> lock(printMutex);
        std::cout << "Serial port " << portName << " opened successfully, baud rate: " << baudRate << std::endl;
        return true;
    }

    void closePort() {
        isRunning = false;
        
        if (listenerThread.joinable()) {
            listenerThread.join();
        }

#ifdef _WIN32
        if (hSerial != INVALID_HANDLE_VALUE) {
            CloseHandle(hSerial);
            hSerial = INVALID_HANDLE_VALUE;
        }
#else
        if (fd >= 0) {
            close(fd);
            fd = -1;
        }
#endif

        std::lock_guard<std::mutex> lock(printMutex);
        std::cout << "Serial port closed" << std::endl;
    }

    void startListening() {
        if (isRunning) {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "Listener thread is already running" << std::endl;
            return;
        }

        isRunning = true;
        listenerThread = std::thread(&SerialPort::listenerThreadFunc, this);
        
        std::lock_guard<std::mutex> lock(printMutex);
        std::cout << "Started listening for serial data..." << std::endl;
    }

private:
    void listenerThreadFunc() {
        char buffer[256];
        
        while (isRunning) {
            int bytesRead = 0;
            
#ifdef _WIN32
            DWORD dwBytesRead = 0;
            if (ReadFile(hSerial, buffer, sizeof(buffer) - 1, &dwBytesRead, NULL)) {
                bytesRead = static_cast<int>(dwBytesRead);
            } else {
                DWORD error = GetLastError();
                if (error != ERROR_TIMEOUT) {
                    std::lock_guard<std::mutex> lock(printMutex);
                    std::cout << "Error reading serial data: " << error << std::endl;
                }
            }
#else
            bytesRead = read(fd, buffer, sizeof(buffer) - 1);
            if (bytesRead < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    std::lock_guard<std::mutex> lock(printMutex);
                    std::cout << "Error reading serial data: " << strerror(errno) << std::endl;
                }
                bytesRead = 0;
            }
#endif

            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                
                // Process received data
                for (int i = 0; i < bytesRead; i++) {
                    if (buffer[i] == 'T' || buffer[i] == 't') {
                        takePhoto();
                    }
                }
                
                std::lock_guard<std::mutex> lock(printMutex);
                std::cout << "Received data: ";
                for (int i = 0; i < bytesRead; i++) {
                    if (buffer[i] >= 32 && buffer[i] <= 126) {
                        std::cout << buffer[i];
                    } else {
                        std::cout << "\\x" << std::hex << (int)(unsigned char)buffer[i] << std::dec;
                    }
                }
                std::cout << std::endl;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    void takePhoto() {
        std::lock_guard<std::mutex> lock(printMutex);
        std::cout << "=== PHOTO TRIGGERED ===" << std::endl;
        std::cout << "Timestamp: " << std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count() << std::endl;
        std::cout << "Photo taken!" << std::endl;
        std::cout << "=====================" << std::endl;
    }
};

int main() {
    std::cout << "Serial Test Program Started" << std::endl;
    std::cout << "Configuration: COM5, 9600 baud, 8 data bits, no parity, 1 stop bit" << std::endl;
    std::cout << "When receiving character 'T' or 't', photo will be triggered" << std::endl;
    std::cout << "Press Enter to exit..." << std::endl;

    SerialPort serialPort;
    
    // Open serial port
    if (!serialPort.openPort("COM5", 9600)) {
        std::cout << "Failed to open serial port, exiting" << std::endl;
        return -1;
    }

    // Start listening
    serialPort.startListening();

    // Wait for user input to exit
    std::cin.get();

    std::cout << "Closing program..." << std::endl;
    serialPort.closePort();
    
    std::cout << "Program exited" << std::endl;
    return 0;
} 