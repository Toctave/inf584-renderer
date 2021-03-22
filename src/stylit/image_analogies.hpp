#pragma once

#include "../Color.hpp"
#include "../Image.hpp"
#include "../Vec.hpp"

#include <vector>

#include <ANN/ANN.h>

typedef RGBColor Feature;

// struct Feature {
//     std::vector<RGBColor> channels;
// };

const size_t FEATURE_DIM = 3;

struct ImageAnalogySystem {
    size_t levels;
    float kappa;

    std::vector<ANNpointArray> neighborhoods;
    
    std::vector<Buffer2D<Feature>> source_unfiltered;
    std::vector<Buffer2D<Feature>> source_filtered;
    
    std::vector<Buffer2D<Feature>> target_unfiltered;
    std::vector<Buffer2D<Feature>> target_filtered;

    std::vector<Buffer2D<Vec2s>> assignments;
    std::vector<ANNkd_tree> kd_trees;
    
    ImageAnalogySystem(const Buffer2D<Feature>& source_unfiltered,
		       const Buffer2D<Feature>& source_filtered,
		       const Buffer2D<Feature>& target_unfiltered,
		       size_t levels,
		       float kappa);
};

Buffer2D<Feature> stylit(const Buffer2D<Feature>& source_unfiltered,
			 const Buffer2D<Feature>& source_filtered,
			 const Buffer2D<Feature>& target_unfiltered);
