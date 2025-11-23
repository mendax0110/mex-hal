#ifndef MEX_HAL_SYS_CONFIG_H
#define MEX_HAL_SYS_CONFIG_H

#include <string>
#include <vector>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief System Configuration class \class SystemConfig
    class SystemConfig
    {
    public:
        /// @brief Configuration status structure \struct ConfigStatus
        struct ConfigStatus
        {
            bool isRoot = false;
            bool hasPreemptRT = false;
            bool cpuGovernorPerformance = false;
            bool irqAffinitySet = false;
            bool limitsConfigured = false;
            bool sysctlConfigured = false;
            bool udevRulesPresent = false;

            std::string kernelVersion;
            std::vector<std::string> warnings;
            std::vector<std::string> errors;
        };

        /**
         * @brief Check the system configuration for real-time operation
         * @return A ConfigStatus struct containing the results of the checks
         */
        [[nodiscard]] static ConfigStatus check() noexcept;

        /**
         * @brief Print a report of the system configuration status
         * @param status The ConfigStatus struct to report on
         */
        static void printReport(const ConfigStatus& status) noexcept;

    private:
        // File paths for configuration checks
        static constexpr auto kLimitsPath = "/etc/security/limits.d/99-realtime.conf";
        static constexpr auto kSysctlPath = "/etc/sysctl.d/99-realtime.conf";
        static constexpr auto kUdevRulesPath = "/etc/udev/rules.d/99-mex-hal.rules";

        /**
         * @brief Check if the Preempt-RT kernel is installed
         * @param kernelVersion The detected kernel version string
         * @return A true if Preempt-RT is installed, false otherwise
         */
        static bool checkPreemptRT(std::string& kernelVersion) noexcept;

        /**
         * @brief Check if the CPU governor is set to performance mode
         * @return A true if the CPU governor is performance, false otherwise
         */
        static bool checkCPUGovernor() noexcept;

        /**
         * @brief Check if IRQ affinity is set correctly
         * @return A true if IRQ affinity is set, false otherwise
         */
        static bool checkIRQAffinity() noexcept;

        /**
         * @brief Check if the limits configuration file is present and correct
         * @return A true if limits are configured, false otherwise
         */
        static bool checkLimitsFile() noexcept;

        /**
         * @brief Check if the sysctl configuration file is present and correct
         * @return A true if sysctl is configured, false otherwise
         */
        static bool checkSysctlFile() noexcept;

        /**
         * @brief Check if the udev rules file is present and correct
         * @return A true if udev rules are present, false otherwise
         */
        static bool checkUdevRules() noexcept;

        /**
         * @brief Log a warning message
         * @param warnings The vector to store warning messages
         * @param msg The warning message to log
         */
        static void logWarning(std::vector<std::string>& warnings, const std::string& msg);

        /**
         * @brief Log an error message
         * @param errors The vector to store error messages
         * @param msg The error message to log
         */
        static void logError(std::vector<std::string>& errors, const std::string& msg);
    };

} // namespace mex_hal



#endif //MEX_HAL_SYS_CONFIG_H