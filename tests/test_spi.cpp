#include <gtest/gtest.h>
#include <hal/core.h>
#include <hal/spi.h>
#include <mock/hal_mock.h>
#include <mock/spi_mock.h>

using namespace mex_hal;

class SPITest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        hal = std::make_unique<HALMock>();
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
    EXPECT_TRUE(spi->init(0, 0, 1000000, SPIMode::MODE_0));
}

TEST_F(SPITest, TransferReturnsData)
{
    ASSERT_TRUE(spi->init(0, 0, 1000000, SPIMode::MODE_0));

    auto* spiMock = dynamic_cast<SPIMock*>(spi.get());
    ASSERT_NE(spiMock, nullptr);

    spiMock->enqueueRxData({0xAA, 0xBB, 0xCC});

    const std::vector<uint8_t> txData = {0x01, 0x02, 0x03};
    std::vector<uint8_t> rxData;

    EXPECT_TRUE(spi->transfer(txData, rxData));
    ASSERT_EQ(rxData.size(), 3u);
    EXPECT_EQ(rxData[0], 0xAA);
    EXPECT_EQ(rxData[1], 0xBB);
    EXPECT_EQ(rxData[2], 0xCC);
}

TEST_F(SPITest, TransferBeforeInitFails)
{
    const std::vector<uint8_t> txData = {0x01};
    std::vector<uint8_t> rxData;
    EXPECT_FALSE(spi->transfer(txData, rxData));
}

TEST_F(SPITest, Write)
{
    ASSERT_TRUE(spi->init(0, 0, 1000000, SPIMode::MODE_0));
    const std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    EXPECT_TRUE(spi->write(data));
}

TEST_F(SPITest, WriteBeforeInitFails)
{
    const std::vector<uint8_t> data = {0x01};
    EXPECT_FALSE(spi->write(data));
}

TEST_F(SPITest, Read)
{
    ASSERT_TRUE(spi->init(0, 0, 1000000, SPIMode::MODE_0));

    auto* spiMock = dynamic_cast<SPIMock*>(spi.get());
    spiMock->enqueueRxData({0xDE, 0xAD});

    std::vector<uint8_t> data;
    EXPECT_TRUE(spi->read(data, 2));
    ASSERT_EQ(data.size(), 2u);
    EXPECT_EQ(data[0], 0xDE);
    EXPECT_EQ(data[1], 0xAD);
}

TEST_F(SPITest, ReadZeroLengthFails)
{
    ASSERT_TRUE(spi->init(0, 0, 1000000, SPIMode::MODE_0));
    std::vector<uint8_t> data;
    EXPECT_FALSE(spi->read(data, 0));
}

TEST_F(SPITest, SetSpeed)
{
    ASSERT_TRUE(spi->init(0, 0, 1000000, SPIMode::MODE_0));
    EXPECT_TRUE(spi->setSpeed(500000));

    const auto* spiMock = dynamic_cast<SPIMock*>(spi.get());
    EXPECT_EQ(spiMock->getSpeed(), 500000u);
}

TEST_F(SPITest, SetSpeedBeforeInitFails)
{
    EXPECT_FALSE(spi->setSpeed(500000));
}

TEST_F(SPITest, SetMode)
{
    ASSERT_TRUE(spi->init(0, 0, 1000000, SPIMode::MODE_0));
    EXPECT_TRUE(spi->setMode(SPIMode::MODE_3));

    const auto* spiMock = dynamic_cast<SPIMock*>(spi.get());
    EXPECT_EQ(spiMock->getMode(), SPIMode::MODE_3);
}

TEST_F(SPITest, DifferentModes)
{
    ASSERT_TRUE(spi->init(0, 0, 1000000, SPIMode::MODE_0));
    EXPECT_TRUE(spi->setMode(SPIMode::MODE_1));
    EXPECT_TRUE(spi->setMode(SPIMode::MODE_2));
    EXPECT_TRUE(spi->setMode(SPIMode::MODE_3));
}
