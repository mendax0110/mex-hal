#include <gtest/gtest.h>
#include <hal/core.h>
#include <hal/adc.h>

using namespace mex_hal;

class ADCTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        hal = createHAL(HALType::LINUX);
        hal->init();
        adc = hal->createADC();
    }

    void TearDown() override
    {
        adc.reset();
        hal->shutdown();
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
    ADCConfig config;
    config.resolution = ADCResolution::BITS_12;
    config.samplingRate = 1000;
    config.continuousMode = false;
    
    EXPECT_NO_THROW(adc->init(0, config));
}

TEST_F(ADCTest, EnableChannel)
{
    ADCConfig config;
    config.resolution = ADCResolution::BITS_12;
    config.samplingRate = 1000;
    config.continuousMode = false;
    
    adc->init(0, config);
    EXPECT_NO_THROW(adc->enableChannel(0));
}

TEST_F(ADCTest, DisableChannel)
{
    ADCConfig config;
    config.resolution = ADCResolution::BITS_12;
    config.samplingRate = 1000;
    config.continuousMode = false;
    
    adc->init(0, config);
    adc->enableChannel(0);
    EXPECT_NO_THROW(adc->disableChannel(0));
}

TEST_F(ADCTest, ReadChannel)
{
    ADCConfig config;
    config.resolution = ADCResolution::BITS_12;
    config.samplingRate = 1000;
    config.continuousMode = false;
    
    adc->init(0, config);
    adc->enableChannel(0);
    
    uint16_t value = 0;
    EXPECT_NO_THROW(value = adc->read(0));
}

TEST_F(ADCTest, ReadVoltage)
{
    ADCConfig config;
    config.resolution = ADCResolution::BITS_12;
    config.samplingRate = 1000;
    config.continuousMode = false;
    
    adc->init(0, config);
    adc->enableChannel(0);
    
    float voltage = 0.0f;
    EXPECT_NO_THROW(voltage = adc->readVoltage(0, 3.3f));
    EXPECT_GE(voltage, 0.0f);
    EXPECT_LE(voltage, 3.3f);
}

TEST_F(ADCTest, SetSamplingRate)
{
    ADCConfig config;
    config.resolution = ADCResolution::BITS_12;
    config.samplingRate = 1000;
    config.continuousMode = false;
    
    adc->init(0, config);
    
    EXPECT_NO_THROW(adc->setSamplingRate(2000));
}

TEST_F(ADCTest, DifferentResolutions)
{
    ADCConfig config;
    config.samplingRate = 1000;
    config.continuousMode = false;
    
    config.resolution = ADCResolution::BITS_8;
    EXPECT_NO_THROW(adc->init(0, config));
    
    config.resolution = ADCResolution::BITS_10;
    EXPECT_NO_THROW(adc->init(0, config));
    
    config.resolution = ADCResolution::BITS_12;
    EXPECT_NO_THROW(adc->init(0, config));
    
    config.resolution = ADCResolution::BITS_16;
    EXPECT_NO_THROW(adc->init(0, config));
}

TEST_F(ADCTest, ContinuousMode)
{
    ADCConfig config;
    config.resolution = ADCResolution::BITS_12;
    config.samplingRate = 1000;
    config.continuousMode = true;
    
    EXPECT_NO_THROW(adc->init(0, config));
}

TEST_F(ADCTest, MultipleChannels)
{
    ADCConfig config;
    config.resolution = ADCResolution::BITS_12;
    config.samplingRate = 1000;
    config.continuousMode = false;
    
    adc->init(0, config);
    
    EXPECT_NO_THROW(adc->enableChannel(0));
    EXPECT_NO_THROW(adc->enableChannel(1));
    EXPECT_NO_THROW(adc->enableChannel(2));
    
    EXPECT_NO_THROW(adc->read(0));
    EXPECT_NO_THROW(adc->read(1));
    EXPECT_NO_THROW(adc->read(2));
}
