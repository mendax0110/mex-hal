#ifndef MEX_HAL_WATCHDOG_H
#define MEX_HAL_WATCHDOG_H

#include <cstdint>
#include <string>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief Watchdog timer interface \class WatchdogInterface
    class WatchdogInterface
    {
    public:
        /**
         * @brief Virtual destructor
         */
        virtual ~WatchdogInterface() = default;

        /**
         * @brief Initialize the watchdog timer with the specified device and timeout
         * @param device The identifier for the watchdog device (e.g., "/dev/watchdog0")
         * @param timeoutMs The timeout value in milliseconds before the watchdog triggers a system reset if not fed
         * @return A boolean indicating whether the watchdog timer was successfully initialized (true) or not (false)
         */
        virtual bool init(const std::string& device, uint32_t timeoutMs) = 0;

        /**
         * @brief Feed (kick) the watchdog timer to prevent it from triggering a system reset
         * @return A boolean indicating whether the watchdog timer was successfully fed (true) or not (false)
         */
        virtual bool kick() = 0;

        /**
         * @brief Set the timeout value for the watchdog timer
         * @param timeoutSec The timeout value in seconds before the watchdog triggers a system reset if not fed
         * @return A boolean indicating whether the timeout value was successfully set (true) or not (false)
         */
        virtual bool setTimeout(uint32_t timeoutSec) = 0;

        /**
         * @brief Get the current timeout value for the watchdog timer
         * @return A uint32_t representing the current timeout value in seconds before the watchdog triggers a system reset if not fed
         */
        [[nodiscard]] virtual uint32_t getTimeout() const = 0;

        /**
         * @brief Enable the watchdog timer.
         * @return A boolean indicating whether the watchdog timer was successfully enabled (true) or not (false)
         */
        virtual bool enable() = 0;

        /**
         * @brief Disable the watchdog timer.
         * @return A boolean indicating whether the watchdog timer was successfully disabled (true) or not (false)
         */
        virtual bool disable() = 0;

        /**
         * @brief Check if the watchdog timer is currently enabled.
         * @return A boolean indicating whether the watchdog timer is currently enabled (true) or not (false)
         */
        [[nodiscard]] virtual bool isEnabled() const = 0;

        /**
         * @brief Get the time remaining before the watchdog timer triggers a system reset if not fed
         * @return A uint32_t representing the time remaining in seconds before the watchdog triggers a system reset if not fed
         */
        [[nodiscard]] virtual uint32_t getTimeRemaining() const = 0;

    protected:
        inline static const std::string DEV_WATCHDOG = "/dev/watchdog";
    };
}

#endif // MEX_HAL_WATCHDOG_H