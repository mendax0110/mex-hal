#ifndef MEX_HAL_UART_MOCK_H
#define MEX_HAL_UART_MOCK_H

#include "../../include/hal/uart.h"
#include <mutex>
#include <deque>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief Mock UART implementation for testing \class UARTMock
    class UARTMock final : public UARTInterface
    {
    private:
        bool initialized_ = false;
        UARTConfig config_{};
        std::string devicePath_;
        mutable std::mutex mutex_;
        std::deque<uint8_t> rxBuffer_;
        std::vector<uint8_t> txBuffer_;

    public:
        /**
         * @brief Constructor
         */
        UARTMock() = default;

        /**
         * @brief Destructor
         */
        ~UARTMock() override = default;

        /**
         * @brief Initialize the UART port
         * @param device The UART device path
         * @param config The UART configuration
         * @return A true if initialization was successful, false otherwise
         */
        bool init(const std::string& device, const UARTConfig& config) override;

        /**
         * @brief Write data to the UART port
         * @param data The data to write
         * @return A true if the data was successfully written, false otherwise
         */
        bool write(const std::vector<uint8_t>& data) override;

        /**
         * @brief Read data from the UART port
         * @param data The buffer to store read data
         * @param length The number of bytes to read
         * @return A true if the data was successfully read, false otherwise
         */
        bool read(std::vector<uint8_t>& data, size_t length) override;

        /**
         * @brief Get the number of bytes available to read
         * @return The number of bytes available
         */
        size_t available() override;

        /**
         * @brief Flush the UART buffers
         * @return A true if the buffers were successfully flushed, false otherwise
         */
        bool flush() override;

        /**
         * @brief Set new UART configuration
         * @param config The new UART configuration
         * @return A true if the configuration was successfully set, false otherwise
         */
        bool setConfig(const UARTConfig& config) override;

        /**
         * @brief Inject data into the rx buffer (test helper)
         * @param data The data to inject
         */
        void injectRxData(const std::vector<uint8_t>& data);

        /**
         * @brief Get transmitted data (test helper)
         * @return The data that was written
         */
        std::vector<uint8_t> getTxData() const;

        /**
         * @brief Check if UART is initialized (test helper)
         * @return A true if initialized, false otherwise
         */
        bool isInitialized() const;
    };
}

#endif //MEX_HAL_UART_MOCK_H
