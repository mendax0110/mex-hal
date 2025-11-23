/**
 * @file timer_demo.cpp
 * @brief Timer demonstration example
 * 
 * This example demonstrates:
 * - Periodic timer creation and usage
 * - One-shot timer usage
 * - Timer callbacks
 * - Timer control (start, stop, reset)
 */

#include <hal/core.h>
#include <hal/timer.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <csignal>
#include <iomanip>
#include <sstream>

std::atomic<bool> running{true};
std::atomic<int> periodicCounter{0};
std::atomic<int> oneShotCounter{0};

void signalHandler(int signal)
{
    if (signal == SIGINT)
    {
        std::cout << "\nReceived SIGINT, shutting down..." << std::endl;
        running.store(false);
    }
}

int main() {
    std::signal(SIGINT, signalHandler);
    
    std::cout << "MEX-HAL Timer Demo" << std::endl;
    std::cout << "==================" << std::endl;
    std::cout << "Press Ctrl+C to exit" << std::endl;
    std::cout << std::endl;
    
    try
    {
        // Create HAL instance
        auto hal = mex_hal::createHAL();
        
        if (!hal->init())
        {
            std::cerr << "Failed to initialize HAL" << std::endl;
            return 1;
        }
        
        // Create two timers
        auto periodicTimer = hal->createTimer();
        auto oneShotTimer = hal->createTimer();
        
        // Initialize periodic timer
        periodicTimer->init(mex_hal::TimerMode::PERIODIC);
        std::cout << "Periodic timer initialized" << std::endl;
        
        // Initialize one-shot timer
        oneShotTimer->init(mex_hal::TimerMode::ONE_SHOT);
        std::cout << "One-shot timer initialized" << std::endl;
        std::cout << std::endl;
        
        // Start periodic timer (1 second interval)
        periodicTimer->start(1000000, []() {
            int count = periodicCounter.fetch_add(1) + 1;
            auto now = std::chrono::system_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) % 1000;
            auto timer = std::chrono::system_clock::to_time_t(now);
            std::tm bt{};
            localtime_r(&timer, &bt);
            
            std::ostringstream oss;
            oss << std::put_time(&bt, "%H:%M:%S");
            oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
            
            std::cout << "[" << oss.str() << "] Periodic timer tick #" << count << std::endl;
        });
        
        std::cout << "Periodic timer started (1 second interval)" << std::endl;
        std::cout << std::endl;
        
        // Main loop
        while (running.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // Every 5 periodic ticks, fire a one-shot timer
            if (periodicCounter.load() % 5 == 0 && periodicCounter.load() > 0)
            {
                if (!oneShotTimer->isRunning())
                {
                    oneShotTimer->start(500000, []() {
                        int count = oneShotCounter.fetch_add(1) + 1;
                        std::cout << "    *** One-shot timer fired! Count: " << count << " ***" << std::endl;
                    });
                }
            }
            
            // Stop after 20 periodic ticks
            if (periodicCounter.load() >= 20)
            {
                std::cout << std::endl;
                std::cout << "Reached 20 periodic ticks, stopping..." << std::endl;
                break;
            }
        }
        
        // Stop timers
        periodicTimer->stop();
        oneShotTimer->stop();
        
        std::cout << std::endl;
        std::cout << "Summary:" << std::endl;
        std::cout << "  Periodic ticks: " << periodicCounter.load() << std::endl;
        std::cout << "  One-shot fires: " << oneShotCounter.load() << std::endl;
        
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
