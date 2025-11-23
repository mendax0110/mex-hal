#ifndef MEX_HAL_ADC_LINUX_H
#define MEX_HAL_ADC_LINUX_H

#include "../../include/hal/adc.h"
#include "../../include/hal/resource_manager.h"
#include <fstream>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief ADC Linux implementation class \class ADCLinux
    class ADCLinux final : public ADCInterface
    {
    private:
        uint8_t device_ = 0;
        ADCConfig config_{};
        std::atomic<bool> continuousRunning_{false};
        std::atomic<bool> shouldStopContinuous_{false};
        std::thread continuousThread_;
        ADCReadCallback continuousCallback_;
        uint8_t continuousChannel_ = 0;
        uint64_t resourceId_ = 0;
        mutable std::mutex adcMutex_;
        std::mutex callbackMutex_;

        /**
         * @brief Get the device path for a given channel
         * @param channel The ADC channel number
         * @return A string representing the device path
         */
        std::string getDevicePath(uint8_t channel) const;

        /**
         * @brief Read raw ADC value from a specific channel
         * @param channel The ADC channel number
         * @return The raw ADC value
         */
        uint16_t readRaw(uint8_t channel) const;

        /**
         * @brief Continuous read loop for ADC
         */
        void continuousReadLoop();
        
    public:
        /**
         * @brief Constructor
         */
        ADCLinux() = default;

        /**
         * @brief Destructor
         */
        ~ADCLinux() override;

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
    };
}

#endif //MEX_HAL_ADC_LINUX_H