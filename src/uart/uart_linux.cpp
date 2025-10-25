#include "uart_linux.h"
#include <cstring>

using namespace mex_hal;

UARTLinux::~UARTLinux()
{
    std::lock_guard<std::mutex> lock(uartMutex_);
    
    if (resourceId_ != 0)
    {
        ResourceManager::getInstance().setInUse(resourceId_, false);
        ResourceManager::getInstance().unregisterResource(resourceId_);
        resourceId_ = 0;
    }
    
    fd_.close();
}

bool UARTLinux::init(const std::string& device, const UARTConfig& config)
{
    std::lock_guard<std::mutex> lock(uartMutex_);

    devicePath_ = device;
    const int fd = open(device.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    
    if (fd < 0)
    {
        return false;
    }
    
    fd_.reset(fd);
    fcntl(fd_.get(), F_SETFL, 0);

    // Register with resource manager
    resourceId_ = ResourceManager::getInstance().registerResource(
        ResourceType::UART_PORT,
        device,
        reinterpret_cast<void*>(static_cast<uintptr_t>(fd))
    );

    const bool result = configurePort(config);
    if (result)
    {
        ResourceManager::getInstance().setInUse(resourceId_, true);
    }
    
    return result;
}

bool UARTLinux::configurePort(const UARTConfig& config)
{
    if (!fd_.isValid()) return false;

    termios options{};
    if (tcgetattr(fd_.get(), &options) != 0)
    {
        return false;
    }
    
    speed_t baudRate;
    switch (config.baudRate)
    {
        case 9600: baudRate = B9600; break;
        case 19200: baudRate = B19200; break;
        case 38400: baudRate = B38400; break;
        case 57600: baudRate = B57600; break;
        case 115200: baudRate = B115200; break;
        case 230400: baudRate = B230400; break;
        case 460800: baudRate = B460800; break;
        case 500000: baudRate = B500000; break;
        case 576000: baudRate = B576000; break;
        case 921600: baudRate = B921600; break;
        case 1000000: baudRate = B1000000; break;
        case 1152000: baudRate = B1152000; break;
        case 1500000: baudRate = B1500000; break;
        case 2000000: baudRate = B2000000; break;
        case 2500000: baudRate = B2500000; break;
        case 3000000: baudRate = B3000000; break;
        case 3500000: baudRate = B3500000; break;
        case 4000000: baudRate = B4000000; break;
        default: baudRate = B115200; break;
    }
    
    cfsetispeed(&options, baudRate);
    cfsetospeed(&options, baudRate);
    
    options.c_cflag &= ~CSIZE;
    switch (config.dataBits)
    {
        case 5: options.c_cflag |= CS5; break;
        case 6: options.c_cflag |= CS6; break;
        case 7: options.c_cflag |= CS7; break;
        case 8: 
        default: options.c_cflag |= CS8; break;
    }
    
    if (config.stopBits == 2)
    {
        options.c_cflag |= CSTOPB;
    }
    else
    {
        options.c_cflag &= ~CSTOPB;
    }
    
    if (config.parityEnable)
    {
        options.c_cflag |= PARENB;
        if (!config.evenParity)
        {
            options.c_cflag |= PARODD;
        }
        else
        {
            options.c_cflag &= ~PARODD;
        }
    }
    else
    {
        options.c_cflag &= ~PARENB;
    }
    
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_oflag &= ~OPOST;
    
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 10;
    
    if (tcsetattr(fd_.get(), TCSANOW, &options) != 0)
    {
        return false;
    }
    
    currentConfig_ = config;
    return true;
}

bool UARTLinux::write(const std::vector<uint8_t>& data)
{
    std::lock_guard<std::mutex> lock(uartMutex_);

    if (!fd_.isValid() || data.empty()) return false;

    const ssize_t bytesWritten = ::write(fd_.get(), data.data(), data.size());
    return bytesWritten == static_cast<ssize_t>(data.size());
}

bool UARTLinux::read(std::vector<uint8_t>& data, size_t length)
{
    std::lock_guard<std::mutex> lock(uartMutex_);

    if (!fd_.isValid() || length == 0) return false;
    
    data.resize(length);
    const ssize_t bytesRead = ::read(fd_.get(), data.data(), length);

    if (bytesRead > 0)
    {
        data.resize(static_cast<size_t>(bytesRead));
        return true;
    }
    
    data.clear();
    return false;
}

size_t UARTLinux::available()
{
    std::lock_guard<std::mutex> lock(uartMutex_);

    if (!fd_.isValid()) return 0;
    
    int bytesAvailable = 0;
    if (ioctl(fd_.get(), FIONREAD, &bytesAvailable) < 0)
    {
        return 0;
    }
    
    return static_cast<size_t>(bytesAvailable);
}

bool UARTLinux::flush()
{
    std::lock_guard<std::mutex> lock(uartMutex_);

    if (!fd_.isValid()) return false;
    return tcflush(fd_.get(), TCIOFLUSH) == 0;
}

bool UARTLinux::setConfig(const UARTConfig& config)
{
    std::lock_guard<std::mutex> lock(uartMutex_);
    return configurePort(config);
}
