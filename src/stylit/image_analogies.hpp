#pragma once

#include "../Color.hpp"

typedef RGBColor Feature;

struct ImageAnalogySystem {
    size_t levels;
    float scale_factor;
    
    std::vector<Buffer2D<Feature>> source_unfiltered;
    std::vector<Buffer2D<Feature>> source_filtered;
    
    std::vector<Buffer2D<Feature>> target_unfiltered;
    std::vector<Buffer2D<Feature>> target_filtered;

    ImageAnalogySystem(const Buffer2D<Feature>& source_unfiltered,
		       const Buffer2D<Feature>& source_filtered,
		       const Buffer2D<Feature>& target_unfiltered);
};
