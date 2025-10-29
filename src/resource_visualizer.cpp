#include "../include/hal/resource_visualizer.h"
#include "../include/hal/resource_manager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <unistd.h>

namespace fs = std::filesystem;
using namespace mex_hal;

ResourceVisualizer::~ResourceVisualizer()
{
    stopLiveUpdate();
}

void ResourceVisualizer::startLiveUpdate(int intervalMs)
{
    if (running_) return;
    running_ = true;

    updateThread_ = std::thread([this, intervalMs]() {
        while (running_)
        {
            gatherResourceData();
            buildResourceGraph();
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
        }
    });
}

void ResourceVisualizer::stopLiveUpdate()
{
    running_ = false;
    if (updateThread_.joinable())
        updateThread_.join();
}

void ResourceVisualizer::gatherResourceData()
{
    std::lock_guard<std::mutex> lock(mutex_);
    const auto& rm = ResourceManager::getInstance();
    const size_t count = rm.getResourceCount();

    resourceUsages_.clear();
    for (uint64_t id = 1; id <= count; ++id)
    {
        if (const auto info = rm.getResourceInfo(id))
        {
            ResourceUsage usage;
            usage.id = id;
            usage.name = info->name;
            usage.refCount = info->refCount.load();
            usage.inUse = info->inUse.load();

            gatherProcessMetrics(usage);

            resourceUsages_.push_back(usage);
        }
    }
}

void ResourceVisualizer::gatherProcessMetrics(ResourceUsage& usage)
{
    static long prevTotal = 0, prevIdle = 0;

    std::ifstream stat("/proc/stat");
    std::string line;
    std::getline(stat, line);
    std::istringstream ss(line);

    std::string cpuLabel;
    long user, nice, system, idle, iowait, irq, softirq, steal;
    ss >> cpuLabel >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
    long total = user + nice + system + idle + iowait + irq + softirq + steal;

    long deltaTotal = total - prevTotal;
    long deltaIdle  = idle - prevIdle;
    prevTotal = total;
    prevIdle  = idle;

    usage.cpuPercent = deltaTotal ? 100.0 * (deltaTotal - deltaIdle) / deltaTotal : 0.0;

    std::ifstream mem("/proc/self/statm");
    size_t totalPages, resident;
    mem >> totalPages >> resident;
    usage.memoryBytes = resident * 4096;

    // Open file descriptors
    size_t fdCount = 0;
    for (auto& entry : fs::directory_iterator("/proc/self/fd"))
    {
        if (entry.exists())
        {
            ++fdCount;
        }
    }
    usage.openFDs = fdCount;
}

void ResourceVisualizer::buildResourceGraph()
{
    std::lock_guard<std::mutex> lock(mutex_);
    resourceGraph_.clear();

    for (const auto& usage : resourceUsages_)
    {
        ResourceNode node;
        node.id = usage.id;
        node.name = usage.name;
        // TODO: populate dependencies if ResourceManager tracks them
        resourceGraph_.push_back(node);
    }
}

void ResourceVisualizer::printResourceUsage() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::cout << "\n=== HAL Resource Usage ===\n";
    std::cout << "ID\tName\tRefCount\tInUse\tCPU%\tMemory KB\tFDs\tCPU Bar\n";

    for (const auto& r : resourceUsages_)
    {
        const int barLen = static_cast<int>(r.cpuPercent / 5);
        std::string cpuBar(barLen, '#');

        std::cout << r.id << "\t"
                  << r.name << "\t"
                  << r.refCount << "\t\t"
                  << (r.inUse ? "Yes" : "No") << "\t"
                  << r.cpuPercent << "\t"
                  << (r.memoryBytes / 1024) << "\t"
                  << r.openFDs << "\t"
                  << cpuBar << "\n";
    }
}

void ResourceVisualizer::printResourceGraph() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::cout << "\n=== Resource Graph ===\n";
    for (const auto&[id
                    , name
                    , dependencies] : resourceGraph_)
    {
        std::cout << name << " [ID: " << id << "] -> ";
        for (const auto dep : dependencies)
            std::cout << dep << " ";
        std::cout << "\n";
    }
}
