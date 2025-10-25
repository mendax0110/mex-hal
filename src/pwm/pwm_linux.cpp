#include "pwm_linux.h"
#include <thread>
#include <chrono>

using namespace mex_hal;

PWMLinux::~PWMLinux()
{
    std::lock_guard<std::mutex> lock(pwmMutex_);
    
    if (enabled_.load(std::memory_order_acquire))
    {
        writeSysfs("enable", "0");
    }
    
    if (resourceId_ != 0)
    {
        ResourceManager::getInstance().setInUse(resourceId_, false);
        ResourceManager::getInstance().unregisterResource(resourceId_);
        resourceId_ = 0;
    }
    
    unexportPWM();
}

std::string PWMLinux::getBasePath() const
{
    return SYS_CLASS_PWM + std::to_string(chip_) + "/pwm" + std::to_string(channel_);
}

bool PWMLinux::exportPWM() const
{
    const std::string exportPath = SYS_CLASS_PWM + std::to_string(chip_) + "/export";
    std::ofstream exportFile(exportPath);
    if (!exportFile.is_open()) return false;
    exportFile << static_cast<int>(channel_);
    exportFile.close();
    
    // Give kernel time to create PWM files
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    return true;
}

bool PWMLinux::unexportPWM() const
{
    const std::string unexportPath = SYS_CLASS_PWM + std::to_string(chip_) + "/unexport";
    std::ofstream unexportFile(unexportPath);
    if (!unexportFile.is_open()) return false;
    unexportFile << static_cast<int>(channel_);
    unexportFile.close();
    return true;
}

bool PWMLinux::writeSysfs(const std::string& attribute, const std::string& value) const
{
    const std::string path = getBasePath() + "/" + attribute;
    std::ofstream file(path);
    if (!file.is_open()) return false;
    file << value;
    file.close();
    return true;
}

std::string PWMLinux::readSysfs(const std::string& attribute) const
{
    const std::string path = getBasePath() + "/" + attribute;
    std::ifstream file(path);
    if (!file.is_open()) return "";
    std::string value;
    file >> value;
    file.close();
    return value;
}

bool PWMLinux::init(const uint8_t chipNum, const uint8_t channelNum)
{
    std::lock_guard<std::mutex> lock(pwmMutex_);

    chip_ = chipNum;
    channel_ = channelNum;
    
    if (!exportPWM())
    {
        return false;
    }

    // Register with resource manager
    const std::string resourceName = "PWM" + std::to_string(chip_) + "." + std::to_string(channel_);
    resourceId_ = ResourceManager::getInstance().registerResource(
        ResourceType::PWM_CHANNEL,
        resourceName,
        reinterpret_cast<void*>(static_cast<uintptr_t>((chip_ << 8) | channel_))
    );
    
    ResourceManager::getInstance().setInUse(resourceId_, true);
    
    return true;
}

bool PWMLinux::enable(const bool shouldEnable)
{
    std::lock_guard<std::mutex> lock(pwmMutex_);

    if (writeSysfs("enable", shouldEnable ? "1" : "0"))
    {
        enabled_.store(shouldEnable, std::memory_order_release);
        return true;
    }
    return false;
}

bool PWMLinux::setPeriod(const uint32_t period)
{
    std::lock_guard<std::mutex> lock(pwmMutex_);

    const bool wasEnabled = enabled_.load(std::memory_order_acquire);
    if (wasEnabled)
    {
        writeSysfs("enable", "0");
    }
    
    if (writeSysfs("period", std::to_string(period)))
    {
        periodNs_.store(period, std::memory_order_release);
        
        if (wasEnabled)
        {
            writeSysfs("enable", "1");
        }
        return true;
    }
    
    if (wasEnabled)
    {
        writeSysfs("enable", "1");
    }
    
    return false;
}

bool PWMLinux::setDutyCycle(const uint32_t dutyCycle)
{
    std::lock_guard<std::mutex> lock(pwmMutex_);

    const uint32_t period = periodNs_.load(std::memory_order_acquire);
    if (dutyCycle > period)
    {
        return false;
    }
    
    if (writeSysfs("duty_cycle", std::to_string(dutyCycle)))
    {
        dutyCycleNs_.store(dutyCycle, std::memory_order_release);
        return true;
    }
    return false;
}

bool PWMLinux::setDutyCyclePercent(const float percent)
{
    if (percent < 0.0f || percent > 100.0f)
    {
        return false;
    }

    const uint32_t period = periodNs_.load(std::memory_order_acquire);
    const auto dutyCycle = static_cast<uint32_t>((period * percent) / 100.0f);
    return setDutyCycle(dutyCycle);
}

bool PWMLinux::setPolarity(const bool invertPolarity)
{
    std::lock_guard<std::mutex> lock(pwmMutex_);

    const bool wasEnabled = enabled_.load(std::memory_order_acquire);
    if (wasEnabled)
    {
        writeSysfs("enable", "0");
    }

    const bool result = writeSysfs("polarity", invertPolarity ? "inversed" : "normal");
    
    if (wasEnabled)
    {
        writeSysfs("enable", "1");
    }
    
    return result;
}

uint32_t PWMLinux::getPeriod() const
{
    return periodNs_.load(std::memory_order_acquire);
}

uint32_t PWMLinux::getDutyCycle() const
{
    return dutyCycleNs_.load(std::memory_order_acquire);
}

bool PWMLinux::isEnabled() const
{
    return enabled_.load(std::memory_order_acquire);
}
