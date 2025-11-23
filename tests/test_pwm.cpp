#include <gtest/gtest.h>
#include <hal/core.h>
#include <hal/pwm.h>

using namespace mex_hal;

class PWMTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        hal = createHAL(HALType::LINUX);
        hal->init();
        pwm = hal->createPWM();
    }

    void TearDown() override
    {
        pwm.reset();
        hal->shutdown();
    }

    std::unique_ptr<HAL> hal;
    std::unique_ptr<PWMInterface> pwm;
};

TEST_F(PWMTest, CreatePWM)
{
    ASSERT_NE(pwm, nullptr);
}

TEST_F(PWMTest, InitPWM)
{
    EXPECT_NO_THROW(pwm->init(0, 0));
}

TEST_F(PWMTest, SetPeriod)
{
    pwm->init(0, 0);
    EXPECT_NO_THROW(pwm->setPeriod(20000000));  // 20ms
}

TEST_F(PWMTest, SetDutyCycle)
{
    pwm->init(0, 0);
    pwm->setPeriod(20000000);
    EXPECT_NO_THROW(pwm->setDutyCycle(10000000));  // 10ms
}

TEST_F(PWMTest, SetDutyCyclePercent)
{
    pwm->init(0, 0);
    pwm->setPeriod(20000000);
    EXPECT_NO_THROW(pwm->setDutyCyclePercent(50.0f));
}

TEST_F(PWMTest, Enable)
{
    pwm->init(0, 0);
    pwm->setPeriod(20000000);
    pwm->setDutyCyclePercent(50.0f);
    EXPECT_NO_THROW(pwm->enable(true));
}

TEST_F(PWMTest, Disable)
{
    pwm->init(0, 0);
    pwm->enable(true);
    EXPECT_NO_THROW(pwm->enable(false));
}

TEST_F(PWMTest, SetPolarity)
{
    pwm->init(0, 0);
    EXPECT_NO_THROW(pwm->setPolarity(false));
    EXPECT_NO_THROW(pwm->setPolarity(true));
}

TEST_F(PWMTest, DutyCycleRange)
{
    pwm->init(0, 0);
    pwm->setPeriod(20000000);
    
    EXPECT_NO_THROW(pwm->setDutyCyclePercent(0.0f));
    EXPECT_NO_THROW(pwm->setDutyCyclePercent(25.0f));
    EXPECT_NO_THROW(pwm->setDutyCyclePercent(50.0f));
    EXPECT_NO_THROW(pwm->setDutyCyclePercent(75.0f));
    EXPECT_NO_THROW(pwm->setDutyCyclePercent(100.0f));
}

TEST_F(PWMTest, PeriodValidation)
{
    pwm->init(0, 0);
    
    // Test common PWM frequencies
    EXPECT_NO_THROW(pwm->setPeriod(20000000));   // 50 Hz (servo)
    EXPECT_NO_THROW(pwm->setPeriod(1000000));    // 1 kHz
    EXPECT_NO_THROW(pwm->setPeriod(100000));     // 10 kHz
}
