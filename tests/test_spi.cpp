#include <gtest/gtest.h>
#include <hal/core.h>
#include <hal/spi.h>

using namespace mex_hal;

class SPITest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        hal = createHAL(HALType::LINUX);
        hal->init();
        spi = hal->createSPI();
    }

    void TearDown() override
    {
        spi.reset();
        hal->shutdown();
    }

    std::unique_ptr<HAL> hal;
    std::unique_ptr<SPIInterface> spi;
};

TEST_F(SPITest, CreateSPI)
{
    ASSERT_NE(spi, nullptr);
}

TEST_F(SPITest, InitSPI)
{
    EXPECT_NO_THROW(spi->init(0, 0, 1000000, SPIMode::MODE_0));
}

TEST_F(SPITest, Transfer)
{
    spi->init(0, 0, 1000000, SPIMode::MODE_0);
    
    std::vector<uint8_t> txData = {0x01, 0x02, 0x03};
    std::vector<uint8_t> rxData;
    
    EXPECT_NO_THROW(spi->transfer(txData, rxData));
}

TEST_F(SPITest, Write)
{
    spi->init(0, 0, 1000000, SPIMode::MODE_0);
    
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    EXPECT_NO_THROW(spi->write(data));
}

TEST_F(SPITest, Read)
{
    spi->init(0, 0, 1000000, SPIMode::MODE_0);
    
    std::vector<uint8_t> data;
    EXPECT_NO_THROW(spi->read(data, 3));
}

TEST_F(SPITest, SetSpeed)
{
    spi->init(0, 0, 1000000, SPIMode::MODE_0);
    EXPECT_NO_THROW(spi->setSpeed(500000));
}

TEST_F(SPITest, SetMode)
{
    spi->init(0, 0, 1000000, SPIMode::MODE_0);
    EXPECT_NO_THROW(spi->setMode(SPIMode::MODE_1));
}



TEST_F(SPITest, DifferentModes)
{
    EXPECT_NO_THROW(spi->init(0, 0, 1000000, SPIMode::MODE_0));
    EXPECT_NO_THROW(spi->setMode(SPIMode::MODE_1));
    EXPECT_NO_THROW(spi->setMode(SPIMode::MODE_2));
    EXPECT_NO_THROW(spi->setMode(SPIMode::MODE_3));
}
