# MEX-HAL Architecture Documentation

## Overview

MEX-HAL is a professional-grade Hardware Abstraction Layer designed for embedded real-time systems. This document describes the architectural decisions, design patterns, and implementation details.

## Architecture Layers

```
┌─────────────────────────────────────────────────────────┐
│                  Application Layer                      │
│  (User applications using HAL interfaces)               │
└─────────────────────────────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────────────┐
│              HAL Abstract Interface Layer               │
│  - HAL (Factory)                                        │
│  - GPIOInterface                                        │
│  - SPIInterface                                         │
│  - I2CInterface                                         │
│  - UARTInterface                                        │
│  - PWMInterface                                         │
│  - TimerInterface                                       │
│  - ADCInterface                                         │
└─────────────────────────────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────────────┐
│         Platform-Specific Implementation Layer          │
│  - GPIOLinux (sysfs)                                    │
│  - SPILinux (spidev)                                    │
│  - I2CLinux (i2c-dev)                                   │
│  - UARTLinux (termios)                                  │
│  - PWMLinux (sysfs)                                     │
│  - TimerLinux (std::thread)                             │
│  - ADCLinux (IIO)                                       │
└─────────────────────────────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────────────┐
│            Operating System Interface Layer             │
│  - sysfs (/sys/class/gpio, /sys/class/pwm, etc.)        │
│  - device files (/dev/spidev*, /dev/i2c-*, /dev/tty*)   │
│  - POSIX APIs (termios, ioctl, etc.)                    │
└─────────────────────────────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────────────┐
│              Linux Kernel (PREEMPT RT)                  │
│  - GPIO subsystem                                       │
│  - SPI subsystem                                        │
│  - I2C subsystem                                        │
│  - TTY/Serial subsystem                                 │
│  - PWM subsystem                                        │
│  - IIO (Industrial I/O) subsystem                       │
└─────────────────────────────────────────────────────────┘
```

## Design Patterns

### 1. Factory Pattern

The Factory Pattern is used to create HAL instances and peripheral interfaces without exposing implementation details.

```cpp
// Factory function to create HAL instance
std::unique_ptr<HAL> createHAL(const std::string& halType = "linux");

// Factory methods for peripherals
auto hal = createHAL();
auto gpio = hal->createGPIO();
auto spi = hal->createSPI();
```

**Benefits:**
- Decouples client code from concrete implementations
- Makes it easy to add new backends (simulator, mock, etc.)
- Centralized object creation logic

### 2. Strategy Pattern

Different backends (Linux, Simulator, Mock) implement the same interface, allowing runtime selection.

```cpp
class GPIOInterface
{
    virtual bool setDirection(...) = 0;
    virtual bool write(...) = 0;
    virtual PinValue read(...) = 0;
};

class GPIOLinux : public GPIOInterface { ... };
class GPIOSimulator : public GPIOInterface { ... };
```

**Benefits:**
- Runtime selection of implementation
- Easy testing with mock implementations
- Platform independence

### 3. Interface Segregation Principle

Each peripheral has its own dedicated interface rather than a monolithic HAL interface.

```cpp
// Separate interfaces for each peripheral type
class GPIOInterface { ... };
class SPIInterface { ... };
class I2CInterface { ... };
```

**Benefits:**
- Classes only depend on methods they use
- Easier to understand and maintain
- Flexible composition

### 4. RAII (Resource Acquisition Is Initialization)

Resources are automatically managed through object lifetime.

```cpp
class SPILinux
{
    int fd = -1;
    
public:
    ~SPILinux()
    {
        if (fd != -1)
        {
            close(fd);  // Automatic cleanup
        }
    }
};
```

**Benefits:**
- No manual cleanup required
- Exception-safe
- Prevents resource leaks

### 5. Observer Pattern

Callbacks are used for asynchronous events like interrupts and timer ticks.

```cpp
using InterruptCallback = std::function<void(uint8_t pin, PinValue value)>;
using TimerCallback = std::function<void()>;

gpio->setInterrupt(pin, EdgeTrigger::RISING, [](uint8_t pin, PinValue val) {
    // Handle interrupt
});
```

**Benefits:**
- Decouples event sources from event handlers
- Supports multiple subscribers
- Flexible event handling

## Component Details

### GPIO (General Purpose Input/Output)

**Interface:** `GPIOInterface`
**Linux Implementation:** `GPIOLinux`
**Kernel Interface:** sysfs (`/sys/class/gpio/`)

**Features:**
- Direction control (input/output)
- Digital read/write
- Interrupt support (rising, falling, both edges)
- Debouncing

**Implementation Notes:**
- Uses Linux sysfs GPIO interface
- Pin export/unexport for access
- Edge detection configured through sysfs attributes

### SPI (Serial Peripheral Interface)

**Interface:** `SPIInterface`
**Linux Implementation:** `SPILinux`
**Kernel Interface:** spidev (`/dev/spidev*`)

**Features:**
- Full-duplex transfer
- Configurable speed (up to hardware limits)
- Four SPI modes (0-3)
- Multiple chip select support

**Implementation Notes:**
- Uses Linux spidev driver
- ioctl() for configuration
- Efficient bulk transfers

### I2C (Inter-Integrated Circuit)

**Interface:** `I2CInterface`
**Linux Implementation:** `I2CLinux`
**Kernel Interface:** i2c-dev (`/dev/i2c-*`)

**Features:**
- Master mode operation
- 7-bit addressing
- Combined write-read operations
- Configurable speed

**Implementation Notes:**
- Uses Linux i2c-dev driver
- ioctl() for device addressing
- Standard I2C protocol support

### UART (Universal Asynchronous Receiver-Transmitter)

**Interface:** `UARTInterface`
**Linux Implementation:** `UARTLinux`
**Kernel Interface:** termios (serial ports)

**Features:**
- Configurable baud rate (up to 4 Mbaud)
- Data bits (5-8)
- Stop bits (1-2)
- Parity (none, even, odd)
- Flow control support

**Implementation Notes:**
- Uses POSIX termios API
- Supports standard and high-speed baud rates
- Non-blocking I/O support

### PWM (Pulse Width Modulation)

**Interface:** `PWMInterface`
**Linux Implementation:** `PWMLinux`
**Kernel Interface:** sysfs (`/sys/class/pwm/`)

**Features:**
- Nanosecond precision
- Period and duty cycle control
- Polarity inversion
- Enable/disable control

**Implementation Notes:**
- Uses Linux PWM sysfs interface
- Separate control of period and duty cycle
- Hardware PWM support

### Timer

**Interface:** `TimerInterface`
**Linux Implementation:** `TimerLinux`
**Kernel Interface:** C++ std::thread and std::chrono

**Features:**
- Periodic and one-shot modes
- Microsecond resolution
- Callback-based notification
- Start/stop/reset control

**Implementation Notes:**
- Uses C++ standard library for portability
- Thread-based implementation
- High-resolution timing with std::chrono

### ADC (Analog-to-Digital Converter)

**Interface:** `ADCInterface`
**Linux Implementation:** `ADCLinux`
**Kernel Interface:** IIO (`/sys/bus/iio/`)

**Features:**
- Single-shot and continuous reading
- Configurable resolution (8, 10, 12, 16 bits)
- Multi-channel support
- Voltage calculation

**Implementation Notes:**
- Uses Linux Industrial I/O (IIO) subsystem
- Channel enable/disable support
- Configurable sampling rate

## Thread Safety

### Synchronization Mechanisms

MEX-HAL provides comprehensive thread safety for all peripheral operations.

**Key Features:**
1. **Mutex Protection:** All shared mutable state protected by mutexes
2. **Atomic Operations:** Lock-free state queries and reference counting
3. **RAII Pattern:** Automatic resource cleanup and management
4. **Lock-free Reads:** State queries use atomic variables
5. **Callback Safety:** Deadlock prevention through lock release before invocation

**Thread-Safe Components:**
- **ResourceManager:** Centralized resource tracking with reference counting
- **FileDescriptor:** Thread-safe RAII wrapper for file descriptors
- **CallbackManager:** Thread-safe callback registration and invocation
- **All Peripherals:** GPIO, SPI, I2C, UART, PWM, ADC, Timer

### Real-Time Considerations

1. **Lock-free where possible:** Atomic operations instead of mutexes in critical paths
2. **Priority Inheritance:** Can be configured at system level
3. **Memory Locking:** `mlockall()` prevents page faults
4. **CPU Affinity:** Applications can pin threads to specific cores
5. **Minimal Lock Contention:** Fast operations with optimized locking

## Error Handling

### Philosophy

- Return `bool` for success/failure of operations
- Return actual values (e.g., `PinValue`, `uint16_t`) where appropriate
- No exceptions in hot paths (real-time safe)
- Validation at API boundaries

### Error Reporting

```cpp
// Boolean return for success/failure
if (!gpio->setDirection(pin, PinDirection::OUTPUT))
{
    // Handle error
}

// Value return with error indicated by special value
uint16_t value = adc->read(channel);  // Returns 0 on error
```

## Performance Considerations

### Latency

- **GPIO:** ~1-10 µs (sysfs overhead)
- **SPI:** Minimal CPU overhead, DMA capable
- **I2C:** Standard protocol timing
- **UART:** Hardware buffering
- **PWM:** Hardware-generated waveforms
- **Timer:** Sub-millisecond accuracy
- **ADC:** Hardware sampling rate

### Optimization Techniques

1. **Zero-copy:** Use of std::vector and move semantics
2. **Batch operations:** Support for multi-byte transfers
3. **Hardware offload:** Leverage kernel drivers and DMA
4. **Minimal allocations:** Pre-allocated buffers where possible

## Extensibility

### Adding a New Peripheral

1. Create interface header in `include/hal/`
2. Create Linux implementation in `src/peripheral_name/`
3. Add factory method to HAL interface
4. Implement factory method in `src/core.cpp`
5. Update CMakeLists.txt
6. Add tests and examples

### Adding a New Backend

1. Create backend implementations (e.g., `GPIOSimulator`)
2. Implement all peripheral interfaces
3. Add backend selection in `createHAL()`
4. Update documentation

## Testing Strategy

### Unit Testing

- Mock implementations for each interface
- Test individual peripheral operations
- Boundary condition testing

### Integration Testing

- Test with actual hardware when available
- Simulator backend for continuous integration
- Real-time performance testing

### Example Testing

- Example programs demonstrate correct usage
- Serve as integration tests
- Documentation validation

## Security Considerations

1. **Input Validation:** All parameters validated at API boundaries
2. **Resource Limits:** File descriptor management
3. **Permissions:** Proper udev rules for hardware access
4. **No Buffer Overflows:** Use of std::vector and safe APIs
5. **Thread Safety:** Proper synchronization

## Future Enhancements

### Planned Features

1. **Additional Peripherals:**
   - CAN bus support
   - Ethernet raw socket access
   - USB device/host support

2. **Enhanced Features:**
   - DMA support for high-speed transfers
   - Interrupt latency measurement
   - Performance profiling tools

3. **Additional Backends:**
   - Simulator for testing without hardware
   - Mock implementations for unit testing
   - Windows/macOS support

4. **Tools:**
   - Code generator for peripheral configurations
   - Real-time performance analyzer
   - Hardware abstraction validator