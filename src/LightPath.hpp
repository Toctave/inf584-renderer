#pragma once

#include <vector>

#include "Ray.hpp"
#include "Color.hpp"
#include "Intersect.hpp"

enum class SurfaceType { EYE, DIFFUSE, SPECULAR, LIGHT };

class LightPath {
private:
    SurfaceType type_;
    Vec3 point_;
    RGBColor emitted_;
    std::vector<const LightPath*> tributaries_;

    void set_effluent(LightPath* effluent);

public:
    LightPath(const Vec3& eye); // eye
    LightPath(LightPath* effluent, SurfaceType type, const Vec3& point); // diffuse/specular
    LightPath(LightPath* effluent, const RGBColor& emitted, const Vec3& point); // light
    
    ~LightPath();

    void add_tributary(const LightPath* tributary);
    RGBColor outgoing_radiance() const;
};
