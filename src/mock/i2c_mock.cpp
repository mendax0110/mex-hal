#include "i2c_mock.h"

using namespace mex_hal;

bool I2CMock::init(const uint8_t bus)
{
    std::lock_guard<std::mutex> lock(mutex_);
    bus_ = bus;
    initialized_ = true;
    return true;
}

bool I2CMock::setDeviceAddress(const uint8_t address)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) return false;
    currentAddress_ = address;
    addressSet_ = true;
    return true;
}

bool I2CMock::write(const std::vector<uint8_t>& data)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_ || !addressSet_) return false;
    lastWritten_[currentAddress_] = data;
    return true;
}

bool I2CMock::read(std::vector<uint8_t>& data, const size_t length)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_ || !addressSet_) return false;

    auto it = deviceData_.find(currentAddress_);
    if (it != deviceData_.end() && !it->second.empty())
    {
        data = it->second.front();
        it->second.pop_front();
        data.resize(length, 0);
    }
    else
    {
        data.assign(length, 0);
    }
    return true;
}

bool I2CMock::writeRead(const uint8_t address, const std::vector<uint8_t>& writeData, std::vector<uint8_t>& readData)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) return false;

    currentAddress_ = address;
    addressSet_ = true;
    lastWritten_[address] = writeData;

    auto it = deviceData_.find(address);
    if (it != deviceData_.end() && !it->second.empty())
    {
        readData = it->second.front();
        it->second.pop_front();
    }
    else
    {
        readData.clear();
    }
    return true;
}

bool I2CMock::setSpeed(const uint32_t speed)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) return false;
    speed_ = speed;
    return true;
}

void I2CMock::enqueueReadData(const uint8_t address, const std::vector<uint8_t>& data)
{
    std::lock_guard<std::mutex> lock(mutex_);
    deviceData_[address].push_back(data);
}

std::vector<uint8_t> I2CMock::getLastWritten(const uint8_t address) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = lastWritten_.find(address);
    if (it != lastWritten_.end())
    {
        return it->second;
    }
    return {};
}

bool I2CMock::isInitialized() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return initialized_;
}
