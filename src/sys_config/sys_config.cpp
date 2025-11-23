#include "sys_config.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <sys/utsname.h>

namespace fs = std::filesystem;
using namespace mex_hal;

static std::string getKernelConfigPath() noexcept
{
    struct utsname unameData;
    if (uname(&unameData) == 0)
    {
        return "/boot/config-" + std::string(unameData.release);
    }
    return "";
}

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
    
    if (kernelVersion.find("PREEMPT RT") != std::string::npos ||
        kernelVersion.find("PREEMPT_RT") != std::string::npos)
    {
        return true;
    }

    std::string configPath = getKernelConfigPath();
    if (!configPath.empty())
    {
        std::ifstream configFile(configPath);
        if (configFile.is_open())
        {
            std::string line;
            while (std::getline(configFile, line))
            {
                if (line.find("CONFIG_PREEMPT_RT=y") != std::string::npos)
                {
                    return true;
                }
            }
        }
    }

    return false;
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
    std::cout << "\n========================================\n";
    std::cout << "  MEX-HAL System Configuration Report\n";
    std::cout << "========================================\n\n";
    
    std::cout << "--- Kernel Information ---\n";
    std::cout << "  Version: " << status.kernelVersion << "\n\n";
    
    std::cout << "--- Preemption Configuration ---\n";
    std::cout << "  PREEMPT_RT detected: " << (status.hasPreemptRT ? "YES" : "NO");
    if (!status.hasPreemptRT)
    {
        std::string configPath = getKernelConfigPath();
        if (!configPath.empty())
        {
            std::ifstream configFile(configPath);
            if (configFile.is_open())
            {
                std::cout << "\n  Detected preempt settings:\n";
                std::string line;
                while (std::getline(configFile, line))
                {
                    if (line.find("CONFIG_PREEMPT") != std::string::npos && 
                        line.find("=y") != std::string::npos)
                    {
                        std::cout << "    " << line << "\n";
                    }
                }
                configFile.close();
            }
            else
            {
                std::cout << "\n  Kernel config not accessible at " << configPath << "\n";
            }
        }
        else
        {
            std::cout << "\n  Unable to determine kernel config path\n";
        }
    }
    else
    {
        std::cout << "\n";
    }
    
    std::cout << "\n--- Privileges ---\n";
    std::cout << "  Running as root: " << (status.isRoot ? "YES" : "NO") << "\n";
    if (!status.isRoot)
    {
        std::cout << "  Note: Root privileges may be required for RT scheduling\n";
    }
    
    std::cout << "\n--- Performance Settings ---\n";
    std::cout << "  CPU governor: " << (status.cpuGovernorPerformance ? "Performance" : "Other (not optimal)") << "\n";
    std::cout << "  IRQ affinity configured: " << (status.irqAffinitySet ? "YES" : "NO") << "\n";
    
    std::cout << "\n--- System Configuration Files ---\n";
    std::cout << "  Realtime limits (/etc/security/limits.d/): " << (status.limitsConfigured ? "Present" : "Missing") << "\n";
    std::cout << "  Sysctl RT config (/etc/sysctl.d/): " << (status.sysctlConfigured ? "Present" : "Missing") << "\n";
    std::cout << "  Udev rules (/etc/udev/rules.d/): " << (status.udevRulesPresent ? "Present" : "Missing") << "\n";

    if (!status.warnings.empty())
    {
        std::cout << "\n--- Warnings ---\n";
        for (const auto& w : status.warnings)
            std::cout << "  [!] " << w << "\n";
    }

    if (!status.errors.empty())
    {
        std::cout << "\n--- Errors ---\n";
        for (const auto& e : status.errors)
            std::cout << "  [X] " << e << "\n";
    }

    std::cout << "\n========================================\n\n";
}

void SystemConfig::logWarning(std::vector<std::string>& warnings, const std::string& msg)
{
    warnings.emplace_back(msg);
}

void SystemConfig::logError(std::vector<std::string>& errors, const std::string& msg)
{
    errors.emplace_back(msg);
}
