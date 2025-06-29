#pragma once
#include <nlohmann/json.hpp>
#include <imgui.h>

class BaseSettings {
public:
    BaseSettings(const std::string& feature_name) : m_feature_name(feature_name) {}
    virtual ~BaseSettings() = default;

    virtual void render_imgui() = 0;
    virtual void to_json(nlohmann::json& j) const = 0;
    virtual void from_json(const nlohmann::json& j) = 0;

    const std::string& get_feature_name() const { return m_feature_name; }

protected:
    std::string m_feature_name;
};

namespace nlohmann {
    template<>
    struct adl_serializer<ImColor> {
        static void to_json(json& j, const ImColor& color) {
            j = json::array({ color.Value.x, color.Value.y, color.Value.z, color.Value.w });
        }

        static void from_json(const json& j, ImColor& color) {
            if (j.is_array() && j.size() == 4) {
                color.Value.x = j[0].get<float>();
                color.Value.y = j[1].get<float>();
                color.Value.z = j[2].get<float>();
                color.Value.w = j[3].get<float>();
            }
        }
    };
}