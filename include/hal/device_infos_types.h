#ifndef MEX_HAL_DEVICE_INFOS_TYPES_H
#define MEX_HAL_DEVICE_INFOS_TYPES_H

#include "device_info_base.h"
#include <optional>

struct SPIInfo : DeviceInfoBase
{
    int bus = -1;
    int chipSelect = -1;
    int mode = 0;
    int maxSpeedHz = 0;
    SPIInfo() { type = DeviceType::SPI; }
};

struct I2CInfo : DeviceInfoBase
{
    int bus = -1;
    int address = -1;
    I2CInfo() { type = DeviceType::I2C; }
};

struct GPIOInfo : DeviceInfoBase
{
    int pin = -1;
    bool exported = false;
    std::string direction;
    GPIOInfo() { type = DeviceType::GPIO; }
};

struct UARTInfo : DeviceInfoBase
{
    std::string device;
    int baudRate = 0;
    UARTInfo() { type = DeviceType::UART; }
};

#endif //MEX_HAL_DEVICE_INFOS_TYPES_H