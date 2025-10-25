#ifndef MEX_HAL_TIMER_LINUX_H
#define MEX_HAL_TIMER_LINUX_H

#include "../../include/hal/timer.h"
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief Timer Linux implementation class \class TimerLinux
    class TimerLinux : public TimerInterface
    {
    private:
        TimerMode mode = TimerMode::ONE_SHOT;
        uint64_t intervalUs = 0;
        std::atomic<bool> running{false};
        std::atomic<bool> shouldStop{false};
        std::thread timerThread;
        TimerCallback callback;
        std::chrono::steady_clock::time_point startTime;
        std::mutex callbackMutex;

        /**
         * @brief Timer loop function
         */
        void timerLoop();
        
    public:
        /**
         * @brief Constructor
         */
        TimerLinux() = default;

        /**
         * @brief Destructor
         */
        ~TimerLinux() override;

        /**
         * @brief Initialize the timer with the specified mode
         * @param mode The timer mode (ONE_SHOT or PERIODIC)
         * @return A true if initialization was successful, false otherwise
         */
        bool init(TimerMode mode) override;

        /**
         * @brief Start the timer with the specified interval and callback
         * @param intervalUs The timer interval in microseconds
         * @param callback The callback function to invoke on timer expiration
         * @return A true if the timer was successfully started, false otherwise
         */
        bool start(uint64_t intervalUs, TimerCallback callback) override;

        /**
         * @brief Stop the timer
         * @return A true if the timer was successfully stopped, false otherwise
         */
        bool stop() override;

        /**
         * @brief Reset the timer to its initial state
         * @return A true if the timer was successfully reset, false otherwise
         */
        bool reset() override;

        /**
         * @brief Set a new interval for the timer
         * @param intervalUs The new timer interval in microseconds
         * @return A true if the interval was successfully set, false otherwise
         */
        bool setInterval(uint64_t intervalUs) override;

        /**
         * @brief Get the current timer interval
         * @return The timer interval in microseconds
         */
        [[nodiscard]] uint64_t getInterval() const override;

        /**
         * @brief Check if the timer is currently running
         * @return A true if the timer is running, false otherwise
         */
        [[nodiscard]] bool isRunning() const override;

        /**
         * @brief Get the elapsed time since the timer started
         * @return The elapsed time in microseconds
         */
        [[nodiscard]] uint64_t getElapsedUs() const override;

        /**
         * @brief Get the current system time in microseconds
         * @return The current time in microseconds
         */
        [[nodiscard]] uint64_t getCurrentTimeUs() const override;
    };
}

#endif //MEX_HAL_TIMER_LINUX_H