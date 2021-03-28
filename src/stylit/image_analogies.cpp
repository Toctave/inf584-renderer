#include "image_analogies.hpp"

// #include "DynaVec.hpp"
#include "../constants.hpp"

#include <ANN/ANN.h>

static const size_t coarse_radius = 1;
static const size_t fine_radius = 2;

static const size_t coarse_diameter = 2 * coarse_radius + 1;
static const size_t fine_diameter = 2 * fine_radius + 1;

static const size_t coarse_area = coarse_diameter * coarse_diameter;
static const size_t fine_area = fine_diameter * fine_diameter;

template<typename Derived>
static Derived concat(const Eigen::DenseBase<Derived>& m1, const Eigen::DenseBase<Derived>& m2) {
    Derived result(m1.rows() + m2.rows(), m1.cols());

    result.block(0, 0, m1.rows(), m1.cols()) = m1;
    result.block(m1.rows(), 0, m2.rows(), m2.cols()) = m2;

    return result;
}

static Eigen::ArrayXXf get_neighborhood(const Buffer2D<Feature>& features, Vec2s p, size_t radius) {
    size_t diameter = 2 * radius + 1;
    return as_eigen(features, FEATURE_DIM).pixel_block(
	p[0] - radius,
	p[1] - radius,
	diameter,
	diameter);
}

static Eigen::ArrayXXf get_partial_neighborhood(const Buffer2D<Feature>& features, Vec2s p, size_t radius) {
    // return the neighborhood up until the center, in scan-line order

    auto view = as_eigen(features, FEATURE_DIM);
    size_t diameter = 2 * radius + 1;
    
    return concat(view.pixel_block(
		      p[0] - radius,
		      p[1] - radius,
		      radius,
		      diameter),
		  view.pixel_block(
		      p[0],
		      p[1] - radius,
		      1,
		      radius));
}

static Eigen::ArrayXXf get_bilevel_neighborhood(
    const std::vector<Buffer2D<Feature>>& features,
    size_t base_level,
    Vec2s p) {

    Vec2s p_lower = p / 2ul;
    return concat(
	get_neighborhood(
	    features[base_level + 1],
	    p_lower,
	    coarse_radius),
	get_neighborhood(
	    features[base_level],
	    p,
	    fine_radius)
	);
}

static Eigen::ArrayXXf get_partial_bilevel_neighborhood(
    const std::vector<Buffer2D<Feature>>& features,
    size_t base_level,
    Vec2s p) {
    
    return concat(
	get_neighborhood(
	    features[base_level + 1],
	    p / 2ul,
	    coarse_radius),
	get_partial_neighborhood(
	    features[base_level],
	    p,
	    fine_radius)
	);
}

static Eigen::ArrayXXf get_neighborhood(
    const ImagePair& pair,
    size_t base_level,
    Vec2s p) {

    Eigen::ArrayXXf neigh_unfiltered, neigh_filtered;
    if (base_level + 1 < pair.unfiltered.size()) {
	neigh_unfiltered = get_bilevel_neighborhood(pair.unfiltered, base_level, p);
	neigh_filtered = get_partial_bilevel_neighborhood(pair.filtered, base_level, p);
    } else {
	neigh_unfiltered = get_neighborhood(
	    pair.unfiltered[base_level],
	    p,
	    fine_radius);
	neigh_filtered = get_partial_neighborhood(
	    pair.filtered[base_level],
	    p,
	    fine_radius);
    }

    // return neigh_unfiltered;
    return concat(neigh_unfiltered, neigh_filtered);
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

	Buffer2D<Feature> resized(rows, columns);
	for (size_t r = 0; r < rows; r++) {
	    for (size_t c = 0; c < columns; c++) {
		resized(r, c) = .25f * (
		    pyramid.back()(2 * r, 2 * c)
		    + pyramid.back()(2 * r + 1, 2 * c)
		    + pyramid.back()(2 * r + 1, 2 * c + 1)
		    + pyramid.back()(2 * r, 2 * c + 1)
		    );
	    }
	}
	pyramid.push_back(resized);
    }
    return pyramid;
}

static size_t source_stride(const ImageAnalogySystem& system, size_t level) {
    return system.source.unfiltered[level].columns() - 4;
}

static Vec2s source_pixel_from_idx(const ImageAnalogySystem& system, size_t level, ANNidx idx) {
    size_t stride = source_stride(system, level);
    return Vec2s(2 + idx / stride, 2 + idx % stride);
}

static Vec2s
best_approximate_match(const ImageAnalogySystem& system, size_t level, Vec2s q, float& dist2_out) {
    static const float eps = .1f; // TODO : pick a value for eps
    Eigen::ArrayXXf query = get_neighborhood(
	system.target.unfiltered[level], q, fine_radius
	);
    ANNidx result_idx;
    
    // libANN does not use const qualifiers, have to cast away constness here :
    const_cast<ANNkd_tree&>(system.kd_trees[level])
	.annkSearch(
	    query.data(),
	    1,
	    &result_idx,
	    &dist2_out,
	    eps);

    Vec2s result = source_pixel_from_idx(system, level, result_idx);
    return result;
}

static Vec2s
best_coherence_match(const ImageAnalogySystem& system, size_t level, Vec2s q, float& dist2_out) {
    Vec2s candidate;
    Vec2s best_candidate;
    float best_dist2 = INFTY;

    Eigen::ArrayXXf target_features = get_neighborhood(system.target, level, q);
    for (candidate[0] = q[0] - 2; candidate[0] <= q[0] + 2; candidate[0]++) {
	for (candidate[1] = q[1] - 2; candidate[1] <= q[1] + 2; candidate[1]++) {
	    Vec2s assgnt = system.assignments[level](candidate);
	    Vec2s candidate_in_source = assgnt + q - candidate;
	    
	    if (candidate_in_source[0] < 2 || candidate_in_source[0] + 2 >= system.source.filtered[level].rows()
		|| candidate_in_source[1] < 2 || candidate_in_source[1] + 2 >= system.source.filtered[level].columns()) {
		// this is a hack to avoid messing with border conditions for now
		// TODO : properly handle such cases
		continue;
	    }
	    
	    Eigen::ArrayXXf candidate_features = get_neighborhood(system.source, level, candidate_in_source);

	    float dist2 = (candidate_features - target_features).matrix().squaredNorm();

	    if (dist2 < best_dist2) {
		best_dist2 = dist2;
		best_candidate = candidate_in_source;
	    }
	}
    }

    assert(best_dist2 < INFTY);
    dist2_out = best_dist2;
    
    return best_candidate;
}

static Vec2s
best_match(const ImageAnalogySystem& system, size_t level, Vec2s q) {
    float d_app, d_coh;
    
    Vec2s p_app = best_approximate_match(system, level, q, d_app);
    Vec2s p_coh = best_coherence_match(system, level,  q, d_coh);

    if (d_coh < d_app * (1.0f * std::pow(.5f, level) * system.kappa)) {
	return p_coh;
    } else {
	return p_app;
    }
}

Vec2s brute_force_best_match(const ImageAnalogySystem& system, size_t level, Vec2s q) {
    Vec2s p;
    Vec2s result;
    float d2max = INFTY;

    auto neigh_q = get_neighborhood(system.target, level, q);
    
    for(p[0] = 2; p[0] + 2 < system.source.filtered[level].rows(); p[0]++) {    
	for(p[1] = 2; p[1] + 2 < system.source.filtered[level].columns(); p[1]++) {
	    auto neigh_p = get_neighborhood(system.source, level, p);
	    
	    float d2 = (neigh_p - neigh_q).matrix().squaredNorm();
	    // std::cout << q << p << d2 << "\n";
	    if (d2 < d2max) {
		result = p;
		d2max = d2;
	    }
	}
    }

    return result;
}

void solve(ImageAnalogySystem& system) {
    std::cout << "Solving!\n";
    for (size_t l = system.levels - 1; l < system.levels; l--) {
	std::cout << "level " << l << "\n";
	Vec2s q;
	for(q[0] = 2; q[0] + 2 < system.target.filtered[l].rows(); q[0]++) {
	    std::cout << "row " << q[0] << " / " << system.target.filtered[l].rows() << "\n";
	    for(q[1] = 2; q[1] + 2 < system.target.filtered[l].columns(); q[1]++) {
		Vec2s p = best_match(system, l, q);
		system.target.filtered[l](q) = system.source.filtered[l](p);
		system.assignments[l](q) = p;
		// std::cout << "dist : " << result_dist << "\n";
		// std::cout << "query : " << q << "\n";
		// float d = feature_diff_neighbourhood(system, l, p, q);
		// std::cout << "result : " << p << ", diff " << d << "\n";
	    }
	}
	// break;
    }
}

ImageAnalogySystem::ImageAnalogySystem(const Buffer2D<Feature>& source_unfiltered_img,
				       const Buffer2D<Feature>& source_filtered_img,
				       const Buffer2D<Feature>& target_unfiltered_img,
				       size_t levels,
				       float kappa)
    : levels(levels), kappa(kappa) {
    
    source.unfiltered =
	build_gaussian_pyramid(source_unfiltered_img, levels);
    source.filtered =
	build_gaussian_pyramid(source_filtered_img, levels);
    target.unfiltered =
	build_gaussian_pyramid(target_unfiltered_img, levels);

    target.filtered = source.filtered;
    for (size_t i = 0; i < levels; i++) {
	Vec2s p;
	for (p[0] = 2; p[0] + 2 < source.unfiltered[i].rows(); p[0]++) {
	    for (p[1] = 2; p[1] + 2 < source.unfiltered[i].columns(); p[1]++) {
		target.filtered[i](p) = RGBColor();
	    }
	}
	
	Buffer2D<Vec2s> assgnt(target.unfiltered[i].rows(), target.unfiltered[i].columns());

	for (p[0] = 0; p[0] < assgnt.rows(); p[0]++) {
	    for (p[1] = 0; p[1] < assgnt.columns(); p[1]++) {
		assgnt(p) = p;
	    }
	}
	assignments.push_back(assgnt);
    }

    kd_trees.reserve(levels);
    for (size_t lvl = 0; lvl < levels; lvl++) {
	std::cout << "Building level " << lvl << "\n";
	Vec2s p;

	size_t neighborhood_count =
	    (source.unfiltered[lvl].rows() - 4) * (source.unfiltered[lvl].columns() - 4);

	size_t neighborhood_dims = FEATURE_DIM * fine_area;
	ANNpoint* neighborhoods_lvl = new ANNpoint[neighborhood_count]; // TODO : delete this
	float* coord_buffer = new float[neighborhood_count * neighborhood_dims]; // TODO : delete this

	size_t nb_idx = 0;
	for (p[0] = 2; p[0] + 2 < source.unfiltered[lvl].rows(); p[0]++) {
	    std::cout << "Building row " << p[0] << "\n";
	    for (p[1] = 2; p[1] + 2 < source.unfiltered[lvl].columns(); p[1]++) {
		Eigen::ArrayXXf neigh = get_neighborhood(source.unfiltered[lvl], p, fine_radius);
		assert(neigh.size() == neighborhood_dims);
		assert(nb_idx < neighborhood_count);
		
		neighborhoods_lvl[nb_idx] = &coord_buffer[nb_idx * neighborhood_dims];
		memcpy(neighborhoods_lvl[nb_idx], neigh.data(), sizeof(float) * neigh.size());

		nb_idx++;
	    }
	}

	assert(nb_idx == neighborhood_count);

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

    return system.target.filtered[0];
}
