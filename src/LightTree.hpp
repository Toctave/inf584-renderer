#pragma once

#include <vector>

#include "Ray.hpp"
#include "Color.hpp"
#include "Intersect.hpp"
#include "Material.hpp"
#include "LightPathExpression.hpp"

class LightTree {
private:
    std::vector<const LightTree*> upstream_;
    std::vector<RGBColor> attenuations_;

    SurfaceType type_;
    RGBColor emitted_;

    RGBColor radiance_channel(const LightPathExpression& channel, int offset, bool include_emitted) const;
    
    // disable copy constructor & assignment operator
    LightTree& operator=(const LightTree& other);
    LightTree(const LightTree& other);
    
    void print(const std::string& prefix) const;

public:
    LightTree(SurfaceType type, const RGBColor& emitted);
    
    RGBColor radiance() const;
    RGBColor radiance_channel(const LightPathExpression& channel) const;
    
    void add_upstream(const LightTree* tree, RGBColor color);
    void add_upstream(const LightTree* tree);

    void print() const;

    ~LightTree();
};

