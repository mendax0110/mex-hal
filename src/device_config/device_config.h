#ifndef MEX_HAL_DEVICE_CONFIG_H
#define MEX_HAL_DEVICE_CONFIG_H

#include <vector>
#include <mutex>
#include "../include/hal/device_infos_types.h"

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    class DeviceConfig
    {
    private:
        DeviceConfig() = default;

        void scanSPI();
        void scanI2C();
        void scanGPIO();
        void scanUART();

        std::vector<SPIInfo> spiDevices_;
        std::vector<I2CInfo> i2cDevices_;
        std::vector<GPIOInfo> gpioDevices_;
        std::vector<UARTInfo> uartDevices_;
        std::mutex scanMutex_;

    public:

        static DeviceConfig& getInstance()
        {
            static DeviceConfig instance;
            return instance;
        }

        void scan();

        void printDeviceInfos();

        [[nodiscard]] const std::vector<SPIInfo>& getSpiInfos() const { return spiDevices_; }
        [[nodiscard]] const std::vector<I2CInfo>& getI2cInfos() const { return i2cDevices_; }
        [[nodiscard]] const std::vector<GPIOInfo>& getGpioInfos() const { return gpioDevices_; }
        [[nodiscard]] const std::vector<UARTInfo>& getUartInfos() const { return uartDevices_; }
        [[nodiscard]] std::optional<GPIOInfo> getGPIOInfoByPin(int pin) const;

    };
}

#endif //MEX_HAL_DEVICE_CONFIG_H

