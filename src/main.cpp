#include "../include/hal/hal_state_engine.h"
#include "../include/hal/resource_manager.h"
#include "../include/hal/callback_manager.h"
#include "../include/hal/error_handler.h"
#include "../include/hal/logger.h"
#include "../src/device_config/device_config.h"
#include "../src/sys_config/sys_config.h"
#include "../include/hal/resource_visualizer.h"
#include "../include/hal/core.h"
#include "../include/hal/gpio.h"
#include "../include/hal/spi.h"
#include "../include/hal/i2c.h"
#include "../include/hal/uart.h"
#include "../include/hal/adc.h"
#include "../include/hal/cli_utils.h"
#include <iostream>
#include <csignal>
#include <thread>
#include <chrono>
#include <limits>
#include <atomic>
#include <termios.h>
#include <unistd.h>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <sys/utsname.h>
#include <cstring>

using namespace mex_hal;

static bool running = true;

void handleSignal(int)
{
    running = false;
    std::cout << "\n[INFO] Signal received, shutting down...\n";
}

void printMenu()
{
    std::cout << "\n========================================\n";
    std::cout << "       MEX-HAL Interactive Menu\n";
    std::cout << "========================================\n";
    std::cout << " 1. Show system configuration\n";
    std::cout << " 2. Show detected hardware interfaces\n";
    std::cout << " 3. Show HAL state\n";
    std::cout << " 4. Set real-time policy (FIFO/RR/NONE)\n";
    std::cout << " 5. Start HAL state engine\n";
    std::cout << " 6. Stop HAL state engine\n";
    std::cout << " 7. Show resource usage (live)\n";
    std::cout << " 8. Show resource graph\n";
    std::cout << " 9. Test GPIO interface\n";
    std::cout << "10. Test SPI interface\n";
    std::cout << "11. Test I2C interface\n";
    std::cout << "12. Test UART interface\n";
    std::cout << "13. Show kernel preempt details\n";
    std::cout << "14. Export hardware report\n";
    std::cout << "15. Rescan hardware interfaces\n";
    std::cout << "16. Show memory allocation report\n";
    std::cout << "17. Show error log\n";
    std::cout << "18. Configure logging\n";
    std::cout << " 0. Exit\n";
    std::cout << "========================================\n";
    std::cout << "Select option: ";
}

void setTerminalRawMode(const bool enable)
{
    static termios oldt;
    termios newt{};
    if (enable)
    {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    }
    else
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
}

void liveResourceView(ResourceVisualizer& visualizer, int intervalMs = 500)
{
    std::atomic<bool> liveRunning{true};
    setTerminalRawMode(true);

    std::cout << "\n=== Live Resource View ===\n";
    std::cout << "Press 'q' to return to menu\n";

    std::thread updateThread([&]()
    {
        while (liveRunning)
        {
            try
            {
                visualizer.gatherResourceData();
                visualizer.buildResourceGraph();
                std::cout << "\033[2J\033[H";
                visualizer.printResourceUsage();
                std::cout << "\nPress 'q' to return to menu\n";
            }
            catch (const std::exception& e)
            {
                std::cout << "[ERROR] Failed to update resource view: " << e.what() << "\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
        }
    });

    char c{};
    while (liveRunning && read(STDIN_FILENO, &c, 1) == 1)
    {
        if (c == 'q' || c == 'Q')
            liveRunning = false;
    }

    updateThread.join();
    setTerminalRawMode(false);
}

void showKernelPreemptDetails()
{
    std::cout << "\n=== Kernel Preemption Details ===\n\n";
    
    std::cout << "--- Kernel Version ---\n";
    struct utsname unameData;
    if (uname(&unameData) == 0)
    {
        std::cout << "  System: " << unameData.sysname << "\n";
        std::cout << "  Node: " << unameData.nodename << "\n";
        std::cout << "  Release: " << unameData.release << "\n";
        std::cout << "  Version: " << unameData.version << "\n";
        std::cout << "  Machine: " << unameData.machine << "\n";
    }
    else
    {
        std::cout << "  [ERROR] Unable to retrieve kernel information\n";
    }
    
    std::cout << "\n--- Preemption Configuration ---\n";
    if (uname(&unameData) == 0)
    {
        std::string configPath = "/boot/config-" + std::string(unameData.release);
        std::ifstream configFile(configPath);
        if (configFile.is_open())
        {
            std::string line;
            bool foundPreempt = false;
            while (std::getline(configFile, line))
            {
                if (line.find("CONFIG_PREEMPT") != std::string::npos)
                {
                    std::cout << "  " << line << "\n";
                    foundPreempt = true;
                }
            }
            if (!foundPreempt)
            {
                std::cout << "  No CONFIG_PREEMPT settings found\n";
            }
        }
        else
        {
            std::cout << "  Kernel configuration not accessible at " << configPath << "\n";
        }
    }
    
    std::cout << "\n--- Scheduling Policies Available ---\n";
    std::cout << "  SCHED_FIFO: " << (sched_get_priority_max(SCHED_FIFO) > 0 ? "Available" : "Not available") << "\n";
    std::cout << "  SCHED_RR: " << (sched_get_priority_max(SCHED_RR) > 0 ? "Available" : "Not available") << "\n";
    std::cout << "  SCHED_OTHER: Available\n";
    
    std::cout << "\n--- Priority Ranges ---\n";
    std::cout << "  SCHED_FIFO: " << sched_get_priority_min(SCHED_FIFO) 
              << " - " << sched_get_priority_max(SCHED_FIFO) << "\n";
    std::cout << "  SCHED_RR: " << sched_get_priority_min(SCHED_RR) 
              << " - " << sched_get_priority_max(SCHED_RR) << "\n";
    
    std::cout << "\n=================================\n";
}

void testGPIOInterface(DeviceConfig& conf)
{
    std::cout << "\n=== GPIO Interface Test ===\n";
    
    auto gpios = conf.getGpioInfos();
    if (gpios.empty())
    {
        std::cout << "[ERROR] No GPIO devices detected on this system\n";
        std::cout << "[INFO] GPIO requires sysfs interface at /sys/class/gpio\n";
        return;
    }
    
    std::cout << "[OK] Found " << gpios.size() << " GPIO device(s)\n";
    for (const auto& gpio : gpios)
    {
        std::cout << "  - " << gpio.name << " at " << gpio.path << "\n";
    }
    
    std::cout << "\n[INFO] GPIO interface available for use\n";
}

void testSPIInterface(DeviceConfig& conf)
{
    std::cout << "\n=== SPI Interface Test ===\n";
    
    auto spis = conf.getSpiInfos();
    if (spis.empty())
    {
        std::cout << "[ERROR] No SPI devices detected on this system\n";
        std::cout << "[INFO] SPI requires spidev kernel module and /dev/spidevX.Y devices\n";
        std::cout << "[INFO] Try: modprobe spi-bcm2835 (on Raspberry Pi)\n";
        return;
    }
    
    std::cout << "[OK] Found " << spis.size() << " SPI device(s)\n";
    for (const auto& spi : spis)
    {
        std::cout << "  - Bus " << spi.bus << ", CS " << spi.chipSelect << " at " << spi.path << "\n";
    }
    
    std::cout << "\n[INFO] SPI interface available for use\n";
}

void testI2CInterface(DeviceConfig& conf)
{
    std::cout << "\n=== I2C Interface Test ===\n";
    
    auto i2cs = conf.getI2cInfos();
    if (i2cs.empty())
    {
        std::cout << "[ERROR] No I2C devices detected on this system\n";
        std::cout << "[INFO] I2C requires i2c-dev kernel module and /dev/i2c-X devices\n";
        std::cout << "[INFO] Try: modprobe i2c-dev\n";
        return;
    }
    
    std::cout << "[OK] Found " << i2cs.size() << " I2C bus(es)\n";
    for (const auto& i2c : i2cs)
    {
        std::cout << "  - Bus " << i2c.bus << " at " << i2c.path << "\n";
    }
    
    std::cout << "\n[INFO] I2C interface available for use\n";
}

void testUARTInterface(DeviceConfig& conf)
{
    std::cout << "\n=== UART Interface Test ===\n";
    
    auto uarts = conf.getUartInfos();
    if (uarts.empty())
    {
        std::cout << "[ERROR] No UART devices detected on this system\n";
        return;
    }
    
    std::cout << "[OK] Found " << uarts.size() << " UART device(s)\n";
    
    int accessible = 0;
    for (const auto& uart : uarts)
    {
        std::ifstream testFile(uart.path);
        if (testFile.good())
        {
            accessible++;
            std::cout << "  [OK] " << uart.device << " (accessible)\n";
        }
        else
        {
            std::cout << "  [!] " << uart.device << " (no permissions)\n";
        }
    }
    
    std::cout << "\n[INFO] " << accessible << "/" << uarts.size() << " UART devices accessible\n";
    if (accessible < uarts.size())
    {
        std::cout << "[INFO] Add user to 'dialout' group for UART access: sudo usermod -a -G dialout $USER\n";
    }
}

int interactiveMode()
{
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    std::cout << "\n========================================\n";
    std::cout << "  MEX-HAL - Hardware Abstraction Layer\n";
    std::cout << "  Real-Time Embedded Systems Interface\n";
    std::cout << "========================================\n\n";

    try
    {
        std::cout << "[INFO] Initializing HAL components...\n";

        auto& resourceManager = ResourceManager::getInstance();
        std::cout << "[OK] ResourceManager initialized\n";

        auto& callbackManager = CallbackManager::getInstance();
        std::cout << "[OK] CallbackManager initialized\n";

        ResourceVisualizer visualizer;
        visualizer.startLiveUpdate(500);
        std::cout << "[OK] ResourceVisualizer started\n";

        auto status = SystemConfig::check();
        std::cout << "[OK] System configuration checked\n";
        
        auto& conf = DeviceConfig::getInstance();
        std::cout << "[INFO] Initial hardware scan...\n";
        conf.scan();
        std::cout << "[OK] Hardware scan complete\n";

        auto& engine = HALStateEngine::getInstance();
        engine.start();
        std::cout << "[OK] HAL State Engine started\n";

        const auto hal = createHAL(HALType::LINUX);
        if (!hal)
        {
            std::cerr << "[ERROR] Failed to create HAL instance\n";
            return 1;
        }
        std::cout << "[OK] HAL instance created\n";

        if (!hal->init())
        {
            std::cerr << "[WARN] HAL initialization had issues (may not affect functionality)\n";
        }

        std::cout << "\n[INFO] Active resources: " << resourceManager.getResourceCount() << "\n";
        std::cout << "[INFO] System ready\n";

        while (running)
        {
            printMenu();
            int choice{};
            if (!(std::cin >> choice))
            {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "[ERROR] Invalid input. Please enter a number.\n";
                continue;
            }

            try
            {
                switch (choice)
                {
                    case 1:
                        std::cout << "\n[INFO] Checking system configuration...\n";
                        status = SystemConfig::check();
                        SystemConfig::printReport(status);
                        break;

                    case 2:
                        std::cout << "\n[INFO] Scanning hardware interfaces...\n";
                        conf.scan();
                        conf.printDeviceInfos();
                        break;

                    case 3:
                    {
                        std::cout << "\n=== HAL State Information ===\n";
                        const auto state = engine.getState();
                        std::cout << "HAL State Engine: " << (state == HALState::RUNNING ? "RUNNING" : "STOPPED") << "\n";

                        const auto rtState = hal->getRealtimeState();
                        std::cout << "Realtime State: ";
                        switch (rtState)
                        {
                            case RealTimeState::RUNNING:     std::cout << "RUNNING\n"; break;
                            case RealTimeState::NOT_RUNNING: std::cout << "NOT RUNNING\n"; break;
                            case RealTimeState::ERROR:       std::cout << "ERROR\n"; break;
                        }

                        std::cout << "Active Resources: " << resourceManager.getResourceCount() << "\n";
                        std::cout << "=============================\n";
                        break;
                    }

                    case 4:
                    {
                        std::cout << "\n=== Set Real-time Policy ===\n";
                        std::cout << "0 = NONE (standard scheduling)\n";
                        std::cout << "1 = FIFO (first-in-first-out RT)\n";
                        std::cout << "2 = RR (round-robin RT)\n";
                        std::cout << "Select policy: ";
                        
                        int pol{};
                        if (std::cin >> pol)
                        {
                            auto p = RealTimePolicy::INVALID;
                            switch (pol)
                            {
                                case 0: p = hal->setRealTimePolicy(RealTimePolicy::NONE); break;
                                case 1: p = hal->setRealTimePolicy(RealTimePolicy::FIFO); break;
                                case 2: p = hal->setRealTimePolicy(RealTimePolicy::RR); break;
                                default: 
                                    std::cout << "[ERROR] Invalid policy selection\n"; 
                                    break;
                            }

                            if (pol >= 0 && pol <= 2 && p != RealTimePolicy::INVALID)
                            {
                                std::cout << "[OK] Real-time policy updated\n";
                            }
                            else if (p == RealTimePolicy::INVALID)
                            {
                                std::cout << "[ERROR] Failed to set policy (may require root privileges)\n";
                            }
                        }
                        else
                        {
                            std::cin.clear();
                            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                            std::cout << "[ERROR] Invalid input\n";
                        }
                        break;
                    }
                    case 5:
                    {
                        std::cout << "\n[INFO] Start HAL State Engine if not running...\n";
                        if (engine.getState() != HALState::RUNNING)
                        {
                            engine.start();
                            std::cout << "[OK] HAL State Engine started\n";
                        }
                        else
                        {
                            std::cout << "[INFO] HAL State Engine already running\n";
                        }
                        break;
                    }
                    case 6:
                    {
                        std::cout << "\n[INFO] Stop HAL State Engine if running...\n";
                        if (engine.getState() == HALState::RUNNING)
                        {
                            engine.stop();
                            std::cout << "[OK] HAL State Engine stopped\n";
                        }
                        else
                        {
                            std::cout << "[INFO] HAL State Engine already stopped\n";
                        }
                        break;
                    }
                    case 7:
                        liveResourceView(visualizer, 500);
                        break;

                    case 8:
                        std::cout << "\n[INFO] Building resource graph...\n";
                        visualizer.buildResourceGraph();
                        visualizer.printResourceGraph();
                        break;

                    case 9:
                        testGPIOInterface(conf);
                        break;

                    case 10:
                        testSPIInterface(conf);
                        break;

                    case 11:
                        testI2CInterface(conf);
                        break;

                    case 12:
                        testUARTInterface(conf);
                        break;

                    case 13:
                        showKernelPreemptDetails();
                        break;

                    case 14:
                        exportHardwareReport(status, conf, "mex-hal-report.txt");
                        std::cout << "[OK] Hardware report exported to: mex-hal-report.txt\n";
                        break;

                    case 15:
                        std::cout << "\n[INFO] Rescanning hardware interfaces...\n";
                        conf.scan();
                        std::cout << "[OK] Scan complete\n";
                        conf.printDeviceInfos();
                        break;

                    case 16:
                        resourceManager.printMemoryReport();
                        break;

                    case 17:
                        ErrorHandler::getInstance().printReport();
                        break;
                    
                    case 18:
                    {
                        std::cout << "\n=== Logging Configuration ===\n";
                        auto& logger = Logger::getInstance();
                        
                        std::cout << "Current log level: ";
                        switch (logger.getLogLevel())
                        {
                            case LogLevel::TRACE: std::cout << "TRACE\n"; break;
                            case LogLevel::DEBUG: std::cout << "DEBUG\n"; break;
                            case LogLevel::INFO:  std::cout << "INFO\n"; break;
                            case LogLevel::WARN:  std::cout << "WARN\n"; break;
                            case LogLevel::ERROR: std::cout << "ERROR\n"; break;
                            case LogLevel::FATAL: std::cout << "FATAL\n"; break;
                            case LogLevel::OFF:   std::cout << "OFF\n"; break;
                        }
                        
                        std::cout << "\n1. Set to TRACE (most verbose)\n";
                        std::cout << "2. Set to DEBUG\n";
                        std::cout << "3. Set to INFO (default)\n";
                        std::cout << "4. Set to WARN\n";
                        std::cout << "5. Set to ERROR\n";
                        std::cout << "6. Set to FATAL\n";
                        std::cout << "7. Set to OFF (no logging)\n";
                        std::cout << "8. Enable file logging\n";
                        std::cout << "9. Disable file logging\n";
                        std::cout << "0. Back to main menu\n";
                        std::cout << "Select option: ";
                        
                        int logChoice{};
                        if (std::cin >> logChoice)
                        {
                            switch (logChoice)
                            {
                                case 1:
                                    logger.setLogLevel(LogLevel::TRACE);
                                    std::cout << "[OK] Log level set to TRACE\n";
                                    break;
                                case 2:
                                    logger.setLogLevel(LogLevel::DEBUG);
                                    std::cout << "[OK] Log level set to DEBUG\n";
                                    break;
                                case 3:
                                    logger.setLogLevel(LogLevel::INFO);
                                    std::cout << "[OK] Log level set to INFO\n";
                                    break;
                                case 4:
                                    logger.setLogLevel(LogLevel::WARN);
                                    std::cout << "[OK] Log level set to WARN\n";
                                    break;
                                case 5:
                                    logger.setLogLevel(LogLevel::ERROR);
                                    std::cout << "[OK] Log level set to ERROR\n";
                                    break;
                                case 6:
                                    logger.setLogLevel(LogLevel::FATAL);
                                    std::cout << "[OK] Log level set to FATAL\n";
                                    break;
                                case 7:
                                    logger.setLogLevel(LogLevel::OFF);
                                    std::cout << "[OK] Logging disabled\n";
                                    break;
                                case 8:
                                {
                                    std::cout << "Enter log file path [default: mex-hal.log]: ";
                                    std::string logPath;
                                    std::cin.ignore();
                                    std::getline(std::cin, logPath);
                                    if (logPath.empty())
                                    {
                                        logPath = "mex-hal.log";
                                    }
                                    logger.enableFileLogging(true, logPath);
                                    std::cout << "[OK] File logging enabled to: " << logPath << "\n";
                                    break;
                                }
                                case 9:
                                    logger.enableFileLogging(false);
                                    std::cout << "[OK] File logging disabled\n";
                                    break;
                                case 0:
                                    break;
                                default:
                                    std::cout << "[ERROR] Invalid option\n";
                                    break;
                            }
                        }
                        else
                        {
                            std::cin.clear();
                            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                            std::cout << "[ERROR] Invalid input\n";
                        }
                        break;
                    }
                    
                    case 0:
                        running = false;
                        std::cout << "\n[INFO] Shutting down...\n";
                        break;

                    default:
                        std::cout << "[ERROR] Unknown option: " << choice << "\n";
                        break;
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << "[ERROR] Exception in menu handler: " << e.what() << "\n";
            }
        }

        std::cout << "[INFO] Stopping HAL components...\n";
        visualizer.stopLiveUpdate();
        std::cout << "[OK] ResourceVisualizer stopped\n";
        
        engine.stop();
        std::cout << "[OK] HAL State Engine stopped\n";
        
        hal->shutdown();
        std::cout << "[OK] HAL shut down\n";
        
        resourceManager.clearAll();
        std::cout << "[OK] Resources cleared\n";
        
        callbackManager.clearAll();
        std::cout << "[OK] Callbacks cleared\n";

        std::cout << "\n[INFO] MEX-HAL terminated successfully\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "\n[FATAL] Unhandled exception: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

int main(int argc, char* argv[])
{
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);
    
    SystemConfig::ConfigStatus status;
    DeviceConfig& conf = DeviceConfig::getInstance();
    
    int cmdResult = processCommandLine(argc, argv, status, conf);
    if (cmdResult == -1)
    {
        return 0;
    }
    else if (cmdResult > 0)
    {
        return cmdResult;
    }
    
    return interactiveMode();
}
