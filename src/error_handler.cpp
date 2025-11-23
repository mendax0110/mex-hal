#include "../include/hal/error_handler.h"
#include "../include/hal/logger.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <sstream>

using namespace mex_hal;

ErrorHandler& ErrorHandler::getInstance()
{
    static ErrorHandler instance;
    return instance;
}

void ErrorHandler::logError(ErrorSeverity severity, ErrorCategory category,
                            const std::string& component, const std::string& message,
                            const std::string& suggestion, int errorCode)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    ErrorRecord record;
    record.timestamp = std::chrono::system_clock::now();
    record.severity = severity;
    record.category = category;
    record.component = component;
    record.message = message;
    record.suggestion = suggestion;
    record.errorCode = errorCode;
    
    errors_.push_back(record);
    
    for (const auto& callback : callbacks_)
    {
        callback(record);
    }
    
    std::ostringstream oss;
    oss << "[" << categoryToString(category) << "] " << component << ": " << message;
    if (!suggestion.empty())
    {
        oss << " -> Suggestion: " << suggestion;
    }
    
    auto& logger = Logger::getInstance();
    switch (severity)
    {
        case ErrorSeverity::INFO:
            logger.info(oss.str());
            break;
        case ErrorSeverity::WARNING:
            logger.warn(oss.str());
            break;
        case ErrorSeverity::ERROR:
            logger.error(oss.str());
            break;
        case ErrorSeverity::CRITICAL:
        case ErrorSeverity::FATAL:
            logger.fatal(oss.str());
            break;
    }
}

void ErrorHandler::logInfo(const std::string& component, const std::string& message)
{
    logError(ErrorSeverity::INFO, ErrorCategory::RUNTIME, component, message);
}

void ErrorHandler::logWarning(const std::string& component, const std::string& message,
                              const std::string& suggestion)
{
    logError(ErrorSeverity::WARNING, ErrorCategory::RUNTIME, component, message, suggestion);
}

void ErrorHandler::logError(const std::string& component, const std::string& message,
                            const std::string& suggestion)
{
    logError(ErrorSeverity::ERROR, ErrorCategory::RUNTIME, component, message, suggestion);
}

void ErrorHandler::logCritical(const std::string& component, const std::string& message,
                               const std::string& suggestion)
{
    logError(ErrorSeverity::CRITICAL, ErrorCategory::RUNTIME, component, message, suggestion);
}

void ErrorHandler::registerCallback(ErrorCallback callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_.push_back(callback);
}

void ErrorHandler::clearCallbacks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_.clear();
}

std::vector<ErrorRecord> ErrorHandler::getErrors(ErrorSeverity minSeverity) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ErrorRecord> filtered;
    
    for (const auto& error : errors_)
    {
        if (error.severity >= minSeverity)
        {
            filtered.push_back(error);
        }
    }
    
    return filtered;
}

void ErrorHandler::printReport() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::cout << "\n========================================\n";
    std::cout << "  Error Report\n";
    std::cout << "========================================\n";
    std::cout << "Total errors logged: " << errors_.size() << "\n";
    
    size_t info = 0, warning = 0, error = 0, critical = 0, fatal = 0;
    for (const auto& rec : errors_)
    {
        switch (rec.severity)
        {
            case ErrorSeverity::INFO: info++; break;
            case ErrorSeverity::WARNING: warning++; break;
            case ErrorSeverity::ERROR: error++; break;
            case ErrorSeverity::CRITICAL: critical++; break;
            case ErrorSeverity::FATAL: fatal++; break;
        }
    }
    
    std::cout << "  INFO: " << info << ", WARNING: " << warning 
              << ", ERROR: " << error << ", CRITICAL: " << critical 
              << ", FATAL: " << fatal << "\n";
    
    if (!errors_.empty())
    {
        std::cout << "\n--- Recent Errors ---\n";
        size_t count = 0;
        for (auto it = errors_.rbegin(); it != errors_.rend() && count < 10; ++it, ++count)
        {
            auto time = std::chrono::system_clock::to_time_t(it->timestamp);
            std::cout << "  [" << severityToString(it->severity) << "] "
                      << it->component << ": " << it->message << "\n";
            if (!it->suggestion.empty())
            {
                std::cout << "    -> " << it->suggestion << "\n";
            }
        }
    }
    
    std::cout << "========================================\n";
}

void ErrorHandler::exportToFile(const std::string& filename) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::ofstream file(filename);
    
    if (!file.is_open())
        return;
    
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    file << "MEX-HAL Error Report\n";
    file << "Generated: " << std::ctime(&time);
    file << "Total Errors: " << errors_.size() << "\n\n";
    
    for (const auto& error : errors_)
    {
        auto time = std::chrono::system_clock::to_time_t(error.timestamp);
        file << "[" << std::ctime(&time) << "] ";
        file << "[" << severityToString(error.severity) << "] ";
        file << "[" << categoryToString(error.category) << "] ";
        file << error.component << ": " << error.message << "\n";
        if (!error.suggestion.empty())
        {
            file << "  Suggestion: " << error.suggestion << "\n";
        }
        file << "\n";
    }
    
    file.close();
}

void ErrorHandler::clear()
{
    std::lock_guard<std::mutex> lock(mutex_);
    errors_.clear();
}

size_t ErrorHandler::getErrorCount(ErrorSeverity severity) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    size_t count = 0;
    for (const auto& error : errors_)
    {
        if (error.severity == severity)
            count++;
    }
    return count;
}

bool ErrorHandler::hasErrors() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& error : errors_)
    {
        if (error.severity >= ErrorSeverity::ERROR)
            return true;
    }
    return false;
}

bool ErrorHandler::hasCriticalErrors() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& error : errors_)
    {
        if (error.severity >= ErrorSeverity::CRITICAL)
            return true;
    }
    return false;
}

const char* ErrorHandler::severityToString(ErrorSeverity severity)
{
    switch (severity)
    {
        case ErrorSeverity::INFO: return "INFO";
        case ErrorSeverity::WARNING: return "WARN";
        case ErrorSeverity::ERROR: return "ERROR";
        case ErrorSeverity::CRITICAL: return "CRITICAL";
        case ErrorSeverity::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

const char* ErrorHandler::categoryToString(ErrorCategory category)
{
    switch (category)
    {
        case ErrorCategory::SYSTEM: return "System";
        case ErrorCategory::HARDWARE: return "Hardware";
        case ErrorCategory::MEMORY: return "Memory";
        case ErrorCategory::PERMISSION: return "Permission";
        case ErrorCategory::CONFIGURATION: return "Config";
        case ErrorCategory::RUNTIME: return "Runtime";
        default: return "Unknown";
    }
}
