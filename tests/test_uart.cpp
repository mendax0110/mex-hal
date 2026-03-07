#include <gtest/gtest.h>
#include <hal/core.h>
#include <hal/uart.h>
#include <mock/hal_mock.h>
#include <mock/uart_mock.h>

using namespace mex_hal;

constexpr const char* TEST_UART_DEVICE = "/dev/ttyMOCK0";

class UARTTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        hal = std::make_unique<HALMock>();
        hal->init();
        uart = hal->createUART();
    }

    void TearDown() override
    {
        uart.reset();
        hal->shutdown();
    }

    UARTConfig makeConfig(uint32_t baud = 115200)
    {
        UARTConfig config;
        config.baudRate = baud;
        config.dataBits = 8;
        config.stopBits = 1;
        config.parityEnable = false;
        return config;
    }

    std::unique_ptr<HAL> hal;
    std::unique_ptr<UARTInterface> uart;
};

TEST_F(UARTTest, CreateUART)
{
    ASSERT_NE(uart, nullptr);
}

TEST_F(UARTTest, InitUART)
{
    EXPECT_TRUE(uart->init(TEST_UART_DEVICE, makeConfig()));
}

TEST_F(UARTTest, WriteAndVerify)
{
    ASSERT_TRUE(uart->init(TEST_UART_DEVICE, makeConfig()));

    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    EXPECT_TRUE(uart->write(data));

    const auto* mock = dynamic_cast<UARTMock*>(uart.get());
    const auto txData = mock->getTxData();
    ASSERT_EQ(txData.size(), 3u);
    EXPECT_EQ(txData[0], 0x01);
    EXPECT_EQ(txData[1], 0x02);
    EXPECT_EQ(txData[2], 0x03);
}

TEST_F(UARTTest, WriteBeforeInitFails)
{
    std::vector<uint8_t> data = {0x01};
    EXPECT_FALSE(uart->write(data));
}

TEST_F(UARTTest, ReadWithInjectedData)
{
    ASSERT_TRUE(uart->init(TEST_UART_DEVICE, makeConfig()));

    auto* mock = dynamic_cast<UARTMock*>(uart.get());
    mock->injectRxData({0xAA, 0xBB, 0xCC});

    std::vector<uint8_t> data;
    EXPECT_TRUE(uart->read(data, 3));
    ASSERT_EQ(data.size(), 3u);
    EXPECT_EQ(data[0], 0xAA);
    EXPECT_EQ(data[1], 0xBB);
    EXPECT_EQ(data[2], 0xCC);
}

TEST_F(UARTTest, Available)
{
    ASSERT_TRUE(uart->init(TEST_UART_DEVICE, makeConfig()));

    EXPECT_EQ(uart->available(), 0u);

    auto* mock = dynamic_cast<UARTMock*>(uart.get());
    mock->injectRxData({0x01, 0x02, 0x03});

    EXPECT_EQ(uart->available(), 3u);
}

TEST_F(UARTTest, Flush)
{
    ASSERT_TRUE(uart->init(TEST_UART_DEVICE, makeConfig()));
    EXPECT_TRUE(uart->flush());
}

TEST_F(UARTTest, FlushBeforeInitFails)
{
    EXPECT_FALSE(uart->flush());
}

TEST_F(UARTTest, DifferentBaudRates)
{
    EXPECT_TRUE(uart->init(TEST_UART_DEVICE, makeConfig(9600)));
    EXPECT_TRUE(uart->init(TEST_UART_DEVICE, makeConfig(115200)));
}

TEST_F(UARTTest, ParityConfiguration)
{
    UARTConfig config = makeConfig();

    config.parityEnable = false;
    EXPECT_TRUE(uart->init(TEST_UART_DEVICE, config));

    config.parityEnable = true;
    config.evenParity = true;
    EXPECT_TRUE(uart->init(TEST_UART_DEVICE, config));

    config.evenParity = false;
    EXPECT_TRUE(uart->init(TEST_UART_DEVICE, config));
}

TEST_F(UARTTest, PartialRead)
{
    ASSERT_TRUE(uart->init(TEST_UART_DEVICE, makeConfig()));

    auto* mock = dynamic_cast<UARTMock*>(uart.get());
    mock->injectRxData({0x01, 0x02, 0x03, 0x04, 0x05});

    std::vector<uint8_t> data;
    EXPECT_TRUE(uart->read(data, 2));
    ASSERT_EQ(data.size(), 2u);
    EXPECT_EQ(data[0], 0x01);
    EXPECT_EQ(data[1], 0x02);

    // Remaining data should still be available
    EXPECT_EQ(uart->available(), 3u);
}
