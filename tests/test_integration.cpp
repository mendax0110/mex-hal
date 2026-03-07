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
#include <mock/gpio_mock.h>
#include <mock/spi_mock.h>
#include <mock/timer_mock.h>
#include <atomic>
#include <chrono>
#include <thread>

using namespace mex_hal;

class IntegrationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        hal = std::make_unique<HALMock>();
        hal->init();
    }

    void TearDown() override
    {
        hal->shutdown();
    }

    std::unique_ptr<HAL> hal;
};

TEST_F(IntegrationTest, AllPeripheralsCoexist)
{
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

    EXPECT_TRUE(gpio->setDirection(0, PinDirection::OUTPUT));
    EXPECT_TRUE(spi->init(0, 0, 1000000, SPIMode::MODE_0));
    EXPECT_TRUE(i2c->init(1));
    EXPECT_TRUE(timer->init(TimerMode::PERIODIC));

    ADCConfig adcConfig;
    adcConfig.resolution = ADCResolution::BITS_12;
    adcConfig.samplingRate = 1000;
    adcConfig.continuousMode = false;
    EXPECT_TRUE(adc->init(0, adcConfig));
    EXPECT_TRUE(pwm->init(0, 0));
}

TEST_F(IntegrationTest, GPIODrivenBySPIData)
{
    const auto gpio = hal->createGPIO();
    const auto spi = hal->createSPI();

    ASSERT_TRUE(gpio->setDirection(13, PinDirection::OUTPUT));
    ASSERT_TRUE(spi->init(0, 0, 1000000, SPIMode::MODE_0));

    auto* spiMock = dynamic_cast<SPIMock*>(spi.get());
    spiMock->enqueueRxData({0x01});

    const std::vector<uint8_t> txData = {0x00};
    std::vector<uint8_t> rxData;
    ASSERT_TRUE(spi->transfer(txData, rxData));
    ASSERT_EQ(rxData.size(), 1u);

    const PinValue val = (rxData[0] != 0) ? PinValue::HIGH : PinValue::LOW;
    EXPECT_TRUE(gpio->write(13, val));
    EXPECT_EQ(gpio->read(13), PinValue::HIGH);
}

TEST_F(IntegrationTest, TimerTogglesGPIO)
{
    const auto gpio = hal->createGPIO();
    const auto timer = hal->createTimer();

    ASSERT_TRUE(gpio->setDirection(5, PinDirection::OUTPUT));
    ASSERT_TRUE(gpio->write(5, PinValue::LOW));
    ASSERT_TRUE(timer->init(TimerMode::ONE_SHOT));

    std::atomic<bool> toggled{false};

    ASSERT_TRUE(timer->start(1000, [&]()
    {
        gpio->write(5, PinValue::HIGH);
        toggled.store(true);
    }));

    for (int i = 0; i < 50 && !toggled.load(); ++i)
    {
        // wait for timer...
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    EXPECT_TRUE(toggled.load());
    EXPECT_EQ(gpio->read(5), PinValue::HIGH);
    timer->stop();
}

TEST_F(IntegrationTest, MultipleGPIOInstances)
{
    const auto gpio1 = hal->createGPIO();
    const auto gpio2 = hal->createGPIO();

    ASSERT_TRUE(gpio1->setDirection(0, PinDirection::OUTPUT));
    ASSERT_TRUE(gpio2->setDirection(1, PinDirection::OUTPUT));

    ASSERT_TRUE(gpio1->write(0, PinValue::HIGH));
    ASSERT_TRUE(gpio2->write(1, PinValue::LOW));

    EXPECT_EQ(gpio1->read(0), PinValue::HIGH);
    EXPECT_EQ(gpio2->read(1), PinValue::LOW);
}

TEST_F(IntegrationTest, RealtimePolicyWithPeripherals)
{
    EXPECT_TRUE(hal->configureRealtime(50));
    EXPECT_TRUE(hal->isRealtimeConfigured());

    const auto gpio = hal->createGPIO();
    ASSERT_NE(gpio, nullptr);
    EXPECT_TRUE(gpio->setDirection(0, PinDirection::OUTPUT));
    EXPECT_TRUE(gpio->write(0, PinValue::HIGH));
}
