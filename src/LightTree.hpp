#pragma once

#include <vector>

#include "Ray.hpp"
#include "Color.hpp"
#include "Intersect.hpp"
#include "Material.hpp"
#include "LightPathExpression.hpp"

class LightTree {
public:
    virtual RGBColor radiance() const = 0;
    virtual RGBColor radiance_channel(const LightPathExpression& channel, int offset) const = 0;
    RGBColor radiance_channel(const LightPathExpression& channel) const;

    virtual ~LightTree();
};

class LightTreeBounce : public LightTree {
private:
    std::vector<const LightTree*> upstream_;
    std::vector<float> pdfs_;
    std::vector<RGBColor> brdfs_;
    std::vector<float> angle_cos_;
    SurfaceType type_;

    virtual RGBColor radiance_channel(const LightPathExpression& channel, int offset) const;
    
    // disable copy constructor & assignment operator
    LightTreeBounce& operator=(const LightTreeBounce& other);
    LightTreeBounce(const LightTreeBounce& other);

public:
    LightTreeBounce(SurfaceType type);
    virtual RGBColor radiance() const;
    
    void add_upstream(const LightTree* tree, float pdf, RGBColor brdf, float angle_cos);
    void add_upstream(const LightTree* tree);

    ~LightTreeBounce();
};

class LightTreeSource : public LightTree {
private:
    RGBColor emitted_;
    virtual RGBColor radiance_channel(const LightPathExpression& channel, int offset) const;
    
public:
    LightTreeSource(const RGBColor& color);
    virtual RGBColor radiance() const;
};
