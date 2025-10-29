#ifndef MEX_HAL_DEVICE_CONFIG_H
#define MEX_HAL_DEVICE_CONFIG_H

#include <vector>
#include <mutex>
#include "../include/hal/device_infos_types.h"

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief Device Configuration class \class DeviceConfig
    class DeviceConfig
    {
    private:
        /**
         * @brief Constructor
         */
        DeviceConfig() = default;

        /**
         * @brief Scan for SPI devices and populate spiDevices_ vector
         */
        void scanSPI();

        /**
         * @brief Scan for I2C devices and populate i2cDevices_ vector
         */
        void scanI2C();

        /**
         * @brief Scan for GPIO devices and populate gpioDevices_ vector
         */
        void scanGPIO();

        /**
         * @brief Scan for UART devices and populate uartDevices_ vector
         */
        void scanUART();

        std::vector<SPIInfo> spiDevices_;
        std::vector<I2CInfo> i2cDevices_;
        std::vector<GPIOInfo> gpioDevices_;
        std::vector<UARTInfo> uartDevices_;
        std::mutex scanMutex_;

    public:

        /**
         * @brief Get the singleton instance of DeviceConfig
         * @return Reference to the DeviceConfig instance
         */
        static DeviceConfig& getInstance()
        {
            static DeviceConfig instance;
            return instance;
        }

        /**
         * @brief Scan for all devices (SPI, I2C, GPIO, UART)
         */
        void scan();

        /**
         * @brief Print information about all detected devices
         */
        void printDeviceInfos();

        /**
         * @brief Get the list of detected SPI devices
         * @return Vector of SPIInfo structures
         */
        [[nodiscard]] const std::vector<SPIInfo>& getSpiInfos() const { return spiDevices_; }

        /**
         * @brief Get the list of detected I2C devices
         * @return Vector of I2CInfo structures
         */
        [[nodiscard]] const std::vector<I2CInfo>& getI2cInfos() const { return i2cDevices_; }

        /**
         * @brief Get the list of detected GPIO devices
         * @return Vector of GPIOInfo structures
         */
        [[nodiscard]] const std::vector<GPIOInfo>& getGpioInfos() const { return gpioDevices_; }

        /**
         * @brief Get the list of detected UART devices
         * @return Vector of UARTInfo structures
         */
        [[nodiscard]] const std::vector<UARTInfo>& getUartInfos() const { return uartDevices_; }

        /**
         * @brief Get SPI device info by bus and chip select
         * @param pin The GPIO pin number
         * @return Vector of GPIOInfo structures
         */
        [[nodiscard]] std::optional<GPIOInfo> getGPIOInfoByPin(int pin) const;

    };
}

#endif //MEX_HAL_DEVICE_CONFIG_H

