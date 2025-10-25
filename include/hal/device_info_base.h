#ifndef MEX_HAL_DEVICE_INFO_BASE_H
#define MEX_HAL_DEVICE_INFO_BASE_H

#include <string>

enum class DeviceType
{
    GPIO,
    SPI,
    I2C,
    PWM,
    UART,
    ADC,
    TIMER,
    UNKNOWN
};

struct DeviceInfoBase
{
    DeviceType type = DeviceType::UNKNOWN;
    std::string name;
    std::string path;
    virtual ~DeviceInfoBase() = default;
};


#endif //MEX_HAL_DEVICE_INFO_BASE_H