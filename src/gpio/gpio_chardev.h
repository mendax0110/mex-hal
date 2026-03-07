#ifndef MEX_HAL_GPIO_CHARDEV_H
#define MEX_HAL_GPIO_CHARDEV_H

#include "../../include/hal/gpio.h"
#include "../../include/hal/file_descriptor.h"
#include <mutex>
#include <unordered_map>
#include <thread>
#include <atomic>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief GPIO character device implementation class \class GPIOChardev
    class GPIOChardev final : public GPIOInterface
    {
    public:
        /**
         * @brief Ctor
         * @param chipPath The path to the GPIO character device (e.g., "/dev/gpiochip0")
         */
        explicit GPIOChardev(const std::string& chipPath = "/dev/gpiochip0");

        /**
         * @brief Dtor
         */
        ~GPIOChardev() override;

        /// @copydoc GPIOInterface::setDirection
        bool setDirection(uint8_t pin, PinDirection direction) override;

        /// @copydoc GPIOInterface::read
        bool write(uint8_t pin, PinValue value) override;

        /// @copydoc GPIOInterface::write
        PinValue read(uint8_t pin) override;

        /// @copydoc GPIOInterface::setInterrupt
        bool setInterrupt(uint8_t pin, EdgeTrigger edge, InterruptCallback callback) override;

        /// @copydoc GPIOInterface::removeInterrupt
        bool removeInterrupt(uint8_t pin) override;

        /// @copydoc GPIOInterface::setDebounce
        bool setDebounce(uint8_t pin, uint32_t debounceTimeMs) override;

    private:

        /// @brief Internal structure to track state of each GPIO line \struct LineState
        struct LineState
        {
            int fd = -1;
            PinDirection direction = PinDirection::INPUT;
            uint32_t debounceUs = 0;
            InterruptCallback interruptCb{nullptr};
            EdgeTrigger edge = EdgeTrigger::RISING;
            std::thread monitorThread;
            std::atomic<bool> interruptActive{false};
        };

        std::string chipPath_;
        int chipFd_ = -1;
        mutable std::mutex mutex_;
        std::unordered_map<uint8_t, LineState> lines_;

        /**
         * @brief Request control of a GPIO line
         * @param pin The GPIO pin number to request
         * @param direction The direction to set for the line (INPUT or OUTPUT)
         * @return A true if the line was successfully requested and configured, false otherwise
         */
        bool requestLine(uint8_t pin, PinDirection direction);

        /**
         * @brief Release control of a GPIO line
         * @param pin The GPIO pin number to release
         */
        void releaseLine(uint8_t pin);

        /**
         * @brief Monitor edge events for a GPIO line and invoke the registered callback
         * @param pin The GPIO pin number to monitor for edge events
         */
        void monitorEdgeEvents(uint8_t pin);
    };
}

#endif // MEX_HAL_GPIO_CHARDEV_H