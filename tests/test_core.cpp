#include <gtest/gtest.h>
#include <hal/core.h>
#include <hal/gpio.h>
#include <hal/spi.h>
#include <hal/i2c.h>
#include <hal/uart.h>
#include <hal/pwm.h>
#include <hal/timer.h>
#include <hal/adc.h>
#include <mock/hal_mock.h>

using namespace mex_hal;

class CoreTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        hal = std::make_unique<HALMock>();
    }

    void TearDown() override
    {
        if (hal)
        {
            hal->shutdown();
        }
    }

    std::unique_ptr<HAL> hal;
};

TEST_F(CoreTest, CreateHAL)
{
    ASSERT_NE(hal, nullptr);
}

TEST_F(CoreTest, InitializeHAL)
{
    EXPECT_TRUE(hal->init());
}

TEST_F(CoreTest, ShutdownHAL)
{
    hal->init();
    EXPECT_NO_THROW(hal->shutdown());
}

TEST_F(CoreTest, CreateGPIO)
{
    hal->init();
    const auto gpio = hal->createGPIO();
    ASSERT_NE(gpio, nullptr);
}

TEST_F(CoreTest, CreateSPI)
{
    hal->init();
    const auto spi = hal->createSPI();
    ASSERT_NE(spi, nullptr);
}

TEST_F(CoreTest, CreateI2C)
{
    hal->init();
    const auto i2c = hal->createI2C();
    ASSERT_NE(i2c, nullptr);
}

TEST_F(CoreTest, CreateUART)
{
    hal->init();
    const auto uart = hal->createUART();
    ASSERT_NE(uart, nullptr);
}

TEST_F(CoreTest, CreatePWM)
{
    hal->init();
    const auto pwm = hal->createPWM();
    ASSERT_NE(pwm, nullptr);
}

TEST_F(CoreTest, CreateTimer)
{
    hal->init();
    const auto timer = hal->createTimer();
    ASSERT_NE(timer, nullptr);
}

TEST_F(CoreTest, CreateADC)
{
    hal->init();
    const auto adc = hal->createADC();
    ASSERT_NE(adc, nullptr);
}

TEST_F(CoreTest, MultiplePeripheralCreation)
{
    hal->init();

    const auto gpio = hal->createGPIO();
    const auto spi = hal->createSPI();
    const auto i2c = hal->createI2C();
    const auto uart = hal->createUART();
    const auto pwm = hal->createPWM();
    const auto timer = hal->createTimer();
    const auto adc = hal->createADC();

    ASSERT_NE(gpio, nullptr);
    ASSERT_NE(spi, nullptr);
    ASSERT_NE(i2c, nullptr);
    ASSERT_NE(uart, nullptr);
    ASSERT_NE(pwm, nullptr);
    ASSERT_NE(timer, nullptr);
    ASSERT_NE(adc, nullptr);
}

TEST_F(CoreTest, InitBeforePeripheralCreation)
{
    EXPECT_TRUE(hal->init());
    const auto gpio = hal->createGPIO();
    ASSERT_NE(gpio, nullptr);
}

TEST_F(CoreTest, ConfigureRealtime)
{
    hal->init();
    EXPECT_TRUE(hal->configureRealtime(50));
    EXPECT_TRUE(hal->isRealtimeConfigured());
}

TEST_F(CoreTest, RealtimeStateTransitions)
{
    hal->init();
    EXPECT_EQ(hal->getRealtimeState(), RealTimeState::NOT_RUNNING);

    hal->configureRealtime(50);
    EXPECT_EQ(hal->getRealtimeState(), RealTimeState::RUNNING);
}

TEST_F(CoreTest, SetAndGetRealTimePolicy)
{
    hal->init();

    const auto result = hal->setRealTimePolicy(RealTimePolicy::FIFO);
    EXPECT_EQ(result, RealTimePolicy::FIFO);
    EXPECT_EQ(hal->getRealTimePolicy(), RealTimePolicy::FIFO);

    EXPECT_EQ(hal->setRealTimePolicy(RealTimePolicy::RR), RealTimePolicy::RR);
    EXPECT_EQ(hal->getRealTimePolicy(), RealTimePolicy::RR);

    EXPECT_EQ(hal->setRealTimePolicy(RealTimePolicy::NONE), RealTimePolicy::NONE);
    EXPECT_EQ(hal->getRealTimePolicy(), RealTimePolicy::NONE);
}
