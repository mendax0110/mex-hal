#ifndef MEX_HAL_GPIO_H
#define MEX_HAL_GPIO_H

#include "types.h"
#include <functional>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief GPIO Interface class \class GPIOInterface
    class GPIOInterface
    {
    public:
        /**
         * @brief Virtual destructor
         */
        virtual ~GPIOInterface() = default;

        /**
         * @brief Set the direction of a GPIO pin
         * @param pin The GPIO pin number
         * @param direction The direction to set (INPUT or OUTPUT)
         * @return A true if the direction was successfully set, false otherwise
         */
        virtual bool setDirection(uint8_t pin, PinDirection direction) = 0;

        /**
         * @brief Write a value to a GPIO pin
         * @param pin The GPIO pin number
         * @param value The value to write (HIGH or LOW)
         * @return A true if the value was successfully written, false otherwise
         */
        virtual bool write(uint8_t pin, PinValue value) = 0;

        /**
         * @brief Read the value of a GPIO pin
         * @param pin The GPIO pin number
         * @return The current value of the pin (HIGH or LOW)
         */
        virtual PinValue read(uint8_t pin) = 0;

        /**
         * @brief Set up an interrupt on a GPIO pin
         * @param pin The GPIO pin number
         * @param edge The edge trigger type (RISING, FALLING, or BOTH)
         * @param callback The callback function to invoke on interrupt
         * @return A true if the interrupt was successfully set, false otherwise
         */
        virtual bool setInterrupt(uint8_t pin, EdgeTrigger edge, InterruptCallback callback) = 0;

        /**
         * @brief Remove an interrupt from a GPIO pin
         * @param pin The GPIO pin number
         * @return A true if the interrupt was successfully removed, false otherwise
         */
        virtual bool removeInterrupt(uint8_t pin) = 0;

        /**
         * @brief Set debounce time for a GPIO pin
         * @param pin The GPIO pin number
         * @param debounceTimeMs The debounce time in milliseconds
         * @return A true if the debounce time was successfully set, false otherwise
         */
        virtual bool setDebounce(uint8_t pin, uint32_t debounceTimeMs) = 0;

    protected:
        inline static const std::string SYS_CLASS_GPIO = "/sys/class/gpio/gpio";
        inline static const std::string SYS_CLASS_GPIO_EXPORT = "/sys/class/gpio/export";
        inline static const std::string SYS_CLASS_GPIO_UNEXPORT = "/sys/class/gpio/unexport";
    };
}

#endif //MEX_HAL_GPIO_H