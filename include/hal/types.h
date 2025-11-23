#ifndef MEX_HAL_TYPES_H
#define MEX_HAL_TYPES_H

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

/// @brief mex_hal Hardware Abstraction Layer \class mex_hal
namespace mex_hal
{
    /// Forward declarations of interface classes
    class GPIOInterface;
    class SPIInterface;
    class I2CInterface;
    class UARTInterface;
    class PWMInterface;
    class TimerInterface;
    class ADCInterface;

    /// @brief Pin direction enumeration \enum PinDirection
    enum class PinDirection
    {
        INPUT,
        OUTPUT
    };

    /// @brief Pin value enumeration \enum PinValue
    enum class PinValue
    {
        LOW,
        HIGH
    };

    /// @brief Logic level enumeration \enum LogicLevel
    enum class LogicLevel
    {
        ACTIVE_LOW,
        ACTIVE_HIGH
    };

    /// @brief Edge trigger enumeration \enum EdgeTrigger
    enum class EdgeTrigger
    {
        RISING,
        FALLING,
        BOTH
    };

    /// @brief SPI mode enumeration \enum SPIMode
    enum class SPIMode
    {
        MODE_0, // CPOL=0, CPHA=0
        MODE_1, // CPOL=0, CPHA=1
        MODE_2, // CPOL=1, CPHA=0
        MODE_3  // CPOL=1, CPHA=1
    };

    /// @brief UART configuration structure \struct UARTConfig
    struct UARTConfig
    {
        uint32_t baudRate;
        uint8_t dataBits;
        uint8_t stopBits;
        bool parityEnable;
        bool evenParity;
    };

    /// @brief Callback type definitions
    using InterruptCallback = std::function<void(uint8_t pin, PinValue value)>;
    using TimerCallback = std::function<void()>;
    using ADCReadCallback = std::function<void(uint16_t value)>;
    using UARTReadCallback = std::function<void(const uint8_t* data, size_t length)>;
    using UARTWriteCallback = std::function<void(const uint8_t* data, size_t length)>;
    using SPIReadCallback = std::function<void(const uint8_t* data, size_t length)>;
    using SPIWriteCallback = std::function<void(const uint8_t* data, size_t length)>;
    using I2CReadCallback = std::function<void(const uint8_t* data, size_t length)>;
    using I2CWriteCallback = std::function<void(const uint8_t* data, size_t length)>;
    using PWMCallback = std::function<void(uint8_t channel, uint16_t dutyCycle)>;
    using ADCChannelCallback = std::function<void(uint8_t channel, uint16_t value)>;

    using TimerEventCallback = std::function<void(uint32_t eventId)>;
    using UARTEventCallback = std::function<void(uint32_t eventId)>;
    using SPIEventCallback = std::function<void(uint32_t eventId)>;
    using I2CEventCallback = std::function<void(uint32_t eventId)>;
    using GPIOEventCallback = std::function<void(uint32_t eventId)>;
    using PWMEventCallback = std::function<void(uint32_t eventId)>;
    using ADCEventCallback = std::function<void(uint32_t eventId)>;

    /// @brief HAL error structure \struct HALError
    struct HALError
    {
        int code;
        std::string message;
    };
}

#endif //MEX_HAL_TYPES_H