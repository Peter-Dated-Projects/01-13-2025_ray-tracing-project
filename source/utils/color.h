

#ifndef color_h
#define color_h

#include "utils/common.h"

#include "math/interval.h"
#include "math/vec3.h"


using color = vec3;

void write_color(std::ostream& out, const color& pixel_color) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // translate [0, 1], component to byte range [0, 255]
    static const interval intensity(0.000, 0.999);
    int rbyte = int(256 * intensity.clamp(r));
    int gbyte = int(256 * intensity.clamp(g));
    int bbyte = int(256 * intensity.clamp(b));

    // write out components

    out << rbyte << ' ' << gbyte << ' ' << bbyte << std::endl;
}

#endif