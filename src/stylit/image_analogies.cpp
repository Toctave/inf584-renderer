#include "image_analogies.hpp"

#include "DynaVec.hpp"

#include <ANN/ANN.h>

// TODO : actually use a gaussian filter
static std::vector<Buffer2D<Feature>>
build_gaussian_pyramid(const Buffer2D<Feature>& base, size_t levels) {
    std::vector<Buffer2D<Feature>> pyramid;
    pyramid.reserve(levels);

    pyramid.push_back(base);

    for (size_t i = 1; i < levels; i++) {
	size_t rows = static_cast<size_t>(pyramid.back().rows() / 2);
	size_t columns = static_cast<size_t>(pyramid.back().columns() / 2);

	pyramid.push_back(resized(pyramid.back(), rows, columns));
    }
    return pyramid;
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
    return (p[0] < img.rows())
	&& (p[1] < img.columns());
}

template<typename T>
static void extend(DynaVec<T>& v, const Feature& feature) {
    v.extend(feature);
}

static DynaVec<float> get_neighborhood(const Buffer2D<Feature>& features, Vec2s p, size_t radius) {
    DynaVec<float> neigh;
    Vec2s offset;

    // assume that the whole neighborhood fits into the image
    for (offset[0] = -radius; offset[0] <= radius; offset[0]++) {
	for (offset[1] = -radius; offset[1] <= radius; offset[1]++) {
	    Vec2s nbp = p + offset;
	    
	    extend(neigh, features(nbp));
	}
    }

    return neigh;
}

static DynaVec<float> get_pyramid_neighborhood(const std::vector<Buffer2D<Feature>>& pyramid, size_t base_level, Vec2s p) {
    DynaVec<float> result = get_neighborhood(pyramid[base_level], p, 2);

    result.extend(get_neighborhood(pyramid[base_level+1], p / 2ul, 1));

    return result;
}

static float
feature_diff_neighbourhood(const ImageAnalogySystem& system, size_t level, Vec2s p, Vec2s q) {
    DynaVec<float> su = get_pyramid_neighborhood(system.source_unfiltered, level, p);
    DynaVec<float> tu = get_pyramid_neighborhood(system.target_unfiltered, level, q);

    DynaVec<float> sf = get_pyramid_neighborhood(system.source_filtered, level, p);
    DynaVec<float> tf = get_pyramid_neighborhood(system.target_filtered, level, q);

    return norm_squared(su - tu) + norm_squared(sf - tf);

    // Vec2s ofs;

    // // fine level
    // for (ofs[0] = -n_fine; ofs[0] <= n_fine; ofs[0]++) {
    // 	for (ofs[1] = -n_fine; ofs[1] <= n_fine; ofs[1]++) {
    // 	    Vec2s nbp = p + ofs;
    // 	    Vec2s nbq = q + ofs;
	    
    // 	    // TODO : convolve with a gaussian kernel here
    // 	    if (in_bounds(system.source_filtered[level], nbp)
    // 		&& in_bounds(system.target_filtered[level], nbq)) {
    // 		diff += filtered_feature_diff(system, level, nbp, nbq);
    // 		diff += unfiltered_feature_diff(system, level, nbp, nbq);
    // 	    }
    // 	}
    // }

    // // coarse level
    // Vec2s p_coarse(static_cast<size_t>(p[0] * system.scale_factor),
    // 		   static_cast<size_t>(p[1] * system.scale_factor));
    // Vec2s q_coarse(static_cast<size_t>(q[0] * system.scale_factor),
    // 		   static_cast<size_t>(q[1] * system.scale_factor));
    // size_t level_coarse = level + 1;

    // for (ofs[0] = -n_coarse; ofs[0] <= n_coarse; ofs[0]++) {
    // 	for (ofs[1] = -n_coarse; ofs[1] <= n_coarse; ofs[1]++) {
    // 	    Vec2s nbp = p_coarse + ofs;
    // 	    Vec2s nbq = q_coarse + ofs;

    // 	    // TODO : convolve with a gaussian kernel here
    // 	    if (in_bounds(system.source_filtered[level_coarse], nbp)
    // 		&& in_bounds(system.target_filtered[level_coarse], nbq)) {
    // 		diff += filtered_feature_diff(system, level_coarse, nbp, nbq);
    // 		diff += unfiltered_feature_diff(system, level_coarse, nbp, nbq);
    // 	    }
    // 	}
    // }
}

static Vec2s
best_match(const ImageAnalogySystem& system, size_t level, Vec2s q) {
    Vec2s p_app = best_approximate_match(system, level, q);
    Vec2s p_coh = best_coherence_match(system, level,  q);

    float d_app = feature_diff_neighbourhood(system, level, p_app, q);
    float d_coh = feature_diff_neighbourhood(system, level, p_coh, q);

    if (d_coh < d_app * (1.0f * std::pow(.5f, level) * system.kappa)) {
	return p_coh;
    } else {
	return p_app;
    }
}

static void solve(ImageAnalogySystem& system) {
    for (size_t l = system.levels - 1; l < system.levels; l--) {
	Vec2s q;
	for(q[0] = 0; q[0] < system.target_filtered[l].rows(); q[0]++) {
	    for(q[1] = 0; q[1] < system.target_filtered[l].columns(); q[1]++) {
		Vec2s p = best_match(system, l, q);
		system.target_filtered[l](q) = system.source_filtered[l](p);
		system.assignments[l](q) = p;
	    }
	}
    }
}

ImageAnalogySystem::ImageAnalogySystem(const Buffer2D<Feature>& source_unfiltered_img,
				       const Buffer2D<Feature>& source_filtered_img,
				       const Buffer2D<Feature>& target_unfiltered_img,
				       size_t levels,
				       float kappa)
    : levels(levels), kappa(kappa) {
    
    source_unfiltered =
	build_gaussian_pyramid(source_unfiltered_img, levels);
    source_filtered =
	build_gaussian_pyramid(source_filtered_img, levels);
    target_unfiltered =
	build_gaussian_pyramid(target_unfiltered_img, levels);

    for (size_t i = 0; i < levels; i++) {
	target_filtered.push_back(Buffer2D<Feature>(target_unfiltered[i].rows(), target_unfiltered[i].columns()));
	assignments.push_back(Buffer2D<Vec2s>(target_unfiltered[i].rows(), target_unfiltered[i].columns()));
    }
}

