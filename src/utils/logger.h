#pragma once

#include <string>
#include <mutex>
#include <iostream>

namespace revres {
namespace utils {

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    LEVEL_ERROR  // Named LEVEL_ERROR to avoid collision with Windows #define ERROR 0
};

class Logger {
public:
    static Logger& instance();

    void log(LogLevel level, const std::string& message);

    // Helpers
    void debug(const std::string& message) { log(LogLevel::DEBUG, message); }
    void info(const std::string& message) { log(LogLevel::INFO, message); }
    void warn(const std::string& message) { log(LogLevel::WARN, message); }
    void error(const std::string& message) { log(LogLevel::LEVEL_ERROR, message); }

private:
    Logger() = default;
    
    // We need a mutex because multiple threads will be writing to stdout simultaneously
    std::mutex mutex_;
};

// Global helper macros
#define LOG_DEBUG(msg) revres::utils::Logger::instance().debug(msg)
#define LOG_INFO(msg)  revres::utils::Logger::instance().info(msg)
#define LOG_WARN(msg)  revres::utils::Logger::instance().warn(msg)
// Note: macro named LOG_ERR (not LOG_ERROR) to avoid any further Windows macro clashes
#define LOG_ERR(msg)   revres::utils::Logger::instance().error(msg)

} // namespace utils
} // namespace revres
