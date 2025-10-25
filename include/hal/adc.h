#ifndef MEX_HAL_ADC_H
#define MEX_HAL_ADC_H

#include "types.h"
#include <cstdint>
#include <vector>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief ADC Resultion enum \enum ADCResolution
    enum class ADCResolution
    {
        BITS_8 = 8,
        BITS_10 = 10,
        BITS_12 = 12,
        BITS_16 = 16
    };

    /// @brief ADC Config struct \struct ADCConfig
    struct ADCConfig
    {
        ADCResolution resolution;
        uint32_t samplingRate;
        bool continuousMode;
    };

    /// @brief ADC Interface class \class ADCInterface
    class ADCInterface
    {
    public:
        /**
         * @brief Virtual destructor
         */
        virtual ~ADCInterface() = default;

        /**
         * @brief Initialize the ADC device
         * @param device The ADC device number
         * @param config The ADC configuration
         * @return A true if initialization was successful, false otherwise
         */
        virtual bool init(uint8_t device, const ADCConfig& config) = 0;

        /**
         * @brief Enable a specific ADC channel
         * @param channel The ADC channel number to enable
         * @return A true if the channel was successfully enabled, false otherwise
         */
        virtual bool enableChannel(uint8_t channel) = 0;

        /**
         * @brief Disable a specific ADC channel
         * @param channel The ADC channel number to disable
         * @return A true if the channel was successfully disabled, false otherwise
         */
        virtual bool disableChannel(uint8_t channel) = 0;

        /**
         * @brief Read a value from a specific ADC channel
         * @param channel The ADC channel number to read from
         * @return A 16-bit unsigned integer representing the ADC value
         */
        virtual uint16_t read(uint8_t channel) = 0;

        /**
         * @brief Read multiple ADC channels
         * @param channels The vector of ADC channel numbers to read from
         * @param values The vector to store the read ADC values
         * @return A true if the read operation was successful, false otherwise
         */
        virtual bool readMultiple(const std::vector<uint8_t>& channels, std::vector<uint16_t>& values) = 0;

        /**
         * @brief Start continuous reading on a specific ADC channel
         * @param channel The ADC channel number to read from
         * @param callback The callback function to handle ADC read values
         * @return A true if continuous reading was successfully started, false otherwise
         */
        virtual bool startContinuous(uint8_t channel, ADCReadCallback callback) = 0;

        /**
         * @brief Stop continuous reading
         * @return A true if continuous reading was successfully stopped, false otherwise
         */
        virtual bool stopContinuous() = 0;

        /**
         * @brief Set the ADC resolution
         * @param resolution the desired ADC resolution
         * @return A true if the resolution was successfully set, false otherwise
         */
        virtual bool setResolution(ADCResolution resolution) = 0;

        /**
         * @brief Set the ADC sampling rate
         * @param samplingRate The desired sampling rate in Hz
         * @return A true if the sampling rate was successfully set, false otherwise
         */
        virtual bool setSamplingRate(uint32_t samplingRate) = 0;

        /**
         * @brief Read the voltage from a specific ADC channel
         * @param channel The ADC channel number to read from
         * @param referenceVoltage The reference voltage for the ADC
         * @return A float representing the measured voltage
         */
        virtual float readVoltage(uint8_t channel, float referenceVoltage) = 0;

    protected:
        inline static const std::string SYS_CLASS_IIO = "/sys/bus/iio/devices/iio:device";
    };
}

#endif //MEX_HAL_ADC_H