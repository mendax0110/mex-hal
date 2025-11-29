# MEX-HAL - Hardware Abstraction Layer for Embedded Linux

A Hardware Abstraction Layer (HAL) designed for embedded real-time systems running Ubuntu with PREEMPT RT patch. MEX-HAL provides a clean, object-oriented interface to common embedded peripherals while maintaining high performance and real-time characteristics.

## Table of Contents

- [Features](#features)
- [Architecture](#architecture)
- [Supported Peripherals](#supported-peripherals)
- [Requirements](#requirements)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [API Documentation](#api-documentation)
- [Real-Time Configuration](#real-time-configuration)
- [Examples](#examples)
- [Design Patterns](#design-patterns)
- [Contributing](#contributing)
- [License](#license)

## Architecture

MEX-HAL follows a layered architecture with clear separation of concerns:

```
┌─────────────────────────────────────────┐
│         Application Layer               │
├─────────────────────────────────────────┤
│      HAL Interface Layer (Abstract)     │
│   GPIO │ SPI │ I2C │ UART │ PWM │ etc.  │
├─────────────────────────────────────────┤
│     Platform-Specific Implementation    │
│           Linux Backend (sysfs)         │
├─────────────────────────────────────────┤
│       Operating System (Ubuntu RT)      │
└─────────────────────────────────────────┘
```

## Supported Peripherals

| Peripheral | Interface    | Linux Backend | Status |
|-----------|--------------|---------------|--------|
| GPIO      | GPIOInterface | sysfs        | DONE   |
| SPI       | SPIInterface  | spidev       | DONE   |
| I2C       | I2CInterface  | i2c-dev      | DONE   |
| UART      | UARTInterface | termios      | DONE   |
| PWM       | PWMInterface  | sysfs        | DONE   |
| Timer     | TimerInterface| POSIX timers | DONE   |
| ADC       | ADCInterface  | IIO          | DONE   |

## Requirements

### System Requirements

- **Operating System**: Ubuntu 20.04 LTS or newer (or compatible Debian-based distribution)
- **Kernel**: Linux kernel with PREEMPT RT patch (recommended) or CONFIG_PREEMPT
- **Architecture**: x86_64, ARM, ARM64

### Build Dependencies

- **Compiler**: GCC 7.0+ or Clang 5.0+ (C++17 support required)
- **CMake**: Version 3.12 or newer
- **Libraries**:
  - pthread (POSIX threads)
  - rt (real-time library)
  - libxml2

### Hardware Access

- Appropriate permissions for `/dev/spidev*`, `/dev/i2c-*`, `/sys/class/gpio`, etc.
- User must be member of `gpio`, `spi`, `i2c`, and `iio` groups

## Installation

### 1. Clone the Repository

```bash
git clone https://github.com/mendax0110/mex-hal.git
cd mex-hal
```

### 2. Install Dependencies

On Ubuntu/Debian:

```bash
sudo apt-get update
sudo apt-get install build-essential cmake pkg-config libxml2-dev
```

### 3. Build the Library

```bash
mkdir -p build
cd build
cmake ..
make
sudo make install
```

### 4. Configure Real-Time System (Optional but Recommended)

```bash
sudo ./scripts/setup_rt.sh
```

This script will:
- Configure CPU governor for performance
- Set up IRQ affinity
- Configure system limits for real-time scheduling
- Create udev rules for hardware access
- Optimize kernel parameters

### 5. Verify Installation

```bash
./scripts/check_dependencies.sh
```

## Quick Start

### Basic GPIO Example

```cpp
#include <hal/core.h>
#include <hal/gpio.h>
#include <iostream>

int main()
{
    // Create HAL instance
    auto hal = mex_hal::createHAL("linux");
    
    // Initialize HAL with real-time priority
    if (!hal->init())
    {
        std::cerr << "Failed to initialize HAL" << std::endl;
        return 1;
    }
    
    // Configure real-time scheduling (priority 80)
    hal->configureRealtime(80);
    
    // Create GPIO interface
    auto gpio = hal->createGPIO();
    
    // Configure pin 17 as output
    gpio->setDirection(17, mex_hal::PinDirection::OUTPUT);
    
    // Blink LED
    for (int i = 0; i < 10; ++i)
    {
        gpio->write(17, mex_hal::PinValue::HIGH);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        gpio->write(17, mex_hal::PinValue::LOW);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    // Cleanup
    hal->shutdown();
    
    return 0;
}
```

### SPI Communication Example

```cpp
#include <hal/core.h>
#include <hal/spi.h>
#include <vector>

int main()
{
    auto hal = mex_hal::createHAL();
    hal->init();
    
    auto spi = hal->createSPI();
    
    // Initialize SPI bus 0, CS 0, 1MHz, Mode 0
    spi->init(0, 0, 1000000, mex_hal::SPIMode::MODE_0);
    
    // Prepare data to send
    std::vector<uint8_t> txData = {0x01, 0x02, 0x03, 0x04};
    std::vector<uint8_t> rxData;
    
    // Transfer data
    if (spi->transfer(txData, rxData))
    {
        // Process received data
        for (auto byte : rxData)
        {
            std::cout << std::hex << static_cast<int>(byte) << " ";
        }
        std::cout << std::endl;
    }
    
    hal->shutdown();
    return 0;
}
```

### I2C Communication Example

```cpp
#include <hal/core.h>
#include <hal/i2c.h>

int main()
{
    auto hal = mex_hal::createHAL();
    hal->init();
    
    auto i2c = hal->createI2C();
    
    // Initialize I2C bus 1
    i2c->init(1);
    
    // Set device address (e.g., 0x48 for many sensors)
    i2c->setDeviceAddress(0x48);
    
    // Write configuration
    std::vector<uint8_t> config = {0x01, 0xC0};
    i2c->write(config);
    
    // Read data
    std::vector<uint8_t> data;
    if (i2c->read(data, 2))
    {
        uint16_t value = (data[0] << 8) | data[1];
        std::cout << "Read value: " << value << std::endl;
    }
    
    hal->shutdown();
    return 0;
}
```

### UART Communication Example

```cpp
#include <hal/core.h>
#include <hal/uart.h>

int main()
{
    auto hal = mex_hal::createHAL();
    hal->init();
    
    auto uart = hal->createUART();
    
    // Configure UART
    mex_hal::UARTConfig config;
    config.baudRate = 115200;
    config.dataBits = 8;
    config.stopBits = 1;
    config.parityEnable = false;
    
    // Initialize UART on /dev/ttyUSB0
    uart->init("/dev/ttyUSB0", config);
    
    // Send data
    std::string message = "Hello, World!";
    std::vector<uint8_t> txData(message.begin(), message.end());
    uart->write(txData);
    
    // Receive data
    std::vector<uint8_t> rxData;
    if (uart->read(rxData, 100))
    {
        std::string received(rxData.begin(), rxData.end());
        std::cout << "Received: " << received << std::endl;
    }
    
    hal->shutdown();
    return 0;
}
```

### PWM Example

```cpp
#include <hal/core.h>
#include <hal/pwm.h>

int main()
{
    auto hal = mex_hal::createHAL();
    hal->init();
    
    auto pwm = hal->createPWM();
    
    // Initialize PWM chip 0, channel 0
    pwm->init(0, 0);
    
    // Set period to 20ms (50Hz)
    pwm->setPeriod(20000000);  // nanoseconds
    
    // Set duty cycle to 50%
    pwm->setDutyCyclePercent(50.0f);
    
    // Enable PWM
    pwm->enable(true);
    
    // Run for 10 seconds
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    // Cleanup
    pwm->enable(false);
    hal->shutdown();
    
    return 0;
}
```

### Timer Example

```cpp
#include <hal/core.h>
#include <hal/timer.h>
#include <iostream>

int main()
{
    auto hal = mex_hal::createHAL();
    hal->init();
    
    auto timer = hal->createTimer();
    
    // Initialize periodic timer
    timer->init(mex_hal::TimerMode::PERIODIC);
    
    // Start timer with 1000ms interval
    int counter = 0;
    timer->start(1000000, [&counter]() {
        std::cout << "Timer tick: " << ++counter << std::endl;
    });
    
    // Run for 10 seconds
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    // Stop timer
    timer->stop();
    hal->shutdown();
    
    return 0;
}
```

### ADC Example

```cpp
#include <hal/core.h>
#include <hal/adc.h>

int main()
{
    auto hal = mex_hal::createHAL();
    hal->init();
    
    auto adc = hal->createADC();
    
    // Configure ADC
    mex_hal::ADCConfig config;
    config.resolution = mex_hal::ADCResolution::BITS_12;
    config.samplingRate = 1000;  // 1kHz
    config.continuousMode = false;
    
    // Initialize ADC device 0
    adc->init(0, config);
    
    // Enable channel 0
    adc->enableChannel(0);
    
    // Read single value
    uint16_t value = adc->read(0);
    std::cout << "ADC value: " << value << std::endl;
    
    // Read voltage (assuming 3.3V reference)
    float voltage = adc->readVoltage(0, 3.3f);
    std::cout << "Voltage: " << voltage << "V" << std::endl;
    
    hal->shutdown();
    return 0;
}
```

## API Documentation

### Core HAL Interface

```cpp
namespace mex_hal
{
    class HAL
    {
    public:
        virtual bool init() = 0;
        virtual void shutdown() = 0;
        virtual bool configureRealtime(int32_t priority) = 0;
        
        virtual std::unique_ptr<GPIOInterface> createGPIO() = 0;
        virtual std::unique_ptr<SPIInterface> createSPI() = 0;
        virtual std::unique_ptr<I2CInterface> createI2C() = 0;
        virtual std::unique_ptr<UARTInterface> createUART() = 0;
        virtual std::unique_ptr<PWMInterface> createPWM() = 0;
        virtual std::unique_ptr<TimerInterface> createTimer() = 0;
        virtual std::unique_ptr<ADCInterface> createADC() = 0;
    };
    
    std::unique_ptr<HAL> createHAL(const std::string& halType = "linux");
}
```

### GPIO Interface

```cpp
class GPIOInterface
{
public:
    virtual bool setDirection(uint8_t pin, PinDirection direction) = 0;
    virtual bool write(uint8_t pin, PinValue value) = 0;
    virtual PinValue read(uint8_t pin) = 0;
    virtual bool setInterrupt(uint8_t pin, EdgeTrigger edge, InterruptCallback callback) = 0;
    virtual bool removeInterrupt(uint8_t pin) = 0;
    virtual bool setDebounce(uint8_t pin, uint32_t debounceTimeMs) = 0;
};
```

For complete API documentation, see the header files in `include/hal/`.

## Real-Time Configuration

### Understanding Real-Time Systems

Real-time systems require deterministic behavior with guaranteed response times. MEX-HAL is designed to work optimally with Linux PREEMPT RT patch.

### Installing PREEMPT RT Kernel

1. Check if RT kernel is available in your distribution:
```bash
apt-cache search linux-image-rt
```

2. Install RT kernel:
```bash
sudo apt-get install linux-image-rt-amd64  # For x86_64
# or
sudo apt-get install linux-image-rt-arm64  # For ARM64
```

3. Reboot and select RT kernel in GRUB

### Configuring Real-Time Priority

```cpp
auto hal = mex_hal::createHAL();
hal->init();

// Set SCHED_FIFO with priority 80 (1-99, higher = more priority)
hal->configureRealtime(80);
```

### Best Practices

1. **CPU Isolation**: Isolate CPUs for real-time tasks using `isolcpus` kernel parameter
2. **IRQ Affinity**: Move hardware interrupts away from real-time CPUs
3. **Memory Locking**: Lock process memory to prevent page faults (`mlockall()`)
4. **CPU Affinity**: Pin threads to specific CPUs
5. **Avoid System Calls**: Minimize system calls in critical paths
6. **Disable Power Management**: Use performance CPU governor

## Examples

Additional examples can be found in the `examples/` directory (coming soon):

- `gpio_interrupt.cpp` - GPIO interrupt handling
- `spi_sensor.cpp` - Reading from SPI sensor
- `i2c_display.cpp` - Controlling I2C display
- `pwm_servo.cpp` - Servo motor control
- `uart_protocol.cpp` - Custom UART protocol
- `realtime_control.cpp` - Real-time control loop

## Troubleshooting

### Permission Denied Errors

If you get permission errors when accessing GPIO/SPI/I2C:

```bash
# Add your user to required groups
sudo usermod -a -G gpio,spi,i2c,iio $USER

# Log out and log back in, or run:
newgrp gpio
```

### Build Errors

```bash
# Ensure all dependencies are installed
sudo apt-get install build-essential cmake pkg-config libxml2-dev

# Clean build
rm -rf build
mkdir build && cd build
cmake .. && make
```

### Real-Time Performance Issues

```bash
# Check if RT kernel is running
uname -v | grep PREEMPT

# Run RT configuration script
sudo ./scripts/setup_rt.sh

# Verify configuration
./scripts/check_dependencies.sh
```

## Performance Considerations

- **GPIO**: ~1-10 µs latency (sysfs-based)
- **SPI**: Up to 125 MHz clock (hardware dependent)
- **I2C**: Standard (100 kHz), Fast (400 kHz), Fast Plus (1 MHz)
- **UART**: Up to 4 Mbaud (hardware dependent)
- **PWM**: Nanosecond precision
- **Timer**: Microsecond resolution
- **ADC**: Depends on hardware sampling rate

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Linux kernel developers for excellent hardware interfaces
- PREEMPT RT project for real-time Linux
- Contributors and users of MEX-HAL
