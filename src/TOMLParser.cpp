#include "TOMLParser.hpp"

#include "Camera.hpp"
#include "Material.hpp"
#include "Sphere.hpp"
#include "TriangleMesh.hpp"

std::string new_name() {
    static int index = 0;
    
    std::stringstream ss;
    ss << "$" << index;

    index++;
    
    return ss.str();
}

static float radians(float deg) {
    return deg * M_PI / 180.0f;
}

template<typename T>
T TOMLParser::decode(const toml::value& base, const std::string& name) {
    return decode<T>(toml::find(base, name));
}

template<>
Vec3 TOMLParser::decode<Vec3>(const toml::value& toml) {
    std::vector<float> coords = toml::get<std::vector<float>>(toml);

    if (coords.size() != 3) {
	throw std::runtime_error("vectors must have 3 coordinates");
    }

    return Vec3(coords[0], coords[1], coords[2]);
}

template<>
Camera TOMLParser::decode<Camera>(const toml::value& toml) {
    Vec3 eye = decode<Vec3>(toml, "eye");
    Vec3 target = decode<Vec3>(toml, "target");
    Vec3 up = decode<Vec3>(toml, "target");
    float fov = radians(toml::find<float>(toml, "fov"));    

    return Camera(eye, target, up, fov, aspect_ratio_);
}

template<>
Material* TOMLParser::decode<Material*>(const toml::value& toml) {
    std::string type = toml::find<std::string>(toml, "type");

    if (type == "lambert") {
	RGBColor albedo(decode<Vec3>(toml, "albedo"));
	return new LambertMaterial(albedo);
    } else if (type == "microfacet") {
	RGBColor albedo(decode<Vec3>(toml, "albedo"));
	float roughness = toml::find<float>(toml, "roughness");
	float ior = toml::find<float>(toml, "ior");

	return new MicrofacetMaterial(albedo, roughness, ior);
    } else if (type == "emission") {
	RGBColor color(decode<Vec3>(toml, "color"));
	float power = toml::find<float>(toml, "power");

	return new Emission(power * color);
    } else {
	throw std::runtime_error("unknown material type '" + type + "'");
    }
}

template<>
Primitive* TOMLParser::decode<Primitive*>(const toml::value& toml) {
    std::string type = toml::find<std::string>(toml, "type");

    if (type == "sphere") {
	Vec3 center = decode<Vec3>(toml, "center");
	float radius = toml::find<float>(toml, "radius");

	return new Sphere(center, radius);
    } else if (type == "mesh") {
	std::string path = toml::find<std::string>(toml, "path");

	return new TriangleMesh(base_path_ + path);
    } else {
	throw std::runtime_error("unknown primitive type '" + type + "'");
    }
}

template<>
Shape* TOMLParser::decode<Shape*>(const toml::value& toml) {
    std::string prim_name = toml::find<std::string>(toml, "primitive");
    std::string mat_name = toml::find<std::string>(toml, "material");

    auto prim = primitive_map_.find(prim_name);
    if (prim == primitive_map_.end()) {
	throw std::runtime_error("unknown primitive '" + prim_name + "'");
    } 

    auto mat = material_map_.find(mat_name);
    if (mat == material_map_.end()) {
	throw std::runtime_error("unknown material '" + mat_name + "'");
    }

    Transform t;
    if (toml.contains("translate")) {
	Vec3 pos = decode<Vec3>(toml, "translate");
	t *= Transform::translate(pos);
    } 
    if (toml.contains("rotate_axis") && toml.contains("rotate_angle")) {
	float angle = toml::find<float>(toml, "rotate_angle");
	Vec3 axis = decode<Vec3>(toml, "rotate_axis");
	t *= Transform::rotate(axis.normalized(), radians(angle));
    } 
    if (toml.contains("scale")) {
	const toml::value val = toml::find(toml, "scale");
	if (val.is_floating()) {
	    float f = toml::get<float>(val);
	    t *= Transform::scale(f);
	} else {
	    Vec3 factor = decode<Vec3>(val);
	    t *= Transform::scale(factor[0], factor[1], factor[2]);
	}
    } 

    Shape* result = new Shape(prim->second, mat->second);

    result->set_transform(t);

    return result;
}

template<>
Light* TOMLParser::decode<Light*>(const toml::value& toml) {
    std::string type = toml::find<std::string>(toml, "type");

    if (type == "area") {
	std::string shape_name = toml::find<std::string>(toml, "shape");
	auto shape = shape_map_.find(shape_name);
	if (shape == shape_map_.end()) {
	    throw std::runtime_error("unknown shape '" + shape_name + "'");
	}
	
	return new AreaLight(shape->second);
    } else if (type == "point") {
	Vec3 position = decode<Vec3>(toml, "position");
	Vec3 color = decode<Vec3>(toml, "color");
	float power = toml::find<float>(toml, "power");

	return new PointLight(position, color, power);
    } else {
	throw std::runtime_error("unknown primitive type '" + type + "'");
    }
}

template<>
Scene TOMLParser::decode<Scene>(const toml::value& toml) {
    Scene result;

    for (const auto& mat : toml::find<std::vector<toml::value>>(toml, "materials")) {
	std::string name = toml::find<std::string>(mat, "name");
	Material* matp = decode<Material*>(mat);

	material_map_[name] = matp;
    }

    for (const auto& prim : toml::find<std::vector<toml::value>>(toml, "primitives")) {
	std::string name = toml::find<std::string>(prim, "name");
	Primitive* primp = decode<Primitive*>(prim);

	primitive_map_[name] = primp;
    }

    for (const auto& shape : toml::find<std::vector<toml::value>>(toml, "shapes")) {
	std::string name;
	try {
	    name = toml::find<std::string>(shape, "name");
	} catch (const std::out_of_range& e) {
	    name = new_name();
	}
	Shape* shapep = decode<Shape*>(shape);

	shape_map_[name] = shapep;
	result.add_shape(shapep);
    }

    for (const auto& light : toml::find<std::vector<toml::value>>(toml, "lights")) {
	std::string name;
	try {
	    name = toml::find<std::string>(light, "name");
	} catch (const std::out_of_range& e) {
	    name = new_name();
	}
	Light* lightp = decode<Light*>(light);

	light_map_[name] = lightp;
	result.add_light(lightp);
    }

    return result;
}

const Camera& TOMLParser::camera() const {
    return camera_;
}

const Scene& TOMLParser::scene() const {
    return scene_;
}

TOMLParser::TOMLParser(const std::string& path, float ar)
    : aspect_ratio_(ar) {
    // TODO : make this portable
    size_t pos = path.find_last_of("/");
    if (pos != std::string::npos)
	base_path_ = path.substr(0, pos + 1);
	
    const auto data = toml::parse(path);

    camera_ = decode<Camera>(toml::find(data, "camera"));
    scene_ = decode<Scene>(data);
}

TOMLParser::~TOMLParser()
{
    for (const auto& pair : primitive_map_) {
	delete pair.second;
    }
    for (const auto& pair : material_map_) {
	delete pair.second;
    }
    for (const auto& pair : shape_map_) {
	delete pair.second;
    }
    for (const auto& pair : light_map_) {
	delete pair.second;
    }
}
