#include <gtest/gtest.h>
#include <hal/core.h>
#include <hal/gpio.h>
#include <hal/spi.h>
#include <hal/i2c.h>
#include <hal/uart.h>
#include <hal/pwm.h>
#include <hal/timer.h>
#include <hal/adc.h>

using namespace mex_hal;

class CoreTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        hal = createHAL(HALType::LINUX);
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

TEST_F(CoreTest, CreateHALWithType)
{
    auto linuxHal = createHAL(HALType::LINUX);
    ASSERT_NE(linuxHal, nullptr);
}

TEST_F(CoreTest, CreateHALInvalidType)
{
    EXPECT_THROW(createHAL(HALType::INVALID), std::invalid_argument);
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
    auto gpio = hal->createGPIO();
    ASSERT_NE(gpio, nullptr);
}

TEST_F(CoreTest, CreateSPI)
{
    hal->init();
    auto spi = hal->createSPI();
    ASSERT_NE(spi, nullptr);
}

TEST_F(CoreTest, CreateI2C)
{
    hal->init();
    auto i2c = hal->createI2C();
    ASSERT_NE(i2c, nullptr);
}

TEST_F(CoreTest, CreateUART)
{
    hal->init();
    auto uart = hal->createUART();
    ASSERT_NE(uart, nullptr);
}

TEST_F(CoreTest, CreatePWM)
{
    hal->init();
    auto pwm = hal->createPWM();
    ASSERT_NE(pwm, nullptr);
}

TEST_F(CoreTest, CreateTimer)
{
    hal->init();
    auto timer = hal->createTimer();
    ASSERT_NE(timer, nullptr);
}

TEST_F(CoreTest, CreateADC)
{
    hal->init();
    auto adc = hal->createADC();
    ASSERT_NE(adc, nullptr);
}

TEST_F(CoreTest, MultiplePeripheralCreation)
{
    hal->init();
    
    auto gpio = hal->createGPIO();
    auto spi = hal->createSPI();
    auto i2c = hal->createI2C();
    auto uart = hal->createUART();
    auto pwm = hal->createPWM();
    auto timer = hal->createTimer();
    auto adc = hal->createADC();
    
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
    auto gpio = hal->createGPIO();
    ASSERT_NE(gpio, nullptr);
}

TEST_F(CoreTest, DISABLED_SetAndGetRealTimePolicy)
{
    hal->init();

    auto result = hal->setRealTimePolicy(RealTimePolicy::FIFO);

    EXPECT_EQ(result, RealTimePolicy::FIFO);
    EXPECT_EQ(hal->getRealTimePolicy(), RealTimePolicy::FIFO);

    EXPECT_EQ(hal->setRealTimePolicy(RealTimePolicy::RR), RealTimePolicy::RR);
    EXPECT_EQ(hal->getRealTimePolicy(), RealTimePolicy::RR);

    EXPECT_EQ(hal->setRealTimePolicy(RealTimePolicy::NONE), RealTimePolicy::NONE);
    EXPECT_EQ(hal->getRealTimePolicy(), RealTimePolicy::NONE);
}

