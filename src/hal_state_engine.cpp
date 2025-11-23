#include "../include/hal/hal_state_engine.h"
#include "../include/hal/locker.h"
#include "../include/hal/core.h"
#include "../include/hal/logger.h"
#include "device_config/device_config.h"
#include "adc/adc_linux.h"
#include "spi/spi_linux.h"
#include "i2c/i2c_linux.h"
#include "uart/uart_linux.h"
#include "pwm/pwm_linux.h"
#include "gpio/gpio_linux.h"
#include "timer/timer_linux.h"
#include <thread>
#include <iostream>
#include <sstream>
#include <memory>
#include <vector>

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
        LOG_WARN("HAL State Engine already running");
        return *this;
    }

    LOG_INFO("Starting HAL State Engine");
    running_ = true;
    stopRequested_ = false;
    worker_ = std::thread(&HALStateEngine::engineLoop, this);
    LOG_INFO("HAL State Engine started successfully");
    return *this;
}

HALStateEngine& HALStateEngine::stop()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!running_)
        {
            LOG_WARN("HAL State Engine already stopped");
            return *this;
        }
        LOG_INFO("Stopping HAL State Engine");
        stopRequested_ = true;
        cv_.notify_all();
    }

    if (worker_.joinable())
    {
        worker_.join();
    }
    running_ = false;
    LOG_INFO("HAL State Engine stopped successfully");
    return *this;
}

HALState HALStateEngine::getState() const
{
    return running_.load() ? HALState::RUNNING : HALState::STOPPED;
}

void HALStateEngine::engineLoop()
{
    LOG_DEBUG("Engine loop starting");
    
    const auto hal = createHAL(HALType::LINUX);
    if (!hal->configureRealtime(10))
    {
        LOG_ERROR("Failed to configure real-time mode");
    }
    else
    {
        LOG_INFO("Real-time mode configured with priority 10");
    }

    auto& deviceConfig = DeviceConfig::getInstance();
    LOG_INFO("Scanning for available devices");
    deviceConfig.scan();
    
    std::vector<std::unique_ptr<ADCInterface>> adcDevices;
    std::vector<std::unique_ptr<SPIInterface>> spiDevices;
    std::vector<std::unique_ptr<I2CInterface>> i2cDevices;
    std::vector<std::unique_ptr<UARTInterface>> uartDevices;
    std::vector<std::unique_ptr<PWMInterface>> pwmDevices;
    std::vector<std::unique_ptr<GPIOInterface>> gpioDevices;
    std::vector<std::unique_ptr<TimerInterface>> timerDevices;
    
    const auto& adcInfos = deviceConfig.getAdcInfos();
    LOG_INFO("Found " + std::to_string(adcInfos.size()) + " ADC device(s)");
    for (const auto& adcInfo : adcInfos)
    {
        LOG_DEBUG("Initializing ADC device: " + adcInfo.name + " at " + adcInfo.path);
        try
        {
            auto adcDevice = hal->createADC();
            constexpr ADCConfig adcConfig{ADCResolution::BITS_12, 3300, true};
            adcDevice->init(adcInfo.device, adcConfig);
            adcDevices.push_back(std::move(adcDevice));
            LOG_INFO("ADC device initialized: " + adcInfo.name);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("Failed to initialize ADC device: " + std::string(e.what()));
        }
    }
    
    const auto& spiInfos = deviceConfig.getSpiInfos();
    LOG_INFO("Found " + std::to_string(spiInfos.size()) + " SPI device(s)");
    for (const auto& spiInfo : spiInfos)
    {
        if (spiInfo.chipSelect < 0)
        {
            LOG_DEBUG("Skipping SPI controller (no chip select): " + spiInfo.name);
            continue;
        }
        
        LOG_DEBUG("Initializing SPI device: bus " + std::to_string(spiInfo.bus) + 
                  " CS " + std::to_string(spiInfo.chipSelect));
        try
        {
            auto spiDevice = hal->createSPI();
            spiDevice->init(spiInfo.bus, spiInfo.chipSelect, 1000000, SPIMode::MODE_0);
            spiDevices.push_back(std::move(spiDevice));
            LOG_INFO("SPI device initialized: bus " + std::to_string(spiInfo.bus) + 
                     " CS " + std::to_string(spiInfo.chipSelect));
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("Failed to initialize SPI device: " + std::string(e.what()));
        }
    }
    
    const auto& i2cInfos = deviceConfig.getI2cInfos();
    LOG_INFO("Found " + std::to_string(i2cInfos.size()) + " I2C bus(es)");
    for (const auto& i2cInfo : i2cInfos)
    {
        LOG_DEBUG("Initializing I2C bus: " + std::to_string(i2cInfo.bus) + " at " + i2cInfo.path);
        try
        {
            auto i2cDevice = hal->createI2C();
            i2cDevice->init(i2cInfo.bus);
            i2cDevices.push_back(std::move(i2cDevice));
            LOG_INFO("I2C bus initialized: " + std::to_string(i2cInfo.bus));
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("Failed to initialize I2C device: " + std::string(e.what()));
        }
    }
    
    const auto& uartInfos = deviceConfig.getUartInfos();
    LOG_INFO("Found " + std::to_string(uartInfos.size()) + " UART device(s)");
    for (const auto& uartInfo : uartInfos)
    {
        LOG_DEBUG("Initializing UART device: " + uartInfo.device + " at " + uartInfo.path);
        try
        {
            auto uartDevice = hal->createUART();
            constexpr UARTConfig uartConfig{9600, 8, 1, false};
            uartDevice->init(uartInfo.path, uartConfig);
            uartDevices.push_back(std::move(uartDevice));
            LOG_INFO("UART device initialized: " + uartInfo.device);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("Failed to initialize UART device: " + std::string(e.what()));
        }
    }
    
    const auto& pwmInfos = deviceConfig.getPwmInfos();
    LOG_INFO("Found " + std::to_string(pwmInfos.size()) + " PWM device(s)");
    for (const auto& pwmInfo : pwmInfos)
    {
        LOG_DEBUG("Initializing PWM device: chip " + std::to_string(pwmInfo.chip) + 
                  " channel " + std::to_string(pwmInfo.channel));
        try
        {
            auto pwmDevice = hal->createPWM();
            pwmDevice->init(pwmInfo.chip, pwmInfo.channel);
            pwmDevices.push_back(std::move(pwmDevice));
            LOG_INFO("PWM device initialized: chip " + std::to_string(pwmInfo.chip) + 
                     " channel " + std::to_string(pwmInfo.channel));
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("Failed to initialize PWM device: " + std::string(e.what()));
        }
    }
    
    const auto& gpioInfos = deviceConfig.getGpioInfos();
    if (!gpioInfos.empty())
    {
        LOG_INFO("GPIO interface available with " + std::to_string(gpioInfos.size()) + " controller(s)");
        try
        {
            auto gpioDevice = hal->createGPIO();
            gpioDevices.push_back(std::move(gpioDevice));
            LOG_DEBUG("GPIO interface initialized");
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("Failed to initialize GPIO interface: " + std::string(e.what()));
        }
    }
    
    LOG_INFO("Creating timer interface");
    try
    {
        auto timerDevice = hal->createTimer();
        constexpr auto timerMode = TimerMode::PERIODIC;
        timerDevice->init(timerMode);
        timerDevices.push_back(std::move(timerDevice));
        LOG_DEBUG("Timer interface initialized");
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Failed to initialize timer: " + std::string(e.what()));
    }

    LOG_INFO("Engine loop running with " + 
             std::to_string(adcDevices.size() + spiDevices.size() + i2cDevices.size() + 
                            uartDevices.size() + pwmDevices.size() + gpioDevices.size() + 
                            timerDevices.size()) + " active device(s)");

    std::unique_lock<std::mutex> lock(mutex_);
    while (!stopRequested_)
    {
        for (auto& adc : adcDevices)
        {
            try
            {
                adc->read(0);
                LOG_TRACE("ADC read cycle completed");
            }
            catch (const std::exception& e)
            {
                LOG_ERROR("ADC read error: " + std::string(e.what()));
            }
        }

        DROP_LOCKER(lock, 10);
    }
    
    LOG_DEBUG("Engine loop exiting");
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
