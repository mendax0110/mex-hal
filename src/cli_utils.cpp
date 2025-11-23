#include "../include/hal/cli_utils.h"
#include "../include/hal/error_handler.h"
#include "../include/hal/logger.h"
#include "../include/hal/gpio.h"
#include "../include/hal/adc.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>

namespace mex_hal
{
    void readGPIOPin(const std::unique_ptr<HAL>& hal, int pin)
    {
        auto& errorHandler = ErrorHandler::getInstance();
        try
        {
            auto gpio = hal->createGPIO();
            if (!gpio)
            {
                errorHandler.logError("GPIO", "Failed to create GPIO interface", 
                                    "Check if GPIO hardware is available");
                return;
            }
            
            PinValue value = gpio->read(pin);
            std::cout << "\n=== GPIO Pin State ===\n";
            std::cout << "Pin " << pin << ": " 
                    << (value == PinValue::HIGH ? "HIGH (1)" : "LOW (0)") << "\n";
            std::cout << "======================\n";
        }
        catch (const std::exception& e)
        {
            errorHandler.logError(ErrorSeverity::ERROR, ErrorCategory::HARDWARE,
                                "GPIO", "Failed to read pin: " + std::string(e.what()),
                                "Ensure GPIO pin is exported and accessible");
        }
    }

    void readADCVoltage(const std::unique_ptr<HAL>& hal, int device, int channel, float refVoltage)
    {
        auto& errorHandler = ErrorHandler::getInstance();
        try
        {
            auto adc = hal->createADC();
            if (!adc)
            {
                errorHandler.logError("ADC", "Failed to create ADC interface",
                                    "Check if ADC/IIO hardware is available");
                return;
            }
            
            ADCConfig config{ADCResolution::BITS_12, 1000, false};
            if (!adc->init(device, config))
            {
                errorHandler.logError("ADC", "Failed to initialize ADC device " + std::to_string(device),
                                    "Check if device exists in /sys/bus/iio/devices/");
                return;
            }
            
            if (!adc->enableChannel(channel))
            {
                errorHandler.logWarning("ADC", "Channel " + std::to_string(channel) + " may not be available");
            }
            
            float voltage = adc->readVoltage(channel, refVoltage);
            uint16_t raw = adc->read(channel);
            
            std::cout << "\n=== ADC Reading ===\n";
            std::cout << "Device: " << device << "\n";
            std::cout << "Channel: " << channel << "\n";
            std::cout << "Voltage: " << std::fixed << std::setprecision(3) << voltage << " V\n";
            std::cout << "Raw value: " << raw << "\n";
            std::cout << "Reference: " << refVoltage << " V\n";
            std::cout << "===================\n";
        }
        catch (const std::exception& e)
        {
            errorHandler.logError(ErrorSeverity::ERROR, ErrorCategory::HARDWARE,
                                "ADC", "Failed to read voltage: " + std::string(e.what()),
                                "Verify ADC device and channel exist");
        }
    }

    void printUsage(const char* progName)
    {
        std::cout << "\nUsage: " << progName << " [OPTIONS]\n\n";
        std::cout << "Options:\n";
        std::cout << "  -h, --help              Show this help message\n";
        std::cout << "  -s, --scan              Scan and display all hardware interfaces\n";
        std::cout << "  -c, --config            Display system configuration\n";
        std::cout << "  -g, --gpio PIN          Read GPIO pin state\n";
        std::cout << "  -a, --adc DEV:CH:VREF   Read ADC voltage (device:channel:refVoltage)\n";
        std::cout << "  -r, --report FILE       Export hardware report to file\n";
        std::cout << "  -e, --errors            Display error log\n";
        std::cout << "  -i, --interactive       Start interactive mode (default if no args)\n";
        std::cout << "\nLogging Options:\n";
        std::cout << "  -l, --log-level LEVEL   Set log level (trace, debug, info, warn, error, fatal, off)\n";
        std::cout << "  -v, --verbose           Enable verbose logging (equivalent to --log-level debug)\n";
        std::cout << "  --log-file FILE         Enable file logging to specified file\n";
        std::cout << "  --no-console-log        Disable console logging\n";
        std::cout << "\nExamples:\n";
        std::cout << "  " << progName << " --gpio 17               # Read GPIO pin 17\n";
        std::cout << "  " << progName << " --adc 0:0:3.3          # Read ADC device 0, channel 0, 3.3V ref\n";
        std::cout << "  " << progName << " --scan --report hw.txt  # Scan hardware and export report\n";
        std::cout << "  " << progName << " -v                      # Start with verbose logging\n";
        std::cout << "  " << progName << " --log-level trace --log-file hal.log  # Full logging to file\n";
        std::cout << "\n";
    }

    void exportHardwareReport(SystemConfig::ConfigStatus& status, DeviceConfig& conf, const std::string& filename);

    int processCommandLine(int argc, char* argv[], SystemConfig::ConfigStatus& status, DeviceConfig& conf)
    {
        auto& logger = Logger::getInstance();
        
        if (argc == 1)
        {
            return 0;
        }
        
        auto& errorHandler = ErrorHandler::getInstance();
        bool interactive = false;
        bool halCreated = false;
        std::unique_ptr<HAL> hal;
        
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            
            if (arg == "-l" || arg == "--log-level")
            {
                if (i + 1 < argc)
                {
                    std::string level = argv[++i];
                    if (level == "trace") logger.setLogLevel(LogLevel::TRACE);
                    else if (level == "debug") logger.setLogLevel(LogLevel::DEBUG);
                    else if (level == "info") logger.setLogLevel(LogLevel::INFO);
                    else if (level == "warn") logger.setLogLevel(LogLevel::WARN);
                    else if (level == "error") logger.setLogLevel(LogLevel::ERROR);
                    else if (level == "fatal") logger.setLogLevel(LogLevel::FATAL);
                    else if (level == "off") logger.setLogLevel(LogLevel::OFF);
                    else
                    {
                        std::cerr << "[ERROR] Invalid log level: " << level << "\n";
                        return 1;
                    }
                    LOG_INFO("Log level set to: " + level);
                }
                else
                {
                    std::cerr << "[ERROR] Missing log level\n";
                    return 1;
                }
            }
            else if (arg == "-v" || arg == "--verbose")
            {
                logger.setLogLevel(LogLevel::DEBUG);
                LOG_INFO("Verbose logging enabled");
            }
            else if (arg == "--log-file")
            {
                if (i + 1 < argc)
                {
                    std::string filename = argv[++i];
                    logger.enableFileLogging(true, filename);
                    LOG_INFO("File logging enabled to: " + filename);
                }
                else
                {
                    std::cerr << "[ERROR] Missing log file name\n";
                    return 1;
                }
            }
            else if (arg == "--no-console-log")
            {
                logger.enableConsoleLogging(false);
            }
        }
        
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            
            if (arg == "-l" || arg == "--log-level" || arg == "-v" || arg == "--verbose" || 
                arg == "--log-file" || arg == "--no-console-log")
            {
                if (arg == "-l" || arg == "--log-level" || arg == "--log-file")
                {
                    ++i;
                }
                continue;
            }
            
            if (!halCreated && arg != "-h" && arg != "--help")
            {
                LOG_INFO("Creating HAL instance");
                hal = createHAL(HALType::LINUX);
                if (!hal)
                {
                    errorHandler.logCritical("Main", "Failed to create HAL instance");
                    return 1;
                }
                
                hal->init();
                conf.scan();
                halCreated = true;
            }
        
            if (arg == "-h" || arg == "--help")
            {
                printUsage(argv[0]);
                return -1;
            }
            else if (arg == "-s" || arg == "--scan")
            {
                conf.printDeviceInfos();
            }
            else if (arg == "-c" || arg == "--config")
            {
                status = SystemConfig::check();
                SystemConfig::printReport(status);
            }
            else if (arg == "-g" || arg == "--gpio")
            {
                if (i + 1 < argc)
                {
                    int pin = std::atoi(argv[++i]);
                    readGPIOPin(hal, pin);
                }
                else
                {
                    errorHandler.logError("CLI", "Missing GPIO pin number");
                    return 1;
                }
            }
            else if (arg == "-a" || arg == "--adc")
            {
                if (i + 1 < argc)
                {
                    std::string adcSpec = argv[++i];
                    try
                    {
                        std::stringstream ss(adcSpec);
                        std::string devStr, chStr, refStr;
                        
                        if (std::getline(ss, devStr, ':') && 
                            std::getline(ss, chStr, ':') && 
                            std::getline(ss, refStr))
                        {
                            int device = std::stoi(devStr);
                            int channel = std::stoi(chStr);
                            float refVoltage = std::stof(refStr);
                            readADCVoltage(hal, device, channel, refVoltage);
                        }
                        else
                        {
                            errorHandler.logError("CLI", "Invalid ADC specification: " + adcSpec,
                                                "Use format: device:channel:refVoltage (e.g., 0:0:3.3)");
                            return 1;
                        }
                    }
                    catch (const std::exception& e)
                    {
                        errorHandler.logError("CLI", "Invalid ADC specification: " + adcSpec + " (" + e.what() + ")",
                                            "Use format: device:channel:refVoltage (e.g., 0:0:3.3)");
                        return 1;
                    }
                }
                else
                {
                    errorHandler.logError("CLI", "Missing ADC specification");
                    return 1;
                }
            }
            else if (arg == "-r" || arg == "--report")
            {
                if (i + 1 < argc)
                {
                    std::string filename = argv[++i];
                    status = SystemConfig::check();
                    exportHardwareReport(status, conf, filename);
                    std::cout << "[OK] Report exported to " << filename << "\n";
                }
                else
                {
                    errorHandler.logError("CLI", "Missing report filename");
                    return 1;
                }
            }
            else if (arg == "-e" || arg == "--errors")
            {
                errorHandler.printReport();
            }
            else if (arg == "-i" || arg == "--interactive")
            {
                interactive = true;
            }
            else
            {
                errorHandler.logWarning("CLI", "Unknown option: " + arg, "Use --help for usage information");
            }
        }
        
        if (halCreated && hal)
        {
            hal->shutdown();
        }
        
        return interactive ? 0 : -1;
    }

    void exportHardwareReport(SystemConfig::ConfigStatus& status, DeviceConfig& conf, const std::string& filename)
    {
        std::ofstream report(filename);
        
        if (!report.is_open())
        {
            std::cerr << "[ERROR] Failed to create report file: " << filename << "\n";
            return;
        }
        
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        report << "========================================\n";
        report << "MEX-HAL Hardware Detection Report\n";
        report << "Generated: " << std::ctime(&time);
        report << "========================================\n\n";
        
        report << "--- System Configuration ---\n";
        report << "Kernel: " << status.kernelVersion << "\n";
        report << "PREEMPT_RT: " << (status.hasPreemptRT ? "Yes" : "No") << "\n";
        report << "Root privileges: " << (status.isRoot ? "Yes" : "No") << "\n";
        report << "CPU governor: " << (status.cpuGovernorPerformance ? "Performance" : "Other") << "\n\n";
        
        auto spis = conf.getSpiInfos();
        auto i2cs = conf.getI2cInfos();
        auto gpios = conf.getGpioInfos();
        auto uarts = conf.getUartInfos();
        auto pwms = conf.getPwmInfos();
        auto adcs = conf.getAdcInfos();
        
        report << "--- Hardware Interfaces ---\n";
        report << "SPI devices: " << spis.size() << "\n";
        for (const auto& spi : spis)
        {
            report << "  - Bus " << spi.bus << ", CS " << spi.chipSelect << ": " << spi.path << "\n";
        }
        
        report << "\nI2C devices: " << i2cs.size() << "\n";
        for (const auto& i2c : i2cs)
        {
            report << "  - Bus " << i2c.bus << ": " << i2c.path << "\n";
        }
        
        report << "\nGPIO devices: " << gpios.size() << "\n";
        for (const auto& gpio : gpios)
        {
            report << "  - Pin " << gpio.pin << ": " << gpio.path << "\n";
        }
        
        report << "\nUART devices: " << uarts.size() << "\n";
        for (const auto& uart : uarts)
        {
            report << "  - " << uart.device << " (baudrate: " << uart.baudRate << ")\n";
        }
        
        report << "\nPWM devices: " << pwms.size() << "\n";
        for (const auto& pwm : pwms)
        {
            report << "  - Chip " << pwm.chip << ", Channel " << pwm.channel << ": " << pwm.path << "\n";
        }
        
        report << "\nADC devices: " << adcs.size() << "\n";
        for (const auto& adc : adcs)
        {
            report << "  - Device " << adc.device << ", Channel " << adc.channel << ": " << adc.name << "\n";
        }
        
        if (!status.warnings.empty())
        {
            report << "\n--- Warnings ---\n";
            for (const auto& w : status.warnings)
            {
                report << "  [!] " << w << "\n";
            }
        }
        
        if (!status.errors.empty())
        {
            report << "\n--- Errors ---\n";
            for (const auto& e : status.errors)
            {
                report << "  [X] " << e << "\n";
            }
        }
        
        report << "\n========================================\n";
        report.close();
    }
}
