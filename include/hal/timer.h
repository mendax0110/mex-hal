#ifndef MEX_HAL_TIMER_H
#define MEX_HAL_TIMER_H

#include "types.h"
#include <cstdint>
#include <functional>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief Timer mode enumeration \enum TimerMode
    enum class TimerMode
    {
        ONE_SHOT,
        PERIODIC
    };

    /// @brief Timer callback type \typedef TimerCallback
    class TimerInterface
    {
    public:
        /**
         * @brief Virtual destructor
         */
        virtual ~TimerInterface() = default;

        /**
         * @brief Initialize the timer with the specified mode
         * @param mode The timer mode (ONE_SHOT or PERIODIC)
         * @return A true if initialization was successful, false otherwise
         */
        virtual bool init(TimerMode mode) = 0;

        /**
         * @brief Start the timer with the specified interval and callback
         * @param intervalUs The timer interval in microseconds
         * @param callback The callback function to invoke on timer expiration
         * @return A true if the timer was successfully started, false otherwise
         */
        virtual bool start(uint64_t intervalUs, TimerCallback callback) = 0;

        /**
         * @brief Stop the timer
         * @return A true if the timer was successfully stopped, false otherwise
         */
        virtual bool stop() = 0;

        /**
         * @brief Reset the timer to its initial state
         * @return A true if the timer was successfully reset, false otherwise
         */
        virtual bool reset() = 0;

        /**
         * @brief Set a new interval for the timer
         * @param intervalUs The new timer interval in microseconds
         * @return A true if the interval was successfully set, false otherwise
         */
        virtual bool setInterval(uint64_t intervalUs) = 0;

        /**
         * @brief Get the current timer interval
         * @return The timer interval in microseconds
         */
        [[nodiscard]] virtual uint64_t getInterval() const = 0;

        /**
         * @brief Check if the timer is currently running
         * @return A true if the timer is running, false otherwise
         */
        [[nodiscard]] virtual bool isRunning() const = 0;

        /**
         * @brief Get the elapsed time since the timer was started
         * @return The elapsed time in microseconds
         */
        [[nodiscard]] virtual uint64_t getElapsedUs() const = 0;

        /**
         * @brief Get the current system time in microseconds
         * @return The current time in microseconds
         */
        [[nodiscard]] virtual uint64_t getCurrentTimeUs() const = 0;
    };
}

#endif //MEX_HAL_TIMER_H