#ifndef MEX_HAL_GPIO_LINUX_H
#define MEX_HAL_GPIO_LINUX_H

#include "../../include/hal/gpio.h"
#include "../../include/hal/resource_manager.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include <stdexcept>
#include <mutex>
#include <unordered_map>
#include <thread>
#include <atomic>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief GPIO Linux implementation class \class GPIOLinux
    class GPIOLinux : public GPIOInterface
    {
    private:
        /// @brief Pin information structure, tracks state of each GPIO pin \struct PinInfo
        struct PinInfo
        {
            uint64_t resourceId = 0;
            PinDirection direction = PinDirection::INPUT;
            bool exported = false;
            std::atomic<bool> interruptActive{false};
            uint64_t callbackId = 0;

            /**
             * @brief Constructor
             */
            PinInfo() = default;

            /**
             * @brief Copy constructor
             * @param other The other PinInfo to copy from
             */
            PinInfo(const PinInfo& other)
                : resourceId(other.resourceId)
                , direction(other.direction)
                , exported(other.exported)
                , interruptActive(other.interruptActive.load())
                , callbackId(other.callbackId)
            {

            }

            /**
             * @brrief Copy assignment operator
             * @param other The other PinInfo to copy from
             * @return A reference to this PinInfo
             */
            PinInfo& operator=(const PinInfo& other)
            {
                if (this != &other)
                {
                    resourceId = other.resourceId;
                    direction = other.direction;
                    exported = other.exported;
                    interruptActive.store(other.interruptActive.load());
                    callbackId = other.callbackId;
                }
                return *this;
            }
        };

        mutable std::mutex pinMutex_;
        std::unordered_map<uint8_t, PinInfo> pins_;

        // Interrupt monitoring
        std::unordered_map<uint8_t, std::unique_ptr<std::thread>> interruptThreads_;
        std::atomic<bool> shutdownRequested_{false};

        /**
         * @brief Export a GPIO pin
         * @param pin The GPIO pin number
         * @return A integer status code (0 = success, -1 = failure)
         */
        static int exportPin(uint8_t pin);

        /**
         * @brief Unexport a GPIO pin
         * @param pin The GPIO pin number
         * @return A integer status code (0 = success, -1 = failure)
         */
        static int unexportPin(uint8_t pin);

        /**
         * @brief Monitor GPIO pin for interrupts
         * @param pin The GPIO pin number
         * @param callbackId The callback ID to invoke on interrupt
         */
        void monitorInterrupt(uint8_t pin, uint64_t callbackId) const;

    public:
        /**
         * @brief Constructor
         */
        GPIOLinux() = default;

        /**
         * @brief Destructor
         */
        ~GPIOLinux() override;

        /**
         * @brief Set the direction of a GPIO pin
         * @param pin The GPIO pin number
         * @param direction The direction to set (INPUT or OUTPUT)
         * @return A true if the direction was successfully set, false otherwise
         */
        bool setDirection(uint8_t pin, PinDirection direction) override;

        /**
         * @brief Write a value to a GPIO pin
         * @param pin The GPIO pin number
         * @param value The value to write (HIGH or LOW)
         * @return A true if the value was successfully written, false otherwise
         */
        bool write(uint8_t pin, PinValue value) override;

        /**
         * @brief Read the value of a GPIO pin
         * @param pin The GPIO pin number
         * @return A PinValue representing the current value of the pin (HIGH or LOW)
         */
        PinValue read(uint8_t pin) override;

        /**
         * @brief Set up an interrupt on a GPIO pin
         * @param pin The GPIO pin number
         * @param edge The edge trigger type (RISING, FALLING, or BOTH)
         * @param callback The callback function to invoke on interrupt
         * @return A true if the interrupt was successfully set, false otherwise
         */
        bool setInterrupt(uint8_t pin, EdgeTrigger edge, InterruptCallback callback) override;

        /**
         * @brief Remove an interrupt from a GPIO pin
         * @param pin The GPIO pin number
         * @return A true if the interrupt was successfully removed, false otherwise
         */
        bool removeInterrupt(uint8_t pin) override;

        /**
         * @brief Set debounce time for a GPIO pin
         * @param pin The GPIO pin number
         * @param debounceTimeMs The debounce time in milliseconds
         * @return A true if the debounce time was successfully set, false otherwise
         */
        bool setDebounce(uint8_t pin, uint32_t debounceTimeMs) override;
    };
}

#endif //MEX_HAL_GPIO_LINUX_H