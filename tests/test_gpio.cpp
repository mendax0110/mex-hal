#include <gtest/gtest.h>
#include <hal/core.h>
#include <hal/gpio.h>
#include <mock/hal_mock.h>
#include <mock/gpio_mock.h>

using namespace mex_hal;

class GPIOTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        hal = std::make_unique<HALMock>();
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
    EXPECT_TRUE(gpio->setDirection(17, PinDirection::OUTPUT));
}

TEST_F(GPIOTest, SetDirectionInput)
{
    EXPECT_TRUE(gpio->setDirection(17, PinDirection::INPUT));
}

TEST_F(GPIOTest, WriteHighReadBack)
{
    ASSERT_TRUE(gpio->setDirection(17, PinDirection::OUTPUT));
    EXPECT_TRUE(gpio->write(17, PinValue::HIGH));
    EXPECT_EQ(gpio->read(17), PinValue::HIGH);
}

TEST_F(GPIOTest, WriteLowReadBack)
{
    ASSERT_TRUE(gpio->setDirection(17, PinDirection::OUTPUT));
    EXPECT_TRUE(gpio->write(17, PinValue::LOW));
    EXPECT_EQ(gpio->read(17), PinValue::LOW);
}

TEST_F(GPIOTest, ReadUnexportedPin)
{
    EXPECT_EQ(gpio->read(99), PinValue::LOW);
}

TEST_F(GPIOTest, WriteToInputPinFails)
{
    ASSERT_TRUE(gpio->setDirection(17, PinDirection::INPUT));
    EXPECT_FALSE(gpio->write(17, PinValue::HIGH));
}

TEST_F(GPIOTest, WriteToUnexportedPinFails)
{
    EXPECT_FALSE(gpio->write(99, PinValue::HIGH));
}

TEST_F(GPIOTest, SetInterrupt)
{
    ASSERT_TRUE(gpio->setDirection(17, PinDirection::INPUT));
    bool callbackCalled = false;

    EXPECT_TRUE(
        gpio->setInterrupt(17, EdgeTrigger::RISING, [&callbackCalled](uint8_t, PinValue)
        {
            callbackCalled = true;
        })
    );
}

TEST_F(GPIOTest, InterruptCallbackInvoked)
{
    auto* gpioMock = dynamic_cast<GPIOMock*>(gpio.get());
    ASSERT_NE(gpioMock, nullptr);

    bool callbackCalled = false;
    uint8_t receivedPin = 0;
    PinValue receivedValue = PinValue::LOW;

    gpioMock->setDirection(17, PinDirection::INPUT);
    gpioMock->setInterrupt(17, EdgeTrigger::RISING, [&](uint8_t pin, PinValue val)
    {
        callbackCalled = true;
        receivedPin = pin;
        receivedValue = val;
    });

    gpioMock->simulateInterrupt(17, PinValue::HIGH);

    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(receivedPin, 17);
    EXPECT_EQ(receivedValue, PinValue::HIGH);
}

TEST_F(GPIOTest, RemoveInterrupt)
{
    gpio->setDirection(17, PinDirection::INPUT);
    gpio->setInterrupt(17, EdgeTrigger::RISING, [](uint8_t, PinValue) {});
    EXPECT_TRUE(gpio->removeInterrupt(17));
}

TEST_F(GPIOTest, RemoveNonexistentInterrupt)
{
    EXPECT_FALSE(gpio->removeInterrupt(99));
}

TEST_F(GPIOTest, SetDebounce)
{
    gpio->setDirection(17, PinDirection::INPUT);
    EXPECT_TRUE(gpio->setDebounce(17, 50));
}

TEST_F(GPIOTest, SetDebounceUnexportedPinFails)
{
    EXPECT_FALSE(gpio->setDebounce(99, 50));
}

TEST_F(GPIOTest, MultiplePins)
{
    EXPECT_TRUE(gpio->setDirection(17, PinDirection::OUTPUT));
    EXPECT_TRUE(gpio->setDirection(27, PinDirection::INPUT));
    EXPECT_TRUE(gpio->write(17, PinValue::HIGH));
    EXPECT_EQ(gpio->read(17), PinValue::HIGH);
    EXPECT_EQ(gpio->read(27), PinValue::LOW);
}

TEST_F(GPIOTest, TogglePin)
{
    ASSERT_TRUE(gpio->setDirection(17, PinDirection::OUTPUT));

    EXPECT_TRUE(gpio->write(17, PinValue::HIGH));
    EXPECT_EQ(gpio->read(17), PinValue::HIGH);

    EXPECT_TRUE(gpio->write(17, PinValue::LOW));
    EXPECT_EQ(gpio->read(17), PinValue::LOW);

    EXPECT_TRUE(gpio->write(17, PinValue::HIGH));
    EXPECT_EQ(gpio->read(17), PinValue::HIGH);
}
