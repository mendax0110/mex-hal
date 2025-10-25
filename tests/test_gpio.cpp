#include <gtest/gtest.h>
#include <hal/core.h>
#include <hal/gpio.h>

using namespace mex_hal;

class GPIOTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        hal = createHAL(HALType::LINUX);
        hal->init();
        gpio = hal->createGPIO();
    }

    void TearDown() override
    {
        gpio.reset();
        hal->shutdown();
    }

    std::unique_ptr<HAL> hal;
    std::unique_ptr<GPIOInterface> gpio;
};

TEST_F(GPIOTest, CreateGPIO)
{
    ASSERT_NE(gpio, nullptr);
}

TEST_F(GPIOTest, SetDirectionOutput)
{
    EXPECT_NO_THROW(gpio->setDirection(17, PinDirection::OUTPUT));
}

TEST_F(GPIOTest, SetDirectionInput)
{
    EXPECT_NO_THROW(gpio->setDirection(17, PinDirection::INPUT));
}

TEST_F(GPIOTest, WriteHigh)
{
    gpio->setDirection(17, PinDirection::OUTPUT);
    EXPECT_NO_THROW(gpio->write(17, PinValue::HIGH));
}

TEST_F(GPIOTest, WriteLow)
{
    gpio->setDirection(17, PinDirection::OUTPUT);
    EXPECT_NO_THROW(gpio->write(17, PinValue::LOW));
}

TEST_F(GPIOTest, ReadPin)
{
    gpio->setDirection(17, PinDirection::INPUT);
    EXPECT_NO_THROW(gpio->read(17));
}

TEST_F(GPIOTest, SetInterrupt)
{
    gpio->setDirection(17, PinDirection::INPUT);
    bool callbackCalled = false;
    
    EXPECT_NO_THROW(
        gpio->setInterrupt(17, EdgeTrigger::RISING, [&callbackCalled](uint8_t, PinValue) {
            callbackCalled = true;
        })
    );
}

TEST_F(GPIOTest, RemoveInterrupt)
{
    gpio->setDirection(17, PinDirection::INPUT);
    gpio->setInterrupt(17, EdgeTrigger::RISING, [](uint8_t, PinValue) {});
    
    EXPECT_NO_THROW(gpio->removeInterrupt(17));
}

TEST_F(GPIOTest, SetDebounce)
{
    gpio->setDirection(17, PinDirection::INPUT);
    EXPECT_NO_THROW(gpio->setDebounce(17, 50));
}

TEST_F(GPIOTest, MultiplePins)
{
    EXPECT_NO_THROW(gpio->setDirection(17, PinDirection::OUTPUT));
    EXPECT_NO_THROW(gpio->setDirection(27, PinDirection::INPUT));
    EXPECT_NO_THROW(gpio->write(17, PinValue::HIGH));
}
