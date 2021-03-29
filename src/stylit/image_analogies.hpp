#pragma once

#include "../Color.hpp"
#include "../Image.hpp"
#include "../Vec.hpp"

#include "EigenArray2D.hpp"

#include <vector>

#include <ANN/ANN.h>

using Feature = Eigen::VectorXf;
using FeatureImage = EigenArray2D<float>;

struct ImagePair {
    std::vector<FeatureImage> filtered;
    std::vector<FeatureImage> unfiltered;
};

struct ImageAnalogySystem {
    size_t levels;
    float kappa;

    std::vector<ANNpointArray> neighborhoods;

    ImagePair source;
    ImagePair target;

    std::vector<Buffer2D<Vec2s>> assignments;
    std::vector<ANNkd_tree> kd_trees;

    ImageAnalogySystem(const FeatureImage& source_unfiltered_img,
		       const FeatureImage& source_filtered_img,
		       const FeatureImage& target_unfiltered_img,
		       size_t levels,
		       float kappa);
};

FeatureImage multichannel_image(const std::vector<Buffer2D<RGBColor>>& images);

void solve(ImageAnalogySystem& system);

Feature rgb_to_feature(const RGBColor& color);
RGBColor feature_to_rgb(const Feature& feature);

Buffer2D<RGBColor> feature_to_rgb(const FeatureImage& image);
