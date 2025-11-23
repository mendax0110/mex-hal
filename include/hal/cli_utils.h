#ifndef MEX_HAL_CLI_UTILS_H
#define MEX_HAL_CLI_UTILS_H

#include "../include/hal/core.h"
#include "../src/device_config/device_config.h"
#include "../src/sys_config/sys_config.h"
#include <memory>
#include <string>

namespace mex_hal
{
    void readGPIOPin(const std::unique_ptr<HAL>& hal, int pin);
    void readADCVoltage(const std::unique_ptr<HAL>& hal, int device, int channel, float refVoltage);
    void printUsage(const char* progName);
    int processCommandLine(int argc, char* argv[], SystemConfig::ConfigStatus& status, DeviceConfig& conf);
    void exportHardwareReport(SystemConfig::ConfigStatus& status, DeviceConfig& conf, const std::string& filename);
}

#endif
