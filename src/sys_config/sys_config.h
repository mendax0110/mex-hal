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

        [[nodiscard]] static ConfigStatus check() noexcept;

        static void printReport(const ConfigStatus& status) noexcept;

    private:
        static constexpr auto kLimitsPath = "/etc/security/limits.d/99-realtime.conf";
        static constexpr auto kSysctlPath = "/etc/sysctl.d/99-realtime.conf";
        static constexpr auto kUdevRulesPath = "/etc/udev/rules.d/99-mex-hal.rules";

        static bool checkPreemptRT(std::string& kernelVersion) noexcept;
        static bool checkCPUGovernor() noexcept;
        static bool checkIRQAffinity() noexcept;
        static bool checkLimitsFile() noexcept;
        static bool checkSysctlFile() noexcept;
        static bool checkUdevRules() noexcept;

        static void logWarning(std::vector<std::string>& warnings, const std::string& msg);
        static void logError(std::vector<std::string>& errors, const std::string& msg);
    };

} // namespace mex_hal



#endif //MEX_HAL_SYS_CONFIG_H