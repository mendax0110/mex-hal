#ifndef MEX_HAL_DEVICE_INFOS_TYPES_H
#define MEX_HAL_DEVICE_INFOS_TYPES_H

#include "device_info_base.h"
#include <optional>

/// @brief SPI information structure \struct SPIInfo
struct SPIInfo final : DeviceInfoBase
{
    int bus = -1;
    int chipSelect = -1;
    int mode = 0;
    int maxSpeedHz = 0;
    SPIInfo() { type = DeviceType::SPI; }
};

/// @brief I2C information structure \struct I2CInfo
struct I2CInfo final : DeviceInfoBase
{
    int bus = -1;
    int address = -1;
    I2CInfo() { type = DeviceType::I2C; }
};

/// @brief GPIO information structure \struct GPIOInfo
struct GPIOInfo final : DeviceInfoBase
{
    int pin = -1;
    bool exported = false;
    std::string direction;
    GPIOInfo() { type = DeviceType::GPIO; }
};

/// @brief UART information structure \struct UARTInfo
struct UARTInfo final : DeviceInfoBase
{
    std::string device;
    int baudRate = 0;
    UARTInfo() { type = DeviceType::UART; }
};

#endif //MEX_HAL_DEVICE_INFOS_TYPES_H