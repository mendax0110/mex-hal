#include "../include/hal/core.h"
#include "../include/hal/logger.h"
#include "gpio/gpio_linux.h"
#include "spi/spi_linux.h"
#include "i2c/i2c_linux.h"
#include "uart/uart_linux.h"
#include "pwm/pwm_linux.h"
#include "adc/adc_linux.h"
#include "timer/timer_linux.h"
#include <sched.h>
#include <sys/mman.h>
#include <stdexcept>
#include <cstdio>

using namespace mex_hal;

class HALLinux final : public HAL
{
public:
    HALLinux() = default;
    ~HALLinux() override = default;

    bool init() override 
    { 
        LOG_INFO("HAL initialized");
        return true; 
    }
    
    void shutdown() override 
    {
        LOG_INFO("HAL shutdown");
    }

    bool configureRealtime(const int32_t priority) override
    {
        LOG_DEBUG("Configuring realtime with priority: " + std::to_string(priority));
        sched_param param{};
        param.sched_priority = priority;

        if (sched_setscheduler(0, SCHED_FIFO, &param) == -1)
        {
            perror("sched_setscheduler failed");
            LOG_ERROR("Failed to set scheduler to SCHED_FIFO");
            return false;
        }

        if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1)
        {
            perror("mlockall failed");
            LOG_ERROR("Failed to lock memory");
            return false;
        }

        LOG_INFO("Realtime configured successfully with priority " + std::to_string(priority));
        return true;
    }

    [[nodiscard]] bool isRealtimeConfigured() const override
    {
        sched_param param{};
        const int policy = sched_getscheduler(0);
        if (policy == -1)
            return false;

        if (policy != SCHED_FIFO)
            return false;

        if (sched_getparam(0, &param) == -1)
            return false;

        return param.sched_priority > 0;
    }

    [[nodiscard]] RealTimeState getRealtimeState() const override
    {
        sched_param param{};
        const int policy = sched_getscheduler(0);
        if (policy == -1)
        {
            return RealTimeState::ERROR;
        }

        if (policy != SCHED_FIFO)
        {
            return RealTimeState::NOT_RUNNING;
        }

        if (sched_getparam(0, &param) == -1)
        {
            return RealTimeState::ERROR;
        }

        return (param.sched_priority > 0)
                ? RealTimeState::RUNNING
                : RealTimeState::NOT_RUNNING;
    }

    RealTimePolicy setRealTimePolicy(const RealTimePolicy policy) override
    {
        switch (policy)
        {
            case RealTimePolicy::FIFO:
            {
                if (configureRealtime(10))
                {
                    return RealTimePolicy::FIFO;
                }
                break;
            }
            case RealTimePolicy::RR:
            {
                sched_param param{};
                param.sched_priority = 10;
                if (sched_setscheduler(0, SCHED_RR, &param) == 0)
                    return RealTimePolicy::RR;
                perror("sched_setscheduler RR failed");
                break;
            }
            case RealTimePolicy::NONE:
            {
                sched_param param{};
                param.sched_priority = 0;
                if (sched_setscheduler(0, SCHED_OTHER, &param) == 0)
                    return RealTimePolicy::NONE;
                perror("sched_setscheduler NONE failed");
                break;
            }
            default:
                break;
        }

        return RealTimePolicy::INVALID;
    }

    [[nodiscard]] RealTimePolicy getRealTimePolicy() const override
    {
        const int policy = sched_getscheduler(0);
        if (policy == -1)
        {
            perror("sched_getscheduler failed");
            return RealTimePolicy::INVALID;
        }

        switch (policy)
        {
            case SCHED_FIFO:  return RealTimePolicy::FIFO;
            case SCHED_RR:    return RealTimePolicy::RR;
            case SCHED_OTHER: return RealTimePolicy::NONE;
            default:          return RealTimePolicy::INVALID;
        }
    }

    std::unique_ptr<GPIOInterface> createGPIO() override 
    { 
        LOG_DEBUG("Creating GPIO interface");
        return std::make_unique<GPIOLinux>(); 
    }
    
    std::unique_ptr<SPIInterface> createSPI() override 
    { 
        LOG_DEBUG("Creating SPI interface");
        return std::make_unique<SPILinux>(); 
    }
    
    std::unique_ptr<I2CInterface> createI2C() override 
    { 
        LOG_DEBUG("Creating I2C interface");
        return std::make_unique<I2CLinux>(); 
    }
    
    std::unique_ptr<UARTInterface> createUART() override 
    { 
        LOG_DEBUG("Creating UART interface");
        return std::make_unique<UARTLinux>(); 
    }
    
    std::unique_ptr<PWMInterface> createPWM() override 
    { 
        LOG_DEBUG("Creating PWM interface");
        return std::make_unique<PWMLinux>(); 
    }
    
    std::unique_ptr<TimerInterface> createTimer() override 
    { 
        LOG_DEBUG("Creating Timer interface");
        return std::make_unique<TimerLinux>(); 
    }
    
    std::unique_ptr<ADCInterface> createADC() override 
    { 
        LOG_DEBUG("Creating ADC interface");
        return std::make_unique<ADCLinux>(); 
    }
};

std::unique_ptr<HAL> mex_hal::createHAL(HALType type)
{
    LOG_DEBUG("Creating HAL instance");
    
    if (type == HALType::LINUX || type == HALType::AUTO)
    {
        LOG_INFO("Creating Linux HAL");
        return std::make_unique<HALLinux>();
    }

    LOG_ERROR("Unsupported HAL type: " + std::to_string(static_cast<int>(type)));
    throw std::invalid_argument("Unsupported HAL type: " + std::to_string(static_cast<int>(type)));
}
