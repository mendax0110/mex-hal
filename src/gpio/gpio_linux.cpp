#include "gpio_linux.h"
#include "../../include/hal/callback_manager.h"
#include <poll.h>
#include <cstring>

using namespace mex_hal;

GPIOLinux::~GPIOLinux()
{
    shutdownRequested_.store(true, std::memory_order_release);

    // Wait for all interrupt threads to finish
    for (auto& [pin, thread] : interruptThreads_)
    {
        if (thread && thread->joinable())
        {
            thread->join();
        }
    }

    // Cleanup all pins
    std::lock_guard<std::mutex> lock(pinMutex_);
    for (const auto& [pin, info] : pins_)
    {
        if (info.exported)
        {
            unexportPin(pin);
        }
        if (info.resourceId != 0)
        {
            ResourceManager::getInstance().unregisterResource(info.resourceId);
        }
        if (info.callbackId != 0)
        {
            CallbackManager::getInstance().unregisterGPIOCallback(info.callbackId);
        }
    }
}

int GPIOLinux::exportPin(const uint8_t pin)
{
    std::ofstream exportFile(SYS_CLASS_GPIO_EXPORT);
    if (!exportFile.is_open()) return -1;
    exportFile << static_cast<int>(pin);
    exportFile.close();
    
    // Give kernel time to create the GPIO files
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    return 0;
}

int GPIOLinux::unexportPin(const uint8_t pin)
{
    std::ofstream unexportFile(SYS_CLASS_GPIO_UNEXPORT);
    if (!unexportFile.is_open()) return -1;
    unexportFile << static_cast<int>(pin);
    unexportFile.close();
    return 0;
}

bool GPIOLinux::setDirection(const uint8_t pin, const PinDirection direction)
{
    std::lock_guard<std::mutex> lock(pinMutex_);

    // Check if pin already exists
    auto it = pins_.find(pin);
    if (it == pins_.end())
    {
        if (exportPin(pin) != 0) return false;

        PinInfo info;
        info.exported = true;
        info.direction = direction;
        
        // Register with resource manager
        info.resourceId = ResourceManager::getInstance().registerResource(
            ResourceType::GPIO_PIN,
            "GPIO" + std::to_string(pin),
            reinterpret_cast<void*>(static_cast<uintptr_t>(pin))
        );
        
        pins_[pin] = info;
    }
    else
    {
        pins_[pin].direction = direction;
    }

    const std::string directionPath = SYS_CLASS_GPIO + std::to_string(pin) + "/direction";
    std::ofstream directionFile(directionPath);
    if (!directionFile.is_open()) return false;

    directionFile << (direction == PinDirection::OUTPUT ? "out" : "in");
    directionFile.close();
    
    ResourceManager::getInstance().setInUse(pins_[pin].resourceId, true);
    
    return true;
}

bool GPIOLinux::write(const uint8_t pin, const PinValue value)
{
    std::lock_guard<std::mutex> lock(pinMutex_);
    
    // Verify pin is configured
    const auto it = pins_.find(pin);
    if (it == pins_.end() || !it->second.exported)
    {
        return false;
    }

    const std::string valuePath = SYS_CLASS_GPIO + std::to_string(pin) + "/value";
    std::ofstream valueFile(valuePath);
    if (!valueFile.is_open()) return false;

    valueFile << (value == PinValue::HIGH ? "1" : "0");
    valueFile.close();
    return true;
}

PinValue GPIOLinux::read(const uint8_t pin)
{
    std::lock_guard<std::mutex> lock(pinMutex_);
    
    // Verify pin is configured
    const auto it = pins_.find(pin);
    if (it == pins_.end() || !it->second.exported)
    {
        return PinValue::LOW;
    }

    const std::string valuePath = SYS_CLASS_GPIO + std::to_string(pin) + "/value";
    std::ifstream valueFile(valuePath);
    if (!valueFile.is_open()) return PinValue::LOW;

    int value;
    valueFile >> value;
    valueFile.close();
    return (value == 1) ? PinValue::HIGH : PinValue::LOW;
}

void GPIOLinux::monitorInterrupt(const uint8_t pin, uint64_t callbackId) const
{
    const std::string valuePath = SYS_CLASS_GPIO + std::to_string(pin) + "/value";

    const int fd = open(valuePath.c_str(), O_RDONLY);
    if (fd < 0)
    {
        return;
    }

    // Initial dummy read to clear any pending interrupts
    char buf[3];
    ::read(fd, buf, sizeof(buf));

    pollfd pfd{};
    pfd.fd = fd;
    pfd.events = POLLPRI | POLLERR;

    while (!shutdownRequested_.load(std::memory_order_acquire))
    {
        const int ret = poll(&pfd, 1, 100); // 100ms timeout
        
        if (ret > 0 && (pfd.revents & POLLPRI))
        {
            // Clear the event
            lseek(fd, 0, SEEK_SET);
            const int len = ::read(fd, buf, sizeof(buf));
            
            if (len > 0)
            {
                // Determine pin value
                const PinValue value = (buf[0] == '1') ? PinValue::HIGH : PinValue::LOW;
                
                // Invoke callback through callback manager
                CallbackManager::getInstance().invokeGPIOCallback(pin, value);
            }
        }
    }

    ::close(fd);
}

bool GPIOLinux::setInterrupt(const uint8_t pin, EdgeTrigger edge, InterruptCallback callback)
{
    std::lock_guard<std::mutex> lock(pinMutex_);

    auto it = pins_.find(pin);
    if (it == pins_.end())
    {
        if (exportPin(pin) != 0) return false;

        PinInfo info;
        info.exported = true;
        info.direction = PinDirection::INPUT;
        info.resourceId = ResourceManager::getInstance().registerResource(
            ResourceType::GPIO_PIN,
            "GPIO" + std::to_string(pin),
            reinterpret_cast<void*>(static_cast<uintptr_t>(pin))
        );
        pins_[pin] = info;
    }

    // Set direction to input
    const std::string directionPath = SYS_CLASS_GPIO + std::to_string(pin) + "/direction";
    std::ofstream directionFile(directionPath);
    if (!directionFile.is_open()) return false;
    directionFile << "in";
    directionFile.close();

    // Configure edge detection
    const std::string edgePath = SYS_CLASS_GPIO + std::to_string(pin) + "/edge";
    std::ofstream edgeFile(edgePath);
    if (!edgeFile.is_open()) return false;

    switch (edge)
    {
        case EdgeTrigger::RISING:
            edgeFile << "rising";
            break;
        case EdgeTrigger::FALLING:
            edgeFile << "falling";
            break;
        case EdgeTrigger::BOTH:
            edgeFile << "both";
            break;
    }
    edgeFile.close();

    // Register callback
    uint64_t callbackId = CallbackManager::getInstance().registerGPIOCallback(pin, callback);
    pins_[pin].callbackId = callbackId;

    // Start interrupt monitoring thread if not already active
    if (!pins_[pin].interruptActive.exchange(true, std::memory_order_acq_rel))
    {
        interruptThreads_[pin] = std::make_unique<std::thread>(
            &GPIOLinux::monitorInterrupt, this, pin, callbackId
        );
    }

    return true;
}

bool GPIOLinux::removeInterrupt(const uint8_t pin)
{
    std::lock_guard<std::mutex> lock(pinMutex_);

    const auto it = pins_.find(pin);
    if (it == pins_.end() || !it->second.interruptActive.load(std::memory_order_acquire))
    {
        return false;
    }

    // Disable edge detection
    const std::string edgePath = SYS_CLASS_GPIO + std::to_string(pin) + "/edge";
    std::ofstream edgeFile(edgePath);
    if (!edgeFile.is_open()) return false;
    edgeFile << "none";
    edgeFile.close();

    // Mark interrupt as inactive
    pins_[pin].interruptActive.store(false, std::memory_order_release);

    // Unregister callback
    if (pins_[pin].callbackId != 0)
    {
        CallbackManager::getInstance().unregisterGPIOCallback(pins_[pin].callbackId);
        pins_[pin].callbackId = 0;
    }

    return true;
}

bool GPIOLinux::setDebounce(const uint8_t pin, const uint32_t debounceTimeMs)
{
    std::lock_guard<std::mutex> lock(pinMutex_);

    const auto it = pins_.find(pin);
    if (it == pins_.end() || !it->second.exported)
    {
        return false;
    }

    const std::string debouncePath = SYS_CLASS_GPIO + std::to_string(pin) + "/debounce";
    std::ofstream debounceFile(debouncePath);
    if (!debounceFile.is_open()) return false;
    debounceFile << debounceTimeMs;
    debounceFile.close();

    return true;
}