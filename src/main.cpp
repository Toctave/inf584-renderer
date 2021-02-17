#include <iostream>
#include <fstream>

#include "Vec.hpp"
#include "Color.hpp"
#include "Image.hpp"

void render(RGBImage& output) {
    RGBColor c1({1, 0, 0});
    RGBColor c2({0, 1, 0});
    
    for (size_t row = 0; row < output.width(); row++) {
        float ratio = static_cast<float>(row) / output.width();
        RGBColor c = lerp(c1, c2, ratio);
        for (size_t col = 0; col < output.height(); col++) {
            output(col, row) = c;
        }
    }
}

int main(int argc, char** argv) {
    RGBImage img(256, 256);

    render(img);
    
    std::ofstream output_file("out.ppm");
    img.output_ppm(output_file);
    return 0;
}
