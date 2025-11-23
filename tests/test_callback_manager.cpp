#include <gtest/gtest.h>
#include <hal/callback_manager.h>
#include <thread>
#include <atomic>
#include <chrono>

using namespace mex_hal;

class CallbackManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        CallbackManager::getInstance().clearAll();
    }

    void TearDown() override
    {
        CallbackManager::getInstance().clearAll();
    }
};

TEST_F(CallbackManagerTest, Singleton)
{
    auto& instance1 = CallbackManager::getInstance();
    auto& instance2 = CallbackManager::getInstance();
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(CallbackManagerTest, RegisterGPIOCallback)
{
    auto& cm = CallbackManager::getInstance();
    bool called = false;
    
    uint64_t id = cm.registerGPIOCallback(17, [&called](uint8_t pin, PinValue value) {
        called = true;
    });
    
    EXPECT_GT(id, 0);
}

TEST_F(CallbackManagerTest, UnregisterGPIOCallback)
{
    auto& cm = CallbackManager::getInstance();
    
    uint64_t id = cm.registerGPIOCallback(17, [](uint8_t, PinValue) {});
    EXPECT_TRUE(cm.unregisterGPIOCallback(id));
}

TEST_F(CallbackManagerTest, UnregisterInvalidGPIOCallback)
{
    auto& cm = CallbackManager::getInstance();
    EXPECT_FALSE(cm.unregisterGPIOCallback(9999));
}

TEST_F(CallbackManagerTest, InvokeGPIOCallback)
{
    auto& cm = CallbackManager::getInstance();
    std::atomic<bool> called{false};
    uint8_t receivedPin = 0;
    PinValue receivedValue = PinValue::LOW;
    
    cm.registerGPIOCallback(17, [&](uint8_t pin, PinValue value) {
        called = true;
        receivedPin = pin;
        receivedValue = value;
    });
    
    cm.invokeGPIOCallback(17, PinValue::HIGH);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    EXPECT_TRUE(called);
    EXPECT_EQ(receivedPin, 17);
    EXPECT_EQ(receivedValue, PinValue::HIGH);
}

TEST_F(CallbackManagerTest, MultipleGPIOCallbacksSamePin)
{
    auto& cm = CallbackManager::getInstance();
    std::atomic<int> callCount{0};
    
    cm.registerGPIOCallback(17, [&](uint8_t, PinValue) {
        callCount++;
    });
    
    cm.registerGPIOCallback(17, [&](uint8_t, PinValue) {
        callCount++;
    });
    
    cm.invokeGPIOCallback(17, PinValue::HIGH);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    EXPECT_EQ(callCount, 2);
}

TEST_F(CallbackManagerTest, GPIOCallbackDifferentPins)
{
    auto& cm = CallbackManager::getInstance();
    std::atomic<int> pin17Count{0};
    std::atomic<int> pin27Count{0};
    
    cm.registerGPIOCallback(17, [&](uint8_t, PinValue) {
        pin17Count++;
    });
    
    cm.registerGPIOCallback(27, [&](uint8_t, PinValue) {
        pin27Count++;
    });
    
    cm.invokeGPIOCallback(17, PinValue::HIGH);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    EXPECT_EQ(pin17Count, 1);
    EXPECT_EQ(pin27Count, 0);
    
    cm.invokeGPIOCallback(27, PinValue::LOW);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    EXPECT_EQ(pin17Count, 1);
    EXPECT_EQ(pin27Count, 1);
}

TEST_F(CallbackManagerTest, RegisterTimerCallback)
{
    auto& cm = CallbackManager::getInstance();
    bool called = false;
    
    uint64_t id = cm.registerTimerCallback(1, [&called]() {
        called = true;
    });
    
    EXPECT_GT(id, 0);
}

TEST_F(CallbackManagerTest, UnregisterTimerCallback)
{
    auto& cm = CallbackManager::getInstance();
    
    uint64_t id = cm.registerTimerCallback(1, []() {});
    EXPECT_TRUE(cm.unregisterTimerCallback(id));
}

TEST_F(CallbackManagerTest, UnregisterInvalidTimerCallback)
{
    auto& cm = CallbackManager::getInstance();
    EXPECT_FALSE(cm.unregisterTimerCallback(9999));
}

TEST_F(CallbackManagerTest, InvokeTimerCallback)
{
    auto& cm = CallbackManager::getInstance();
    std::atomic<bool> called{false};
    uint32_t receivedTimerId = 0;
    
    cm.registerTimerCallback(1, [&]() {
        called = true;
    });
    
    cm.invokeTimerCallback(1);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    EXPECT_TRUE(called);
}

TEST_F(CallbackManagerTest, MultipleTimerCallbacksSameId)
{
    auto& cm = CallbackManager::getInstance();
    std::atomic<int> callCount{0};
    
    cm.registerTimerCallback(1, [&]() {
        callCount++;
    });
    
    cm.registerTimerCallback(1, [&]() {
        callCount++;
    });
    
    cm.invokeTimerCallback(1);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    EXPECT_EQ(callCount, 2);
}

TEST_F(CallbackManagerTest, TimerCallbackDifferentIds)
{
    auto& cm = CallbackManager::getInstance();
    std::atomic<int> timer1Count{0};
    std::atomic<int> timer2Count{0};
    
    cm.registerTimerCallback(1, [&]() {
        timer1Count++;
    });
    
    cm.registerTimerCallback(2, [&]() {
        timer2Count++;
    });
    
    cm.invokeTimerCallback(1);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    EXPECT_EQ(timer1Count, 1);
    EXPECT_EQ(timer2Count, 0);
    
    cm.invokeTimerCallback(2);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    EXPECT_EQ(timer1Count, 1);
    EXPECT_EQ(timer2Count, 1);
}

TEST_F(CallbackManagerTest, ClearAll)
{
    auto& cm = CallbackManager::getInstance();
    std::atomic<int> gpioCallCount{0};
    std::atomic<int> timerCallCount{0};
    
    cm.registerGPIOCallback(17, [&](uint8_t, PinValue) {
        gpioCallCount++;
    });
    
    cm.registerTimerCallback(1, [&]() {
        timerCallCount++;
    });
    
    cm.clearAll();
    
    cm.invokeGPIOCallback(17, PinValue::HIGH);
    cm.invokeTimerCallback(1);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    EXPECT_EQ(gpioCallCount, 0);
    EXPECT_EQ(timerCallCount, 0);
}

TEST_F(CallbackManagerTest, ThreadSafetyGPIO)
{
    auto& cm = CallbackManager::getInstance();
    std::atomic<int> callCount{0};
    const int numThreads = 10;
    
    // Register callbacks from multiple threads
    std::vector<std::thread> registerThreads;
    for (int i = 0; i < numThreads; ++i)
    {
        registerThreads.emplace_back([&cm, &callCount, i]() {
            cm.registerGPIOCallback(i, [&callCount](uint8_t, PinValue) {
                callCount++;
            });
        });
    }
    
    for (auto& t : registerThreads)
    {
        t.join();
    }
    
    // Invoke callbacks from multiple threads
    std::vector<std::thread> invokeThreads;
    for (int i = 0; i < numThreads; ++i)
    {
        invokeThreads.emplace_back([&cm, i]() {
            cm.invokeGPIOCallback(i, PinValue::HIGH);
        });
    }
    
    for (auto& t : invokeThreads)
    {
        t.join();
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    EXPECT_EQ(callCount, numThreads);
}

TEST_F(CallbackManagerTest, ThreadSafetyTimer)
{
    auto& cm = CallbackManager::getInstance();
    std::atomic<int> callCount{0};
    const int numThreads = 10;
    
    // Register callbacks from multiple threads
    std::vector<std::thread> registerThreads;
    for (int i = 0; i < numThreads; ++i)
    {
        registerThreads.emplace_back([&cm, &callCount, i]() {
            cm.registerTimerCallback(i, [&callCount]() {
                callCount++;
            });
        });
    }
    
    for (auto& t : registerThreads)
    {
        t.join();
    }
    
    // Invoke callbacks from multiple threads
    std::vector<std::thread> invokeThreads;
    for (int i = 0; i < numThreads; ++i)
    {
        invokeThreads.emplace_back([&cm, i]() {
            cm.invokeTimerCallback(i);
        });
    }
    
    for (auto& t : invokeThreads)
    {
        t.join();
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    EXPECT_EQ(callCount, numThreads);
}
