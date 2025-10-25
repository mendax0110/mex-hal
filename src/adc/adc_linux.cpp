#include "adc_linux.h"
#include <chrono>
#include <thread>

using namespace mex_hal;

ADCLinux::~ADCLinux()
{
    stopContinuous();
    
    std::lock_guard<std::mutex> lock(adcMutex_);
    if (resourceId_ != 0)
    {
        ResourceManager::getInstance().setInUse(resourceId_, false);
        ResourceManager::getInstance().unregisterResource(resourceId_);
        resourceId_ = 0;
    }
}

std::string ADCLinux::getDevicePath(const uint8_t channel) const
{
    return SYS_CLASS_IIO + std::to_string(device_) +
           "/in_voltage" + std::to_string(channel) + "_raw";
}

uint16_t ADCLinux::readRaw(const uint8_t channel) const
{
    std::lock_guard<std::mutex> lock(adcMutex_);
    
    const std::string path = getDevicePath(channel);
    std::ifstream file(path);
    
    if (!file.is_open())
    {
        return 0;
    }
    
    uint16_t value;
    file >> value;
    file.close();
    
    return value;
}

bool ADCLinux::init(uint8_t device, const ADCConfig& config)
{
    std::lock_guard<std::mutex> lock(adcMutex_);

    device_ = device;
    config_ = config;
    
    std::string devicePath = SYS_CLASS_IIO + std::to_string(device_);
    std::ifstream testFile(devicePath + "/name");

    if (!testFile.is_open())
    {
        return false;
    }

    // Register with resource manager
    std::string resourceName = "ADC" + std::to_string(device_);
    resourceId_ = ResourceManager::getInstance().registerResource(
        ResourceType::ADC_CHANNEL,
        resourceName,
        reinterpret_cast<void*>(static_cast<uintptr_t>(device_))
    );
    
    ResourceManager::getInstance().setInUse(resourceId_, true);
    
    return true;
}

bool ADCLinux::enableChannel(const uint8_t channel)
{
    std::lock_guard<std::mutex> lock(adcMutex_);

    const std::string scanEnablePath = SYS_CLASS_IIO + std::to_string(device_) +
                                  "/scan_elements/in_voltage" + std::to_string(channel) + "_en";
    
    std::ofstream file(scanEnablePath);
    if (!file.is_open()) return false;
    
    file << "1";
    file.close();
    return true;
}

bool ADCLinux::disableChannel(const uint8_t channel)
{
    std::lock_guard<std::mutex> lock(adcMutex_);

    const std::string scanEnablePath = SYS_CLASS_IIO + std::to_string(device_) +
                                  "/scan_elements/in_voltage" + std::to_string(channel) + "_en";
    
    std::ofstream file(scanEnablePath);
    if (!file.is_open()) return false;
    
    file << "0";
    file.close();
    return true;
}

uint16_t ADCLinux::read(const uint8_t channel)
{
    return readRaw(channel);
}

bool ADCLinux::readMultiple(const std::vector<uint8_t>& channels, std::vector<uint16_t>& values)
{
    values.clear();
    values.reserve(channels.size());
    
    for (const uint8_t channel : channels)
    {
        values.push_back(readRaw(channel));
    }
    
    return true;
}

void ADCLinux::continuousReadLoop()
{
    const uint64_t delayUs = config_.samplingRate > 0 ? (1000000 / config_.samplingRate) : 1000;
    
    while (!shouldStopContinuous_.load(std::memory_order_acquire))
    {
        const uint16_t value = readRaw(continuousChannel_);
        
        {
            std::lock_guard<std::mutex> lock(callbackMutex_);
            if (continuousCallback_)
            {
                continuousCallback_(value);
            }
        }
        
        std::this_thread::sleep_for(std::chrono::microseconds(delayUs));
    }
    
    continuousRunning_.store(false, std::memory_order_release);
}

bool ADCLinux::startContinuous(const uint8_t channel, const ADCReadCallback callback)
{
    if (continuousRunning_.load(std::memory_order_acquire))
    {
        return false;
    }
    
    continuousChannel_ = channel;
    
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        continuousCallback_ = callback;
    }
    
    shouldStopContinuous_.store(false, std::memory_order_release);
    continuousRunning_.store(true, std::memory_order_release);
    
    continuousThread_ = std::thread(&ADCLinux::continuousReadLoop, this);
    
    return true;
}

bool ADCLinux::stopContinuous()
{
    if (!continuousRunning_.load(std::memory_order_acquire))
    {
        return false;
    }
    
    shouldStopContinuous_.store(true, std::memory_order_release);
    
    if (continuousThread_.joinable())
    {
        continuousThread_.join();
    }
    
    continuousRunning_.store(false, std::memory_order_release);
    return true;
}

bool ADCLinux::setResolution(const ADCResolution resolution)
{
    std::lock_guard<std::mutex> lock(adcMutex_);
    config_.resolution = resolution;
    return true;
}

bool ADCLinux::setSamplingRate(const uint32_t samplingRate)
{
    std::lock_guard<std::mutex> lock(adcMutex_);

    std::string samplingFreqPath = SYS_CLASS_IIO + std::to_string(device_) +
                                    "/sampling_frequency";
    
    std::ofstream file(samplingFreqPath);
    if (!file.is_open()) return false;
    
    file << samplingRate;
    file.close();
    
    config_.samplingRate = samplingRate;
    return true;
}

float ADCLinux::readVoltage(const uint8_t channel, const float referenceVoltage)
{
    const uint16_t rawValue = readRaw(channel);
    
    std::lock_guard<std::mutex> lock(adcMutex_);
    const uint16_t maxValue = (1 << static_cast<int>(config_.resolution)) - 1;
    
    return (static_cast<float>(rawValue) / static_cast<float>(maxValue)) * referenceVoltage;
}
