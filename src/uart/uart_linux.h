#ifndef MEX_HAL_UART_LINUX_H
#define MEX_HAL_UART_LINUX_H

#include "../../include/hal/uart.h"
#include "../../include/hal/file_descriptor.h"
#include "../../include/hal/resource_manager.h"
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdexcept>
#include <string>
#include <mutex>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief UART Linux implementation class \class UARTLinux
    class UARTLinux : public UARTInterface
    {
    private:
        FileDescriptor fd_;
        std::string devicePath_;
        UARTConfig currentConfig_;
        uint64_t resourceId_ = 0;
        mutable std::mutex uartMutex_;

        /**
         * @brief Configure the UART port with the specified settings
         * @param config The UART configuration
         * @return A true if configuration was successful, false otherwise
         */
        bool configurePort(const UARTConfig& config);
        
    public:
        /**
         * @brief Constructor
         */
        UARTLinux() = default;

        /**
         * @brief Destructor
         */
        ~UARTLinux() override;

        /**
         * @brief Initialize the UART port
         * @param device The UART device path (e.g., "/dev/ttyS0")
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
    };
}

#endif //MEX_HAL_UART_LINUX_H