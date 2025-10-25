#ifndef MEX_HAL_SPI_LINUX_H
#define MEX_HAL_SPI_LINUX_H

#include "../../include/hal/spi.h"
#include "../../include/hal/file_descriptor.h"
#include "../../include/hal/resource_manager.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <stdexcept>
#include <mutex>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief SPI Linux implementation class \class SPILinux
    class SPILinux : public SPIInterface
    {
    private:
        FileDescriptor fd_;
        uint8_t currentBus_ = 0;
        uint8_t currentCS_ = 0;
        uint64_t resourceId_ = 0;
        mutable std::mutex spiMutex_;

    public:
        /**
         * @brief Constructor
         */
        SPILinux() = default;

        /**
         * @brief Destructor
         */
        ~SPILinux() override;

        /**
         * @brief Initialize the SPI bus
         * @param bus The SPI bus number
         * @param cs The chip select number
         * @param speed The SPI clock speed in Hz
         * @param mode The SPI mode (clock polarity and phase)
         * @return A true if initialization was successful, false otherwise
         */
        bool init(uint8_t bus, uint8_t cs, uint32_t speed, SPIMode mode) override;

        /**
         * @brief Transfer data over SPI (full-duplex)
         * @param txData The data to transmit
         * @param rxData The buffer to store received data
         * @return A true if the transfer was successful, false otherwise
         */
        bool transfer(const std::vector<uint8_t>& txData, std::vector<uint8_t>& rxData) override;

        /**
         * @brief Write data to the SPI bus
         * @param data The data to write
         * @return A true if the data was successfully written, false otherwise
         */
        bool write(const std::vector<uint8_t>& data) override;

        /**
         * @brief Read data from the SPI bus
         * @param data The buffer to store read data
         * @param length The number of bytes to read
         * @return A true if the data was successfully read, false otherwise
         */
        bool read(std::vector<uint8_t>& data, size_t length) override;

        /**
         * @brief Set the SPI clock speed
         * @param speed The SPI clock speed in Hz
         * @return A true if the speed was successfully set, false otherwise
         */
        bool setSpeed(uint32_t speed) override;

        /**
         * @brief Set the SPI mode (clock polarity and phase)
         * @param mode The SPI mode
         * @return A true if the mode was successfully set, false otherwise
         */
        bool setMode(SPIMode mode) override;
    };
}

#endif //MEX_HAL_SPI_LINUX_H