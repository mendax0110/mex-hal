#include "device_config.h"
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

    scanSPI();
    scanI2C();
    scanGPIO();
    scanUART();
}

void DeviceConfig::scanSPI()
{
    for (const auto& entry : fs::directory_iterator("/dev"))
    {
        if (entry.path().string().find("spidev") != std::string::npos)
        {
            SPIInfo info;
            info.path = entry.path();
            std::regex re(R"(spidev(\d+)\.(\d+))");
            std::smatch match;
            if (std::regex_search(info.path, match, re))
            {
                info.bus = std::stoi(match[1]);
                info.chipSelect = std::stoi(match[2]);
            }
            spiDevices_.push_back(info);
        }
    }
}

void DeviceConfig::scanI2C()
{
    for (const auto& entry : fs::directory_iterator("/dev"))
    {
        if (entry.path().string().find("i2c-") != std::string::npos)
        {
            I2CInfo info;
            info.path = entry.path();
            std::regex re(R"(i2c-(\\d+))");
            std::smatch match;
            if (std::regex_search(info.path, match, re))
            {
                info.bus = std::stoi(match[1]);
            }
            i2cDevices_.push_back(info);
        }
    }
}

void DeviceConfig::scanGPIO()
{
    const std::string base = "/sys/class/gpio";

    for (const auto& entry : fs::directory_iterator(base))
    {
        if (entry.path().filename().string().rfind("gpio", 0) == 0 &&
            entry.path().filename().string() != "gpiochip0")
        {
            GPIOInfo info;
            info.path = entry.path();
            std::string name = entry.path().filename();
            info.name = name;
            std::regex re(R"(gpio(\\d+))");
            std::smatch match;

            if (std::regex_search(name, match, re))
            {
                info.pin = std::stoi(match[1]);
            }

            std::ifstream dirFile(entry.path() / "direction");
            if (dirFile.good())
            {
                dirFile >> info.direction;
            }

            info.exported = false;
            gpioDevices_.push_back(info);
        }
    }
}

void DeviceConfig::scanUART()
{
    for (const auto& entry : fs::directory_iterator("/dev"))
    {
        std::string name = entry.path().filename();
        if (name.find("ttyS") == 0 ||
            name.find("ttyUSB") == 0 ||
            name.find("ttyAMA") == 0)
        {
            UARTInfo info;
            info.device = entry.path();
            info.path = entry.path();
            info.baudRate = entry.path().string().find("ttyUSB") != std::string::npos ? 115200 : 9600;
            uartDevices_.push_back(info);

        }
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

void DeviceConfig::printDeviceInfos()
{
    scan();
    std::stringstream ss;
    ss << "=== SPI Devices ===" << std::endl;
    for (const auto& spi : spiDevices_)
    {
        ss << "Path: " << spi.path << ", Bus: " << spi.bus << ", CS: " << spi.chipSelect << std::endl;
    }

    ss << "=== I2C Devices ===" << std::endl;
    for (const auto& i2c : i2cDevices_)
    {
        ss << "Path: " << i2c.path << ", Bus: " << i2c.bus << std::endl;
    }

    ss << "=== GPIO Devices ===" << std::endl;
    for (const auto& gpio : gpioDevices_)
    {
        ss << "Path: " << gpio.path << ", Pin: " << gpio.pin << ", Direction: " << gpio.direction << std::endl;
    }

    ss << "=== UART Devices ===" << std::endl;
    for (const auto& uart : uartDevices_)
    {
        ss << "Path: " << uart.path << ", Device: " << uart.device << ", BaudRate: " << uart.baudRate << std::endl;
    }

    std::cout << ss.str();
}
