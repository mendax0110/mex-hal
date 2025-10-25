#ifndef MEX_HAL_RESOURCE_VISUALIZER_H
#define MEX_HAL_RESOURCE_VISUALIZER_H

#include <string>
#include <cstdint>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>

namespace mex_hal
{
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

    struct ResourceNode
    {
        uint64_t id;
        std::string name;
        std::vector<uint64_t> dependencies;
    };

    class ResourceVisualizer
    {
    public:
        ResourceVisualizer() = default;
        ~ResourceVisualizer();

        void startLiveUpdate(int intervalMs = 500);
        void stopLiveUpdate();

        void gatherResourceData();
        void buildResourceGraph();

        // Print snapshots to console
        void printResourceUsage() const;
        void printResourceGraph() const;

    private:
        mutable std::mutex mutex_;
        std::atomic<bool> running_{false};
        std::thread updateThread_;

        std::vector<ResourceUsage> resourceUsages_;
        std::vector<ResourceNode> resourceGraph_;

        static void gatherProcessMetrics(ResourceUsage& usage);
    };
}

#endif // MEX_HAL_RESOURCE_VISUALIZER_H
