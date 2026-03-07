#ifndef MEX_HAL_GPIO_MOCK_H
#define MEX_HAL_GPIO_MOCK_H

#include "../../include/hal/gpio.h"
#include <mutex>
#include <unordered_map>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief Mock GPIO implementation for testing \class GPIOMock
    class GPIOMock final : public GPIOInterface
    {
    private:
        struct PinState
        {
            PinDirection direction = PinDirection::INPUT;
            PinValue value = PinValue::LOW;
            bool exported = false;
            uint32_t debounceMs = 0;
            InterruptCallback interruptCb{nullptr};
            EdgeTrigger edge = EdgeTrigger::RISING;
        };

        mutable std::mutex mutex_;
        std::unordered_map<uint8_t, PinState> pins_;

    public:
        /**
         * @brief Constructor
         */
        GPIOMock() = default;

        /**
         * @brief Destructor
         */
        ~GPIOMock() override = default;

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
         * @return The current value of the pin (HIGH or LOW)
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

        /**
         * @brief Simulate an interrupt on a pin (test helper)
         * @param pin The GPIO pin number
         * @param value The pin value to trigger callback with
         */
        void simulateInterrupt(uint8_t pin, PinValue value);

        /**
         * @brief Get the stored pin value (test helper)
         * @param pin The GPIO pin number
         * @return The stored PinValue
         */
        PinValue getStoredValue(uint8_t pin) const;

        /**
         * @brief Check if a pin is exported (test helper)
         * @param pin The GPIO pin number
         * @return A true if the pin is exported, false otherwise
         */
        bool isExported(uint8_t pin) const;

        /**
         * @brief Get the direction of a pin (test helper)
         * @param pin The GPIO pin number
         * @return The PinDirection of the pin
         */
        PinDirection getDirection(uint8_t pin) const;
    };
}

#endif //MEX_HAL_GPIO_MOCK_H
