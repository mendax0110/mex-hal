#ifndef MEX_HAL_UART_H
#define MEX_HAL_UART_H

#include "types.h"
#include <cstdint>
#include <vector>
#include <memory>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief UART Interface class \class UARTInterface
    class UARTInterface
    {
    public:
        /**
         * @brief Virtual destructor
         */
        virtual ~UARTInterface() = default;

        /**
         * @brief Initialize the UART port
         * @param device The UART device path (e.g., "/dev/ttyS0")
         * @param config The UART configuration
         * @return A true if initialization was successful, false otherwise
         */
        virtual bool init(const std::string& device, const UARTConfig& config) = 0;

        /**
         * @brief Write data to the UART port
         * @param data The data to write
         * @return A true if the data was successfully written, false otherwise
         */
        virtual bool write(const std::vector<uint8_t>& data) = 0;

        /**
         * @brief Read data from the UART port
         * @param data The buffer to store read data
         * @param length The number of bytes to read
         * @return A true if the data was successfully read, false otherwise
         */
        virtual bool read(std::vector<uint8_t>& data, size_t length) = 0;

        /**
         * @brief Get the number of bytes available to read
         * @return The number of bytes available
         */
        virtual size_t available() = 0;

        /**
         * @brief Flush the UART buffers
         * @return A true if the buffers were successfully flushed, false otherwise
         */
        virtual bool flush() = 0;

        /**
         * @brief Set new UART configuration
         * @param config The new UART configuration
         * @return A true if the configuration was successfully set, false otherwise
         */
        virtual bool setConfig(const UARTConfig& config) = 0;
    };
}


#endif //MEX_HAL_UART_H