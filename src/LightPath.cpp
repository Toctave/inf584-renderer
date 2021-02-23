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

char surface_type_letter(SurfaceType type) {
    switch(type) {
    case SurfaceType::EYE:
        return 'E';
    case SurfaceType::DIFFUSE:
        return 'D';
    case SurfaceType::SPECULAR:
        return 'S';
    case SurfaceType::LIGHT:
        return 'L';
    default:
        return '?';
    }
}

void LightPath::print_subpaths(const std::string& base) const {
    std::string s = surface_type_letter(type_) + base;
    if (tributaries_.size() == 0) {
        std::cout << s << " " << emitted_ << "\n";
    } else {
        for (const LightPath* path: tributaries_) {
            path->print_subpaths(s);
        }
    }
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
