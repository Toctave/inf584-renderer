#include "LightTree.hpp"

RGBColor LightTree::radiance_channel(const LightPathExpression& channel) const {
    return radiance_channel(channel, channel.length() - 1, true);
}

LightTree::LightTree(SurfaceType type, const RGBColor& emitted)
    : type_(type), emitted_(emitted) {
}

RGBColor LightTree::radiance() const {
    RGBColor out;
    for (size_t i = 0; i < upstream_.size(); i++) {
	out += brdfs_[i] * angle_cos_[i] * upstream_[i]->radiance() / pdfs_[i];
    }
    if (upstream_.size() > 0) {
	out /= upstream_.size();
    }

    out += emitted_;

    return out;
}

void LightTree::add_upstream(const LightTree* tree, float pdf, RGBColor brdf, float angle_cos) {
    upstream_.push_back(tree);
    pdfs_.push_back(pdf);
    brdfs_.push_back(brdf);
    angle_cos_.push_back(angle_cos);
}

void LightTree::add_upstream(const LightTree* tree) {
    add_upstream(tree, 1.0f, RGBColor::gray(1.0f), 1.0f);
}

LightTree::~LightTree() {
    for (const LightTree* tree : upstream_) {
        delete tree;
    }
}

RGBColor LightTree::radiance_channel(const LightPathExpression& channel, int offset, bool include_emitted) const {
    if (offset < 0) {
	return RGBColor();
    }

    RGBColor transmitted;

    if (channel[offset] == SurfaceType::REPEAT) {
	for (size_t i = 0; i < upstream_.size(); i++) {
	    transmitted += brdfs_[i] * angle_cos_[i]
		* (upstream_[i]->radiance_channel(channel, offset - 1, true)
		   + upstream_[i]->radiance_channel(channel, offset, false))
		   / pdfs_[i];
	}
    } else if (channel[offset] == type_ || channel[offset] == SurfaceType::ANY) {
	for (size_t i = 0; i < upstream_.size(); i++) {
	    transmitted += brdfs_[i] * angle_cos_[i] * upstream_[i]->radiance_channel(channel, offset - 1, true) / pdfs_[i];
	}
    } else {
	return RGBColor();
    }
    
    if (upstream_.size() > 0) {
	transmitted /= upstream_.size();
    }

    if (include_emitted) {
	return transmitted + emitted_;
    } else {
	return transmitted;
    }
}

void LightTree::print(const std::string& suffix) const {
    std::string more = surface_type_to_char(type_) + suffix;
    if (type_ == LIGHT && upstream_.size() == 0) {
	std::cout << more << "\n";
    } else {
	for (const LightTree* tree : upstream_) {
	    tree->print(more);
	}
    }
}

void LightTree::print() const {
    print("");
}
