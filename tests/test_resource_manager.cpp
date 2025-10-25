#include <gtest/gtest.h>
#include <hal/resource_manager.h>
#include <thread>
#include <vector>

using namespace mex_hal;

class ResourceManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ResourceManager::getInstance().clearAll();
    }

    void TearDown() override
    {
        ResourceManager::getInstance().clearAll();
    }
};

TEST_F(ResourceManagerTest, Singleton)
{
    auto& instance1 = ResourceManager::getInstance();
    auto& instance2 = ResourceManager::getInstance();
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(ResourceManagerTest, RegisterResource)
{
    auto& rm = ResourceManager::getInstance();
    int dummyHandle = 42;
    
    uint64_t id = rm.registerResource(
        ResourceType::FILE_DESCRIPTOR, 
        "test_fd", 
        &dummyHandle
    );
    
    EXPECT_GT(id, 0);
    EXPECT_EQ(rm.getResourceCount(), 1);
}

TEST_F(ResourceManagerTest, UnregisterResource)
{
    auto& rm = ResourceManager::getInstance();
    int dummyHandle = 42;
    
    uint64_t id = rm.registerResource(
        ResourceType::FILE_DESCRIPTOR, 
        "test_fd", 
        &dummyHandle
    );
    
    // Release the initial reference count
    rm.release(id);
    
    EXPECT_TRUE(rm.unregisterResource(id));
    EXPECT_EQ(rm.getResourceCount(), 0);
}

TEST_F(ResourceManagerTest, UnregisterInvalidResource)
{
    auto& rm = ResourceManager::getInstance();
    EXPECT_FALSE(rm.unregisterResource(9999));
}

TEST_F(ResourceManagerTest, ReferenceCount)
{
    auto& rm = ResourceManager::getInstance();
    int dummyHandle = 42;
    
    uint64_t id = rm.registerResource(
        ResourceType::GPIO_PIN, 
        "gpio_17", 
        &dummyHandle
    );
    
    EXPECT_EQ(rm.getRefCount(id), 1);  // Starts at 1
    
    rm.addRef(id);
    EXPECT_EQ(rm.getRefCount(id), 2);
    
    rm.addRef(id);
    EXPECT_EQ(rm.getRefCount(id), 3);
    
    rm.release(id);
    EXPECT_EQ(rm.getRefCount(id), 2);
    
    rm.release(id);
    EXPECT_EQ(rm.getRefCount(id), 1);
}

TEST_F(ResourceManagerTest, InUseFlag)
{
    auto& rm = ResourceManager::getInstance();
    int dummyHandle = 42;
    
    uint64_t id = rm.registerResource(
        ResourceType::SPI_BUS, 
        "spi_0", 
        &dummyHandle
    );
    
    EXPECT_FALSE(rm.isInUse(id));
    
    rm.setInUse(id, true);
    EXPECT_TRUE(rm.isInUse(id));
    
    rm.setInUse(id, false);
    EXPECT_FALSE(rm.isInUse(id));
}

TEST_F(ResourceManagerTest, GetResourceInfo)
{
    auto& rm = ResourceManager::getInstance();
    int dummyHandle = 42;
    
    uint64_t id = rm.registerResource(
        ResourceType::I2C_BUS, 
        "i2c_1", 
        &dummyHandle
    );
    
    const ResourceInfo* info = rm.getResourceInfo(id);
    ASSERT_NE(info, nullptr);
    EXPECT_EQ(info->type, ResourceType::I2C_BUS);
    EXPECT_EQ(info->name, "i2c_1");
    EXPECT_EQ(info->handle, &dummyHandle);
}

TEST_F(ResourceManagerTest, GetResourceInfoInvalid)
{
    auto& rm = ResourceManager::getInstance();
    const ResourceInfo* info = rm.getResourceInfo(9999);
    EXPECT_EQ(info, nullptr);
}

TEST_F(ResourceManagerTest, MultipleResources)
{
    auto& rm = ResourceManager::getInstance();
    int handle1 = 1, handle2 = 2, handle3 = 3;
    
    uint64_t id1 = rm.registerResource(ResourceType::GPIO_PIN, "gpio_1", &handle1);
    uint64_t id2 = rm.registerResource(ResourceType::SPI_BUS, "spi_0", &handle2);
    uint64_t id3 = rm.registerResource(ResourceType::I2C_BUS, "i2c_1", &handle3);
    
    EXPECT_EQ(rm.getResourceCount(), 3);
    EXPECT_NE(id1, id2);
    EXPECT_NE(id2, id3);
    EXPECT_NE(id1, id3);
}

TEST_F(ResourceManagerTest, ClearAll)
{
    auto& rm = ResourceManager::getInstance();
    int handle1 = 1, handle2 = 2, handle3 = 3;
    
    rm.registerResource(ResourceType::GPIO_PIN, "gpio_1", &handle1);
    rm.registerResource(ResourceType::SPI_BUS, "spi_0", &handle2);
    rm.registerResource(ResourceType::I2C_BUS, "i2c_1", &handle3);
    
    EXPECT_EQ(rm.getResourceCount(), 3);
    
    rm.clearAll();
    EXPECT_EQ(rm.getResourceCount(), 0);
}

TEST_F(ResourceManagerTest, ThreadSafety)
{
    auto& rm = ResourceManager::getInstance();
    const int numThreads = 10;
    const int resourcesPerThread = 100;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < numThreads; ++i)
    {
        threads.emplace_back([&rm, i, resourcesPerThread]() {
            for (int j = 0; j < resourcesPerThread; ++j)
            {
                int* handle = new int(i * resourcesPerThread + j);
                uint64_t id = rm.registerResource(
                    ResourceType::GPIO_PIN,
                    "gpio_" + std::to_string(i) + "_" + std::to_string(j),
                    handle
                );
                rm.addRef(id);
                rm.setInUse(id, true);
                rm.release(id);
            }
        });
    }
    
    for (auto& thread : threads)
    {
        thread.join();
    }
    
    EXPECT_EQ(rm.getResourceCount(), numThreads * resourcesPerThread);
    
    // Cleanup
    rm.clearAll();
}

TEST_F(ResourceManagerTest, ResourceGuardRAII)
{
    auto& rm = ResourceManager::getInstance();
    int dummyHandle = 42;
    
    uint64_t id = rm.registerResource(
        ResourceType::TIMER, 
        "timer_0", 
        &dummyHandle
    );
    
    EXPECT_EQ(rm.getRefCount(id), 1);  // Starts at 1
    
    {
        ResourceGuard guard(id);
        EXPECT_EQ(rm.getRefCount(id), 2);  // Guard adds 1
        EXPECT_EQ(guard.getResourceId(), id);
    }
    
    EXPECT_EQ(rm.getRefCount(id), 1);  // Back to 1 after guard destroyed
}

TEST_F(ResourceManagerTest, ResourceGuardMove)
{
    auto& rm = ResourceManager::getInstance();
    int dummyHandle = 42;
    
    uint64_t id = rm.registerResource(
        ResourceType::ADC_CHANNEL, 
        "adc_0", 
        &dummyHandle
    );
    
    ResourceGuard guard1(id);
    EXPECT_EQ(rm.getRefCount(id), 2);  // Initial 1 + guard 1
    
    ResourceGuard guard2(std::move(guard1));
    EXPECT_EQ(rm.getRefCount(id), 2);  // Still 2 after move
}
