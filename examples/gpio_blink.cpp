/**
 * @file gpio_blink.cpp
 * @brief Simple GPIO LED blinking example
 * 
 * This example demonstrates basic GPIO operations including:
 * - HAL initialization
 * - Real-time configuration
 * - GPIO direction setting
 * - Digital output control
 */

#include <hal/core.h>
#include <hal/gpio.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>

using namespace mex_hal;

std::atomic<bool> running{true};

void signalHandler(int signal)
{
    if (signal == SIGINT)
    {
        std::cout << "\nReceived SIGINT, shutting down..." << std::endl;
        running.store(false);
    }
}

int main(int argc, char** argv)
{
    // Set up signal handler for graceful shutdown
    std::signal(SIGINT, signalHandler);
    
    // Default GPIO pin (can be overridden by command line)
    uint8_t ledPin = 17;
    
    if (argc > 1)
    {
        ledPin = static_cast<uint8_t>(std::stoi(argv[1]));
    }
    
    std::cout << "MEX-HAL GPIO Blink Example" << std::endl;
    std::cout << "==========================" << std::endl;
    std::cout << "LED Pin: " << static_cast<int>(ledPin) << std::endl;
    std::cout << "Press Ctrl+C to exit" << std::endl;
    std::cout << std::endl;
    
    try
    {
        // Create HAL instance
        auto hal = mex_hal::createHAL(HALType::LINUX);
        
        // Initialize HAL
        if (!hal->init())
        {
            std::cerr << "Failed to initialize HAL" << std::endl;
            return 1;
        }
        
        std::cout << "HAL initialized successfully" << std::endl;
        
        // Configure real-time scheduling (priority 50)
        // Note: This requires appropriate permissions (CAP_SYS_NICE)
        if (hal->configureRealtime(50))
        {
            std::cout << "Real-time scheduling configured (priority: 50)" << std::endl;
        }
        else
        {
            std::cout << "Warning: Could not configure real-time scheduling" << std::endl;
            std::cout << "Consider running with sudo or setting appropriate capabilities" << std::endl;
        }
        
        // Create GPIO interface
        auto gpio = hal->createGPIO();
        std::cout << "GPIO interface created" << std::endl;
        
        // Configure pin as output
        if (!gpio->setDirection(ledPin, mex_hal::PinDirection::OUTPUT))
        {
            std::cerr << "Failed to set pin direction" << std::endl;
            std::cerr << "Note: You may need appropriate permissions" << std::endl;
            hal->shutdown();
            return 1;
        }
        
        std::cout << "Pin " << static_cast<int>(ledPin) << " configured as OUTPUT" << std::endl;
        std::cout << "Starting blink loop..." << std::endl;
        std::cout << std::endl;
        
        // Blink LED
        int blinkCount = 0;
        while (running.load())
        {
            // Turn LED ON
            if (!gpio->write(ledPin, mex_hal::PinValue::HIGH))
            {
                std::cerr << "Failed to write HIGH to pin" << std::endl;
                break;
            }
            std::cout << "Blink #" << ++blinkCount << " - LED ON" << std::endl;
            
            // Wait 500ms
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            if (!running.load()) break;
            
            // Turn LED OFF
            if (!gpio->write(ledPin, mex_hal::PinValue::LOW))
            {
                std::cerr << "Failed to write LOW to pin" << std::endl;
                break;
            }
            std::cout << "Blink #" << blinkCount << " - LED OFF" << std::endl;
            
            // Wait 500ms
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
        // Ensure LED is off before exiting
        gpio->write(ledPin, mex_hal::PinValue::LOW);
        
        std::cout << std::endl;
        std::cout << "Total blinks: " << blinkCount << std::endl;
        
        // Cleanup
        hal->shutdown();
        std::cout << "HAL shutdown complete" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
