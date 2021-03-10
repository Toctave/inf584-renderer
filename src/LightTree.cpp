#include "LightTree.hpp"

RGBColor LightTree::radiance_channel(const LightPathExpression& channel) const {
    return radiance_channel(channel, channel.length() - 1);
}

LightTree::~LightTree() {
}

LightTreeBounce::LightTreeBounce(SurfaceType type)
    : type_(type) {
}

RGBColor LightTreeBounce::radiance() const {
    RGBColor out;
    for (size_t i = 0; i < upstream_.size(); i++) {
	out += brdfs_[i] * angle_cos_[i] * upstream_[i]->radiance() / pdfs_[i];
    }

    return out;
}

void LightTreeBounce::add_upstream(const LightTree* tree, float pdf, RGBColor brdf, float angle_cos) {
    upstream_.push_back(tree);
    pdfs_.push_back(pdf);
    brdfs_.push_back(brdf);
    angle_cos_.push_back(angle_cos);
}

void LightTreeBounce::add_upstream(const LightTree* tree) {
    add_upstream(tree, 1.0f, RGBColor::gray(1.0f), 1.0f);
}


LightTreeBounce::~LightTreeBounce() {
    for (const LightTree* tree : upstream_) {
        delete tree;
    }
}

RGBColor LightTreeBounce::radiance_channel(const LightPathExpression& channel, int offset) const {
    if (offset <= 0) {
	return RGBColor();
    }

    if (channel[offset] == SurfaceType::REPEAT) {
	RGBColor out;
	for (size_t i = 0; i < upstream_.size(); i++) {
	    out += brdfs_[i] * angle_cos_[i]
		* (upstream_[i]->radiance_channel(channel, offset - 1)
		   + upstream_[i]->radiance_channel(channel, offset))
		   / pdfs_[i];
	}

	return out;
    }

    if (channel[offset] == type_ || channel[offset] == SurfaceType::ANY) {
	RGBColor out;
	for (size_t i = 0; i < upstream_.size(); i++) {
	    out += brdfs_[i] * angle_cos_[i] * upstream_[i]->radiance_channel(channel, offset - 1) / pdfs_[i];
	}

	return out;
    }

    return RGBColor();
}

LightTreeSource::LightTreeSource(const RGBColor& emitted)
    : emitted_(emitted) {
}

RGBColor LightTreeSource::radiance() const {
    return emitted_;
}

RGBColor LightTreeSource::radiance_channel(const LightPathExpression& channel, int offset) const {
    if (offset < 0 || channel[offset] != SurfaceType::LIGHT) {
	return RGBColor();
    }

    return emitted_;
}

