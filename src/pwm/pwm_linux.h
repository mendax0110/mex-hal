#ifndef MEX_HAL_PWM_LINUX_H
#define MEX_HAL_PWM_LINUX_H

#include "../../include/hal/pwm.h"
#include "../../include/hal/resource_manager.h"
#include <fstream>
#include <string>
#include <stdexcept>
#include <mutex>
#include <atomic>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief PWM Linux implementation class \class PWMLinux
    class PWMLinux : public PWMInterface
    {
    private:
        uint8_t chip_ = 0;
        uint8_t channel_ = 0;
        std::atomic<uint32_t> periodNs_{0};
        std::atomic<uint32_t> dutyCycleNs_{0};
        std::atomic<bool> enabled_{false};
        uint64_t resourceId_ = 0;
        mutable std::mutex pwmMutex_;

        /**
         * @brief Get the base sysfs path for the PWM channel
         * @return A string representing the base path
         */
        std::string getBasePath() const;

        /**
         * @brief Export the PWM channel
         * @return A true if export was successful, false otherwise
         */
        bool exportPWM() const;

        /**
         * @brief Unexport the PWM channel
         * @return A true if unexport was successful, false otherwise
         */
        bool unexportPWM() const;

        /**
         * @brief Write a value to a sysfs attribute
         * @param attribute The attribute name
         * @param value The value to write
         * @return A true if the write was successful, false otherwise
         */
        bool writeSysfs(const std::string& attribute, const std::string& value) const;

        /**
         * @brief Read a value from a sysfs attribute
         * @param attribute The attribute name
         * @return The read value as a string
         */
        std::string readSysfs(const std::string& attribute) const;
        
    public:
        /**
         * @brief Constructor
         */
        PWMLinux() = default;

        /**
         * @brief Destructor
         */
        ~PWMLinux() override;

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
        uint32_t getPeriod() const override;

        /**
         * @brief Get the PWM duty cycle
         * @return The duty cycle in nanoseconds
         */
        uint32_t getDutyCycle() const override;

        /**
         * @brief Check if the PWM output is enabled
         * @return True if enabled, false otherwise
         */
        bool isEnabled() const override;
    };
}

#endif //MEX_HAL_PWM_LINUX_H