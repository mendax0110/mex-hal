#ifndef MEX_HAL_DEVICE_TREE_H
#define MEX_HAL_DEVICE_TREE_H

#include <cstdint>
#include <string>
#include <vector>
#include <optional>

/// @brief mex_hal Hardware Abstraction Layer - Device Tree Representation \namespace mex_hal
namespace mex_hal
{
    /// @brief Device Tree Property \struct DTProperty
    struct DTProperty
    {
        std::string name;
        std::vector<uint8_t> value;
    };

    /// @brief Device Tree Node \struct DTNode
    struct DTNode
    {
        std::string name;
        std::string path;
        std::vector<DTProperty> properties;
        std::vector<DTNode> children;
    };

    /// @brief Device Tree Interface \class DeviceTreeInterface
    class DeviceTreeInterface
    {
    public:
        /**
         * @brief Virtual descturto
         */
        virtual ~DeviceTreeInterface() = default;

        /**
         * @brief Load the device tree from a specified path
         * @param overlayPath The file path to the device tree overlay (e.g., .dtbo file)
         * @return A boolean indicating success or failure of loading the device tree
         */
        virtual bool loadOverlay(const std::string& overlayPath) = 0;

        /**
         * @brief Remove a previously loaded device tree overlay by name
         * @param overlayName The name of the device tree overlay to remove (e.g., "my_overlay")
         * @return A boolean indicating success or failure of removing the device tree overlay
         */
        virtual bool removeOverlay(const std::string& overlayName) = 0;

        /**
         * @brief List the currently loaded device tree overlays
         * @return A vector of strings containing the names of the currently loaded device tree overlays
         */
        [[nodiscard]] virtual std::vector<std::string> listOverlays() const = 0;

        /**
         * @brief Read a property from a device tree node
         * @param nodePath The path to the device tree node (e.g., "/soc/i2c@40000000")
         * @param propertyName The name of the property to read (e.g., "compatible")
         * @return An optional containing the DTProperty if found, or std::nullopt if not found
         */
        [[nodiscard]] virtual std::optional<DTProperty> readProperty(
            const std::string& nodePath,
            const std::string& propertyName) const = 0;

        /**
         * @brief Check if a device tree node exists at the specified path
         * @param nodePath A string representing the path to the device tree node (e.g., "/soc/i2c@40000000")
         * @return The boolean value indicating whether the node exists (true) or not (false)
         */
        [[nodiscard]] virtual bool nodeExists(const std::string& nodePath) const = 0;

        /**
         * @brief Get the compatible string of a device tree node
         * @param nodePath The path to the device tree node (e.g., "/soc/i2c@40000000")
         * @return A string containing the compatible value if found, or an empty string if not found
         */
        [[nodiscard]] virtual std::string getCompatible(const std::string& nodePath) const = 0;

        /**
         * @brief Get the status of a device tree node
         * @param nodePath The path to the device tree node (e.g., "/soc/i2c@40000000")
         * @return A string containing the status value if found, or an empty string if not found
         */
        [[nodiscard]] virtual std::string getNodeStatus(const std::string& nodePath) const = 0;

    protected:
        inline static const std::string DT_BASE_PATH = "/proc/device-tree";
        inline static const std::string DT_OVERLAY_PATH = "/sys/kernel/config/device-tree/overlays";
    };
}

#endif // MEX_HAL_DEVICE_TREE_H