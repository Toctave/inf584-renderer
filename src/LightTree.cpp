#include "LightTree.hpp"

LightTree::LightTree(SurfaceType type, const RGBColor& emitted)
    : type_(type), emitted_(emitted) {
}

RGBColor LightTree::radiance() const {
    RGBColor out;
    for (size_t i = 0; i < upstream_.size(); i++) {
	out += attenuations_[i] * upstream_[i]->radiance();
    }
    if (upstream_.size() > 0) {
	out /= upstream_.size();
    }

    out += emitted_;

    return out;
}

void LightTree::add_upstream(const LightTree* tree, RGBColor attenuation) {
    upstream_.push_back(tree);
    attenuations_.push_back(attenuation);
}

void LightTree::add_upstream(const LightTree* tree) {
    add_upstream(tree, RGBColor::gray(1.0f));
}

LightTree::~LightTree() {
    for (const LightTree* tree : upstream_) {
        delete tree;
    }
}

void LightTree::print(std::vector<bool>& last_child) const {
    char c = surface_type_to_char(type_);
    
    for (size_t i = 0; i < last_child.size(); i++) {
	if (i + 1 == last_child.size()) {
	    std::cout << "+-";
	} else if (last_child[i]) {
	    std::cout << "  ";
	} else {
	    std::cout << "| ";
	}
    }
    std::cout << c;
    std::cout << "\n";

    for (size_t i = 0; i < upstream_.size(); i++) {
	last_child.push_back((i + 1) == upstream_.size());
	upstream_[i]->print(last_child);
	last_child.pop_back();
    }
}

void LightTree::print() const {
    std::vector<bool> b;
    print(b);
}


void LightTree::get_all_radiances(
    LightPathExpression& base,
    std::vector<std::pair<LightPathExpression, RGBColor>>& radiances,
    RGBColor attenuation
    ) const {
    base.push_back(type_);
    radiances.push_back(std::make_pair(base, emitted_ * attenuation));

    for (size_t i = 0; i < upstream_.size(); i++) {
	RGBColor att = attenuation * attenuations_[i] / static_cast<float>(upstream_.size());
	upstream_[i]->get_all_radiances(base, radiances, att);
    }
    
    base.pop_back();
}


std::vector<std::pair<LightPathExpression, RGBColor>>
LightTree::get_all_radiances() const {
    LightPathExpression base;
    std::vector<std::pair<LightPathExpression, RGBColor>> radiances;
    get_all_radiances(base, radiances, RGBColor::gray(1.0f));

    for (auto& p : radiances) {
	p.first.reverse();
    }

    return radiances;
}
