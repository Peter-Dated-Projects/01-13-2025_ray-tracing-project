

#ifndef color_h
#define color_h

#include "utils/common.h"

#include "math/vec3.h"


using color = vec3;

void write_color(std::ostream& out, const color& pixel_color) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // translate [0, 1], component to byte range [0, 255]
    int rbyte = int(255.999 * r);
    int gbyte = int(255.999 * g);
    int bbyte = int(255.999 * b);

    // write out components

    out << rbyte << ' ' << gbyte << ' ' << bbyte << std::endl;
}

#endif