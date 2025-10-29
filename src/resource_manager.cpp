#include "../include/hal/resource_manager.h"
#include <stdexcept>

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
