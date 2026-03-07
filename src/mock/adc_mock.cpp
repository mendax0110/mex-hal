#include "adc_mock.h"
#include <chrono>

using namespace mex_hal;

ADCMock::~ADCMock()
{
    stopContinuous();
}

bool ADCMock::init(const uint8_t device, const ADCConfig& config)
{
    std::lock_guard<std::mutex> lock(mutex_);
    device_ = device;
    config_ = config;
    initialized_ = true;
    return true;
}

bool ADCMock::enableChannel(const uint8_t channel)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) return false;
    enabledChannels_[channel] = true;
    return true;
}

bool ADCMock::disableChannel(uint8_t channel)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) return false;
    enabledChannels_[channel] = false;
    return true;
}

uint16_t ADCMock::read(const uint8_t channel)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = channelValues_.find(channel);
    if (it != channelValues_.end())
    {
        return it->second;
    }
    return 0;
}

bool ADCMock::readMultiple(const std::vector<uint8_t>& channels, std::vector<uint16_t>& values)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) return false;

    values.clear();
    values.reserve(channels.size());
    for (const uint8_t ch : channels)
    {
        auto it = channelValues_.find(ch);
        values.push_back((it != channelValues_.end()) ? it->second : 0);
    }
    return true;
}

bool ADCMock::startContinuous(const uint8_t channel, const ADCReadCallback callback)
{
    if (continuousRunning_.load()) return false;

    continuousChannel_ = channel;
    continuousCallback_ = callback;
    shouldStopContinuous_.store(false);
    continuousRunning_.store(true);

    continuousThread_ = std::thread([this]()
    {
        const uint64_t delayUs = config_.samplingRate > 0 ? (1000000 / config_.samplingRate) : 1000;
        while (!shouldStopContinuous_.load())
        {
            uint16_t value = read(continuousChannel_);
            if (continuousCallback_)
            {
                continuousCallback_(value);
            }
            std::this_thread::sleep_for(std::chrono::microseconds(delayUs));
        }
        continuousRunning_.store(false);
    });

    return true;
}

bool ADCMock::stopContinuous()
{
    if (!continuousRunning_.load()) return false;

    shouldStopContinuous_.store(true);
    if (continuousThread_.joinable())
    {
        continuousThread_.join();
    }
    continuousRunning_.store(false);
    return true;
}

bool ADCMock::setResolution(const ADCResolution resolution)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) return false;
    config_.resolution = resolution;
    return true;
}

bool ADCMock::setSamplingRate(const uint32_t samplingRate)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) return false;
    config_.samplingRate = samplingRate;
    return true;
}

float ADCMock::readVoltage(const uint8_t channel, const float referenceVoltage)
{
    const uint16_t raw = read(channel);
    const uint32_t maxVal = (1 << static_cast<int>(config_.resolution)) - 1;
    if (maxVal == 0) return 0.0f;
    return (static_cast<float>(raw) / static_cast<float>(maxVal)) * referenceVoltage;
}

void ADCMock::setChannelValue(const uint8_t channel, const uint16_t value)
{
    std::lock_guard<std::mutex> lock(mutex_);
    channelValues_[channel] = value;
}

bool ADCMock::isInitialized() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return initialized_;
}
