#pragma once
#include <iostream>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <config/build/settings.hpp>

namespace logger {
    enum class LogLevel : int {
        LOG_DEBUG = 0,
        LOG_INFO = 1,
        LOG_WARN = 2,
        LOG_ERROR = 3,
        LOG_CRITICAL = 4,
        LOG_OFF = 5
    };

    namespace colors {
        constexpr const char* RESET = "\033[0m";
        constexpr const char* BOLD = "\033[1m";
        
        constexpr const char* BLACK = "\033[30m";
        constexpr const char* RED = "\033[31m";
        constexpr const char* GREEN = "\033[32m";
        constexpr const char* YELLOW = "\033[33m";
        constexpr const char* BLUE = "\033[34m";
        constexpr const char* MAGENTA = "\033[35m";
        constexpr const char* CYAN = "\033[36m";
        constexpr const char* WHITE = "\033[37m";
        
        constexpr const char* BRIGHT_BLACK = "\033[90m";
        constexpr const char* BRIGHT_RED = "\033[91m";
        constexpr const char* BRIGHT_GREEN = "\033[92m";
        constexpr const char* BRIGHT_YELLOW = "\033[93m";
        constexpr const char* BRIGHT_BLUE = "\033[94m";
        constexpr const char* BRIGHT_MAGENTA = "\033[95m";
        constexpr const char* BRIGHT_CYAN = "\033[96m";
        constexpr const char* BRIGHT_WHITE = "\033[97m";
    }

    constexpr bool should_log(LogLevel level) {
        return static_cast<int>(level) >= LOG_LEVEL;
    }
    
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void debug(const std::string& message);
    void critical(const std::string& message);
    void success(const std::string& message);

    std::string get_timestamp();
    void log_step(const std::string& step_name, const std::string& details = "");
    void log_failure(const std::string& component_name, const std::string& reason = "");
    
    void enable_colors();
    void disable_colors();
    bool are_colors_enabled();
    
    const char* get_log_level_name(LogLevel level);
    LogLevel get_current_log_level();


    template<typename T>
    void log_value(const std::string& name, const T& value, bool hex = false)
    {
        std::ostringstream oss;
        oss << name << ": ";

        if constexpr (std::is_integral_v<T>) {
            bool use_hex = hex ||
                std::is_same_v<T, uintptr_t> ||
                std::is_same_v<T, void*> ||
                std::is_pointer_v<T> ||
                (sizeof(T) >= sizeof(void*) && value > 0xFFFF);

            if (use_hex) {
                oss << "0x" << std::hex << std::uppercase << value;
            }
            else {
                oss << value;
            }
        }
        else if constexpr (std::is_floating_point_v<T>) {
            oss << std::fixed << std::setprecision(2) << value;
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            oss << value;
        }
        else if constexpr (std::is_convertible_v<T, std::string>) {
            oss << static_cast<std::string>(value);
        }
        else {
            oss << "Unsupported type";
        }

        info(oss.str());
    }
}