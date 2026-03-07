#include "uart_mock.h"

using namespace mex_hal;

bool UARTMock::init(const std::string& device, const UARTConfig& config)
{
    std::lock_guard<std::mutex> lock(mutex_);
    devicePath_ = device;
    config_ = config;
    initialized_ = true;
    return true;
}

bool UARTMock::write(const std::vector<uint8_t>& data)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_ || data.empty()) return false;
    txBuffer_.insert(txBuffer_.end(), data.begin(), data.end());
    return true;
}

bool UARTMock::read(std::vector<uint8_t>& data, const size_t length)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_ || length == 0) return false;

    size_t toRead = std::min(length, rxBuffer_.size());
    if (toRead == 0) return false;

    data.resize(toRead);
    for (size_t i = 0; i < toRead; ++i)
    {
        data[i] = rxBuffer_.front();
        rxBuffer_.pop_front();
    }
    return true;
}

size_t UARTMock::available()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) return 0;
    return rxBuffer_.size();
}

bool UARTMock::flush()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) return false;
    txBuffer_.clear();
    rxBuffer_.clear();
    return true;
}

bool UARTMock::setConfig(const UARTConfig& config)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) return false;
    config_ = config;
    return true;
}

void UARTMock::injectRxData(const std::vector<uint8_t>& data)
{
    std::lock_guard<std::mutex> lock(mutex_);
    rxBuffer_.insert(rxBuffer_.end(), data.begin(), data.end());
}

std::vector<uint8_t> UARTMock::getTxData() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return txBuffer_;
}

bool UARTMock::isInitialized() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return initialized_;
}
