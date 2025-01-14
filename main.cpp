
#include "utils/common.h"
#include "utils/color.h"

#include "physics/hittable.h"
#include "physics/hittable_list.h"
#include "physics/sphere.h"


// this is kind of like our shader program
color ray_color(const ray& r, const hittable& world) {

    hit_record rec;
    if (world.hit(r, interval(0, infinity), rec)){
        return 0.5 * (rec.normal + color(1, 1, 1));
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5 * (unit_direction.y() + 1.0);
    return (1.0-a)*color(1, 1, 1) + a*color(0.5, 0.7, 1.0);
}


int main () {

    double aspect_ratio = 16.0 / 9.0;
    int width = 400;

    // calculate image height
    int height = int(width / aspect_ratio);
    height = (height < 1) ? 1 : height;         // no height of 0

    // camera
    double focal_length = 1.0;
    double viewport_height = 2.0;
    double viewport_width = viewport_height * (double(width) / double(height));
    point3 camera_center = point3(0, 0, 0);

    // world
    hittable_list world;

    world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
    world.add(make_shared<sphere>(point3(0, -100.5, -1), 100));

    // calculate vectors across horizontal + down vertical viewport edges
    vec3 viewport_u = vec3(viewport_width, 0, 0);
    vec3 viewport_v = vec3(0, -viewport_height, 0);

    // calculate horizontal + vertical delta vectors from pixel to pixel
    vec3 pixel_delta_u = viewport_u / width;
    vec3 pixel_delta_v = viewport_v / height;

    // calculate location of upper left pixel (0th pixel)
    point3 viewport_upper_left = camera_center - vec3(0, 0, focal_length) - viewport_u / 2 - viewport_v / 2;
    point3 pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

    // render
    std::cout << "P3" << std::endl << width << ' ' << height << std::endl << 255 << std::endl;

    double r, g, b;
    int ir, ig, ib;
    
    
    for (int j = 0; j < height; j++) {
        // log the progress
        std::clog << "\rScanlines remaining: " << (height - j) << " " << std::flush;

        for (int i = 0; i < width; i++) {
            auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
            auto ray_direction = pixel_center - camera_center;
            ray r(camera_center, ray_direction);

            color pixel_color = ray_color(r, world);
            write_color(std::cout, pixel_color);
        }
    }

    std::clog << "\rDone.               \n";


    return 0;
}