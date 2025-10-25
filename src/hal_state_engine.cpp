#include "../include/hal/hal_state_engine.h"
#include "../include/hal/locker.h"
#include "../include/hal/core.h"
#include "adc/adc_linux.h"
#include "spi/spi_linux.h"
#include "i2c/i2c_linux.h"
#include "uart/uart_linux.h"
#include "pwm/pwm_linux.h"
#include "gpio/gpio_linux.h"
#include "timer/timer_linux.h"
#include <thread>
#include <iostream>

using namespace mex_hal;

HALStateEngine::~HALStateEngine()
{
    stop();
}

HALStateEngine& HALStateEngine::start()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (running_)
    {
        return *this;
    }

    running_ = true;
    stopRequested_ = false;
    worker_ = std::thread(&HALStateEngine::engineLoop, this);
    return *this;
}

HALStateEngine& HALStateEngine::stop()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!running_)
        {
            return *this;
        }
        stopRequested_ = true;
        cv_.notify_all();
    }

    if (worker_.joinable())
    {
        worker_.join();
    }
    running_ = false;
    return *this;
}

HALState HALStateEngine::getState() const
{
    return running_.load() ? HALState::RUNNING : HALState::STOPPED;
}

void HALStateEngine::engineLoop()
{
    const auto hal = createHAL(HALType::LINUX);
    if (!hal->configureRealtime(10))
    {
        std::cerr << "[HALStateEngine] Failed to configure real-time mode\n";
    }

    const ADCConfig adcConfig{ADCResolution::BITS_12, 3300, true};
    const auto adcDevice = hal->createADC();
    adcDevice->init(0, adcConfig);

    const auto spiDevice = hal->createSPI();
    spiDevice->init(1, 0, 1000000, SPIMode::MODE_0);

    const auto i2cDevice = hal->createI2C();
    i2cDevice->init(2);

    const UARTConfig uartConfig{9600, 8, 1, false};
    const auto uartDevice = hal->createUART();
    uartDevice->init("/dev/ttyS0", uartConfig);

    const auto pwmDevice = hal->createPWM();
    pwmDevice->init(3, 1);

    const auto gpioDevice = hal->createGPIO();
    gpioDevice->read(4);

    constexpr auto timerMode = TimerMode::PERIODIC;
    const auto timerDevice = hal->createTimer();
    timerDevice->init(timerMode);

    std::unique_lock<std::mutex> lock(mutex_);
    while (!stopRequested_)
    {
        adcDevice->read(0);

        DROP_LOCKER(lock, 10);
    }
}

void HALStateEngine::waitForStop()
{
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return stopRequested_; });
}

HALStateEngine& HALStateEngine::getInstance()
{
    static HALStateEngine instance;
    return instance;
}
