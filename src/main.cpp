#include <iostream>
#include <fstream>

#include "Vec.hpp"
#include "Color.hpp"
#include "Image.hpp"

int main(int argc, char** argv) {
    RGBImage img(256, 256);
    std::ofstream output_file("out.ppm");

    img.output_ppm(output_file);
    return 0;
}
