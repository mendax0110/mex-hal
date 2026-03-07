#include <gtest/gtest.h>
#include <hal/core.h>
#include <hal/pwm.h>
#include <mock/hal_mock.h>
#include <mock/pwm_mock.h>

using namespace mex_hal;

class PWMTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        hal = std::make_unique<HALMock>();
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
    EXPECT_TRUE(pwm->init(0, 0));
}

TEST_F(PWMTest, SetPeriod)
{
    ASSERT_TRUE(pwm->init(0, 0));
    EXPECT_TRUE(pwm->setPeriod(20000000));
    EXPECT_EQ(pwm->getPeriod(), 20000000u);
}

TEST_F(PWMTest, SetPeriodBeforeInitFails)
{
    EXPECT_FALSE(pwm->setPeriod(20000000));
}

TEST_F(PWMTest, SetDutyCycle)
{
    ASSERT_TRUE(pwm->init(0, 0));
    ASSERT_TRUE(pwm->setPeriod(20000000));
    EXPECT_TRUE(pwm->setDutyCycle(10000000));
    EXPECT_EQ(pwm->getDutyCycle(), 10000000u);
}

TEST_F(PWMTest, SetDutyCyclePercent)
{
    ASSERT_TRUE(pwm->init(0, 0));
    ASSERT_TRUE(pwm->setPeriod(20000000));
    EXPECT_TRUE(pwm->setDutyCyclePercent(50.0f));
    EXPECT_EQ(pwm->getDutyCycle(), 10000000u);
}

TEST_F(PWMTest, Enable)
{
    ASSERT_TRUE(pwm->init(0, 0));
    ASSERT_TRUE(pwm->setPeriod(20000000));
    ASSERT_TRUE(pwm->setDutyCyclePercent(50.0f));
    EXPECT_TRUE(pwm->enable(true));
    EXPECT_TRUE(pwm->isEnabled());
}

TEST_F(PWMTest, Disable)
{
    ASSERT_TRUE(pwm->init(0, 0));
    ASSERT_TRUE(pwm->enable(true));
    EXPECT_TRUE(pwm->isEnabled());
    EXPECT_TRUE(pwm->enable(false));
    EXPECT_FALSE(pwm->isEnabled());
}

TEST_F(PWMTest, EnableBeforeInitFails)
{
    EXPECT_FALSE(pwm->enable(true));
}

TEST_F(PWMTest, SetPolarity)
{
    ASSERT_TRUE(pwm->init(0, 0));
    EXPECT_TRUE(pwm->setPolarity(false));
    EXPECT_TRUE(pwm->setPolarity(true));
}

TEST_F(PWMTest, DutyCycleRange)
{
    ASSERT_TRUE(pwm->init(0, 0));
    ASSERT_TRUE(pwm->setPeriod(20000000));

    EXPECT_TRUE(pwm->setDutyCyclePercent(0.0f));
    EXPECT_EQ(pwm->getDutyCycle(), 0u);

    EXPECT_TRUE(pwm->setDutyCyclePercent(100.0f));
    EXPECT_EQ(pwm->getDutyCycle(), 20000000u);
}

TEST_F(PWMTest, PeriodValidation)
{
    ASSERT_TRUE(pwm->init(0, 0));

    EXPECT_TRUE(pwm->setPeriod(20000000));
    EXPECT_EQ(pwm->getPeriod(), 20000000u);

    EXPECT_TRUE(pwm->setPeriod(1000000));
    EXPECT_EQ(pwm->getPeriod(), 1000000u);

    EXPECT_TRUE(pwm->setPeriod(100000));
    EXPECT_EQ(pwm->getPeriod(), 100000u);
}
