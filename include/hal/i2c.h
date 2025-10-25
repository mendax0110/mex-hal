#ifndef MEX_HAL_I2C_H
#define MEX_HAL_I2C_H

#include "types.h"
#include <cstdint>
#include <vector>
#include <memory>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief I2C Interface class \class I2CInterface
    class I2CInterface
    {
    public:
        /**
         * @brief Virtual destructor
         */
        virtual ~I2CInterface() = default;

        /**
         * @brief Initialize the I2C bus
         * @param bus The I2C bus number
         * @return A true if initialization was successful, false otherwise
         */
        virtual bool init(uint8_t bus) = 0;

        /**
         * @brief Set the I2C device address
         * @param address The I2C device address
         * @return A true if the address was successfully set, false otherwise
         */
        virtual bool setDeviceAddress(uint8_t address) = 0;

        /**
         * @brief Write data to the I2C device
         * @param data The data to write
         * @return A true if the data was successfully written, false otherwise
         */
        virtual bool write(const std::vector<uint8_t>& data) = 0;

        /**
         * @brief Read data from the I2C device
         * @param data The buffer to store read data
         * @param length The number of bytes to read
         * @return A true if the data was successfully read, false otherwise
         */
        virtual bool read(std::vector<uint8_t>& data, size_t length) = 0;

        /**
         * @brief Write and then read data from the I2C device
         * @param address The I2C device address
         * @param writeData The data to write
         * @param readData The buffer to store read data
         * @return A true if the operation was successful, false otherwise
         */
        virtual bool writeRead(uint8_t address, const std::vector<uint8_t>& writeData, std::vector<uint8_t>& readData) = 0;

        /**
         * @brief Set the I2C bus speed
         * @param speed The bus speed in Hz
         * @return A true if the speed was successfully set, false otherwise
         */
        virtual bool setSpeed(uint32_t speed) = 0;

    protected:
        inline static const std::string SYS_CALL_I2C_ADAPTERS = "/sys/class/i2c-adapter/i2c-";
    };
}

#endif //MEX_HAL_I2C_H