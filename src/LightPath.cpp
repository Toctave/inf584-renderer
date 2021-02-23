#include "LightPath.hpp"

void LightPath::set_effluent(LightPath* effluent) {
    if (effluent) {
        effluent->add_tributary(this);
    }
}

LightPath::LightPath(const Vec3& eye)
    : type_(SurfaceType::EYE), point_(eye), emitted_() {
}

LightPath::LightPath(LightPath* effluent, SurfaceType type, const Vec3& point)
    : type_(type), point_(point), emitted_() {
    set_effluent(effluent);
}

LightPath::LightPath(LightPath* effluent, const RGBColor& emitted, const Vec3& point)
    : type_(SurfaceType::LIGHT), point_(point), emitted_(emitted) {
    set_effluent(effluent);
}

void LightPath::add_tributary(const LightPath* tributary) {
    tributaries_.push_back(this);
}

LightPath::~LightPath() {
    for (const LightPath* upriver : tributaries_) {
        delete upriver;
    }
}

RGBColor LightPath::outgoing_radiance() const {
    RGBColor out = emitted_;
    for (const LightPath* upriver : tributaries_) {
        out += upriver->outgoing_radiance();
    }
    return out;
}

