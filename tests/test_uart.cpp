#include <gtest/gtest.h>
#include <hal/core.h>
#include <hal/uart.h>

using namespace mex_hal;

constexpr const char* TEST_UART_DEVICE = "/dev/ttyUSB0";

class UARTTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        hal = createHAL(HALType::LINUX);
        hal->init();
        uart = hal->createUART();
    }

    void TearDown() override
    {
        uart.reset();
        hal->shutdown();
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
    UARTConfig config;
    config.baudRate = 115200;
    config.dataBits = 8;
    config.stopBits = 1;
    config.parityEnable = false;
    
    EXPECT_NO_THROW(uart->init(TEST_UART_DEVICE, config));
}

TEST_F(UARTTest, Write)
{
    UARTConfig config;
    config.baudRate = 115200;
    config.dataBits = 8;
    config.stopBits = 1;
    config.parityEnable = false;
    
    uart->init(TEST_UART_DEVICE, config);
    
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    EXPECT_NO_THROW(uart->write(data));
}

TEST_F(UARTTest, Read)
{
    UARTConfig config;
    config.baudRate = 115200;
    config.dataBits = 8;
    config.stopBits = 1;
    config.parityEnable = false;
    
    uart->init(TEST_UART_DEVICE, config);
    
    std::vector<uint8_t> data;
    EXPECT_NO_THROW(uart->read(data, 10));
}

TEST_F(UARTTest, Available)
{
    UARTConfig config;
    config.baudRate = 115200;
    config.dataBits = 8;
    config.stopBits = 1;
    config.parityEnable = false;
    
    uart->init(TEST_UART_DEVICE, config);
    
    EXPECT_NO_THROW(uart->available());
}

TEST_F(UARTTest, Flush)
{
    UARTConfig config;
    config.baudRate = 115200;
    config.dataBits = 8;
    config.stopBits = 1;
    config.parityEnable = false;
    
    uart->init(TEST_UART_DEVICE, config);
    
    EXPECT_NO_THROW(uart->flush());
}

TEST_F(UARTTest, DifferentBaudRates)
{
    UARTConfig config;
    config.dataBits = 8;
    config.stopBits = 1;
    config.parityEnable = false;
    
    config.baudRate = 9600;
    EXPECT_NO_THROW(uart->init(TEST_UART_DEVICE, config));
    
    config.baudRate = 115200;
    EXPECT_NO_THROW(uart->init(TEST_UART_DEVICE, config));
}

TEST_F(UARTTest, ParityConfiguration)
{
    UARTConfig config;
    config.baudRate = 115200;
    config.dataBits = 8;
    config.stopBits = 1;
    
    config.parityEnable = false;
    EXPECT_NO_THROW(uart->init(TEST_UART_DEVICE, config));
    
    config.parityEnable = true;
    config.evenParity = true;
    EXPECT_NO_THROW(uart->init(TEST_UART_DEVICE, config));
    
    config.evenParity = false;
    EXPECT_NO_THROW(uart->init(TEST_UART_DEVICE, config));
}
