#ifndef MEX_HAL_CORE_H
#define MEX_HAL_CORE_H

#include <memory>
#include <string>
#include "types.h"

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief HAL types enumeration \enum HALType
    enum class HALType
    {
        AUTO,
        LINUX,
        INVALID
    };

    /// @brief Real-time scheduling policies enumeration \enum RealTimePolicy
    enum class RealTimePolicy
    {
        NONE,
        FIFO,
        RR,
        INVALID
    };

    /// @brief Real-time state enumeration \enum RealTimeState
    enum class RealTimeState
    {
        RUNNING,
        NOT_RUNNING,
        ERROR
    };

    /// @brief HAL Interface class \class HAL
    class HAL
    {
    public:
        /**
         * @brief Virtual destructor
         */
        virtual ~HAL() = default;

        /**
         * @brief Initialize the HAL
         * @return A true if initialization was successful, false otherwise
         */
        virtual bool init() = 0;

        /**
         * @brief Shutdown the HAL
         */
        virtual void shutdown() = 0;

        /**
         * @brief Configure real-time scheduling for the process
         * @param priority The real-time priority level
         * @return A true if configuration was successful, false otherwise
         */
        virtual bool configureRealtime(int32_t priority) = 0;

        /**
         * @brief Check if real-time scheduling is configured
         * @return A true if real-time scheduling is configured, false otherwise
         */
        [[nodiscard]] virtual bool isRealtimeConfigured() const = 0;

        /**
         * @brief Get the current real-time state
         * @return The current RealTimeState
         */
        [[nodiscard]] virtual RealTimeState getRealtimeState() const = 0;

        /**
         * @brief Set the real-time scheduling policy
         * @param policy The desired RealTimePolicy
         * @return The applied RealTimePolicy
         */
        virtual RealTimePolicy setRealTimePolicy(RealTimePolicy policy) = 0;

        /**
         * @brief Get the current real-time scheduling policy
         * @return The current RealTimePolicy
         */
        [[nodiscard]] virtual RealTimePolicy getRealTimePolicy() const = 0;


        /// @brief Create GPIO interface
        virtual std::unique_ptr<GPIOInterface> createGPIO() = 0;
        virtual std::unique_ptr<SPIInterface> createSPI() = 0;
        virtual std::unique_ptr<I2CInterface> createI2C() = 0;
        virtual std::unique_ptr<UARTInterface> createUART() = 0;
        virtual std::unique_ptr<PWMInterface> createPWM() = 0;
        virtual std::unique_ptr<TimerInterface> createTimer() = 0;
        virtual std::unique_ptr<ADCInterface> createADC() = 0;
    };

    std::unique_ptr<HAL> createHAL(HALType type = HALType::LINUX);
}

#endif //MEX_HAL_CORE_H