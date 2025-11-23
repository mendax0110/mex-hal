#ifndef MEX_HAL_LOGGER_H
#define MEX_HAL_LOGGER_H

#include <string>
#include <mutex>
#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief Log levels \enum LogLevel
    enum class LogLevel
    {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        OFF
    };

    /// @brief Centralized logging class \class Logger
    class Logger
    {
    public:
        /**
         * @brief Delete copy constructor
         * 
         */
        Logger(const Logger&) = delete;
        
        /**
         * @brief Delete assignment operator
         * 
         */
        Logger& operator=(const Logger&) = delete;

        /**
         * @brief Destroy the Logger object
         * 
         */
        ~Logger();

        /**
         * @brief Get the Instance object
         * 
         * @return Logger& 
         */
        static Logger& getInstance();

        /**
         * @brief Set the current log level
         * 
         * @param level LogLevel to set
         */
        void setLogLevel(LogLevel level);
        
        /**
         * @brief Get the current log level
         * 
         * @return LogLevel 
         */
        [[nodiscard]] LogLevel getLogLevel() const;

        /**
         * @brief Enable or disable console logging
         * 
         * @param enable True to enable, false to disable
         */
        void enableConsoleLogging(bool enable);
        
        /**
         * @brief Enable or disable file logging
         * 
         * @param enable True to enable, false to disable
         * @param filepath Optional file path for the log file
         */
        void enableFileLogging(bool enable, const std::string& filepath = "mex-hal.log");

        /**
         * @brief Log a message at the specified log level
         * 
         * @param level LogLevel of the message
         * @param message Message to log
         */
        void log(LogLevel level, const std::string& message);
        
        /**
         * @brief Log a TRACE level message
         * 
         * @param message Message to log
         */
        void trace(const std::string& message);
        
        /**
         * @brief Log a DEBUG level message
         * 
         * @param message Message to log
         */
        void debug(const std::string& message);
        
        /**
         * @brief Log an INFO level message
         * 
         * @param message Message to log
         */
        void info(const std::string& message);
        
        /**
         * @brief Log a WARN level message
         * 
         * @param message Message to log
         */
        void warn(const std::string& message);
        
        /**
         * @brief Log an ERROR level message
         * 
         * @param message Message to log
         */
        void error(const std::string& message);
        
        /**
         * @brief Log a FATAL level message
         * 
         * @param message Message to log
         */
        void fatal(const std::string& message);
        
    private:
        LogLevel currentLevel_;
        bool logToConsole_;
        bool logToFile_;
        std::string logFilePath_;
        std::ofstream logFile_;
        std::mutex logMutex_;

        /**
         * @brief Construct a new Logger object
         * 
         */
        Logger();

        /**
         * @brief Get the Current Timestamp object
         * 
         * @return std::string 
         */
        std::string getCurrentTimestamp() const;
        
        /**
         * @brief Convert LogLevel to string
         * 
         * @param level LogLevel to convert
         * @return std::string 
         */
        std::string logLevelToString(LogLevel level) const;
        
        /**
         * @brief Get the Log Color object
         * 
         * @param level LogLevel
         * @return std::string 
         */
        std::string getLogColor(LogLevel level) const;
        
        /**
         * @brief Write the log message to the appropriate outputs
         * 
         * @param level LogLevel of the message
         * @param message Message to log
         */
        void writeLog(LogLevel level, const std::string& message);
    };

/**
 * @brief Logger macros for different log levels
 * 
 */
#define LOG_TRACE(msg) mex_hal::Logger::getInstance().trace(msg)
#define LOG_DEBUG(msg) mex_hal::Logger::getInstance().debug(msg)
#define LOG_INFO(msg) mex_hal::Logger::getInstance().info(msg)
#define LOG_WARN(msg) mex_hal::Logger::getInstance().warn(msg)
#define LOG_ERROR(msg) mex_hal::Logger::getInstance().error(msg)
#define LOG_FATAL(msg) mex_hal::Logger::getInstance().fatal(msg)
}

#endif
