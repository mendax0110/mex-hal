#include "pwm_mock.h"

using namespace mex_hal;

bool PWMMock::init(const uint8_t chip, const uint8_t channel)
{
    std::lock_guard<std::mutex> lock(mutex_);
    chip_ = chip;
    channel_ = channel;
    initialized_ = true;
    return true;
}

bool PWMMock::enable(const bool shouldEnable)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) return false;
    enabled_ = shouldEnable;
    return true;
}

bool PWMMock::setPeriod(const uint32_t periodNs)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) return false;
    periodNs_ = periodNs;
    return true;
}

bool PWMMock::setDutyCycle(const uint32_t dutyCycleNs)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) return false;
    if (dutyCycleNs > periodNs_) return false;
    dutyCycleNs_ = dutyCycleNs;
    return true;
}

bool PWMMock::setDutyCyclePercent(const float percent)
{
    if (percent < 0.0f || percent > 100.0f) return false;

    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) return false;
    dutyCycleNs_ = static_cast<uint32_t>((periodNs_ * percent) / 100.0f);
    return true;
}

bool PWMMock::setPolarity(const bool invertPolarity)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) return false;
    invertedPolarity_ = invertPolarity;
    return true;
}

uint32_t PWMMock::getPeriod() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return periodNs_;
}

uint32_t PWMMock::getDutyCycle() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return dutyCycleNs_;
}

bool PWMMock::isEnabled() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return enabled_;
}

bool PWMMock::isInitialized() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return initialized_;
}
