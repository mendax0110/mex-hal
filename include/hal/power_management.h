#ifndef MEX_HAL_POWER_MANAGEMENT_H
#define MEX_HAL_POWER_MANAGEMENT_H

#include <cstdint>
#include <string>
#include <functional>

/// @brief mex_hal Hardware Abstraction Layer - Power Management \namespace mex_hal
namespace mex_hal
{
    /// @brief Power state enumeration \enum PowerState
    enum class PowerState
    {
        ACTIVE,
        IDLE,
        STANDBY,
        SUSPEND,
        HIBERNATE
    };

    /// @brief CPU governor enumeration \enum CPUGovernor
    enum class CPUGovernor
    {
        PERFORMANCE,
        POWERSAVE,
        ONDEMAND,
        CONSERVATIVE,
        SCHEDUTIL
    };

    /// @brief Power management interface
    using PowerEventCallback = std::function<void(PowerState oldState, PowerState newState)>;

    /// @brief Power management interface \class PowerManagementInterface
    class PowerManagementInterface
    {
    public:
        /**
         * @brief Virtual destructor
         */
        virtual ~PowerManagementInterface() = default;

        /**
         * @brief Get the Power State.
         * @return A PowerState enumeration value representing the current power state of the system.
         */
        [[nodiscard]] virtual PowerState getPowerState() const = 0;

        /**
         * @brief Request a change in power state.
         * @param newState The desired new power state to transition to, represented as a PowerState enumeration value.
         * @return A boolean indicating whether the request to change the power state was successful (true) or not (false).
         */
        virtual bool requestPowerState(PowerState newState) = 0;

        /**
         * @brief Set the CPU governor.
         * @param governor The desired CPU governor to set, represented as a CPUGovernor enumeration value.
         * @return A boolean indicating whether the request to set the CPU governor was successful (true) or not (false).
         */
        virtual bool setCPUGovernor(CPUGovernor governor) = 0;

        /**
         * @brief Get the current CPU governor.
         * @return A CPUGovernor enumeration value representing the current CPU governor in use.
         */
        [[nodiscard]] virtual CPUGovernor getCPUGovernor() const = 0;

        /**
         * @brief Set the CPU frequency.
         * @param frequencyMHz The desired CPU frequency to set, in megahertz (MHz).
         * @return A boolean indicating whether the request to set the CPU frequency was successful (true) or not (false).
         */
        virtual bool setCPUFrequency(uint32_t frequencyMHz) = 0;

        /**
         * @brief Get the current CPU frequency.
         * @return A uint32_t value representing the current CPU frequency in megahertz (MHz).
         */
        [[nodiscard]] virtual uint32_t getCPUFrequency() const = 0;

        /**
         * @brief Register a callback function to be called when the power state changes.
         * @param callback A PowerEventCallback function that will be called with the old and new power states whenever a power state change occurs.
         */
        virtual void onPowerStateChange(const PowerEventCallback& callback) = 0;

        /**
         * @brief Get the system uptime.
         * @return A uint64_t value representing the system uptime in milliseconds.
         */
        [[nodiscard]] virtual uint64_t getUptime() const = 0;
    };
}

#endif // MEX_HAL_POWER_MANAGEMENT_H