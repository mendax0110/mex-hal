#ifndef MEX_HAL_SPI_H
#define MEX_HAL_SPI_H

#include "types.h"
#include <cstdint>
#include <vector>
#include <memory>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief SPI Interface class \class SPIInterface
    class SPIInterface
    {
    public:
        /**
         * @brief Virtual destructor
         */
        virtual ~SPIInterface() = default;

        /**
         * @brief Initialize the SPI bus
         * @param bus The SPI bus number
         * @param cs The chip select number
         * @param speed The SPI clock speed in Hz
         * @param mode The SPI mode (clock polarity and phase)
         * @return A true if initialization was successful, false otherwise
         */
        virtual bool init(uint8_t bus, uint8_t cs, uint32_t speed, SPIMode mode) = 0;

        /**
         * @brief Transfer data over SPI (full-duplex)
         * @param txData The data to transmit
         * @param rxData The buffer to store received data
         * @return A true if the transfer was successful, false otherwise
         */
        virtual bool transfer(const std::vector<uint8_t>& txData, std::vector<uint8_t>& rxData) = 0;

        /**
         * @brief Write data to the SPI bus
         * @param data The data to write
         * @return A true if the data was successfully written, false otherwise
         */
        virtual bool write(const std::vector<uint8_t>& data) = 0;

        /**
         * @brief Read data from the SPI bus
         * @param data The buffer to store read data
         * @param length The number of bytes to read
         * @return A true if the data was successfully read, false otherwise
         */
        virtual bool read(std::vector<uint8_t>& data, size_t length) = 0;

        /**
         * @brief Set the SPI clock speed
         * @param speed The SPI clock speed in Hz
         * @return A true if the speed was successfully set, false otherwise
         */
        virtual bool setSpeed(uint32_t speed) = 0;

        /**
         * @brief Set the SPI mode (clock polarity and phase)
         * @param mode The SPI mode
         * @return A true if the mode was successfully set, false otherwise
         */
        virtual bool setMode(SPIMode mode) = 0;

    protected:
        inline static const std::string DEV_SPIDEV = "/dev/spidev";
    };
}

#endif //MEX_HAL_SPI_H