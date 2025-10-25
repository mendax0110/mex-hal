#ifndef MEX_HAL_HAL_STATE_ENGINE_H
#define MEX_HAL_HAL_STATE_ENGINE_H

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "../include/hal/core.h"

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief HAL State enumeration \enum HALState
    enum class HALState
    {
        IDLE,
        RUNNING,
        STOPPED
    };

    /// @brief HAL State Engine class \class HALStateEngine
    class HALStateEngine : public std::enable_shared_from_this<HALStateEngine>
    {
    private:
        HALStateEngine() = default;

        std::thread worker_;
        std::mutex mutex_;
        std::condition_variable cv_;
        std::atomic<bool> running_{false};
        bool stopRequested_{false};

        /**
         * @brief Engine main loop
         */
        void engineLoop();

    public:

        /**
         * @brief Constructor
         */
        HALStateEngine(const HALStateEngine&) = delete;
        HALStateEngine& operator=(const HALStateEngine&) = delete;

        /**
         * @brief Destructor
         */
        ~HALStateEngine();

        /**
         * @brief Start the HAL state engine
         */
        HALStateEngine& start();

        /**
         * @brief Stop the HAL state engine
         */
        HALStateEngine& stop();

        /**
         * @brief Check if the engine is running
         * @return True if running, false otherwise
         */
        HALState getState() const;

        /**
         * @brief Wait for the engine to stop
         */
        void waitForStop();

        static HALStateEngine& getInstance();
    };
}

#endif //MEX_HAL_HAL_STATE_ENGINE_H