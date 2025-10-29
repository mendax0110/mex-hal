#include "../include/hal/callback_manager.h"
#include <algorithm>

using namespace mex_hal;

CallbackManager &CallbackManager::getInstance()
{
    static CallbackManager instance;
    return instance;
}

uint64_t CallbackManager::registerGPIOCallback(const uint8_t pin, InterruptCallback callback)
{
    std::unique_lock<std::shared_mutex> lock(gpioCallbackMutex_);

    const uint64_t callbackId = nextCallbackId_.fetch_add(1, std::memory_order_relaxed);

    GPIOCallbackInfo info;
    info.pin = pin;
    info.callback = std::move(callback);

    gpioCallbacks_[callbackId] = std::move(info);
    gpioCallbacksByPin_[pin].push_back(callbackId);

    return callbackId;
}

bool CallbackManager::unregisterGPIOCallback(uint64_t callbackId)
{
    std::unique_lock<std::shared_mutex> lock(gpioCallbackMutex_);

    const auto it = gpioCallbacks_.find(callbackId);
    if (it == gpioCallbacks_.end())
    {
        return false;
    }

    const uint8_t pin = it->second.pin;
    gpioCallbacks_.erase(it);

    // Remove from pin-specific list
    auto &callbacks = gpioCallbacksByPin_[pin];
    callbacks.erase(std::remove(callbacks.begin(), callbacks.end(), callbackId), callbacks.end());

    if (callbacks.empty())
    {
        gpioCallbacksByPin_.erase(pin);
    }

    return true;
}

void CallbackManager::invokeGPIOCallback(const uint8_t pin, const PinValue value)
{
    std::shared_lock<std::shared_mutex> lock(gpioCallbackMutex_);

    const auto it = gpioCallbacksByPin_.find(pin);
    if (it == gpioCallbacksByPin_.end())
    {
        return;
    }

    // Create a copy of callback IDs to prevent deadlock if callback modifies callbacks
    const std::vector<uint64_t> callbackIds = it->second;
    lock.unlock();

    // Invoke callbacks without holding the lock
    for (uint64_t callbackId: callbackIds)
    {
        std::shared_lock<std::shared_mutex> callbackLock(gpioCallbackMutex_);
        auto callbackIt = gpioCallbacks_.find(callbackId);
        if (callbackIt != gpioCallbacks_.end() && callbackIt->second.callback)
        {
            // Copy the callback to invoke outside the lock
            auto callback = callbackIt->second.callback;
            callbackLock.unlock();
            callback(pin, value);
        }
    }
}

uint64_t CallbackManager::registerTimerCallback(const uint32_t timerId, TimerCallback callback)
{
    std::unique_lock<std::shared_mutex> lock(timerCallbackMutex_);

    const uint64_t callbackId = nextCallbackId_.fetch_add(1, std::memory_order_relaxed);

    TimerCallbackInfo info;
    info.timerId = timerId;
    info.callback = std::move(callback);

    timerCallbacks_[callbackId] = std::move(info);
    timerCallbacksById_[timerId].push_back(callbackId);

    return callbackId;
}

bool CallbackManager::unregisterTimerCallback(const uint64_t callbackId)
{
    std::unique_lock<std::shared_mutex> lock(timerCallbackMutex_);

    const auto it = timerCallbacks_.find(callbackId);
    if (it == timerCallbacks_.end())
    {
        return false;
    }

    const uint32_t timerId = it->second.timerId;
    timerCallbacks_.erase(it);

    // Remove from timer-specific list
    auto &callbacks = timerCallbacksById_[timerId];
    callbacks.erase(std::remove(callbacks.begin(), callbacks.end(), callbackId), callbacks.end());

    if (callbacks.empty())
    {
        timerCallbacksById_.erase(timerId);
    }

    return true;
}

void CallbackManager::invokeTimerCallback(const uint32_t timerId)
{
    std::shared_lock<std::shared_mutex> lock(timerCallbackMutex_);

    const auto it = timerCallbacksById_.find(timerId);
    if (it == timerCallbacksById_.end())
    {
        return;
    }

    // Create a copy of callback IDs to prevent deadlock
    const std::vector<uint64_t> callbackIds = it->second;
    lock.unlock();

    // Invoke callbacks without holding the lock
    for (uint64_t callbackId: callbackIds)
    {
        std::shared_lock<std::shared_mutex> callbackLock(timerCallbackMutex_);
        auto callbackIt = timerCallbacks_.find(callbackId);
        if (callbackIt != timerCallbacks_.end() && callbackIt->second.callback)
        {
            auto callback = callbackIt->second.callback;
            callbackLock.unlock();
            callback();
        }
    }
}

void CallbackManager::clearAll()
{
    {
        std::unique_lock<std::shared_mutex> lock(gpioCallbackMutex_);
        gpioCallbacks_.clear();
        gpioCallbacksByPin_.clear();
    }

    {
        std::unique_lock<std::shared_mutex> lock(timerCallbackMutex_);
        timerCallbacks_.clear();
        timerCallbacksById_.clear();
    }
}

