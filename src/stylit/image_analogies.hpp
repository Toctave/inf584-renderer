#pragma once

#include "../Color.hpp"
#include "../Image.hpp"
#include "../Vec.hpp"

#include <vector>

typedef RGBColor Feature;

struct ImageAnalogySystem {
    size_t levels;
    float kappa;
    
    std::vector<Buffer2D<Feature>> source_unfiltered;
    std::vector<Buffer2D<Feature>> source_filtered;
    
    std::vector<Buffer2D<Feature>> target_unfiltered;
    std::vector<Buffer2D<Feature>> target_filtered;

    std::vector<Buffer2D<Vec2s>> assignments;
    
    ImageAnalogySystem(const Buffer2D<Feature>& source_unfiltered,
		       const Buffer2D<Feature>& source_filtered,
		       const Buffer2D<Feature>& target_unfiltered,
		       size_t levels,
		       float kappa);
};
