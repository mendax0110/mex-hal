#ifndef MEX_HAL_ADC_MOCK_H
#define MEX_HAL_ADC_MOCK_H

#include "../../include/hal/adc.h"
#include <mutex>
#include <unordered_map>
#include <thread>
#include <atomic>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief Mock ADC implementation for testing \class ADCMock
    class ADCMock final : public ADCInterface
    {
    private:
        bool initialized_ = false;
        uint8_t device_ = 0;
        ADCConfig config_{};
        mutable std::mutex mutex_;
        std::unordered_map<uint8_t, bool> enabledChannels_;
        std::unordered_map<uint8_t, uint16_t> channelValues_;
        std::atomic<bool> continuousRunning_{false};
        std::atomic<bool> shouldStopContinuous_{false};
        std::thread continuousThread_;
        ADCReadCallback continuousCallback_{nullptr};
        uint8_t continuousChannel_ = 0;

    public:
        /**
         * @brief Constructor
         */
        ADCMock() = default;

        /**
         * @brief Destructor
         */
        ~ADCMock() override;

        /**
         * @brief Initialize the ADC device
         * @param device The ADC device number
         * @param config The ADC configuration
         * @return A true if initialization was successful, false otherwise
         */
        bool init(uint8_t device, const ADCConfig& config) override;

        /**
         * @brief Enable a specific ADC channel
         * @param channel The ADC channel number to enable
         * @return A true if the channel was successfully enabled, false otherwise
         */
        bool enableChannel(uint8_t channel) override;

        /**
         * @brief Disable a specific ADC channel
         * @param channel The ADC channel number to disable
         * @return A true if the channel was successfully disabled, false otherwise
         */
        bool disableChannel(uint8_t channel) override;

        /**
         * @brief Read a value from a specific ADC channel
         * @param channel The ADC channel number to read from
         * @return A 16-bit unsigned integer representing the ADC value
         */
        uint16_t read(uint8_t channel) override;

        /**
         * @brief Read multiple ADC channels
         * @param channels The vector of ADC channel numbers to read from
         * @param values The vector to store the read ADC values
         * @return A true if the read operation was successful, false otherwise
         */
        bool readMultiple(const std::vector<uint8_t>& channels, std::vector<uint16_t>& values) override;

        /**
         * @brief Start continuous reading on a specific channel
         * @param channel The ADC channel number to read from
         * @param callback The callback function to invoke with read values
         * @return A true if continuous reading was successfully started, false otherwise
         */
        bool startContinuous(uint8_t channel, ADCReadCallback callback) override;

        /**
         * @brief Stop continuous reading
         * @return A true if continuous reading was successfully stopped, false otherwise
         */
        bool stopContinuous() override;

        /**
         * @brief Set ADC resolution
         * @param resolution The desired ADC resolution
         * @return A true if the resolution was successfully set, false otherwise
         */
        bool setResolution(ADCResolution resolution) override;

        /**
         * @brief Set ADC sampling rate
         * @param samplingRate The desired sampling rate in Hz
         * @return A true if the sampling rate was successfully set, false otherwise
         */
        bool setSamplingRate(uint32_t samplingRate) override;

        /**
         * @brief Read voltage from a specific ADC channel
         * @param channel The ADC channel number to read from
         * @param referenceVoltage The reference voltage for conversion
         * @return The voltage value as a float
         */
        float readVoltage(uint8_t channel, float referenceVoltage) override;

        /**
         * @brief Set the value for a channel (test helper)
         * @param channel The ADC channel number
         * @param value The raw ADC value to set
         */
        void setChannelValue(uint8_t channel, uint16_t value);

        /**
         * @brief Check if ADC is initialized (test helper)
         * @return A true if initialized, false otherwise
         */
        bool isInitialized() const;
    };
}

#endif //MEX_HAL_ADC_MOCK_H
