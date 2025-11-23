#include <gtest/gtest.h>
#include <hal/core.h>
#include <hal/timer.h>
#include <atomic>
#include <chrono>
#include <thread>

using namespace mex_hal;

class TimerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        hal = createHAL(HALType::LINUX);
        hal->init();
        timer = hal->createTimer();
    }

    void TearDown() override
    {
        if (timer && timer->isRunning())
        {
            timer->stop();
        }
        timer.reset();
        hal->shutdown();
    }

    std::unique_ptr<HAL> hal;
    std::unique_ptr<TimerInterface> timer;
};

TEST_F(TimerTest, CreateTimer)
{
    ASSERT_NE(timer, nullptr);
}

TEST_F(TimerTest, InitTimer)
{
    EXPECT_NO_THROW(timer->init(TimerMode::PERIODIC));
}

TEST_F(TimerTest, InitOneShot)
{
    EXPECT_NO_THROW(timer->init(TimerMode::ONE_SHOT));
}

TEST_F(TimerTest, StartTimer)
{
    timer->init(TimerMode::PERIODIC);
    std::atomic<int> count{0};
    
    EXPECT_NO_THROW(timer->start(100000, [&count]() {
        count++;
    }));
    
    timer->stop();
}

TEST_F(TimerTest, StopTimer)
{
    timer->init(TimerMode::PERIODIC);
    timer->start(100000, []() {});
    
    EXPECT_NO_THROW(timer->stop());
}

TEST_F(TimerTest, PeriodicCallback)
{
    timer->init(TimerMode::PERIODIC);
    std::atomic<int> count{0};
    
    timer->start(50000, [&count]() {  // 50ms
        count++;
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    timer->stop();
    
    // Should have called ~4-5 times in 250ms
    EXPECT_GE(count, 3);
    EXPECT_LE(count, 6);
}

TEST_F(TimerTest, OneShotCallback)
{
    timer->init(TimerMode::ONE_SHOT);
    std::atomic<int> count{0};
    std::atomic<bool> finished{false};
    
    timer->start(50000, [&count, &finished]() {
        count++;
        finished = true;
    });
    
    // Wait for the callback to execute
    int waitCount = 0;
    while (!finished && waitCount < 100)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        waitCount++;
    }
    
    // Should have called only once
    EXPECT_EQ(count, 1);
    EXPECT_TRUE(finished);
    
    // Give more time to make sure it doesn't call again
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(count, 1);
    
    // For ONE_SHOT, the timer stops automatically, 
    // so we should wait a bit before destruction
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

TEST_F(TimerTest, ResetTimer)
{
    timer->init(TimerMode::PERIODIC);
    timer->start(100000, []() {});
    
    EXPECT_NO_THROW(timer->reset());
}

TEST_F(TimerTest, GetElapsedTime)
{
    timer->init(TimerMode::PERIODIC);
    timer->start(100000, []() {});
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    uint64_t elapsed = timer->getElapsedUs();
    EXPECT_GT(elapsed, 0);
    
    timer->stop();
}

TEST_F(TimerTest, IsRunning)
{
    timer->init(TimerMode::PERIODIC);
    
    EXPECT_FALSE(timer->isRunning());
    
    timer->start(100000, []() {});
    EXPECT_TRUE(timer->isRunning());
    
    timer->stop();
    EXPECT_FALSE(timer->isRunning());
}

TEST_F(TimerTest, MultipleStarts)
{
    timer->init(TimerMode::PERIODIC);
    std::atomic<int> count{0};
    
    timer->start(50000, [&count]() {
        count++;
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    int firstCount = count;
    
    timer->stop();
    timer->start(50000, [&count]() {
        count++;
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_GT(count, firstCount);
    
    timer->stop();
}

TEST_F(TimerTest, DifferentIntervals)
{
    std::atomic<int> count1{0};
    std::atomic<int> count2{0};
    
    auto timer1 = hal->createTimer();
    auto timer2 = hal->createTimer();
    
    timer1->init(TimerMode::PERIODIC);
    timer2->init(TimerMode::PERIODIC);
    
    timer1->start(50000, [&count1]() { count1++; });   // 50ms
    timer2->start(100000, [&count2]() { count2++; });  // 100ms
    
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    
    timer1->stop();
    timer2->stop();
    
    // Timer1 should have fired approximately twice as often as Timer2
    EXPECT_GT(count1, count2);
}
