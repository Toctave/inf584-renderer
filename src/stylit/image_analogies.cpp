#include "image_analogies.hpp"

// TODO : actually use a gaussian filter
static std::vector<Buffer2D<Feature>>
build_gaussian_pyramid(const Buffer2D<Feature>& base, size_t levels, float factor) {
    std::vector<Buffer2D<Feature>> pyramid;
    pyramid.reserve(levels);

    pyramid.push_back(base);

    for (size_t i = 1; i < levels; i++) {
	size_t rows = static_cast<size_t>(pyramid.back().rows() * factor);
	size_t columns = static_cast<size_t>(pyramid.back().columns() * factor);

	pyramid.push_back(resized(pyramid.back(), rows, columns));
    }
}

static Vec2s
best_approximate_match(const ImageAnalogySystem& system, size_t level, Vec2s q) {
}

static Vec2s
best_coherence_match(const ImageAnalogySystem& system, size_t level, Vec2s q) {
}

static float filtered_feature_diff(const ImageAnalogySystem& system, size_t level, Vec2s p, Vec2s q) {
    return norm_squared(system.source_filtered[level](p) - system.target_filtered[level](q));
}

static float unfiltered_feature_diff(const ImageAnalogySystem& system, size_t level, Vec2s p, Vec2s q) {
    return norm_squared(system.source_unfiltered[level](p) - system.target_unfiltered[level](q));
}

static bool in_bounds(const Buffer2D<Feature>& img, Vec2s p) {
    return (p[0] >= 0 && p[0] < img.rows())
	&& (p[1] >= 0 && p[1] < img.columns());
}

static float
feature_diff_neighbourhood(const ImageAnalogySystem& system, size_t level, Vec2s p, Vec2s q) {
    const size_t n_fine = 2;
    const size_t n_coarse = 1;

    size_t rows_fine = system.source_unfiltered[level].rows();
    size_t columns_fine = system.source_unfiltered[level].columns();

    size_t rows_coarse = system.source_unfiltered[level + 1].rows();
    size_t columns_coarse = system.source_unfiltered[level + 1].columns();

    float diff = 0.0f;

    Vec2s ofs;

    // fine level
    for (ofs[0] = -n_fine; ofs[0] <= n_fine; ofs[0]++) {
	for (ofs[1] = -n_fine; ofs[1] <= n_fine; ofs[1]++) {
	    Vec2s nbp = p + ofs;
	    Vec2s nbq = q + ofs;
	    
	    // TODO : convolve with a gaussian kernel here
	    if (!in_bounds(system.source_filtered[level], nbp)
		|| !in_bounds(system.target_filtered[level], nbq)) {
		diff += filtered_feature_diff(system, level, nbp, nbq);
		diff += unfiltered_feature_diff(system, level, nbp, nbq);
	    }
	}
    }

    // coarse level
    Vec2s p_coarse(static_cast<size_t>(p[0] * system.scale_factor),
		   static_cast<size_t>(p[1] * system.scale_factor));
    Vec2s q_coarse(static_cast<size_t>(q[0] * system.scale_factor),
		   static_cast<size_t>(q[1] * system.scale_factor));
    size_t level_coarse = level + 1;

    for (ofs[0] = -n_coarse; ofs[0] <= n_coarse; ofs[0]++) {
	for (ofs[1] = -n_coarse; ofs[1] <= n_coarse; ofs[1]++) {
	    Vec2s nbp = p_coarse + ofs;
	    Vec2s nbq = q_coarse + ofs;

	    // TODO : convolve with a gaussian kernel here
	    if (!in_bounds(system.source_filtered[level_coarse], nbp)
		|| !in_bounds(system.target_filtered[level_coarse], nbq)) {
		diff += filtered_feature_diff(system, level_coarse, nbp, nbq);
		diff += unfiltered_feature_diff(system, level_coarse, nbp, nbq);
	    }
	}
    }

    return diff;
}

static Vec2s
best_match(const ImageAnalogySystem& system, size_t level, Vec2s q) {
    Vec2s p_app = best_approximate_match(system, level, q);
    Vec2s p_coh = best_coherence_match(system, level,  q);

    float d_app = feature_diff(system, level, p_app, q);
    float d_coh = feature_diff(system, level, p_coh, q);

    if (d_coh < d_app * (1.0f * std::pow(.5f, level))) {
	return p_coh;
    } else {
	return p_app;
    }
}

ImageAnalogySystem::ImageAnalogySystem(const Buffer2D<Feature>& source_unfiltered,
				       const Buffer2D<Feature>& source_filtered,
				       const Buffer2D<Feature>& target_unfiltered,
				       size_t levels,
				       float scale_factor)
    : levels(levels), scale_factor(scale_factor) {
    std::vector<Buffer2D<Feature>> source_unfiltered_pyramid =
	build_gaussian_pyramid(source_unfiltered, levels, factor);
    std::vector<Buffer2D<Feature>> source_filtered_pyramid =
	build_gaussian_pyramid(source_filtered, levels, factor);
    std::vector<Buffer2D<Feature>> target_unfiltered_pyramid =
	build_gaussian_pyramid(target_unfiltered, levels, factor);
}

