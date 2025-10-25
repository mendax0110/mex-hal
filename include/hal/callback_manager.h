#ifndef MEX_HAL_CALLBACK_MANAGER_H
#define MEX_HAL_CALLBACK_MANAGER_H

#include "types.h"
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <shared_mutex>

namespace mex_hal
{
    /**
     * @brief Thread-safe callback manager for handling asynchronous events
     * 
     * Provides thread-safe registration, unregistration, and invocation
     * of callbacks for various HAL events (interrupts, timers, etc.)
     */
    class CallbackManager
    {
    public:
        /**
         * @brief Get singleton instance
         */
        static CallbackManager& getInstance();

        /**
         * @brief Register a GPIO interrupt callback
         * @param pin GPIO pin number
         * @param callback Callback function
         * @return Callback ID for future reference
         */
        uint64_t registerGPIOCallback(uint8_t pin, InterruptCallback callback);

        /**
         * @brief Unregister a GPIO interrupt callback
         * @param callbackId Callback ID returned from register
         * @return true if successful
         */
        bool unregisterGPIOCallback(uint64_t callbackId);

        /**
         * @brief Invoke GPIO interrupt callback
         * @param pin GPIO pin number
         * @param value Pin value
         */
        void invokeGPIOCallback(uint8_t pin, PinValue value);

        /**
         * @brief Register a timer callback
         * @param timerId Timer identifier
         * @param callback Callback function
         * @return Callback ID for future reference
         */
        uint64_t registerTimerCallback(uint32_t timerId, TimerCallback callback);

        /**
         * @brief Unregister a timer callback
         * @param callbackId Callback ID returned from register
         * @return true if successful
         */
        bool unregisterTimerCallback(uint64_t callbackId);

        /**
         * @brief Invoke timer callback
         * @param timerId Timer identifier
         */
        void invokeTimerCallback(uint32_t timerId);

        /**
         * @brief Clear all callbacks
         */
        void clearAll();

        // Prevent copying and assignment
        CallbackManager(const CallbackManager&) = delete;
        CallbackManager& operator=(const CallbackManager&) = delete;
        CallbackManager(CallbackManager&&) = delete;
        CallbackManager& operator=(CallbackManager&&) = delete;

    private:
        CallbackManager() = default;
        ~CallbackManager() = default;

        /// @brief GPIO Callback Info struct \struct GPIOCallbackInfo
        struct GPIOCallbackInfo
        {
            uint8_t pin;
            InterruptCallback callback;
        };

        /// @brief Timer Callback Info struct \struct TimerCallbackInfo
        struct TimerCallbackInfo
        {
            uint32_t timerId;
            TimerCallback callback;
        };

        // Use shared_mutex for read-heavy scenarios (callback invocation is more frequent than registration)
        mutable std::shared_mutex gpioCallbackMutex_;
        mutable std::shared_mutex timerCallbackMutex_;

        std::unordered_map<uint64_t, GPIOCallbackInfo> gpioCallbacks_;
        std::unordered_map<uint8_t, std::vector<uint64_t>> gpioCallbacksByPin_;

        std::unordered_map<uint64_t, TimerCallbackInfo> timerCallbacks_;
        std::unordered_map<uint32_t, std::vector<uint64_t>> timerCallbacksById_;

        std::atomic<uint64_t> nextCallbackId_{1};
    };

} // namespace mex_hal

#endif // MEX_HAL_CALLBACK_MANAGER_H
