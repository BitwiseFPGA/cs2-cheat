#include <iostream>
#include <memory>
#include <csignal>
#include <thread>

#include <logger/logger.hpp>
#include <config/build/settings.hpp>
#include <engine/engine.hpp>
#include <config/runtime/settings.hpp>

std::unique_ptr<Engine> g_engine = nullptr;

void print_banner() {
    const int banner_width = 40;
    const std::string separator(banner_width, '=');
    
    logger::info(separator);
    
    std::string name = CHEAT_NAME;
    size_t padding = (banner_width - name.length()) / 2;
    std::string banner = std::string(padding, ' ') + name;
    
    logger::info(banner);
    logger::info(separator);
    logger::info("Version: " + std::string(VERSION));
    logger::info("Build: " + std::string(__DATE__) + " " + std::string(__TIME__));
    logger::info(separator);
}

bool initialize_engine() {    
    try {
        print_banner();
                
        g_engine = std::make_unique<Engine>();
        if (!g_engine) {
            logger::critical("Failed to create engine instance");
            return false;
        }
        if (!g_engine->initialize()) {
            logger::critical("Engine initialization failed");
            return false;
        }
        return true;
        
    } catch (const std::exception& e) {
        logger::critical("Engine initialization failed with exception: " + std::string(e.what()));
        return false;
    } catch (...) {
        logger::critical("Engine initialization failed with unknown exception");
        return false;
    }
}

void run_main_loop() {    
    try {
        g_engine->run();
    } catch (const std::exception& e) {
        logger::error("Main loop exception: " + std::string(e.what()));
    } catch (...) {
        logger::error("Main loop failed with unknown exception");
    }
    
    logger::info("Main loop ended");
}

void shutdown_engine() {    
    try {
        if (g_engine) {
            g_engine->shutdown();
            g_engine.reset();
        }        
    } catch (const std::exception& e) {
        logger::error("Shutdown exception: " + std::string(e.what()));
    } catch (...) {
        logger::error("Shutdown failed with unknown exception");
    }
}

int main() {    
    int exit_code = 0;
    
    try {
        if (!initialize_engine()) {
            logger::critical("Engine initialization failed");
            exit_code = 1;
            goto cleanup;
        }
        
        run_main_loop();
    } catch (const std::exception& e) {
        logger::critical("Unhandled exception in main: " + std::string(e.what()));
        exit_code = 1;
    } catch (...) {
        logger::critical("Unhandled unknown exception in main");
        exit_code = 1;
    }
    
cleanup:
    shutdown_engine();
    
    logger::info("Engine exit with code: " + std::to_string(exit_code));
    
    return exit_code;
}