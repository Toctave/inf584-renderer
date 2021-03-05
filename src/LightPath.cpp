#include "LightPath.hpp"

LightPath::LightPath(const Vec3& eye)
    : type_(SurfaceType::EYE), point_(eye) {
}

LightPath::LightPath(SurfaceType type, const Vec3& point)
    : type_(type), point_(point) {
}

LightPath::LightPath(const RGBColor& emitted, const Vec3& point)
    : type_(SurfaceType::LIGHT), point_(point), emitted_(emitted)  {
}

void LightPath::add_tributary(LightPath* path,
                              float pdf,
                              const RGBColor& brdf,
                              float angle_cos) {
    tributaries_.push_back(path);
    pdfs_.push_back(pdf);
    brdfs_.push_back(brdf);
    angle_cos_.push_back(angle_cos);
}

LightPath::~LightPath() {
    for (const LightPath* upriver : tributaries_) {
        delete upriver;
    }
}


RGBColor LightPath::radiance_channel(const LightPathExpression& channel, int offset) const {
    if (offset < 0) {
        return RGBColor();
    }
    
    SurfaceType first = channel[offset];
    if (first == SurfaceType::REPEAT) {
        RGBColor r = emitted_;
        for (size_t i = 0; i < tributaries_.size(); i++) {
            RGBColor tr =
                tributaries_[i]->radiance_channel(channel, offset - 1)
                + tributaries_[i]->radiance_channel(channel, offset);
            r += brdfs_[i] * angle_cos_[i] * tr / pdfs_[i];
        }
        return r;
    }
    
    if (first == type_ || first == SurfaceType::ANY) {
        RGBColor r;
	if (type_ == SurfaceType::LIGHT && offset == 0) {
	    r += emitted_;
	}
        for (size_t i = 0; i < tributaries_.size(); i++) {
            r += brdfs_[i] * angle_cos_[i]
                * tributaries_[i]->radiance_channel(channel, offset - 1)
                / pdfs_[i];
        }
        return r;
    } else {
        return RGBColor();
    }
}

RGBColor LightPath::radiance_channel(const LightPathExpression& channel) const {
    int length = channel.length();
    assert(channel[length - 1] == SurfaceType::EYE);
    
    return radiance_channel(channel, length - 2);
}

RGBColor LightPath::radiance() const {
    RGBColor r = emitted_;

    for (size_t i = 0; i < tributaries_.size(); i++) {
        r += brdfs_[i] * angle_cos_[i] * tributaries_[i]->radiance()
            / pdfs_[i];
    }

    return r;
}

void LightPath::set_emission(const RGBColor& emission) {
    emitted_ = emission;
}

