#ifndef MEX_HAL_RESOURCE_MANAGER_H
#define MEX_HAL_RESOURCE_MANAGER_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <string>
#include <atomic>
#include <vector>
#include <chrono>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /**
     * @brief Resource type enumeration for tracking different resource types
     */
    enum class ResourceType
    {
        FILE_DESCRIPTOR,
        GPIO_PIN,
        SPI_BUS,
        I2C_BUS,
        UART_PORT,
        PWM_CHANNEL,
        TIMER,
        ADC_CHANNEL,
        MEMORY_ALLOCATION
    };

    /**
     * @brief Resource information structure
     */
    struct ResourceInfo
    {
        ResourceType type;
        std::string name;
        void* handle;
        std::atomic<uint32_t> refCount{0};
        std::atomic<bool> inUse{false};
        size_t size{0};
        std::chrono::system_clock::time_point timestamp;
        std::string location;
    };

    /**
     * @brief Thread-safe resource manager with allocation tracking and reference counting
     * 
     * Implements singleton pattern for centralized resource management.
     * Provides thread-safe allocation tracking and reference counting for
     * all hardware resources used by the HAL.
     */
    class ResourceManager
    {
    public:
        /**
         * @brief Get singleton instance of resource manager
         */
        static ResourceManager& getInstance();

        /**
         * @brief Register a resource for tracking
         * @param type Resource type
         * @param name Resource name/identifier
         * @param handle Pointer to resource handle
         * @return Resource ID for future reference
         */
        uint64_t registerResource(ResourceType type, const std::string& name, void* handle);

        /**
         * @brief Unregister a resource
         * @param resourceId Resource ID returned from registerResource
         * @return true if successful
         */
        bool unregisterResource(uint64_t resourceId);

        /**
         * @brief Increment reference count for a resource
         * @param resourceId Resource ID
         * @return New reference count
         */
        uint32_t addRef(uint64_t resourceId);

        /**
         * @brief Decrement reference count for a resource
         * @param resourceId Resource ID
         * @return New reference count
         */
        uint32_t release(uint64_t resourceId);

        /**
         * @brief Get current reference count
         * @param resourceId Resource ID
         * @return Current reference count
         */
        uint32_t getRefCount(uint64_t resourceId) const;

        /**
         * @brief Check if resource is in use
         * @param resourceId Resource ID
         * @return true if resource is in use
         */
        bool isInUse(uint64_t resourceId) const;

        /**
         * @brief Mark resource as in use
         * @param resourceId Resource ID
         * @param inUse Usage flag
         */
        void setInUse(uint64_t resourceId, bool inUse);

        /**
         * @brief Get resource info
         * @param resourceId Resource ID
         * @return Pointer to resource info, nullptr if not found
         */
        const ResourceInfo* getResourceInfo(uint64_t resourceId) const;

        /**
         * @brief Get total count of registered resources
         * @return Number of registered resources
         */
        size_t getResourceCount() const;

        /**
         * @brief Clear all resources (for cleanup)
         */
        void clearAll();

        /**
         * @brief Track a memory allocation
         * @param address Pointer to the allocated memory
         * @param size Size of the allocation in bytes
         * @param location Description of the allocation location
         * @return Resource ID for the tracked allocation
         */
        uint64_t trackMemoryAllocation(void* address, size_t size, const std::string& location);

        /**
         * @brief Track a memory deallocation
         * @param address Pointer to the memory being deallocated
         * @return true if successful
         */
        bool trackMemoryDeallocation(void* address);

        /**
         * @brief Get total allocated memory
         * @return Total bytes allocated
         */
        size_t getTotalAllocatedMemory() const;

        /**
         * @brief Get total number of memory allocations
         * @return Number of tracked allocations
         */
        size_t getTotalMemoryAllocations() const;

        /**
         * @brief Get list of memory leaks (allocations with ref count 0)
         * @return Vector of resource info for leaked memory
         */
        std::vector<const ResourceInfo*> getMemoryLeaks() const;

        /**
         * @brief Print memory allocation report
         */
        void printMemoryReport() const;

        /**
         * @brief Add a dependency between two resources (e.g., resourceId depends on dependsOnId)
         * @param resourceId The resource that has a dependency
         * @param dependsOnId The resource that is depended on
         * @return A boolean indicating whether the dependency was successfully added (true) or not (false)
         */
        bool addDependency(uint64_t resourceId, uint64_t dependsOnId);

        /**
         * @brief Get dependencies for a resource
         * @param resourceId A resource ID to query dependencies for
         * @return A vector of resource IDs that the specified resource depends on, or an empty vector if there are no dependencies or if the resource ID is invalid
         */
        std::vector<uint64_t> getDependencies(uint64_t resourceId) const;

        // Prevent copying and assignment
        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;
        ResourceManager(ResourceManager&&) = delete;
        ResourceManager& operator=(ResourceManager&&) = delete;

    private:
        ResourceManager() = default;
        ~ResourceManager() = default;

        mutable std::mutex resourceMutex_;
        std::unordered_map<uint64_t, std::unique_ptr<ResourceInfo>> resources_;
        std::unordered_map<void*, uint64_t> memoryAddressToId_;
        std::unordered_map<uint64_t, std::vector<uint64_t>> dependencies_;
        std::atomic<uint64_t> nextResourceId_{1};
        size_t totalAllocatedMemory_{0};
    };

    /**
     * @brief RAII wrapper for resource management
     * 
     * Automatically manages reference counting through RAII pattern.
     */
    class ResourceGuard
    {
    public:
        /**
         * @brief Construct and add reference to resource
         * @param resourceId The resource ID to manage
         */
        explicit ResourceGuard(uint64_t resourceId);

        /*
         * @brief Destructor
         */
        ~ResourceGuard();

        /// @brief Prevent copying, assignment, and moving
        ResourceGuard(const ResourceGuard&) = delete;
        ResourceGuard& operator=(const ResourceGuard&) = delete;
        ResourceGuard(ResourceGuard&& other) noexcept;
        ResourceGuard& operator=(ResourceGuard&& other) noexcept;

        /**
         * @brief Get the managed resource ID
         * @return A resource ID
         */
        [[nodiscard]] uint64_t getResourceId() const { return resourceId_; }

    private:
        uint64_t resourceId_;
        bool valid_;
    };

} // namespace mex_hal

#endif // MEX_HAL_RESOURCE_MANAGER_H
