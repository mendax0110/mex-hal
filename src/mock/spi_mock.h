#ifndef MEX_HAL_SPI_MOCK_H
#define MEX_HAL_SPI_MOCK_H

#include "../../include/hal/spi.h"
#include <mutex>
#include <deque>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief Mock SPI implementation for testing \class SPIMock
    class SPIMock final : public SPIInterface
    {
    private:
        bool initialized_ = false;
        uint8_t bus_ = 0;
        uint8_t cs_ = 0;
        uint32_t speed_ = 0;
        SPIMode mode_ = SPIMode::MODE_0;
        mutable std::mutex mutex_;
        std::deque<std::vector<uint8_t>> rxQueue_;

    public:
        /**
         * @brief Constructor
         */
        SPIMock() = default;

        /**
         * @brief Destructor
         */
        ~SPIMock() override = default;

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

        /**
         * @brief Enqueue data to be returned on next read/transfer (test helper)
         * @param data The data to enqueue
         */
        void enqueueRxData(const std::vector<uint8_t>& data);

        /**
         * @brief Check if SPI is initialized (test helper)
         * @return A true if initialized, false otherwise
         */
        bool isInitialized() const;

        /**
         * @brief Get the current speed (test helper)
         * @return The current speed in Hz
         */
        uint32_t getSpeed() const;

        /**
         * @brief Get the current mode (test helper)
         * @return The current SPIMode
         */
        SPIMode getMode() const;
    };
}

#endif //MEX_HAL_SPI_MOCK_H
