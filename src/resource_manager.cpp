#include "../include/hal/resource_manager.h"
#include "../include/hal/logger.h"
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <vector>

using namespace mex_hal;

ResourceManager& ResourceManager::getInstance()
{
    static ResourceManager instance;
    return instance;
}

uint64_t ResourceManager::registerResource(const ResourceType type, const std::string& name, void* handle)
{
    std::lock_guard<std::mutex> lock(resourceMutex_);

    const uint64_t resourceId = nextResourceId_.fetch_add(1, std::memory_order_relaxed);
        
    auto resource = std::make_unique<ResourceInfo>();
    resource->type = type;
    resource->name = name;
    resource->handle = handle;
    resource->refCount.store(1, std::memory_order_release);
    resource->inUse.store(false, std::memory_order_release);
        
    resources_[resourceId] = std::move(resource);
        
    return resourceId;
}

bool ResourceManager::unregisterResource(const uint64_t resourceId)
{
    std::lock_guard<std::mutex> lock(resourceMutex_);

    const auto it = resources_.find(resourceId);
    if (it == resources_.end())
    {
        return false;
    }
        
    // Only allow unregistration if ref count is 0
    if (it->second->refCount.load(std::memory_order_acquire) > 0)
    {
        return false;
    }
        
    resources_.erase(it);
    return true;
}

uint32_t ResourceManager::addRef(const uint64_t resourceId)
{
    std::lock_guard<std::mutex> lock(resourceMutex_);

    const auto it = resources_.find(resourceId);
    if (it == resources_.end())
    {
        return 0;
    }
        
    return it->second->refCount.fetch_add(1, std::memory_order_acq_rel) + 1;
}

uint32_t ResourceManager::release(const uint64_t resourceId)
{
    std::lock_guard<std::mutex> lock(resourceMutex_);

    const auto it = resources_.find(resourceId);
    if (it == resources_.end())
    {
        return 0;
    }

    const uint32_t oldCount = it->second->refCount.load(std::memory_order_acquire);
    if (oldCount == 0)
    {
        return 0;
    }
        
    return it->second->refCount.fetch_sub(1, std::memory_order_acq_rel) - 1;
}

uint32_t ResourceManager::getRefCount(const uint64_t resourceId) const
{
    std::lock_guard<std::mutex> lock(resourceMutex_);

    const auto it = resources_.find(resourceId);
    if (it == resources_.end())
    {
        return 0;
    }
        
    return it->second->refCount.load(std::memory_order_acquire);
}

bool ResourceManager::isInUse(const uint64_t resourceId) const
{
    std::lock_guard<std::mutex> lock(resourceMutex_);

    const auto it = resources_.find(resourceId);
    if (it == resources_.end())
    {
        return false;
    }
        
    return it->second->inUse.load(std::memory_order_acquire);
}

void ResourceManager::setInUse(const uint64_t resourceId, const bool inUse)
{
    std::lock_guard<std::mutex> lock(resourceMutex_);

    const auto it = resources_.find(resourceId);

    if (it != resources_.end())
    {
        it->second->inUse.store(inUse, std::memory_order_release);
    }
}

const ResourceInfo *ResourceManager::getResourceInfo(const uint64_t resourceId) const
{
    std::lock_guard<std::mutex> lock(resourceMutex_);

    const auto it = resources_.find(resourceId);
    if (it == resources_.end())
    {
        return nullptr;
    }

    return it->second.get();
}

size_t ResourceManager::getResourceCount() const
{
    std::lock_guard<std::mutex> lock(resourceMutex_);
    return resources_.size();
}

void ResourceManager::clearAll()
{
    std::lock_guard<std::mutex> lock(resourceMutex_);
    resources_.clear();
    memoryAddressToId_.clear();
    totalAllocatedMemory_ = 0;
}

uint64_t ResourceManager::trackMemoryAllocation(void* address, size_t size, const std::string& location)
{
    if (!address)
    {
        LOG_WARN("Attempted to track null memory allocation");
        return 0;
    }

    std::lock_guard<std::mutex> lock(resourceMutex_);

    const uint64_t resourceId = nextResourceId_.fetch_add(1, std::memory_order_relaxed);
        
    auto resource = std::make_unique<ResourceInfo>();
    resource->type = ResourceType::MEMORY_ALLOCATION;
    resource->name = location;
    resource->handle = address;
    resource->size = size;
    resource->location = location;
    resource->timestamp = std::chrono::system_clock::now();
    resource->refCount.store(1, std::memory_order_release);
    resource->inUse.store(false, std::memory_order_release);
        
    resources_[resourceId] = std::move(resource);
    memoryAddressToId_[address] = resourceId;
    totalAllocatedMemory_ += size;

    LOG_TRACE("Memory allocated: " + std::to_string(size) + " bytes at " + location);
        
    return resourceId;
}

bool ResourceManager::trackMemoryDeallocation(void* address)
{
    if (!address)
    {
        LOG_WARN("Attempted to track null memory deallocation");
        return false;
    }

    std::lock_guard<std::mutex> lock(resourceMutex_);
    
    auto addrIt = memoryAddressToId_.find(address);
    if (addrIt == memoryAddressToId_.end())
    {
        LOG_WARN("Attempted to deallocate untracked memory");
        return false;
    }

    uint64_t resourceId = addrIt->second;
    auto resIt = resources_.find(resourceId);
    if (resIt == resources_.end())
    {
        memoryAddressToId_.erase(addrIt);
        return false;
    }

    totalAllocatedMemory_ -= resIt->second->size;
    LOG_TRACE("Memory deallocated: " + std::to_string(resIt->second->size) + " bytes");

    resources_.erase(resIt);
    memoryAddressToId_.erase(addrIt);
    
    return true;
}

size_t ResourceManager::getTotalAllocatedMemory() const
{
    std::lock_guard<std::mutex> lock(resourceMutex_);
    return totalAllocatedMemory_;
}

size_t ResourceManager::getTotalMemoryAllocations() const
{
    std::lock_guard<std::mutex> lock(resourceMutex_);
    
    size_t count = 0;
    for (const auto& [id, res] : resources_)
    {
        if (res->type == ResourceType::MEMORY_ALLOCATION)
        {
            ++count;
        }
    }
    return count;
}

std::vector<const ResourceInfo*> ResourceManager::getMemoryLeaks() const
{
    std::lock_guard<std::mutex> lock(resourceMutex_);
    std::vector<const ResourceInfo*> leaks;
    
    for (const auto& [id, res] : resources_)
    {
        if (res->type == ResourceType::MEMORY_ALLOCATION && res->refCount.load(std::memory_order_acquire) == 0)
        {
            leaks.push_back(res.get());
        }
    }
    
    return leaks;
}

void ResourceManager::printMemoryReport() const
{
    std::lock_guard<std::mutex> lock(resourceMutex_);
    
    size_t memAllocCount = 0;
    size_t totalMem = 0;
    
    std::cout << "\n========================================\n";
    std::cout << "  Memory & Resource Allocation Report\n";
    std::cout << "========================================\n";
    
    for (const auto& [id, res] : resources_)
    {
        if (res->type == ResourceType::MEMORY_ALLOCATION)
        {
            memAllocCount++;
            totalMem += res->size;
        }
    }
    
    std::cout << "Total memory allocations: " << memAllocCount << "\n";
    std::cout << "Total allocated memory: " << totalMem << " bytes\n";
    std::cout << "Total resources: " << resources_.size() << "\n";
    
    if (memAllocCount > 0)
    {
        std::cout << "\n--- Active Memory Allocations ---\n";
        for (const auto& [id, res] : resources_)
        {
            if (res->type == ResourceType::MEMORY_ALLOCATION)
            {
                std::cout << "  [" << res->handle << "] " << res->size << " bytes, "
                          << "refs: " << res->refCount.load(std::memory_order_acquire) << ", "
                          << "location: " << res->location << "\n";
            }
        }
    }
    
    size_t otherResources = resources_.size() - memAllocCount;
    if (otherResources > 0)
    {
        std::cout << "\n--- Other Resources ---\n";
        for (const auto& [id, res] : resources_)
        {
            if (res->type != ResourceType::MEMORY_ALLOCATION)
            {
                std::string typeStr;
                switch (res->type)
                {
                    case ResourceType::FILE_DESCRIPTOR: typeStr = "FILE_DESCRIPTOR"; break;
                    case ResourceType::GPIO_PIN: typeStr = "GPIO_PIN"; break;
                    case ResourceType::SPI_BUS: typeStr = "SPI_BUS"; break;
                    case ResourceType::I2C_BUS: typeStr = "I2C_BUS"; break;
                    case ResourceType::UART_PORT: typeStr = "UART_PORT"; break;
                    case ResourceType::PWM_CHANNEL: typeStr = "PWM_CHANNEL"; break;
                    case ResourceType::TIMER: typeStr = "TIMER"; break;
                    case ResourceType::ADC_CHANNEL: typeStr = "ADC_CHANNEL"; break;
                    default: typeStr = "UNKNOWN"; break;
                }
                std::cout << "  [" << res->name << "] Type: " << typeStr 
                          << ", refs: " << res->refCount.load(std::memory_order_acquire) 
                          << ", in use: " << (res->inUse.load(std::memory_order_acquire) ? "yes" : "no") << "\n";
            }
        }
    }
    
    std::cout << "========================================\n";
}

// ResourceGuard implementation
ResourceGuard::ResourceGuard(const uint64_t resourceId)
    : resourceId_(resourceId)
, valid_(true)
{
    if (resourceId_ != 0)
    {
        ResourceManager::getInstance().addRef(resourceId_);
    }
}

ResourceGuard::~ResourceGuard()
{
    if (valid_ && resourceId_ != 0)
    {
        ResourceManager::getInstance().release(resourceId_);
    }
}

ResourceGuard::ResourceGuard(ResourceGuard &&other) noexcept
    : resourceId_(other.resourceId_)
    , valid_(other.valid_)
{
    other.valid_ = false;
}

ResourceGuard &ResourceGuard::operator=(ResourceGuard &&other) noexcept
{
    if (this != &other)
    {
        if (valid_ && resourceId_ != 0)
        {
            ResourceManager::getInstance().release(resourceId_);
        }

        resourceId_ = other.resourceId_;
        valid_ = other.valid_;
        other.valid_ = false;
    }
    return *this;
}
