#include "image_analogies.hpp"

#include "DynaVec.hpp"
#include "../constants.hpp"

#include <ANN/ANN.h>

static void extend(DynaVec<float>& v, const Feature& feature) {
    v.extend(feature);
}

static DynaVec<float> get_neighborhood(const Buffer2D<Feature>& features, Vec2s p, size_t radius) {
    DynaVec<float> neigh;
    Vec2s nbp;

    // assume that the whole neighborhood fits into the image
    for (nbp[0] = p[0] - radius; nbp[0] <= p[0] + radius; nbp[0]++) {
	for (nbp[1] = p[1] - radius; nbp[1] <= p[1] + radius; nbp[1]++) {
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

static DynaVec<float> get_source_neighborhood(const ImageAnalogySystem& system, size_t base_level, Vec2s p) {
    return get_pyramid_neighborhood(system.source_unfiltered, base_level, p)
	.extend(get_pyramid_neighborhood(system.source_filtered, base_level, p));
}

static DynaVec<float> get_target_neighborhood(const ImageAnalogySystem& system, size_t base_level, Vec2s q) {
    return get_pyramid_neighborhood(system.target_unfiltered, base_level, q)
	.extend(get_pyramid_neighborhood(system.target_filtered, base_level, q));
}

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

static size_t source_stride(const ImageAnalogySystem& system, size_t level) {
    return system.source_unfiltered[level].columns() - 4;
}

static Vec2s source_pixel_from_idx(const ImageAnalogySystem& system, size_t level, ANNidx idx) {
    size_t stride = source_stride(system, level);
    return Vec2s(2 + idx / stride, 2 + idx % stride);
}

static Vec2s
best_approximate_match(const ImageAnalogySystem& system, size_t level, Vec2s q) {
    static const float eps = .001f; // TODO : pick a value for eps
    DynaVec<float> query = get_target_neighborhood(system, level, q);
    ANNidx result_idx;
    ANNdist result_dist;
    
    // libANN does not use const qualifiers, have to cast away constness here :
    const_cast<ImageAnalogySystem&>(system)
	.kd_trees[level].annkSearch(
	    const_cast<ANNpoint>(query.data()),
	    1,
	    &result_idx,
	    &result_dist,
	    eps);

    return source_pixel_from_idx(system, level, result_idx);
}

static Vec2s
best_coherence_match(const ImageAnalogySystem& system, size_t level, Vec2s q) {
    Vec2s candidate;
    Vec2s best_candidate;
    float best_dist2 = INFTY;

    DynaVec<float> target_features = get_target_neighborhood(system, level, q);
    for (candidate[0] = q[0] - 2; candidate[0] <= q[0] + 2; candidate[0]++) {
	for (candidate[1] = q[1] - 2; candidate[1] <= q[1] + 2; candidate[1]++) {
	    Vec2s assgnt = system.assignments[level](candidate);
	    Vec2s candidate_in_source = assgnt + q - candidate;
	    DynaVec<float> candidate_features = get_source_neighborhood(system, level, candidate_in_source);

	    if (candidate_features.dim() != target_features.dim()) {
		// this is a hack to avoid messing with border conditions for now
		// TODO : properly handle such cases
		continue;
	    }
	    
	    float dist2 = norm_squared(candidate_features - target_features);

	    if (dist2 < best_dist2) {
		best_dist2 = dist2;
		best_candidate = candidate_in_source;
	    }
	}
    }

    assert(best_dist2 < INFTY);
    
    return best_candidate;
}

static float
feature_diff_neighbourhood(const ImageAnalogySystem& system, size_t level, Vec2s p, Vec2s q) {
    DynaVec<float> s = get_source_neighborhood(system, level, p);
    DynaVec<float> t = get_target_neighborhood(system, level, q);

    return norm_squared(s - t);
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

void solve(ImageAnalogySystem& system) {
    std::cout << "Solving!\n";
    for (size_t l = system.levels - 2; l < system.levels - 1; l--) {
	std::cout << "level " << l << "\n";
	Vec2s q;
	for(q[0] = 2; q[0] + 2 < system.target_filtered[l].rows(); q[0]++) {
	    std::cout << "row " << q[0] << " / " << system.target_filtered[l].rows() << "\n";
	    for(q[1] = 2; q[1] + 2 < system.target_filtered[l].columns(); q[1]++) {
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
	
	Buffer2D<Vec2s> assgnt(target_unfiltered[i].rows(), target_unfiltered[i].columns());

	Vec2s p;
	for (p[0] = 0; p[0] < assgnt.rows(); p[0]++) {
	    for (p[1] = 0; p[1] < assgnt.columns(); p[1]++) {
		assgnt(p) = p;
	    }
	}
	assignments.push_back(assgnt);
    }

    kd_trees.reserve(levels - 1);
    for (size_t lvl = 0; lvl + 1 < levels; lvl++) {

	std::cout << "Building level " << lvl << "\n";
	Vec2s p;

	size_t neighborhood_count =
	    (source_unfiltered[lvl].rows() - 4) * (source_unfiltered[lvl].columns() - 4);

	size_t neighborhood_dims = 2 * FEATURE_DIM * (5 * 5 + 3 * 3);
	ANNpoint* neighborhoods_lvl = new ANNpoint[neighborhood_count]; // TODO : delete this
	float* coord_buffer = new float[neighborhood_count * neighborhood_dims]; // TODO : delete this

	size_t nb_idx = 0;
	for (p[0] = 2; p[0] + 2 < source_unfiltered[lvl].rows(); p[0]++) {
	    std::cout << "Building row " << p[0] << "\n";
	    for (p[1] = 2; p[1] + 2 < source_unfiltered[lvl].columns(); p[1]++) {
		DynaVec<float> neigh = get_source_neighborhood(*this, lvl, p);
		assert(neigh.dim() == neighborhood_dims);

		assert(nb_idx < neighborhood_count);
		
		neighborhoods_lvl[nb_idx] = &coord_buffer[nb_idx * neighborhood_dims];
		memcpy(neighborhoods_lvl[nb_idx], neigh.data(), sizeof(float) * neighborhood_dims);

		nb_idx++;
	    }
	}

	neighborhoods.push_back(neighborhoods_lvl);
	kd_trees.emplace_back(neighborhoods_lvl, neighborhood_count, neighborhood_dims);
    }

    std::cout << "Done building the system\n";
}

Buffer2D<Feature> stylit(const Buffer2D<Feature>& source_unfiltered,
			 const Buffer2D<Feature>& source_filtered,
			 const Buffer2D<Feature>& target_unfiltered,
			 size_t levels,
			 float kappa) {
    ImageAnalogySystem system(source_unfiltered, source_filtered, target_unfiltered, levels, kappa);

    solve(system);

    return system.target_filtered[0];
}
