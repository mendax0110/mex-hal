#include "gpio_mock.h"

using namespace mex_hal;

bool GPIOMock::setDirection(const uint8_t pin, const PinDirection direction)
{
    std::lock_guard<std::mutex> lock(mutex_);
    pins_[pin].direction = direction;
    pins_[pin].exported = true;
    return true;
}

bool GPIOMock::write(const uint8_t pin, const PinValue value)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = pins_.find(pin);
    if (it == pins_.end() || !it->second.exported)
    {
        return false;
    }
    if (it->second.direction != PinDirection::OUTPUT)
    {
        return false;
    }
    it->second.value = value;
    return true;
}

PinValue GPIOMock::read(const uint8_t pin)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = pins_.find(pin);
    if (it == pins_.end() || !it->second.exported)
    {
        return PinValue::LOW;
    }
    return it->second.value;
}

bool GPIOMock::setInterrupt(const uint8_t pin, const EdgeTrigger edge, InterruptCallback callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = pins_.find(pin);
    if (it == pins_.end())
    {
        pins_[pin].exported = true;
        pins_[pin].direction = PinDirection::INPUT;
    }
    pins_[pin].interruptCb = std::move(callback);
    pins_[pin].edge = edge;
    return true;
}

bool GPIOMock::removeInterrupt(const uint8_t pin)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = pins_.find(pin);
    if (it == pins_.end())
    {
        return false;
    }
    it->second.interruptCb = nullptr;
    return true;
}

bool GPIOMock::setDebounce(const uint8_t pin, const uint32_t debounceTimeMs)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = pins_.find(pin);
    if (it == pins_.end() || !it->second.exported)
    {
        return false;
    }
    it->second.debounceMs = debounceTimeMs;
    return true;
}

void GPIOMock::simulateInterrupt(const uint8_t pin, const PinValue value)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = pins_.find(pin);
    if (it != pins_.end() && it->second.interruptCb)
    {
        it->second.value = value;
        it->second.interruptCb(pin, value);
    }
}

PinValue GPIOMock::getStoredValue(const uint8_t pin) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = pins_.find(pin);
    if (it == pins_.end())
    {
        return PinValue::LOW;
    }
    return it->second.value;
}

bool GPIOMock::isExported(const uint8_t pin) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = pins_.find(pin);
    return it != pins_.end() && it->second.exported;
}

PinDirection GPIOMock::getDirection(const uint8_t pin) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = pins_.find(pin);
    if (it == pins_.end())
    {
        return PinDirection::INPUT;
    }
    return it->second.direction;
}
