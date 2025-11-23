#ifndef MEX_HAL_PWM_H
#define MEX_HAL_PWM_H

#include "types.h"
#include <cstdint>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief PWM Interface class \class PWMInterface
    class PWMInterface
    {
    public:
        /**
         * @brief Virtual destructor
         */
        virtual ~PWMInterface() = default;

        /**
         * @brief Initialize the PWM channel
         * @param chip The PWM chip number
         * @param channel The PWM channel number
         * @return A true if initialization was successful, false otherwise
         */
        virtual bool init(uint8_t chip, uint8_t channel) = 0;

        /**
         * @brief Enable or disable the PWM output
         * @param enabled True to enable, false to disable
         * @return A true if the operation was successful, false otherwise
         */
        virtual bool enable(bool enabled) = 0;

        /**
         * @brief Set the PWM period
         * @param periodNs The period in nanoseconds
         * @return A true if the period was successfully set, false otherwise
         */
        virtual bool setPeriod(uint32_t periodNs) = 0;

        /**
         * @brief Set the PWM duty cycle
         * @param dutyCycleNs The duty cycle in nanoseconds
         * @return A true if the duty cycle was successfully set, false otherwise
         */
        virtual bool setDutyCycle(uint32_t dutyCycleNs) = 0;

        /**
         * @brief Set the PWM duty cycle as a percentage of the period
         * @param percent The duty cycle percentage (0.0 to 100.0)
         * @return A true if the duty cycle was successfully set, false otherwise
         */
        virtual bool setDutyCyclePercent(float percent) = 0;

        /**
         * @brief Set the PWM signal polarity
         * @param invertPolarity True to invert polarity, false for normal
         * @return A true if the polarity was successfully set, false otherwise
         */
        virtual bool setPolarity(bool invertPolarity) = 0;

        /**
         * @brief Get the PWM period
         * @return The period in nanoseconds
         */
        [[nodiscard]] virtual uint32_t getPeriod() const = 0;

        /**
         * @brief Get the PWM duty cycle
         * @return The duty cycle in nanoseconds
         */
        [[nodiscard]] virtual uint32_t getDutyCycle() const = 0;

        /**
         * @brief Check if the PWM output is enabled
         * @return True if enabled, false otherwise
         */
        [[nodiscard]] virtual bool isEnabled() const = 0;

    protected:
        inline static const std::string SYS_CLASS_PWM = "/sys/class/pwm/pwmchip";
    };
}

#endif //MEX_HAL_PWM_H