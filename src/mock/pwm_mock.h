#ifndef MEX_HAL_PWM_MOCK_H
#define MEX_HAL_PWM_MOCK_H

#include "../../include/hal/pwm.h"
#include <mutex>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief Mock PWM implementation for testing \class PWMMock
    class PWMMock final : public PWMInterface
    {
    private:
        bool initialized_ = false;
        uint8_t chip_ = 0;
        uint8_t channel_ = 0;
        uint32_t periodNs_ = 0;
        uint32_t dutyCycleNs_ = 0;
        bool enabled_ = false;
        bool invertedPolarity_ = false;
        mutable std::mutex mutex_;

    public:
        /**
         * @brief Constructor
         */
        PWMMock() = default;

        /**
         * @brief Destructor
         */
        ~PWMMock() override = default;

        /**
         * @brief Initialize the PWM channel
         * @param chip The PWM chip number
         * @param channel The PWM channel number
         * @return A true if initialization was successful, false otherwise
         */
        bool init(uint8_t chip, uint8_t channel) override;

        /**
         * @brief Enable or disable the PWM output
         * @param enabled True to enable, false to disable
         * @return A true if the operation was successful, false otherwise
         */
        bool enable(bool enabled) override;

        /**
         * @brief Set the PWM period
         * @param periodNs The period in nanoseconds
         * @return A true if the period was successfully set, false otherwise
         */
        bool setPeriod(uint32_t periodNs) override;

        /**
         * @brief Set the PWM duty cycle
         * @param dutyCycleNs The duty cycle in nanoseconds
         * @return A true if the duty cycle was successfully set, false otherwise
         */
        bool setDutyCycle(uint32_t dutyCycleNs) override;

        /**
         * @brief Set the PWM duty cycle as a percentage of the period
         * @param percent The duty cycle percentage (0.0 to 100.0)
         * @return A true if the duty cycle was successfully set, false otherwise
         */
        bool setDutyCyclePercent(float percent) override;

        /**
         * @brief Set the PWM signal polarity
         * @param invertPolarity True to invert polarity, false for normal
         * @return A true if the polarity was successfully set, false otherwise
         */
        bool setPolarity(bool invertPolarity) override;

        /**
         * @brief Get the PWM period
         * @return The period in nanoseconds
         */
        [[nodiscard]] uint32_t getPeriod() const override;

        /**
         * @brief Get the PWM duty cycle
         * @return The duty cycle in nanoseconds
         */
        [[nodiscard]] uint32_t getDutyCycle() const override;

        /**
         * @brief Check if the PWM output is enabled
         * @return True if enabled, false otherwise
         */
        [[nodiscard]] bool isEnabled() const override;

        /**
         * @brief Check if PWM is initialized (test helper)
         * @return A true if initialized, false otherwise
         */
        bool isInitialized() const;
    };
}

#endif //MEX_HAL_PWM_MOCK_H
