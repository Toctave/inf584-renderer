#pragma once

#include "Scene.hpp"
#include "Camera.hpp"

#include <string>
#include <unordered_map>

#include <toml.hpp>

class TOMLParser {
    template<typename T>
    T decode(const toml::value& toml);
    template<typename T>
    T decode(const toml::value& base, const std::string& name);

    float aspect_ratio_;

    std::unordered_map<std::string, Primitive*> primitive_map_;
    std::unordered_map<std::string, Shape*> shape_map_;
    std::unordered_map<std::string, Material*> material_map_;
    std::unordered_map<std::string, Light*> light_map_;

    std::string base_path_;
    Scene scene_;
    Camera camera_;
    
public:
    const Camera& camera() const;
    const Scene& scene() const;
    
    TOMLParser(const std::string& path, float ar);
    ~TOMLParser();
};
