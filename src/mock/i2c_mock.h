#ifndef MEX_HAL_I2C_MOCK_H
#define MEX_HAL_I2C_MOCK_H

#include "../../include/hal/i2c.h"
#include <mutex>
#include <unordered_map>
#include <deque>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief Mock I2C implementation for testing \class I2CMock
    class I2CMock final : public I2CInterface
    {
    private:
        bool initialized_ = false;
        uint8_t bus_ = 0;
        uint8_t currentAddress_ = 0;
        bool addressSet_ = false;
        uint32_t speed_ = 100000;
        mutable std::mutex mutex_;
        std::unordered_map<uint8_t, std::deque<std::vector<uint8_t>>> deviceData_;
        std::unordered_map<uint8_t, std::vector<uint8_t>> lastWritten_;

    public:
        /**
         * @brief Constructor
         */
        I2CMock() = default;

        /**
         * @brief Destructor
         */
        ~I2CMock() override = default;

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

        /**
         * @brief Enqueue read data for a device address (test helper)
         * @param address The I2C device address
         * @param data The data to return on read
         */
        void enqueueReadData(uint8_t address, const std::vector<uint8_t>& data);

        /**
         * @brief Get last written data for a device address (test helper)
         * @param address The I2C device address
         * @return The last written data
         */
        std::vector<uint8_t> getLastWritten(uint8_t address) const;

        /**
         * @brief Check if I2C is initialized (test helper)
         * @return A true if initialized, false otherwise
         */
        bool isInitialized() const;
    };
}

#endif //MEX_HAL_I2C_MOCK_H
