#include <gtest/gtest.h>
#include <hal/core.h>
#include <hal/i2c.h>
#include <mock/hal_mock.h>
#include <mock/i2c_mock.h>

using namespace mex_hal;

class I2CTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        hal = std::make_unique<HALMock>();
        hal->init();
        i2c = hal->createI2C();
    }

    void TearDown() override
    {
        i2c.reset();
        hal->shutdown();
    }

    std::unique_ptr<HAL> hal;
    std::unique_ptr<I2CInterface> i2c;
};

TEST_F(I2CTest, CreateI2C)
{
    ASSERT_NE(i2c, nullptr);
}

TEST_F(I2CTest, InitI2C)
{
    EXPECT_TRUE(i2c->init(1));
}

TEST_F(I2CTest, SetDeviceAddress)
{
    ASSERT_TRUE(i2c->init(1));
    EXPECT_TRUE(i2c->setDeviceAddress(0x48));
}

TEST_F(I2CTest, SetDeviceAddressBeforeInitFails)
{
    EXPECT_FALSE(i2c->setDeviceAddress(0x48));
}

TEST_F(I2CTest, WriteAndVerify)
{
    ASSERT_TRUE(i2c->init(1));
    ASSERT_TRUE(i2c->setDeviceAddress(0x48));

    std::vector<uint8_t> data = {0x01, 0x02};
    EXPECT_TRUE(i2c->write(data));

    auto* mock = dynamic_cast<I2CMock*>(i2c.get());
    ASSERT_NE(mock, nullptr);
    auto written = mock->getLastWritten(0x48);
    ASSERT_EQ(written.size(), 2u);
    EXPECT_EQ(written[0], 0x01);
    EXPECT_EQ(written[1], 0x02);
}

TEST_F(I2CTest, WriteBeforeInitFails)
{
    std::vector<uint8_t> data = {0x01};
    EXPECT_FALSE(i2c->write(data));
}

TEST_F(I2CTest, ReadWithInjectedData)
{
    ASSERT_TRUE(i2c->init(1));
    ASSERT_TRUE(i2c->setDeviceAddress(0x48));

    auto* mock = dynamic_cast<I2CMock*>(i2c.get());
    mock->enqueueReadData(0x48, {0xAA, 0xBB});

    std::vector<uint8_t> data;
    EXPECT_TRUE(i2c->read(data, 2));
    ASSERT_EQ(data.size(), 2u);
    EXPECT_EQ(data[0], 0xAA);
    EXPECT_EQ(data[1], 0xBB);
}

TEST_F(I2CTest, WriteRead)
{
    ASSERT_TRUE(i2c->init(1));
    ASSERT_TRUE(i2c->setDeviceAddress(0x48));

    auto* mock = dynamic_cast<I2CMock*>(i2c.get());
    mock->enqueueReadData(0x48, {0xDE, 0xAD});

    std::vector<uint8_t> writeData = {0x01};
    std::vector<uint8_t> readData;

    EXPECT_TRUE(i2c->writeRead(0x48, writeData, readData));
    ASSERT_EQ(readData.size(), 2u);
    EXPECT_EQ(readData[0], 0xDE);
    EXPECT_EQ(readData[1], 0xAD);
}

TEST_F(I2CTest, SetSpeed)
{
    ASSERT_TRUE(i2c->init(1));
    EXPECT_TRUE(i2c->setSpeed(400000));
}

TEST_F(I2CTest, SetSpeedBeforeInitFails)
{
    EXPECT_FALSE(i2c->setSpeed(400000));
}

TEST_F(I2CTest, DifferentAddresses)
{
    ASSERT_TRUE(i2c->init(1));
    EXPECT_TRUE(i2c->setDeviceAddress(0x48));
    EXPECT_TRUE(i2c->setDeviceAddress(0x50));
    EXPECT_TRUE(i2c->setDeviceAddress(0x68));
}

TEST_F(I2CTest, MultiDeviceIsolation)
{
    ASSERT_TRUE(i2c->init(1));

    auto* mock = dynamic_cast<I2CMock*>(i2c.get());

    ASSERT_TRUE(i2c->setDeviceAddress(0x48));
    i2c->write({0x11, 0x22});

    ASSERT_TRUE(i2c->setDeviceAddress(0x50));
    i2c->write({0x33, 0x44});

    auto written48 = mock->getLastWritten(0x48);
    auto written50 = mock->getLastWritten(0x50);
    EXPECT_EQ(written48[0], 0x11);
    EXPECT_EQ(written50[0], 0x33);
}
