#ifndef MEX_HAL_ERROR_HANDLER_H
#define MEX_HAL_ERROR_HANDLER_H

#include <string>
#include <vector>
#include <mutex>
#include <chrono>
#include <functional>

/// @brief mex_hal Hardware Abstraction Layer \namespace mex_hal
namespace mex_hal
{
    /// @brief Error severity levels \enum ErrorSeverity
    enum class ErrorSeverity
    {
        INFO,
        WARNING,
        ERROR,
        CRITICAL,
        FATAL
    };

    /// @brief Error categories \enum ErrorCategory
    enum class ErrorCategory
    {
        SYSTEM,
        HARDWARE,
        MEMORY,
        PERMISSION,
        CONFIGURATION,
        RUNTIME,
        UNKNOWN
    };

    /// @brief Error record structure \struct ErrorRecord
    struct ErrorRecord
    {
        std::chrono::system_clock::time_point timestamp;
        ErrorSeverity severity;
        ErrorCategory category;
        std::string component;
        std::string message;
        std::string suggestion;
        int errorCode;
    };

    using ErrorCallback = std::function<void(const ErrorRecord&)>;

    /// @brief Centralized error handling class \class ErrorHandler
    class ErrorHandler
    {
    public:
        /**
         * @brief Get the Instance object
         * 
         * @return ErrorHandler& 
         */
        static ErrorHandler& getInstance();

        /**
         * @brief Log an error with detailed information
         * 
         * @param severity Severity level of the error
         * @param category Category of the error
         * @param component Component where the error occurred
         * @param message Descriptive error message
         * @param suggestion Optional suggestion for resolution
         * @param errorCode Optional error code
         */
        void logError(ErrorSeverity severity, ErrorCategory category,
                     const std::string& component, const std::string& message,
                     const std::string& suggestion = "", int errorCode = 0);

        /**
         * @brief Log an informational message
         * 
         * @param component Component where the message originated
         * @param message Informational message to log
         */
        void logInfo(const std::string& component, const std::string& message);

        /**
         * @brief Log a warning message
         * 
         * @param component Component where the message originated
         * @param message Warning message to log
         * @param suggestion Optional suggestion for resolution
         */
        void logWarning(const std::string& component, const std::string& message,
                       const std::string& suggestion = "");

        /**
         * @brief Log an error message
         * 
         * @param component Component where the message originated
         * @param message Error message to log
         * @param suggestion Optional suggestion for resolution
         */
        void logError(const std::string& component, const std::string& message,
                     const std::string& suggestion = "");

        /**
         * @brief Log a critical error message
         * 
         * @param component Component where the message originated
         * @param message Critical error message to log
         * @param suggestion Optional suggestion for resolution
         */
        void logCritical(const std::string& component, const std::string& message,
                        const std::string& suggestion = "");

        /**
         * @brief Register a callback function to be called on error events
         * 
         * @param callback The callback function to register
         */
        void registerCallback(ErrorCallback callback);

        /**
         * @brief Clear all registered error callbacks
         */
        void clearCallbacks();

        /**
         * @brief Get the Errors object
         * 
         * @param minSeverity Minimum severity level to include
         * @return std::vector<ErrorRecord>
         */
        std::vector<ErrorRecord> getErrors(ErrorSeverity minSeverity = ErrorSeverity::INFO) const;

        /**
         * @brief Print a report of all logged errors to the console
         */
        void printReport() const;

        /**
         * @brief Export all logged errors to a file
         * 
         * @param filename Name of the file to export to
         */
        void exportToFile(const std::string& filename) const;
        
        /**
         * @brief Clear all logged errors
         */
        void clear();

        /**
         * @brief Get the count of errors by severity
         * 
         * @param severity Severity level to count
         * @return size_t 
         */
        size_t getErrorCount(ErrorSeverity severity) const;
        
        /**
         * @brief Check if any errors have been logged
         * 
         * @return true if there are logged errors
         * @return false if no errors are logged
         */
        bool hasErrors() const;

        /**
         * @brief Check if any critical or higher severity errors have been logged
         * 
         * @return true if there are critical or higher severity errors
         * @return false if no critical or higher severity errors are logged
         */
        bool hasCriticalErrors() const;

        /**
         * @brief Construct a new Error Handler object
         * 
         */
        ErrorHandler(const ErrorHandler&) = delete;

        /**
         * @brief Assignment operator ctor
         * 
         * @return ErrorHandler& 
         */
        ErrorHandler& operator=(const ErrorHandler&) = delete;

    private:
        /**
         * @brief Construct a new Error Handler object
         * 
         */
        ErrorHandler() = default;

        /**
         * @brief Destroy the Error Handler object
         * 
         */
        ~ErrorHandler() = default;

        mutable std::mutex mutex_;
        std::vector<ErrorRecord> errors_;
        std::vector<ErrorCallback> callbacks_;

        /**
         * @brief Convert severity enum to string
         * 
         * @param severity ErrorSeverity enum value
         * @return const char* Corresponding string representation
         */
        static const char* severityToString(ErrorSeverity severity);

        /**
         * @brief Convert category enum to string
         * 
         * @param category ErrorCategory enum value
         * @return const char* Corresponding string representation
         */
        static const char* categoryToString(ErrorCategory category);
    };
}

#endif
