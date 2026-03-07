#include <gtest/gtest.h>
#include <hal/core.h>
#include <hal/adc.h>
#include <mock/hal_mock.h>
#include <mock/adc_mock.h>
#include <atomic>
#include <chrono>
#include <thread>

using namespace mex_hal;

class ADCTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        hal = std::make_unique<HALMock>();
        hal->init();
        adc = hal->createADC();
    }

    void TearDown() override
    {
        adc.reset();
        hal->shutdown();
    }

    static ADCConfig makeConfig(const ADCResolution res = ADCResolution::BITS_12,
                                const uint32_t rate = 1000,
                                const bool continuous = false)
    {
        ADCConfig config{};
        config.resolution = res;
        config.samplingRate = rate;
        config.continuousMode = continuous;
        return config;
    }

    std::unique_ptr<HAL> hal;
    std::unique_ptr<ADCInterface> adc;
};

TEST_F(ADCTest, CreateADC)
{
    ASSERT_NE(adc, nullptr);
}

TEST_F(ADCTest, InitADC)
{
    EXPECT_TRUE(adc->init(0, makeConfig()));
}

TEST_F(ADCTest, EnableChannel)
{
    ASSERT_TRUE(adc->init(0, makeConfig()));
    EXPECT_TRUE(adc->enableChannel(0));
}

TEST_F(ADCTest, EnableChannelBeforeInitFails)
{
    EXPECT_FALSE(adc->enableChannel(0));
}

TEST_F(ADCTest, DisableChannel)
{
    ASSERT_TRUE(adc->init(0, makeConfig()));
    ASSERT_TRUE(adc->enableChannel(0));
    EXPECT_TRUE(adc->disableChannel(0));
}

TEST_F(ADCTest, ReadChannel)
{
    ASSERT_TRUE(adc->init(0, makeConfig()));
    ASSERT_TRUE(adc->enableChannel(0));

    auto* mock = dynamic_cast<ADCMock*>(adc.get());
    mock->setChannelValue(0, 2048);

    uint16_t value = adc->read(0);
    EXPECT_EQ(value, 2048u);
}

TEST_F(ADCTest, ReadVoltage)
{
    auto config = makeConfig();
    ASSERT_TRUE(adc->init(0, config));
    ASSERT_TRUE(adc->enableChannel(0));

    auto* mock = dynamic_cast<ADCMock*>(adc.get());
    mock->setChannelValue(0, 2048);

    float voltage = adc->readVoltage(0, 3.3f);
    EXPECT_GT(voltage, 0.0f);
    EXPECT_LE(voltage, 3.3f);
}

TEST_F(ADCTest, ReadVoltageFullScale)
{
    ASSERT_TRUE(adc->init(0, makeConfig()));
    ASSERT_TRUE(adc->enableChannel(0));

    auto* mock = dynamic_cast<ADCMock*>(adc.get());
    mock->setChannelValue(0, 4095);

    float voltage = adc->readVoltage(0, 3.3f);
    EXPECT_NEAR(voltage, 3.3f, 0.01f);
}

TEST_F(ADCTest, ReadVoltageZero)
{
    ASSERT_TRUE(adc->init(0, makeConfig()));
    ASSERT_TRUE(adc->enableChannel(0));

    auto* mock = dynamic_cast<ADCMock*>(adc.get());
    mock->setChannelValue(0, 0);

    float voltage = adc->readVoltage(0, 3.3f);
    EXPECT_NEAR(voltage, 0.0f, 0.01f);
}

TEST_F(ADCTest, SetSamplingRate)
{
    ASSERT_TRUE(adc->init(0, makeConfig()));
    EXPECT_TRUE(adc->setSamplingRate(2000));
}

TEST_F(ADCTest, DifferentResolutions)
{
    EXPECT_TRUE(adc->init(0, makeConfig(ADCResolution::BITS_8)));
    EXPECT_TRUE(adc->init(0, makeConfig(ADCResolution::BITS_10)));
    EXPECT_TRUE(adc->init(0, makeConfig(ADCResolution::BITS_12)));
    EXPECT_TRUE(adc->init(0, makeConfig(ADCResolution::BITS_16)));
}

TEST_F(ADCTest, MultipleChannels)
{
    ASSERT_TRUE(adc->init(0, makeConfig()));

    auto* mock = dynamic_cast<ADCMock*>(adc.get());

    ASSERT_TRUE(adc->enableChannel(0));
    ASSERT_TRUE(adc->enableChannel(1));
    ASSERT_TRUE(adc->enableChannel(2));

    mock->setChannelValue(0, 100);
    mock->setChannelValue(1, 200);
    mock->setChannelValue(2, 300);

    EXPECT_EQ(adc->read(0), 100u);
    EXPECT_EQ(adc->read(1), 200u);
    EXPECT_EQ(adc->read(2), 300u);
}

TEST_F(ADCTest, ContinuousMode)
{
    auto config = makeConfig(ADCResolution::BITS_12, 1000, true);
    ASSERT_TRUE(adc->init(0, config));
    ASSERT_TRUE(adc->enableChannel(0));

    auto* mock = dynamic_cast<ADCMock*>(adc.get());
    mock->setChannelValue(0, 1234);

    std::atomic<bool> callbackCalled{false};
    std::atomic<uint16_t> lastValue{0};

    EXPECT_TRUE(adc->startContinuous(0, [&](uint16_t val)
    {
        lastValue.store(val);
        callbackCalled.store(true);
    }));

    // Wait briefly for the continuous thread to fire
    for (int i = 0; i < 50 && !callbackCalled.load(); ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    EXPECT_TRUE(callbackCalled.load());
    EXPECT_EQ(lastValue.load(), 1234u);

    EXPECT_TRUE(adc->stopContinuous());
}
