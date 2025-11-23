#include "device_config.h"
#include "../include/hal/logger.h"
#include <filesystem>
#include <fstream>
#include <regex>
#include <iostream>
#include <mutex>

namespace fs = std::filesystem;

using namespace mex_hal;

void DeviceConfig::scan()
{
    std::lock_guard<std::mutex> lock(scanMutex_);

    spiDevices_.clear();
    i2cDevices_.clear();
    gpioDevices_.clear();
    uartDevices_.clear();
    pwmDevices_.clear();
    adcDevices_.clear();

    scanSPI();
    scanI2C();
    scanGPIO();
    scanUART();
    scanPWM();
    scanADC();
}

void DeviceConfig::scanSPI()
{
    try
    {
        // First, scan /dev for spidev devices (actual configured devices)
        if (fs::exists("/dev") && fs::is_directory("/dev"))
        {
            for (const auto& entry : fs::directory_iterator("/dev"))
            {
                if (entry.path().string().find("spidev") != std::string::npos)
                {
                    SPIInfo info;
                    info.path = entry.path();
                    std::regex re(R"(spidev(\d+)\.(\d+))");
                    std::smatch match;
                    std::string pathStr = info.path;
                    if (std::regex_search(pathStr, match, re))
                    {
                        info.bus = std::stoi(match[1]);
                        info.chipSelect = std::stoi(match[2]);
                    }
                    info.name = "SPI Device";
                    spiDevices_.push_back(info);
                }
            }
        }

        // Also scan /sys/class/spi_master for available SPI controllers
        const std::string spiMasterPath = "/sys/class/spi_master";
        if (fs::exists(spiMasterPath) && fs::is_directory(spiMasterPath))
        {
            for (const auto& entry : fs::directory_iterator(spiMasterPath))
            {
                std::string filename = entry.path().filename().string();
                if (filename.find("spi") == 0)
                {
                    std::regex re(R"(spi(\d+))");
                    std::smatch match;
                    if (std::regex_search(filename, match, re))
                    {
                        int bus = std::stoi(match[1]);
                        
                        // Check if we already have this bus from /dev scan
                        bool alreadyExists = false;
                        for (const auto& dev : spiDevices_)
                        {
                            if (dev.bus == bus && dev.chipSelect == -1)
                            {
                                alreadyExists = true;
                                break;
                            }
                        }
                        
                        if (!alreadyExists)
                        {
                            SPIInfo info;
                            info.bus = bus;
                            info.chipSelect = -1; // Controller, not a specific device
                            info.path = entry.path();
                            info.name = "SPI Controller";
                            spiDevices_.push_back(info);
                        }
                    }
                }
            }
        }

        // Check /sys/bus/spi/devices for additional SPI devices
        const std::string spiBusPath = "/sys/bus/spi/devices";
        if (fs::exists(spiBusPath) && fs::is_directory(spiBusPath))
        {
            for (const auto& entry : fs::directory_iterator(spiBusPath))
            {
                std::string filename = entry.path().filename().string();
                std::regex re(R"(spi(\d+)\.(\d+))");
                std::smatch match;
                if (std::regex_search(filename, match, re))
                {
                    int bus = std::stoi(match[1]);
                    int cs = std::stoi(match[2]);
                    
                    // Check if we already have this device
                    bool alreadyExists = false;
                    for (const auto& dev : spiDevices_)
                    {
                        if (dev.bus == bus && dev.chipSelect == cs)
                        {
                            alreadyExists = true;
                            break;
                        }
                    }
                    
                    if (!alreadyExists)
                    {
                        SPIInfo info;
                        info.bus = bus;
                        info.chipSelect = cs;
                        info.path = entry.path();
                        info.name = "SPI Device (sysfs)";
                        spiDevices_.push_back(info);
                    }
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        LOG_WARN(std::string("SPI device scan failed: ") + e.what());
    }
}

void DeviceConfig::scanI2C()
{
    try
    {
        // First, scan /dev for i2c-X devices (actual device nodes)
        if (fs::exists("/dev") && fs::is_directory("/dev"))
        {
            for (const auto& entry : fs::directory_iterator("/dev"))
            {
                if (entry.path().string().find("i2c-") != std::string::npos)
                {
                    I2CInfo info;
                    info.path = entry.path();
                    std::regex re(R"(i2c-(\d+))");
                    std::smatch match;
                    std::string pathStr = info.path;
                    if (std::regex_search(pathStr, match, re))
                    {
                        info.bus = std::stoi(match[1]);
                    }
                    info.name = "I2C Bus";
                    i2cDevices_.push_back(info);
                }
            }
        }

        // Also scan /sys/class/i2c-adapter for available I2C adapters
        const std::string i2cAdapterPath = "/sys/class/i2c-adapter";
        if (fs::exists(i2cAdapterPath) && fs::is_directory(i2cAdapterPath))
        {
            for (const auto& entry : fs::directory_iterator(i2cAdapterPath))
            {
                std::string filename = entry.path().filename().string();
                if (filename.find("i2c-") == 0)
                {
                    std::regex re(R"(i2c-(\d+))");
                    std::smatch match;
                    if (std::regex_search(filename, match, re))
                    {
                        int bus = std::stoi(match[1]);
                        
                        // Check if we already have this bus from /dev scan
                        bool alreadyExists = false;
                        for (const auto& dev : i2cDevices_)
                        {
                            if (dev.bus == bus)
                            {
                                alreadyExists = true;
                                break;
                            }
                        }
                        
                        if (!alreadyExists)
                        {
                            I2CInfo info;
                            info.bus = bus;
                            info.path = entry.path();
                            
                            // Try to read the adapter name
                            auto namePath = entry.path() / "name";
                            if (fs::exists(namePath))
                            {
                                std::ifstream nameFile(namePath);
                                std::getline(nameFile, info.name);
                            }
                            else
                            {
                                info.name = "I2C Adapter";
                            }
                            
                            i2cDevices_.push_back(info);
                        }
                        else
                        {
                            // Update the name if we can read it
                            auto namePath = entry.path() / "name";
                            if (fs::exists(namePath))
                            {
                                for (auto& dev : i2cDevices_)
                                {
                                    if (dev.bus == bus && dev.name.empty())
                                    {
                                        std::ifstream nameFile(namePath);
                                        std::getline(nameFile, dev.name);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Check /sys/bus/i2c/devices for connected I2C devices
        const std::string i2cBusPath = "/sys/bus/i2c/devices";
        if (fs::exists(i2cBusPath) && fs::is_directory(i2cBusPath))
        {
            for (const auto& entry : fs::directory_iterator(i2cBusPath))
            {
                std::string filename = entry.path().filename().string();
                std::regex re(R"((\d+)-([0-9a-fA-F]+))");
                std::smatch match;
                if (std::regex_search(filename, match, re))
                {
                    int bus = std::stoi(match[1]);
                    int address = std::stoi(match[2], nullptr, 16);
                    
                    // Check if this exact bus+address combination already exists
                    bool alreadyExists = false;
                    for (const auto& dev : i2cDevices_)
                    {
                        if (dev.bus == bus && dev.address == address)
                        {
                            alreadyExists = true;
                            break;
                        }
                    }
                    
                    // Add as new entry if not already present
                    if (!alreadyExists)
                    {
                        I2CInfo info;
                        info.bus = bus;
                        info.address = address;
                        info.path = entry.path();
                        info.name = "I2C Device";
                        
                        // Try to read device name
                        auto namePath = entry.path() / "name";
                        if (fs::exists(namePath))
                        {
                            std::ifstream nameFile(namePath);
                            std::getline(nameFile, info.name);
                        }
                        
                        i2cDevices_.push_back(info);
                    }
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        LOG_WARN(std::string("I2C device scan failed: ") + e.what());
    }
}

void DeviceConfig::scanGPIO()
{
    try
    {
        // Scan for modern GPIO character devices (/dev/gpiochipX)
        if (fs::exists("/dev") && fs::is_directory("/dev"))
        {
            for (const auto& entry : fs::directory_iterator("/dev"))
            {
                std::string filename = entry.path().filename().string();
                if (filename.find("gpiochip") == 0)
                {
                    GPIOInfo info;
                    info.path = entry.path();
                    info.name = filename;
                    std::regex re(R"(gpiochip(\d+))");
                    std::smatch match;

                    if (std::regex_search(filename, match, re))
                    {
                        info.pin = std::stoi(match[1]);
                    }

                    info.exported = false; // chardev doesn't use export
                    
                    // Try to read label from sysfs
                    std::string sysfsPath = "/sys/class/gpio/" + filename;
                    if (fs::exists(sysfsPath))
                    {
                        auto labelPath = fs::path(sysfsPath) / "label";
                        if (fs::exists(labelPath))
                        {
                            std::ifstream labelFile(labelPath);
                            std::string label;
                            std::getline(labelFile, label);
                            if (!label.empty())
                            {
                                info.name = filename + " (" + label + ")";
                            }
                        }
                        
                        // Read ngpio (number of GPIO lines)
                        auto ngpioPath = fs::path(sysfsPath) / "ngpio";
                        if (fs::exists(ngpioPath))
                        {
                            std::ifstream ngpioFile(ngpioPath);
                            int ngpio = 0;
                            if (ngpioFile >> ngpio)
                            {
                                info.direction = "lines: " + std::to_string(ngpio);
                            }
                        }
                    }
                    
                    gpioDevices_.push_back(info);
                }
            }
        }

        // Also scan legacy sysfs GPIO interface (/sys/class/gpio)
        const std::string base = "/sys/class/gpio";
        if (fs::exists(base) && fs::is_directory(base))
        {
            for (const auto& entry : fs::directory_iterator(base))
            {
                const std::string filename = entry.path().filename().string();
                if (filename.rfind("gpiochip", 0) == 0)
                {
                    // Check if we already found this via /dev
                    std::regex re(R"(gpiochip(\d+))");
                    std::smatch match;
                    if (std::regex_search(filename, match, re))
                    {
                        int chipNum = std::stoi(match[1]);
                        bool alreadyExists = false;
                        
                        for (const auto& dev : gpioDevices_)
                        {
                            if (dev.pin == chipNum && dev.name.find("gpiochip") != std::string::npos)
                            {
                                alreadyExists = true;
                                break;
                            }
                        }
                        
                        if (!alreadyExists)
                        {
                            GPIOInfo info;
                            info.path = entry.path();
                            info.name = filename;
                            info.pin = chipNum;
                            info.exported = false;
                            
                            // Try to read label
                            auto labelPath = entry.path() / "label";
                            if (fs::exists(labelPath))
                            {
                                std::ifstream labelFile(labelPath);
                                std::string label;
                                std::getline(labelFile, label);
                                if (!label.empty())
                                {
                                    info.name = filename + " (" + label + ")";
                                }
                            }
                            
                            // Read ngpio
                            auto ngpioPath = entry.path() / "ngpio";
                            if (fs::exists(ngpioPath))
                            {
                                std::ifstream ngpioFile(ngpioPath);
                                int ngpio = 0;
                                if (ngpioFile >> ngpio)
                                {
                                    info.direction = "lines: " + std::to_string(ngpio);
                                }
                            }
                            
                            gpioDevices_.push_back(info);
                        }
                    }
                }
                else if (filename.rfind("gpio", 0) == 0 && filename.find("gpiochip") == std::string::npos)
                {
                    // Individual exported GPIO pins
                    GPIOInfo info;
                    info.path = entry.path();
                    info.name = filename;
                    std::regex re(R"(gpio(\d+))");
                    std::smatch match;

                    if (std::regex_search(filename, match, re))
                    {
                        info.pin = std::stoi(match[1]);
                    }

                    std::ifstream dirFile(entry.path() / "direction");
                    if (dirFile.good())
                    {
                        dirFile >> info.direction;
                    }

                    info.exported = true;
                    gpioDevices_.push_back(info);
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        LOG_WARN(std::string("GPIO device scan failed: ") + e.what());
    }
}

void DeviceConfig::scanUART()
{
    if (!fs::exists("/dev") || !fs::is_directory("/dev"))
        return;

    try
    {
        for (const auto& entry : fs::directory_iterator("/dev"))
        {
            std::string name = entry.path().filename();
            
            // Detect various UART device types
            bool isUART = false;
            std::string deviceType;
            int defaultBaud = 9600;
            
            if (name.find("ttyS") == 0)
            {
                isUART = true;
                deviceType = "Serial Port";
                defaultBaud = 9600;
            }
            else if (name.find("ttyUSB") == 0)
            {
                isUART = true;
                deviceType = "USB Serial";
                defaultBaud = 115200;
            }
            else if (name.find("ttyAMA") == 0)
            {
                isUART = true;
                deviceType = "ARM PL011 UART";
                defaultBaud = 115200;
            }
            else if (name.find("ttyACM") == 0)
            {
                isUART = true;
                deviceType = "USB CDC ACM";
                defaultBaud = 115200;
            }
            else if (name.find("ttyTHS") == 0)
            {
                isUART = true;
                deviceType = "Tegra High Speed UART";
                defaultBaud = 115200;
            }
            else if (name.find("ttyO") == 0)
            {
                isUART = true;
                deviceType = "OMAP UART";
                defaultBaud = 115200;
            }
            else if (name.find("ttymxc") == 0)
            {
                isUART = true;
                deviceType = "i.MX UART";
                defaultBaud = 115200;
            }
            
            if (isUART)
            {
                UARTInfo info;
                info.device = entry.path();
                info.path = entry.path();
                info.baudRate = defaultBaud;
                info.name = name + " (" + deviceType + ")";
                
                // Try to determine if the device is actually usable
                // by checking if it's a character device
                try
                {
                    if (fs::is_character_file(entry.path()))
                    {
                        uartDevices_.push_back(info);
                    }
                    else
                    {
                        LOG_DEBUG(std::string("Skipping ") + name + " (not a character device)");
                    }
                }
                catch (const std::exception& e)
                {
                    // If we can't check, add it anyway and log the issue
                    LOG_DEBUG(std::string("Could not verify ") + name + " device type: " + e.what() + ", adding anyway");
                    uartDevices_.push_back(info);
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        LOG_WARN(std::string("UART device scan failed: ") + e.what());
    }
}

std::optional<GPIOInfo> DeviceConfig::getGPIOInfoByPin(const int pin) const
{
    for (const auto& info : gpioDevices_)
    {
        if (info.pin == pin)
        {
            return info;
        }
    }
    return std::nullopt;
}

void DeviceConfig::scanPWM()
{
    const std::string base = "/sys/class/pwm";

    if (!fs::exists(base) || !fs::is_directory(base))
        return;

    try
    {
        for (const auto& entry : fs::directory_iterator(base))
        {
            std::string filename = entry.path().filename().string();
            if (filename.rfind("pwmchip", 0) == 0)
            {
                std::regex re(R"(pwmchip(\d+))");
                std::smatch match;
                int chip = -1;

                if (std::regex_search(filename, match, re))
                {
                    chip = std::stoi(match[1]);
                }

                // Try to read device label/name
                std::string deviceName;
                auto devicePath = entry.path() / "device";
                if (fs::exists(devicePath) && fs::is_symlink(devicePath))
                {
                    auto target = fs::read_symlink(devicePath);
                    deviceName = target.filename().string();
                }

                auto npwmPath = entry.path() / "npwm";
                if (fs::exists(npwmPath))
                {
                    std::ifstream npwmFile(npwmPath);
                    int numChannels = 0;
                    if (npwmFile >> numChannels)
                    {
                        if (numChannels > 0)
                        {
                            for (int ch = 0; ch < numChannels; ++ch)
                            {
                                PWMInfo info;
                                info.chip = chip;
                                info.channel = ch;
                                info.path = entry.path();
                                
                                if (!deviceName.empty())
                                {
                                    info.name = filename + "/pwm" + std::to_string(ch) + " (" + deviceName + ")";
                                }
                                else
                                {
                                    info.name = filename + "/pwm" + std::to_string(ch);
                                }
                                
                                // Check if this PWM channel is already exported
                                auto pwmPath = entry.path() / ("pwm" + std::to_string(ch));
                                if (fs::exists(pwmPath))
                                {
                                    info.name += " [exported]";
                                }
                                
                                pwmDevices_.push_back(info);
                            }
                        }
                        else
                        {
                            // Add the chip even if it has no channels configured
                            PWMInfo info;
                            info.chip = chip;
                            info.channel = -1; // Indicates chip with no channels
                            info.path = entry.path();
                            if (!deviceName.empty())
                            {
                                info.name = filename + " (" + deviceName + ") [no channels]";
                            }
                            else
                            {
                                info.name = filename + " [no channels]";
                            }
                            pwmDevices_.push_back(info);
                        }
                    }
                }
                else
                {
                    // pwmchip exists but no npwm file - still report it
                    PWMInfo info;
                    info.chip = chip;
                    info.channel = -1;
                    info.path = entry.path();
                    if (!deviceName.empty())
                    {
                        info.name = filename + " (" + deviceName + ")";
                    }
                    else
                    {
                        info.name = filename;
                    }
                    pwmDevices_.push_back(info);
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        LOG_WARN(std::string("PWM device scan failed: ") + e.what());
    }
}

void DeviceConfig::scanADC()
{
    const std::string base = "/sys/bus/iio/devices";

    if (!fs::exists(base) || !fs::is_directory(base))
        return;

    try
    {
        for (const auto& entry : fs::directory_iterator(base))
        {
            std::string filename = entry.path().filename().string();
            if (filename.rfind("iio:device", 0) == 0)
            {
                std::regex re(R"(iio:device(\d+))");
                std::smatch match;
                int device = -1;

                if (std::regex_search(filename, match, re))
                {
                    device = std::stoi(match[1]);
                }

                auto namePath = entry.path() / "name";
                std::string deviceName;
                if (fs::exists(namePath))
                {
                    std::ifstream nameFile(namePath);
                    std::getline(nameFile, deviceName);
                }

                // Count available channels
                int channelCount = 0;
                bool hasChannels = false;
                
                for (const auto& scanEntry : fs::directory_iterator(entry.path()))
                {
                    std::string scanFile = scanEntry.path().filename().string();
                    
                    // Look for voltage channels
                    std::regex chRe(R"(in_voltage(\d+)_raw)");
                    std::smatch chMatch;

                    if (std::regex_search(scanFile, chMatch, chRe))
                    {
                        int channel = std::stoi(chMatch[1]);
                        ADCInfo info;
                        info.device = device;
                        info.channel = channel;
                        info.path = entry.path();
                        info.iioPath = scanEntry.path();
                        info.name = deviceName.empty() ? filename : deviceName;
                        
                        // Try to read scale if available
                        auto scalePath = entry.path() / ("in_voltage" + std::to_string(channel) + "_scale");
                        if (fs::exists(scalePath))
                        {
                            std::ifstream scaleFile(scalePath);
                            float scale;
                            if (scaleFile >> scale)
                            {
                                info.name += " (scale: " + std::to_string(scale) + ")";
                            }
                        }
                        
                        adcDevices_.push_back(info);
                        hasChannels = true;
                        channelCount++;
                    }
                }
                
                // If no specific channels found, but IIO device exists, still report it
                if (!hasChannels)
                {
                    ADCInfo info;
                    info.device = device;
                    info.channel = -1; // No specific channel
                    info.path = entry.path();
                    info.name = deviceName.empty() ? filename : deviceName;
                    info.name += " (IIO device, no voltage channels detected)";
                    adcDevices_.push_back(info);
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        LOG_WARN(std::string("ADC device scan failed: ") + e.what());
    }
}

void DeviceConfig::printDeviceInfos()
{
    scan();
    std::stringstream ss;
    
    ss << "\n=== Hardware Interface Detection ===" << std::endl;
    
    ss << "\n--- SPI Devices ---" << std::endl;
    if (spiDevices_.empty())
    {
        ss << "  No SPI devices detected" << std::endl;
        ss << "  [INFO] SPI may require kernel modules: spi-bcm2835, spi-bcm2708, spidev" << std::endl;
        ss << "  [INFO] Enable SPI in device tree or use: raspi-config (on Raspberry Pi)" << std::endl;
    }
    else
    {
        for (const auto& spi : spiDevices_)
        {
            ss << "  [SPI] " << (spi.name.empty() ? "Device" : spi.name);
            ss << " - Bus: " << spi.bus;
            if (spi.chipSelect != -1)
            {
                ss << ", CS: " << spi.chipSelect;
            }
            ss << std::endl;
            ss << "        Path: " << spi.path << std::endl;
        }
    }

    ss << "\n--- I2C Devices ---" << std::endl;
    if (i2cDevices_.empty())
    {
        ss << "  No I2C devices detected" << std::endl;
        ss << "  [INFO] I2C may require kernel modules: i2c-dev, i2c-bcm2835" << std::endl;
        ss << "  [INFO] Enable I2C in device tree or use: raspi-config (on Raspberry Pi)" << std::endl;
    }
    else
    {
        for (const auto& i2c : i2cDevices_)
        {
            ss << "  [I2C] " << (i2c.name.empty() ? "Bus" : i2c.name);
            ss << " - Bus: " << i2c.bus;
            if (i2c.address != -1)
            {
                ss << ", Address: 0x" << std::hex << i2c.address << std::dec;
            }
            ss << std::endl;
            ss << "        Path: " << i2c.path << std::endl;
        }
    }

    ss << "\n--- GPIO Devices ---" << std::endl;
    if (gpioDevices_.empty())
    {
        ss << "  No GPIO devices detected" << std::endl;
        ss << "  [INFO] GPIO requires kernel support via sysfs or character device interface" << std::endl;
        ss << "  [INFO] Check for /dev/gpiochip* or /sys/class/gpio/" << std::endl;
    }
    else
    {
        for (const auto& gpio : gpioDevices_)
        {
            ss << "  [GPIO] " << gpio.name;
            if (gpio.exported)
            {
                ss << " - Pin: " << gpio.pin << " [exported]";
            }
            else if (gpio.pin != -1)
            {
                ss << " - Base: " << gpio.pin;
            }
            if (!gpio.direction.empty())
            {
                ss << ", " << gpio.direction;
            }
            ss << std::endl;
            ss << "        Path: " << gpio.path << std::endl;
        }
    }

    ss << "\n--- UART Devices ---" << std::endl;
    if (uartDevices_.empty())
    {
        ss << "  No UART devices detected" << std::endl;
        ss << "  [INFO] UART devices typically appear as /dev/ttyS*, /dev/ttyAMA*, /dev/ttyUSB*" << std::endl;
        ss << "  [INFO] Some UARTs may need to be enabled in device tree" << std::endl;
    }
    else
    {
        for (const auto& uart : uartDevices_)
        {
            ss << "  [UART] " << (uart.name.empty() ? uart.device : uart.name);
            ss << " - Default baud: " << uart.baudRate << std::endl;
            ss << "        Path: " << uart.path << std::endl;
        }
    }

    ss << "\n--- PWM Devices ---" << std::endl;
    if (pwmDevices_.empty())
    {
        ss << "  No PWM devices detected" << std::endl;
        ss << "  [INFO] PWM requires /sys/class/pwm interface" << std::endl;
        ss << "  [INFO] Enable PWM in device tree or load pwm-bcm2835 module (on Raspberry Pi)" << std::endl;
    }
    else
    {
        for (const auto& pwm : pwmDevices_)
        {
            ss << "  [PWM] " << pwm.name;
            ss << " - Chip: " << pwm.chip;
            if (pwm.channel != -1)
            {
                ss << ", Channel: " << pwm.channel;
            }
            ss << std::endl;
            ss << "        Path: " << pwm.path << std::endl;
        }
    }

    ss << "\n--- ADC Devices ---" << std::endl;
    if (adcDevices_.empty())
    {
        ss << "  No ADC devices detected (IIO subsystem)" << std::endl;
        ss << "  [INFO] ADC requires Industrial I/O (IIO) subsystem support" << std::endl;
        ss << "  [INFO] Many ARM SoCs have built-in ADCs accessible via /sys/bus/iio/devices/" << std::endl;
        ss << "  [INFO] External ADCs (e.g., MCP3008) require SPI and appropriate drivers" << std::endl;
    }
    else
    {
        for (const auto& adc : adcDevices_)
        {
            ss << "  [ADC] " << adc.name;
            ss << " - Device: " << adc.device;
            if (adc.channel != -1)
            {
                ss << ", Channel: " << adc.channel;
            }
            ss << std::endl;
            ss << "        Path: " << adc.path << std::endl;
        }
    }

    ss << "\n=== Summary ===" << std::endl;
    ss << "  Total interfaces detected: " << (spiDevices_.size() + i2cDevices_.size() + gpioDevices_.size() + 
                                              uartDevices_.size() + pwmDevices_.size() + adcDevices_.size()) << std::endl;
    ss << "  SPI: " << spiDevices_.size() << ", I2C: " << i2cDevices_.size() << ", GPIO: " << gpioDevices_.size() 
       << ", UART: " << uartDevices_.size() << ", PWM: " << pwmDevices_.size() << ", ADC: " << adcDevices_.size() << std::endl;
    
    if (spiDevices_.empty() && i2cDevices_.empty() && gpioDevices_.empty() && 
        uartDevices_.empty() && pwmDevices_.empty() && adcDevices_.empty())
    {
        ss << "\n[WARN] No hardware interfaces detected!" << std::endl;
        ss << "[INFO] This could be due to:" << std::endl;
        ss << "  1. Hardware not enabled in device tree/BIOS" << std::endl;
        ss << "  2. Missing kernel modules (use 'lsmod' to check)" << std::endl;
        ss << "  3. Insufficient permissions (may need root or specific groups)" << std::endl;
        ss << "  4. Running in a virtual environment without hardware access" << std::endl;
    }

    std::cout << ss.str();
}
