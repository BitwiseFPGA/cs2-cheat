#include <logger/logger.hpp>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#endif

namespace logger {
    static bool colors_enabled = true;
    
#ifdef _WIN32
    static bool windows_colors_initialized = false;
    
    void initialize_windows_console() {
        if (windows_colors_initialized) return;
        
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole != INVALID_HANDLE_VALUE) {
            DWORD consoleMode;
            if (GetConsoleMode(hConsole, &consoleMode)) {
                consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(hConsole, consoleMode);
            }
        }
        
        hConsole = GetStdHandle(STD_ERROR_HANDLE);
        if (hConsole != INVALID_HANDLE_VALUE) {
            DWORD consoleMode;
            if (GetConsoleMode(hConsole, &consoleMode)) {
                consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(hConsole, consoleMode);
            }
        }
        
        windows_colors_initialized = true;
    }
#endif

    std::string get_timestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
#pragma warning(disable : 4996) 
        ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
#pragma warning(default: 4996)
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }
    
    std::string get_colored_timestamp() {
        if (!colors_enabled) return get_timestamp();
        return std::string(colors::BRIGHT_BLACK) + get_timestamp() + colors::RESET;
    }
    
    void enable_colors() {
#ifdef _WIN32
        initialize_windows_console();
#endif
        colors_enabled = true;
    }
    
    void disable_colors() {
        colors_enabled = false;
    }
    
    bool are_colors_enabled() {
        return colors_enabled;
    }

    const char* get_log_level_name(LogLevel level) {
        switch (level) {
            case LogLevel::LOG_DEBUG: return "DEBUG";
            case LogLevel::LOG_INFO: return "INFO";
            case LogLevel::LOG_WARN: return "WARN";
            case LogLevel::LOG_ERROR: return "ERROR";
            case LogLevel::LOG_CRITICAL: return "CRITICAL";
            case LogLevel::LOG_OFF: return "OFF";
            default: return "UNKNOWN";
        }
    }

    LogLevel get_current_log_level() {
        return static_cast<LogLevel>(LOG_LEVEL);
    }

    void debug(const std::string& message) {
        if constexpr (!should_log(LogLevel::LOG_DEBUG)) return;
        
#ifdef _WIN32
        if (colors_enabled && !windows_colors_initialized) {
            initialize_windows_console();
        }
#endif
        if (colors_enabled) {
            std::cout << "[" << get_colored_timestamp() << "] " 
                      << colors::BRIGHT_MAGENTA << "[DEBUG]" << colors::RESET 
                      << " " << message << std::endl;
        } else {
            std::cout << "[" << get_timestamp() << "] [DEBUG] " << message << std::endl;
        }
    }

    void info(const std::string& message) {
        if constexpr (!should_log(LogLevel::LOG_INFO)) return;
        
#ifdef _WIN32
        if (colors_enabled && !windows_colors_initialized) {
            initialize_windows_console();
        }
#endif
        if (colors_enabled) {
            std::cout << "[" << get_colored_timestamp() << "] " 
                      << colors::BRIGHT_BLUE << "[INFO]" << colors::RESET 
                      << " " << message << std::endl;
        } else {
            std::cout << "[" << get_timestamp() << "] [INFO] " << message << std::endl;
        }
    }
    
    void warn(const std::string& message) {
        if constexpr (!should_log(LogLevel::LOG_WARN)) return;
        
#ifdef _WIN32
        if (colors_enabled && !windows_colors_initialized) {
            initialize_windows_console();
        }
#endif
        if (colors_enabled) {
            std::cout << "[" << get_colored_timestamp() << "] " 
                      << colors::BRIGHT_YELLOW << "[WARN]" << colors::RESET 
                      << " " << message << std::endl;
        } else {
            std::cout << "[" << get_timestamp() << "] [WARN] " << message << std::endl;
        }
    }
    
    void error(const std::string& message) {
        if constexpr (!should_log(LogLevel::LOG_ERROR)) return;
        
#ifdef _WIN32
        if (colors_enabled && !windows_colors_initialized) {
            initialize_windows_console();
        }
#endif
        if (colors_enabled) {
            std::cerr << "[" << get_colored_timestamp() << "] " 
                      << colors::BRIGHT_RED << "[ERROR]" << colors::RESET 
                      << " " << message << std::endl;
        } else {
            std::cerr << "[" << get_timestamp() << "] [ERROR] " << message << std::endl;
        }
    }
    
    void critical(const std::string& message) {
        if constexpr (!should_log(LogLevel::LOG_CRITICAL)) return;
        
#ifdef _WIN32
        if (colors_enabled && !windows_colors_initialized) {
            initialize_windows_console();
        }
#endif
        if (colors_enabled) {
            std::cerr << "[" << get_colored_timestamp() << "] " 
                      << colors::BOLD << colors::BRIGHT_RED << "[CRITICAL]" << colors::RESET 
                      << " " << message << std::endl;
        } else {
            std::cerr << "[" << get_timestamp() << "] [CRITICAL] " << message << std::endl;
        }
    }
    
    void success(const std::string& message) {
        if constexpr (!should_log(LogLevel::LOG_INFO)) return;
        
#ifdef _WIN32
        if (colors_enabled && !windows_colors_initialized) {
            initialize_windows_console();
        }
#endif
        if (colors_enabled) {
            std::cout << "[" << get_colored_timestamp() << "] " 
                      << colors::BRIGHT_GREEN << "[SUCCESS]" << colors::RESET 
                      << " " << message << std::endl;
        } else {
            std::cout << "[" << get_timestamp() << "] [SUCCESS] " << message << std::endl;
        }
    }

    void log_step(const std::string& step_name, const std::string& details) {
        std::string message = "Step: " + step_name;
        if (!details.empty()) {
            message += " - " + details;
        }
        info(message);
    }
    
    void log_failure(const std::string& component_name, const std::string& reason) {
        std::string message = "Failed to initialize " + component_name;
        if (!reason.empty()) {
            message += ": " + reason;
        }
        error(message);
    }
}