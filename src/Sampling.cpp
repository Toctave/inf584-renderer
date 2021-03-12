#include "Sampling.hpp"

#include <random>

static std::uniform_real_distribution<float> g_distribution;
static std::mt19937 g_mt;

void initialize_random_system(unsigned int seed) {
    g_mt = std::mt19937(seed);
    g_distribution = std::uniform_real_distribution<float>(0.0f, 1.0f);
    std::cout << "RNG seed : " << seed << "\n";
}

float random_01() {
    float x;
    #pragma omp critical
    {
        x = g_distribution(g_mt);
    }
    return x;
}

Vec2 sample_unit_square() {
    return Vec2({random_01(), random_01()});
}

Vec2 sample_unit_disc() {
    Vec2 sq = sample_unit_square();
    float theta = 2.0f * M_PI * sq[0];
    float r = std::sqrt(sq[1]);
    
    return Vec2({r * std::cos(theta), r * std::sin(theta)});
}

Vec3 sample_unit_sphere() {
    Vec2 sq = sample_unit_square();

    float phi = 2.0f * M_PI * sq[0];
    float theta = std::acos(1.0f - 2.0f * sq[1]);

    float sp = std::sin(phi);
    float cp = std::cos(phi);
    float st = std::sin(theta);
    float ct = std::cos(theta);

    return Vec3({
            st * cp,
            st * sp,
            ct
        });
}

Vec3 sample_hemisphere_cosine_weighted(float* pdf) {
    Vec2 disc = sample_unit_disc();

    Vec3 sample({disc[0], disc[1], std::sqrt(1.0f - norm_squared(disc))});

    *pdf = sample[2] / M_PI;

    return sample;
}

