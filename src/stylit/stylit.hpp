#pragma once

#include "../Image.hpp"
#include "FeatureVector.hpp"
#include "MipmapPyramid.hpp"

typedef Buffer2D<FeatureVector> FeatureBuffer;
typedef MipmapPyramid<FeatureVector> FeaturePyramid;

struct StylitSystem {
    FeaturePyramid reference_render;
};

