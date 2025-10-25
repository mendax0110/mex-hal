#include "sys_config.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unistd.h>

namespace fs = std::filesystem;
using namespace mex_hal;

SystemConfig::ConfigStatus SystemConfig::check() noexcept
{
    ConfigStatus s;

    try
    {
        s.isRoot = (geteuid() == 0);
        s.hasPreemptRT = checkPreemptRT(s.kernelVersion);
        s.cpuGovernorPerformance = checkCPUGovernor();
        s.irqAffinitySet = checkIRQAffinity();
        s.limitsConfigured = checkLimitsFile();
        s.sysctlConfigured = checkSysctlFile();
        s.udevRulesPresent = checkUdevRules();

        if (!s.hasPreemptRT)
            logWarning(s.warnings, "PREEMPT RT kernel not detected. Real-time performance may be reduced.");
        if (!s.cpuGovernorPerformance)
            logWarning(s.warnings, "CPU governor not set to 'performance'. Timing may be unstable.");
        if (!s.limitsConfigured)
            logWarning(s.warnings, "Missing realtime limits file: " + std::string(kLimitsPath));
        if (!s.sysctlConfigured)
            logWarning(s.warnings, "Missing sysctl realtime config: " + std::string(kSysctlPath));
        if (!s.udevRulesPresent)
            logWarning(s.warnings, "Missing udev rules: " + std::string(kUdevRulesPath));
    }
    catch (const std::exception& e)
    {
        logError(s.errors, std::string("Exception during config check: ") + e.what());
    }

    return s;
}

bool SystemConfig::checkPreemptRT(std::string& kernelVersion) noexcept
{
    std::ifstream f("/proc/version");
    if (!f.is_open()) return false;

    std::getline(f, kernelVersion);
    return kernelVersion.find("PREEMPT RT") != std::string::npos;
}

bool SystemConfig::checkCPUGovernor() noexcept
{
    try
    {
        for (const auto& entry : fs::directory_iterator("/sys/devices/system/cpu"))
        {
            if (!entry.is_directory()) continue;

            auto name = entry.path().filename().string();
            if (name.rfind("cpu", 0) != 0) continue;

            auto govPath = entry.path() / "cpufreq/scaling_governor";
            if (fs::exists(govPath))
            {
                std::ifstream f(govPath);
                std::string mode;
                if (!(f >> mode) || mode != "performance") return false;
            }
        }
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool SystemConfig::checkIRQAffinity() noexcept
{
    std::ifstream f("/proc/irq/0/smp_affinity");
    if (!f.good()) return false;
    std::string val;
    f >> val;
    return val == "1";
}

bool SystemConfig::checkLimitsFile() noexcept
{
    return fs::exists(kLimitsPath);
}

bool SystemConfig::checkSysctlFile() noexcept
{
    return fs::exists(kSysctlPath);
}

bool SystemConfig::checkUdevRules() noexcept
{
    return fs::exists(kUdevRulesPath);
}

void SystemConfig::printReport(const ConfigStatus& status) noexcept
{
    std::cout << "\n==== MEX-HAL System Configuration Report ====\n";
    std::cout << "Kernel: " << status.kernelVersion << "\n";
    std::cout << "Root privileges: " << (status.isRoot ? "Yes" : "No") << "\n";
    std::cout << "PREEMPT RT kernel: " << (status.hasPreemptRT ? "Yes" : "No") << "\n";
    std::cout << "CPU governor performance: " << (status.cpuGovernorPerformance ? "Yes" : "No") << "\n";
    std::cout << "IRQ affinity set: " << (status.irqAffinitySet ? "Yes" : "No") << "\n";
    std::cout << "Realtime limits configured: " << (status.limitsConfigured ? "Yes" : "No") << "\n";
    std::cout << "Sysctl tuned for RT: " << (status.sysctlConfigured ? "Yes" : "No") << "\n";
    std::cout << "Udev rules installed: " << (status.udevRulesPresent ? "Yes" : "No") << "\n\n";

    if (!status.warnings.empty())
    {
        std::cout << "Warnings:\n";
        for (const auto& w : status.warnings)
            std::cout << "  - " << w << "\n";
    }

    if (!status.errors.empty())
    {
        std::cout << "\nErrors:\n";
        for (const auto& e : status.errors)
            std::cout << "  - " << e << "\n";
    }

    std::cout << "=============================================\n\n";
}

void SystemConfig::logWarning(std::vector<std::string>& warnings, const std::string& msg)
{
    warnings.emplace_back(msg);
}

void SystemConfig::logError(std::vector<std::string>& errors, const std::string& msg)
{
    errors.emplace_back(msg);
}
