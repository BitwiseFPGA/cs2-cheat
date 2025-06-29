#pragma once
#include <string>
#include <memory>
#include <vector>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <imgui.h>
#include <config/build/settings.hpp>
#include <features/settings/base_settings.hpp>
#include <logger/logger.hpp>

#include <Windows.h>
#include <ShlObj.h>

namespace settings {
    inline bool g_show_menu = false;
    
    inline std::string get_default_config_directory() {
        PWSTR appdata_path = nullptr;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &appdata_path))) {
            std::wstring wide_path(appdata_path);
            CoTaskMemFree(appdata_path);
            
            int size_needed = WideCharToMultiByte(CP_UTF8, 0, wide_path.c_str(), -1, nullptr, 0, nullptr, nullptr);
            std::string appdata_str(size_needed - 1, 0);
            WideCharToMultiByte(CP_UTF8, 0, wide_path.c_str(), -1, &appdata_str[0], size_needed, nullptr, nullptr);
            
            std::string config_dir = appdata_str + "\\" + CHEAT_NAME;
            
            try {
                std::filesystem::create_directories(config_dir);
            } catch (const std::exception&) {
                return "./config";
            }
            
            return config_dir;
        }

        std::string fallback_dir = "./config";
        try {
            std::filesystem::create_directories(fallback_dir);
        } catch (const std::exception&) {
            return ".";
        }
        return fallback_dir;
    }
    
    inline std::string get_default_config_file() {
        return get_default_config_directory() + "\\" + CONFIG_FILE_NAME;
    }
}

class SettingsManager {
public:
    SettingsManager() = default;
    ~SettingsManager() = default;
    
    void register_settings(BaseSettings* settings) {
        if (settings) {
            m_settings.push_back(settings);
        }
    }
    
    bool save() {
        return save_to_file(settings::get_default_config_file());
    }
    
    bool load() {
        return load_from_file(settings::get_default_config_file());
    }
   
private: 
    bool save_to_file(const std::string& filename) {
        logger::info("Saving config to " + filename);

        try {
            nlohmann::json config;
            
            config["global"]["show_menu"] = settings::g_show_menu;
            
            for (auto* settings : m_settings) {
                nlohmann::json feature_json;
                settings->to_json(feature_json);
                config["features"][settings->get_feature_name()] = feature_json;
            }
            
            std::ofstream file(filename);
            if (!file.is_open()) {
                return false;
            }
            
            file << config.dump(4);
            file.close();
            return true;
            
        } catch (const std::exception&) {
            logger::error("Failed to save config to " + filename);
            return false;
        }
    }
    
    bool load_from_file(const std::string& filename) {
        logger::info("Loading config from " + filename);

        try {
            std::ifstream file(filename);
            if (!file.is_open()) {
                return false;
            }
            
            nlohmann::json config;
            config = nlohmann::json::parse(file);
            file.close();
            
            if (config.contains("global") && config["global"].contains("show_menu")) {
                settings::g_show_menu = config["global"]["show_menu"];
            }
            
            if (config.contains("features")) {
                for (auto* settings : m_settings) {
                    const std::string& feature_name = settings->get_feature_name();
                    if (config["features"].contains(feature_name)) {
                        settings->from_json(config["features"][feature_name]);
                    }
                }
            }
            
            return true;
            
        } catch (const std::exception&) {
            logger::error("Failed to load config from " + filename);
            return false;
        }
    }
    
    std::vector<BaseSettings*> m_settings;
};
