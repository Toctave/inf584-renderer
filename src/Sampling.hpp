#pragma once

#include "Vec.hpp"

void initialize_random_system(unsigned int seed);
float random_01();
Vec2 sample_unit_square();
Vec2 sample_unit_disc();
Vec3 sample_hemisphere_cosine_weighted(float* pdf);
Vec3 sample_unit_sphere();

