#ifndef MEX_HAL_RESOURCE_VISUALIZER_H
#define MEX_HAL_RESOURCE_VISUALIZER_H

#include <string>
#include <cstdint>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief Resource usage metrics structure \struct ResourceUsage
    struct ResourceUsage
    {
        uint64_t id;
        std::string name;
        uint32_t refCount;
        bool inUse;

        // Live metrics
        double cpuPercent;
        size_t memoryBytes;
        size_t openFDs;
    };

    /// @brief Resource graph node structure \struct ResourceNode
    struct ResourceNode
    {
        uint64_t id;
        std::string name;
        std::vector<uint64_t> dependencies;
    };

    /// @brief Resource Visualizer class \class ResourceVisualizer
    class ResourceVisualizer
    {
    public:
        /**
         * @brief Constructor
         */
        ResourceVisualizer() = default;

        /**
         * @brief Destructor
         */
        ~ResourceVisualizer();

        /**
         * @brief Start live updating resource data
         * @param intervalMs Update interval in milliseconds
         */
        void startLiveUpdate(int intervalMs = 500);

        /**
         * @brief Stop live updating resource data
         */
        void stopLiveUpdate();

        /**
         * @brief Gather resource usage data
         */
        void gatherResourceData();

        /**
         * @brief Build resource dependency graph
         */
        void buildResourceGraph();

        /**
         * @brief Print resource usage metrics
         */
        void printResourceUsage() const;

        /**
         * @brief Print resource dependency graph
         */
        void printResourceGraph() const;

    private:
        mutable std::mutex mutex_;
        std::atomic<bool> running_{false};
        std::thread updateThread_;

        std::vector<ResourceUsage> resourceUsages_;
        std::vector<ResourceNode> resourceGraph_;

        /**
         * @brief Gather process metrics for a resource
         * @param usage Reference to ResourceUsage struct to populate
         */
        static void gatherProcessMetrics(ResourceUsage& usage);
    };
}

#endif // MEX_HAL_RESOURCE_VISUALIZER_H
