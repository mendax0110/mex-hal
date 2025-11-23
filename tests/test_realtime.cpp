#include <gtest/gtest.h>
#include <hal/core.h>
#include <sched.h>
#include <sys/mman.h>
#include <thread>
#include <chrono>

using namespace mex_hal;

class RealtimeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        hal = createHAL(HALType::LINUX);
        hal->init();
    }

    void TearDown() override
    {
        hal->shutdown();
    }

    std::unique_ptr<HAL> hal;
    
    bool isRootOrRTCapable()
    {
        // Check if we have capabilities to set real-time priority
        sched_param param{};
        param.sched_priority = 1;
        int result = sched_setscheduler(0, SCHED_FIFO, &param);
        if (result == 0)
        {
            // Reset to normal scheduling
            param.sched_priority = 0;
            sched_setscheduler(0, SCHED_OTHER, &param);
            return true;
        }
        return false;
    }
};

TEST_F(RealtimeTest, ConfigureRealtimeWithoutPermissions)
{
    EXPECT_NO_THROW(hal->configureRealtime(50));
}

TEST_F(RealtimeTest, ConfigureRealtimeWithPermissions)
{
    if (!isRootOrRTCapable())
    {
        GTEST_SKIP() << "Skipping: Requires root or RT capabilities";
    }
    
    EXPECT_TRUE(hal->configureRealtime(50));
    
    // Verify scheduling policy
    int policy = sched_getscheduler(0);
    EXPECT_EQ(policy, SCHED_FIFO);
    
    // Reset to normal scheduling
    sched_param param{};
    param.sched_priority = 0;
    sched_setscheduler(0, SCHED_OTHER, &param);
}

TEST_F(RealtimeTest, PriorityRange)
{
    if (!isRootOrRTCapable())
    {
        GTEST_SKIP() << "Skipping: Requires root or RT capabilities";
    }
    
    // Test valid priority range (1-99 for SCHED_FIFO)
    EXPECT_NO_THROW(hal->configureRealtime(1));
    EXPECT_NO_THROW(hal->configureRealtime(50));
    EXPECT_NO_THROW(hal->configureRealtime(99));
    
    // Reset
    sched_param param{};
    param.sched_priority = 0;
    sched_setscheduler(0, SCHED_OTHER, &param);
}

TEST_F(RealtimeTest, CPUAffinitySetup)
{
    // Test that we can set CPU affinity (part of RT task management)
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    
    int result = sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
    
    if (result == 0)
    {
        cpu_set_t verify;
        CPU_ZERO(&verify);
        sched_getaffinity(0, sizeof(cpu_set_t), &verify);
        EXPECT_TRUE(CPU_ISSET(0, &verify));
    }
}

TEST_F(RealtimeTest, SchedulingPolicyQueries)
{
    // Test that we can query scheduling parameters
    sched_param param{};
    int policy = sched_getscheduler(0);
    
    EXPECT_GE(policy, 0);
    
    int result = sched_getparam(0, &param);
    EXPECT_EQ(result, 0);
}

TEST_F(RealtimeTest, MinMaxPriority)
{
    // Verify priority bounds for different policies
    int fifo_min = sched_get_priority_min(SCHED_FIFO);
    int fifo_max = sched_get_priority_max(SCHED_FIFO);
    
    EXPECT_GT(fifo_max, fifo_min);
    EXPECT_GE(fifo_min, 1);
    EXPECT_LE(fifo_max, 99);
    
    int rr_min = sched_get_priority_min(SCHED_RR);
    int rr_max = sched_get_priority_max(SCHED_RR);
    
    EXPECT_GT(rr_max, rr_min);
}

TEST_F(RealtimeTest, ThreadSchedulingPolicy)
{
    // Test setting scheduling policy on a thread
    std::thread testThread([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    });
    
    pthread_t nativeHandle = testThread.native_handle();
    
    sched_param param{};
    int policy = 0;
    int result = pthread_getschedparam(nativeHandle, &policy, &param);
    
    EXPECT_EQ(result, 0);
    
    testThread.join();
}

TEST_F(RealtimeTest, MemoryLockingCapability)
{
    // Test if memory locking is available (part of RT configuration)
    int result = mlockall(MCL_CURRENT | MCL_FUTURE);
    
    if (result == 0)
    {
        // Successfully locked memory
        EXPECT_EQ(result, 0);
        munlockall();  // Cleanup
    }
    else
    {
        // Expected to fail in CI without permissions
        EXPECT_EQ(result, -1);
    }
}

TEST_F(RealtimeTest, PageFaultConfiguration)
{
    // Test that we can configure to avoid page faults
    // This is critical for real-time performance
    
    // Allocate a buffer
    size_t bufferSize = 4096;
    void* buffer = malloc(bufferSize);
    ASSERT_NE(buffer, nullptr);
    
    // Try to lock it in memory
    int result = mlock(buffer, bufferSize);
    
    if (result == 0)
    {
        // Successfully locked
        EXPECT_EQ(result, 0);
        munlock(buffer, bufferSize);
    }
    
    free(buffer);
}

TEST_F(RealtimeTest, TimerResolution)
{
    // Verify high-resolution timer availability
    struct timespec res;
    int result = clock_getres(CLOCK_MONOTONIC, &res);
    
    EXPECT_EQ(result, 0);
    EXPECT_LE(res.tv_nsec, 1000000);  // Should be better than 1ms
}