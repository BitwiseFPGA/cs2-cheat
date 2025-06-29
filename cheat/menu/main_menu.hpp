#pragma once
#include <string>
#include <memory>
#include <vector>

class Renderer;
class BaseFeature;
class EspFeature;
class AimbotFeature;
class SettingsManager;

class MainMenu {
public:
    MainMenu();
    ~MainMenu();
    
    bool initialize(Renderer* renderer, SettingsManager* settings_manager);
    void shutdown();
    
    void render();
    
    void add_feature(std::shared_ptr<BaseFeature> feature);
    void render_feature_settings();
        
private:
    bool m_initialized;
    Renderer* m_renderer;
    SettingsManager* m_settings_manager;
    
    std::vector<std::shared_ptr<BaseFeature>> m_features;
    
    void setup_style();
}; 