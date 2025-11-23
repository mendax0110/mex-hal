#include "../include/hal/logger.h"
#include <ctime>
#include <iomanip>

using namespace mex_hal;

Logger::Logger()
    : currentLevel_(LogLevel::INFO)
    , logToConsole_(true)
    , logToFile_(false)
    , logFilePath_("mex-hal.log")
{
}

Logger::~Logger()
{
    if (logFile_.is_open())
    {
        logFile_.close();
    }
}

Logger& Logger::getInstance()
{
    static Logger instance;
    return instance;
}

void Logger::setLogLevel(LogLevel level)
{
    std::lock_guard<std::mutex> lock(logMutex_);
    currentLevel_ = level;
}

LogLevel Logger::getLogLevel() const
{
    return currentLevel_;
}

void Logger::enableConsoleLogging(bool enable)
{
    std::lock_guard<std::mutex> lock(logMutex_);
    logToConsole_ = enable;
}

void Logger::enableFileLogging(bool enable, const std::string& filepath)
{
    std::lock_guard<std::mutex> lock(logMutex_);
    
    if (logFile_.is_open())
    {
        logFile_.close();
    }
    
    logFilePath_ = filepath;
    logToFile_ = enable;
    
    if (logToFile_)
    {
        logFile_.open(logFilePath_, std::ios::app);
        if (!logFile_.is_open())
        {
            std::cerr << "[LOGGER] Failed to open log file: " << logFilePath_ << std::endl;
            logToFile_ = false;
        }
    }
}

std::string Logger::getCurrentTimestamp() const
{
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    std::tm tm_buf{};
#ifdef _WIN32
    localtime_s(&tm_buf, &now_time_t);
#else
    localtime_r(&now_time_t, &tm_buf);
#endif
    
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << now_ms.count();
    return oss.str();
}

std::string Logger::logLevelToString(LogLevel level) const
{
    switch (level)
    {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        case LogLevel::OFF:   return "OFF";
        default:              return "UNKNOWN";
    }
}

std::string Logger::getLogColor(LogLevel level) const
{
    switch (level)
    {
        case LogLevel::TRACE: return "\033[37m";
        case LogLevel::DEBUG: return "\033[36m";
        case LogLevel::INFO:  return "\033[32m";
        case LogLevel::WARN:  return "\033[33m";
        case LogLevel::ERROR: return "\033[31m";
        case LogLevel::FATAL: return "\033[35m";
        default:              return "\033[0m";
    }
}

void Logger::writeLog(LogLevel level, const std::string& message)
{
    if (level < currentLevel_ || currentLevel_ == LogLevel::OFF)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(logMutex_);
    
    std::string timestamp = getCurrentTimestamp();
    std::string levelStr = logLevelToString(level);
    std::string logMessage = "[" + timestamp + "] [" + levelStr + "] " + message;
    
    if (logToConsole_)
    {
        std::string colorCode = getLogColor(level);
        std::cout << colorCode << logMessage << "\033[0m" << std::endl;
    }
    
    if (logToFile_ && logFile_.is_open())
    {
        logFile_ << logMessage << std::endl;
        logFile_.flush();
    }
}

void Logger::log(LogLevel level, const std::string& message)
{
    writeLog(level, message);
}

void Logger::trace(const std::string& message)
{
    writeLog(LogLevel::TRACE, message);
}

void Logger::debug(const std::string& message)
{
    writeLog(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message)
{
    writeLog(LogLevel::INFO, message);
}

void Logger::warn(const std::string& message)
{
    writeLog(LogLevel::WARN, message);
}

void Logger::error(const std::string& message)
{
    writeLog(LogLevel::ERROR, message);
}

void Logger::fatal(const std::string& message)
{
    writeLog(LogLevel::FATAL, message);
}
