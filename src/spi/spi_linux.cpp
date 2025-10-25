#include "spi_linux.h"

using namespace mex_hal;

SPILinux::~SPILinux()
{
    std::lock_guard<std::mutex> lock(spiMutex_);
    
    if (resourceId_ != 0)
    {
        ResourceManager::getInstance().setInUse(resourceId_, false);
        ResourceManager::getInstance().unregisterResource(resourceId_);
        resourceId_ = 0;
    }
    
    fd_.close();
}

bool SPILinux::init(const uint8_t bus, const uint8_t cs, uint32_t speed, SPIMode mode)
{
    std::lock_guard<std::mutex> lock(spiMutex_);

    const std::string devicePath = DEV_SPIDEV + std::to_string(bus) + "." + std::to_string(cs);

    const int fd = open(devicePath.c_str(), O_RDWR);
    if (fd < 0) return false;

    fd_.reset(fd);
    currentBus_ = bus;
    currentCS_ = cs;

    // Register with resource manager
    resourceId_ = ResourceManager::getInstance().registerResource(
        ResourceType::SPI_BUS,
        devicePath,
        reinterpret_cast<void*>(static_cast<uintptr_t>(fd))
    );

    auto spiMode = static_cast<uint8_t>(mode);
    if (ioctl(fd_.get(), SPI_IOC_WR_MODE, &spiMode) < 0) 
    {
        fd_.close();
        return false;
    }

    uint8_t bits = 8;
    if (ioctl(fd_.get(), SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) 
    {
        fd_.close();
        return false;
    }
    
    if (ioctl(fd_.get(), SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) 
    {
        fd_.close();
        return false;
    }

    ResourceManager::getInstance().setInUse(resourceId_, true);
    return true;
}

bool SPILinux::transfer(const std::vector<uint8_t> &txData, std::vector<uint8_t> &rxData)
{
    std::lock_guard<std::mutex> lock(spiMutex_);

    if (!fd_.isValid()) return false;

    rxData.resize(txData.size());

    spi_ioc_transfer tr = {
        .tx_buf = reinterpret_cast<uint64_t>(txData.data()),
        .rx_buf = reinterpret_cast<uint64_t>(rxData.data()),
        .len = static_cast<__u32>(txData.size()),
        .speed_hz = 0, // Use current speed
        .delay_usecs = 0,
        .bits_per_word = 8,
        .cs_change = 0,
        .pad = 0
    };

    return ioctl(fd_.get(), SPI_IOC_MESSAGE(1), &tr) >= 0;
}

bool SPILinux::write(const std::vector<uint8_t> &data)
{
    std::vector<uint8_t> dummy;
    return transfer(data, dummy);
}

bool SPILinux::read(std::vector<uint8_t> &data, size_t length)
{
    std::lock_guard<std::mutex> lock(spiMutex_);

    if (!fd_.isValid() || length == 0) return false;
    
    data.resize(length);
    const std::vector<uint8_t> dummy(length, 0);
    
    // Unlock before calling transfer to avoid deadlock
    spiMutex_.unlock();
    const bool result = transfer(dummy, data);
    spiMutex_.lock();
    
    return result;
}

bool SPILinux::setSpeed(uint32_t speed)
{
    std::lock_guard<std::mutex> lock(spiMutex_);

    if (!fd_.isValid()) return false;
    return ioctl(fd_.get(), SPI_IOC_WR_MAX_SPEED_HZ, &speed) >= 0;
}

bool SPILinux::setMode(SPIMode mode)
{
    std::lock_guard<std::mutex> lock(spiMutex_);

    if (!fd_.isValid()) return false;
    auto spiMode = static_cast<uint8_t>(mode);
    return ioctl(fd_.get(), SPI_IOC_WR_MODE, &spiMode) >= 0;
}