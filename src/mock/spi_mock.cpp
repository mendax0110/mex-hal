#include "spi_mock.h"

using namespace mex_hal;

bool SPIMock::init(const uint8_t bus, const uint8_t cs, const uint32_t speed, const SPIMode mode)
{
    std::lock_guard<std::mutex> lock(mutex_);
    bus_ = bus;
    cs_ = cs;
    speed_ = speed;
    mode_ = mode;
    initialized_ = true;
    return true;
}

bool SPIMock::transfer(const std::vector<uint8_t>& txData, std::vector<uint8_t>& rxData)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) return false;

    if (!rxQueue_.empty())
    {
        rxData = rxQueue_.front();
        rxQueue_.pop_front();
        rxData.resize(txData.size(), 0);
    }
    else
    {
        rxData.assign(txData.size(), 0xFF);
    }
    return true;
}

bool SPIMock::write(const std::vector<uint8_t>& data)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) return false;
    return true;
}

bool SPIMock::read(std::vector<uint8_t>& data, const size_t length)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_ || length == 0) return false;

    if (!rxQueue_.empty())
    {
        data = rxQueue_.front();
        rxQueue_.pop_front();
        data.resize(length, 0);
    }
    else
    {
        data.assign(length, 0xFF);
    }
    return true;
}

bool SPIMock::setSpeed(const uint32_t speed)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) return false;
    speed_ = speed;
    return true;
}

bool SPIMock::setMode(const SPIMode mode)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) return false;
    mode_ = mode;
    return true;
}

void SPIMock::enqueueRxData(const std::vector<uint8_t>& data)
{
    std::lock_guard<std::mutex> lock(mutex_);
    rxQueue_.push_back(data);
}

bool SPIMock::isInitialized() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return initialized_;
}

uint32_t SPIMock::getSpeed() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return speed_;
}

SPIMode SPIMock::getMode() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return mode_;
}
