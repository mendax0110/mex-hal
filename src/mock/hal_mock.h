#ifndef MEX_HAL_MOCK_H
#define MEX_HAL_MOCK_H

#include "../../include/hal/core.h"
#include "gpio_mock.h"
#include "spi_mock.h"
#include "i2c_mock.h"
#include "uart_mock.h"
#include "pwm_mock.h"
#include "adc_mock.h"
#include "timer_mock.h"

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief Mock HAL implementation for testing \class HALMock
    class HALMock final : public HAL
    {
    public:
        /**
         * @brief Constructor
         */
        HALMock() = default;

        /**
         * @brief Destructor
         */
        ~HALMock() override = default;

        /**
         * @brief Inits the mock
         * @return A true if initialization was successful, false otherwise
         */
        bool init() override { return true; }

        /**
         * @brief Shuts down the mock
         */
        void shutdown() override {}

        /// @copydoc HAL::configureRealtime
        bool configureRealtime(const int32_t priority) override
        {
            rtPriority_ = priority;
            rtConfigured_ = true;
            return true;
        }

        /// @copydoc HAL::isRealtimeConfigured
        [[nodiscard]] bool isRealtimeConfigured() const override { return rtConfigured_; }

        /// @copydoc HAL::getRealtimeState
        [[nodiscard]] RealTimeState getRealtimeState() const override
        {
            return rtConfigured_ ? RealTimeState::RUNNING : RealTimeState::NOT_RUNNING;
        }

        /// @copydoc HAL::setRealTimePolicy
        RealTimePolicy setRealTimePolicy(const RealTimePolicy policy) override
        {
            rtPolicy_ = policy;
            return policy;
        }

        /// @copydoc HAL::getRealTimePolicy
        [[nodiscard]] RealTimePolicy getRealTimePolicy() const override { return rtPolicy_; }

        std::unique_ptr<GPIOInterface> createGPIO() override { return std::make_unique<GPIOMock>(); }
        std::unique_ptr<SPIInterface> createSPI() override { return std::make_unique<SPIMock>(); }
        std::unique_ptr<I2CInterface> createI2C() override { return std::make_unique<I2CMock>(); }
        std::unique_ptr<UARTInterface> createUART() override { return std::make_unique<UARTMock>(); }
        std::unique_ptr<PWMInterface> createPWM() override { return std::make_unique<PWMMock>(); }
        std::unique_ptr<ADCInterface> createADC() override { return std::make_unique<ADCMock>(); }
        std::unique_ptr<TimerInterface> createTimer() override { return std::make_unique<TimerMock>(); }

    private:
        bool rtConfigured_ = false;
        int32_t rtPriority_ = 0;
        RealTimePolicy rtPolicy_ = RealTimePolicy::NONE;
    };
}

#endif // MEX_HAL_MOCK_H