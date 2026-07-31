#include "logger.h"
#include <chrono>
#include <iomanip>
#include <sstream>

namespace revres {
namespace utils {

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

static std::string level_to_string(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:       return "DEBUG";
        case LogLevel::INFO:        return "INFO ";
        case LogLevel::WARN:        return "WARN ";
        case LogLevel::LEVEL_ERROR: return "ERROR";
        default:                    return "UNKNOWN";
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    // Get current time
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    
    // Lock the mutex so threads don't interleave their output
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::ostream& out = (level == LogLevel::ERROR) ? std::cerr : std::cout;
    out << "[" << ss.str() << "] [" << level_to_string(level) << "] " << message << "\n";
}

} // namespace utils
} // namespace revres
