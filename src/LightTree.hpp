#pragma once

#include <vector>
#include <utility>

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

    // disable copy constructor & assignment operator
    LightTree& operator=(const LightTree& other);
    LightTree(const LightTree& other);
    
    void print(std::vector<bool>& last_child) const;

    void get_all_radiances(
	LightPathExpression& base,
	std::vector<std::pair<LightPathExpression, RGBColor>>& radiances,
	RGBColor attenuation
	) const;
    
public:
    LightTree(SurfaceType type, const RGBColor& emitted);
    
    RGBColor radiance() const;
    
    void add_upstream(const LightTree* tree, RGBColor color);
    void add_upstream(const LightTree* tree);

    void print() const;

    std::vector<std::pair<LightPathExpression, RGBColor>>
    get_all_radiances() const;

    ~LightTree();
};

