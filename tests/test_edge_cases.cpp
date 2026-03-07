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
#include <mock/i2c_mock.h>
#include <mock/uart_mock.h>
#include <mock/pwm_mock.h>
#include <mock/adc_mock.h>
#include <mock/timer_mock.h>

using namespace mex_hal;

class EdgeCaseTest : public ::testing::Test
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


TEST_F(EdgeCaseTest, GPIOReadUnconfiguredPin)
{
    const auto gpio = hal->createGPIO();
    EXPECT_EQ(gpio->read(99), PinValue::LOW);
}

TEST_F(EdgeCaseTest, GPIOWriteToInputPin)
{
    const auto gpio = hal->createGPIO();
    ASSERT_TRUE(gpio->setDirection(0, PinDirection::INPUT));
    // Writing to an input pin is rejected
    EXPECT_FALSE(gpio->write(0, PinValue::HIGH));
}

TEST_F(EdgeCaseTest, GPIODoubleSetDirection)
{
    const auto gpio = hal->createGPIO();
    EXPECT_TRUE(gpio->setDirection(0, PinDirection::OUTPUT));
    EXPECT_TRUE(gpio->setDirection(0, PinDirection::INPUT));

    const auto* mock = dynamic_cast<GPIOMock*>(gpio.get());
    EXPECT_EQ(mock->getDirection(0), PinDirection::INPUT);
}

TEST_F(EdgeCaseTest, GPIORemoveNonExistentInterrupt)
{
    const auto gpio = hal->createGPIO();
    // Removing interrupt from unknown pin returns false
    EXPECT_FALSE(gpio->removeInterrupt(0));
}

TEST_F(EdgeCaseTest, SPITransferEmptyData)
{
    const auto spi = hal->createSPI();
    ASSERT_TRUE(spi->init(0, 0, 1000000, SPIMode::MODE_0));

    const std::vector<uint8_t> txData;
    std::vector<uint8_t> rxData;
    // Empty transfer
    EXPECT_TRUE(spi->transfer(txData, rxData));
}

TEST_F(EdgeCaseTest, SPIDoubleInit)
{
    const auto spi = hal->createSPI();
    EXPECT_TRUE(spi->init(0, 0, 1000000, SPIMode::MODE_0));
    EXPECT_TRUE(spi->init(1, 1, 2000000, SPIMode::MODE_3));
}

TEST_F(EdgeCaseTest, I2CWriteWithoutAddressSet)
{
    const auto i2c = hal->createI2C();
    ASSERT_TRUE(i2c->init(1));
    // Write without setting device address first
    const std::vector<uint8_t> data = {0x01};
    EXPECT_FALSE(i2c->write(data));
}

TEST_F(EdgeCaseTest, I2CReadWithNoData)
{
    const auto i2c = hal->createI2C();
    ASSERT_TRUE(i2c->init(1));
    ASSERT_TRUE(i2c->setDeviceAddress(0x48));

    std::vector<uint8_t> data;
    // No data enqueued, should still return something (zeros)
    EXPECT_TRUE(i2c->read(data, 2));
}

TEST_F(EdgeCaseTest, I2CDoubleInit)
{
    const auto i2c = hal->createI2C();
    EXPECT_TRUE(i2c->init(1));
    EXPECT_TRUE(i2c->init(2));
}

TEST_F(EdgeCaseTest, UARTReadEmptyBuffer)
{
    const auto uart = hal->createUART();
    UARTConfig config;
    config.baudRate = 115200;
    config.dataBits = 8;
    config.stopBits = 1;
    config.parityEnable = false;
    ASSERT_TRUE(uart->init("/dev/ttyMOCK0", config));

    std::vector<uint8_t> data;
    // Reading from empty buffer returns false
    EXPECT_FALSE(uart->read(data, 5));
}

TEST_F(EdgeCaseTest, UARTFlushIdempotent)
{
    const auto uart = hal->createUART();
    UARTConfig config;
    config.baudRate = 115200;
    config.dataBits = 8;
    config.stopBits = 1;
    config.parityEnable = false;
    ASSERT_TRUE(uart->init("/dev/ttyMOCK0", config));

    EXPECT_TRUE(uart->flush());
    EXPECT_TRUE(uart->flush());
}

TEST_F(EdgeCaseTest, PWMSetDutyCycleWithoutPeriod)
{
    const auto pwm = hal->createPWM();
    ASSERT_TRUE(pwm->init(0, 0));
    // Period is 0, setting duty cycle percent should handle gracefully
    EXPECT_TRUE(pwm->setDutyCyclePercent(50.0f));
}

TEST_F(EdgeCaseTest, PWMEnableDisableToggle)
{
    const auto pwm = hal->createPWM();
    ASSERT_TRUE(pwm->init(0, 0));

    EXPECT_TRUE(pwm->enable(true));
    EXPECT_TRUE(pwm->isEnabled());
    EXPECT_TRUE(pwm->enable(false));
    EXPECT_FALSE(pwm->isEnabled());
    EXPECT_TRUE(pwm->enable(true));
    EXPECT_TRUE(pwm->isEnabled());
}

TEST_F(EdgeCaseTest, TimerStopBeforeStart)
{
    const auto timer = hal->createTimer();
    ASSERT_TRUE(timer->init(TimerMode::ONE_SHOT));
    // Stopping without starting returns false (no thread to join)
    EXPECT_FALSE(timer->stop());
}

TEST_F(EdgeCaseTest, TimerIsNotRunningInitially)
{
    const auto timer = hal->createTimer();
    ASSERT_TRUE(timer->init(TimerMode::PERIODIC));
    EXPECT_FALSE(timer->isRunning());
}

TEST_F(EdgeCaseTest, TimerGetElapsedBeforeStart)
{
    const auto timer = hal->createTimer();
    ASSERT_TRUE(timer->init(TimerMode::ONE_SHOT));
    // Should return 0 or some sensible value before start
    const auto elapsed = timer->getElapsedUs();
    EXPECT_GE(elapsed, 0u);
}

TEST_F(EdgeCaseTest, ADCReadDisabledChannel)
{
    const auto adc = hal->createADC();
    ADCConfig config;
    config.resolution = ADCResolution::BITS_12;
    config.samplingRate = 1000;
    config.continuousMode = false;
    ASSERT_TRUE(adc->init(0, config));

    // Read from channel that was not enabled - should return 0
    const uint16_t val = adc->read(0);
    EXPECT_EQ(val, 0u);
}

TEST_F(EdgeCaseTest, ADCDisableAlreadyDisabledChannel)
{
    const auto adc = hal->createADC();
    ADCConfig config;
    config.resolution = ADCResolution::BITS_12;
    config.samplingRate = 1000;
    config.continuousMode = false;
    ASSERT_TRUE(adc->init(0, config));

    // Disable a channel that was never enabled
    EXPECT_TRUE(adc->disableChannel(0));
}

TEST_F(EdgeCaseTest, ADCStopContinuousWithoutStart)
{
    const auto adc = hal->createADC();
    ADCConfig config;
    config.resolution = ADCResolution::BITS_12;
    config.samplingRate = 1000;
    config.continuousMode = true;
    ASSERT_TRUE(adc->init(0, config));

    // Stopping continuous when not running returns false
    EXPECT_FALSE(adc->stopContinuous());
}
