#include "i2c_linux.h"

#include <fstream>

using namespace mex_hal;

I2CLinux::~I2CLinux()
{
    std::lock_guard<std::mutex> lock(i2cMutex_);
    
    if (resourceId_ != 0)
    {
        ResourceManager::getInstance().setInUse(resourceId_, false);
        ResourceManager::getInstance().unregisterResource(resourceId_);
        resourceId_ = 0;
    }
    
    fd_.close();
}

bool I2CLinux::init(const uint8_t bus)
{
    std::lock_guard<std::mutex> lock(i2cMutex_);

    const std::string devicePath = "/dev/i2c-" + std::to_string(bus);
    const int fd = open(devicePath.c_str(), O_RDWR);
    if (fd < 0) return false;

    fd_.reset(fd);
    currentBus_ = bus;

    // Register with resource manager
    resourceId_ = ResourceManager::getInstance().registerResource(
        ResourceType::I2C_BUS,
        devicePath,
        reinterpret_cast<void*>(static_cast<uintptr_t>(fd))
    );

    ResourceManager::getInstance().setInUse(resourceId_, true);
    return true;
}

bool I2CLinux::setDeviceAddress(const uint8_t address)
{
    std::lock_guard<std::mutex> lock(i2cMutex_);

    if (!fd_.isValid()) return false;
    if (ioctl(fd_.get(), I2C_SLAVE, address) < 0) return false;
    currentAddress_ = address;
    return true;
}

bool I2CLinux::write(const std::vector<uint8_t>& data)
{
    std::lock_guard<std::mutex> lock(i2cMutex_);

    if (!fd_.isValid() || currentAddress_ == 0) return false;
    return ::write(fd_.get(), data.data(), data.size()) == static_cast<ssize_t>(data.size());
}

bool I2CLinux::read(std::vector<uint8_t>& data, const size_t length)
{
    std::lock_guard<std::mutex> lock(i2cMutex_);

    if (!fd_.isValid() || currentAddress_ == 0) return false;
    data.resize(length);
    const ssize_t bytesRead = ::read(fd_.get(), data.data(), length);
    return bytesRead == static_cast<ssize_t>(length);
}

bool I2CLinux::writeRead(const uint8_t address, const std::vector<uint8_t> &writeData, std::vector<uint8_t> &readData)
{
    if (!setDeviceAddress(address)) return false;
    if (!write(writeData)) return false;
    return read(readData, writeData.size());
}

bool I2CLinux::setSpeed(const uint32_t speed)
{
    std::lock_guard<std::mutex> lock(i2cMutex_);

    if (!fd_.isValid()) return false;
    const std::string filePath = SYS_CALL_I2C_ADAPTERS + std::to_string(currentBus_) + "/speed";
    std::ofstream speedFile(filePath);
    if (!speedFile.is_open()) return false;
    speedFile << speed;
    speedFile.close();
    return true;
}

