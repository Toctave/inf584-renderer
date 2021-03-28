#pragma once

#include "../Color.hpp"
#include "../Image.hpp"
#include "../Vec.hpp"

#include "EigenBuffer2DView.hpp"

#include <vector>

#include <ANN/ANN.h>

typedef RGBColor Feature;

// struct Feature {
//     std::vector<RGBColor> channels;
// };

const size_t FEATURE_DIM = 3;

struct ImagePair {
    std::vector<Buffer2D<Feature>> unfiltered;
    std::vector<Buffer2D<Feature>> filtered;
};

struct ImageAnalogySystem {
    size_t levels;
    float kappa;

    std::vector<ANNpointArray> neighborhoods;

    ImagePair source;
    ImagePair target;

    std::vector<Buffer2D<Vec2s>> assignments;
    std::vector<ANNbruteForce> kd_trees;
    
    ImageAnalogySystem(const Buffer2D<Feature>& source_unfiltered,
		       const Buffer2D<Feature>& source_filtered,
		       const Buffer2D<Feature>& target_unfiltered,
		       size_t levels,
		       float kappa);
};

Buffer2D<Feature> stylit(const Buffer2D<Feature>& source_unfiltered,
			 const Buffer2D<Feature>& source_filtered,
			 const Buffer2D<Feature>& target_unfiltered,
			 size_t levels,
			 float kappa);
void solve(ImageAnalogySystem& system);
