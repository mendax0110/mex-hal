#include <gtest/gtest.h>
#include <hal/core.h>
#include <hal/i2c.h>

using namespace mex_hal;

class I2CTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        hal = createHAL(HALType::LINUX);
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
    EXPECT_NO_THROW(i2c->init(1));
}

TEST_F(I2CTest, SetDeviceAddress)
{
    i2c->init(1);
    EXPECT_NO_THROW(i2c->setDeviceAddress(0x48));
}

TEST_F(I2CTest, Write)
{
    i2c->init(1);
    i2c->setDeviceAddress(0x48);
    
    std::vector<uint8_t> data = {0x01, 0x02};
    EXPECT_NO_THROW(i2c->write(data));
}

TEST_F(I2CTest, Read)
{
    i2c->init(1);
    i2c->setDeviceAddress(0x48);
    
    std::vector<uint8_t> data;
    EXPECT_NO_THROW(i2c->read(data, 2));
}

TEST_F(I2CTest, WriteRead)
{
    i2c->init(1);
    i2c->setDeviceAddress(0x48);
    
    std::vector<uint8_t> writeData = {0x01};
    std::vector<uint8_t> readData;
    
    EXPECT_NO_THROW(i2c->writeRead(0x48, writeData, readData));
}

TEST_F(I2CTest, SetSpeed)
{
    i2c->init(1);
    EXPECT_NO_THROW(i2c->setSpeed(400000));
}

TEST_F(I2CTest, DifferentAddresses)
{
    i2c->init(1);
    EXPECT_NO_THROW(i2c->setDeviceAddress(0x48));
    EXPECT_NO_THROW(i2c->setDeviceAddress(0x50));
    EXPECT_NO_THROW(i2c->setDeviceAddress(0x68));
}
