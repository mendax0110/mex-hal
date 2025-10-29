#ifndef MEX_HAL_I2C_LINUX_H
#define MEX_HAL_I2C_LINUX_H

#include "../../include/hal/i2c.h"
#include "../../include/hal/file_descriptor.h"
#include "../../include/hal/resource_manager.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdexcept>
#include <mutex>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief I2C Linux implementation class \class I2CLinux
    class I2CLinux final : public I2CInterface
    {
    private:
        FileDescriptor fd_;
        uint8_t currentBus_ = 0;
        uint8_t currentAddress_ = 0;
        uint64_t resourceId_ = 0;
        mutable std::mutex i2cMutex_;

    public:
        /**
         * @brief Constructor
         */
        I2CLinux() = default;

        /**
         * @brief Destructor
         */
        ~I2CLinux() override;

        /**
         * @brief Initialize the I2C bus
         * @param bus The I2C bus number
         * @return A true if initialization was successful, false otherwise
         */
        bool init(uint8_t bus) override;

        /**
         * @brief Set the I2C device address
         * @param address The I2C device address
         * @return A true if the address was successfully set, false otherwise
         */
        bool setDeviceAddress(uint8_t address) override;

        /**
         * @brief Write data to the I2C device
         * @param data The data to write
         * @return A true if the data was successfully written, false otherwise
         */
        bool write(const std::vector<uint8_t>& data) override;

        /**
         * @brief Read data from the I2C device
         * @param data The buffer to store read data
         * @param length The number of bytes to read
         * @return A true if the data was successfully read, false otherwise
         */
        bool read(std::vector<uint8_t>& data, size_t length) override;

        /**
         * @brief Write and then read data from the I2C device
         * @param address The I2C device address
         * @param writeData The data to write
         * @param readData The buffer to store read data
         * @return A true if the operation was successful, false otherwise
         */
        bool writeRead(uint8_t address, const std::vector<uint8_t>& writeData, std::vector<uint8_t>& readData) override;

        /**
         * @brief Set the I2C bus speed
         * @param speed The bus speed in Hz
         * @return A true if the speed was successfully set, false otherwise
         */
        bool setSpeed(uint32_t speed) override;
    };
}

#endif //MEX_HAL_I2C_LINUX_H