#pragma once

#define VERSION "1.0.0" // Version of the cheat
#define CHEAT_NAME "cs2-cheat" // Name of the cheat (used for logging and config file)
#define TARGET_PROCESS_NAME "cs2.exe" // Process to read and write memory from
#define CONFIG_FILE_NAME "config.json" // Settings file name

#define USE_DMA_MEMORY 0 // Otherwise, use the Windows API to read and write memory

// LOG_LEVEL defines the logging level for the cheat
// 0 = DEBUG (all messages)
// 1 = INFO 
// 2 = WARN
// 3 = ERROR
// 4 = CRITICAL
// 5 = OFF (no logging)
#ifndef LOG_LEVEL
    #ifdef _DEBUG
        #define LOG_LEVEL 0
    #else
        #define LOG_LEVEL 1
    #endif
#endif
