

#ifndef camera_h
#define camera_h

#include "hittable.h"
#include "material.h"

class camera {
private:
    // Private Camera Variables here

    int height;                     // Rendered image height
    point3 center;                  // Camera center
    point3 pixel00_loc;             // Location of pixel 0, 
    vec3 pixel_delta_u;             // Offset to pixel to the right
    vec3 pixel_delta_v;             // Offset to pixel below
    double pixel_samples_scale;     // Color scale factor for a sum of pixel samples -- anti-aliasing
    vec3 u, v, w;                   // Camera frame basis vectors

    void initialize() {
        height = int(width / aspect_ratio);
        height = (height < 1) ? 1 : height;

        pixel_samples_scale = 1.0 / samples_per_pixel;

        center = lookfrom;

        // Determine viewport dimensions
        double focal_length = (lookfrom - lookat).length();
        double theta = degrees_to_radians(vfov);
        double h = std::tan(theta/2);
        double viewport_height = 2 * h * focal_length;
        double viewport_width = viewport_height * (double(width) / double(height));

        // calculate the uv,w unit basis vectors for the camera coordinate frame
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        // calculate vectors across horizontal + down vertical viewport edges
        vec3 viewport_u = viewport_width * u;       // across viewport horizontal edge
        vec3 viewport_v = viewport_height * -v;     // down vec viewport vertical edge

        // calculate horizontal + vertical delta vectors from pixel to pixel
        pixel_delta_u = viewport_u / width;
        pixel_delta_v = viewport_v / height;

        // calculate location of upper left pixel (0th pixel)
        auto viewport_upper_left = center - (focal_length * w) - viewport_u/2 - viewport_v/2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);



    }

    ray get_ray(int i, int j) const {
        // Constructs a camera ray originating from the origin and directed at randomly sampled
        // point around the pixel location i, j

        auto offset = sample_square();
        auto pixel_sample = pixel00_loc + ((i + offset.x()) * pixel_delta_u) + ((j + offset.y()) * pixel_delta_v);

        auto ray_origin = center;
        auto ray_direction = pixel_sample - ray_origin;

        return ray(ray_origin, ray_direction);
    }

    vec3 sample_square() const {
        // Returns the vector to a random point in teh [+-.5, +-.5] unit square range
        return vec3(random_double() - 0.5, random_double() + 0.5, 0);
    }

    color ray_color(const ray& r, int depth, const hittable& world) const {
        if (depth <= 0) {
            return color(0, 0, 0);
        }

        hit_record rec;
        if (world.hit(r, interval(0.001, infinity), rec)) {             // the 0.001 fixes shadow acne
            ray scattered;
            color attenuation;

            // scatter has valid calculations
            if (rec.mat -> scatter(r, rec, attenuation, scattered)){
                // calculate loss of color by reflection
                return attenuation * ray_color(scattered, depth-1, world);
            }
            return color(0, 0, 0);
        }

        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5 * (unit_direction.y() + 1.0);
        return (1.0-a) * color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
    }

public:
    double aspect_ratio = 1.0;      // ratio of width over height
    int width = 100;                // rendered image width in pixel count 
    int samples_per_pixel = 10;     // Count of random samples for each pixel
    int max_depth = 10;             // Max number of ray bounces into scene

    double vfov         = 90;                   // vertical fov
    point3 lookfrom     = point3(0,0,0);        // point camera is located
    point3 lookat       = point3(0, 0, -1);     // point camera is looking at
    vec3 vup            = vec3(0, 1, 0);        // camera relative "up" vec3

    void render(const hittable& world) {   
        initialize();

        std::cout << "P3" << std::endl << width << ' ' << height << std::endl << 255 << std::endl;

        double r, g, b;
        int ir, ig, ib;
        for (int j = 0; j < height; j++) {
            // log the progress
            std::clog << "\rScanlines remaining: " << (height - j) << " " << std::flush;

            for (int i = 0; i < width; i++) {
                color pixel_color(0, 0, 0);

                // anti-aliasing
                for (int sample = 0; sample < samples_per_pixel; sample++) {
                    ray r = get_ray(i, j);
                    pixel_color += ray_color(r, max_depth, world);
                }
                write_color(std::cout, pixel_samples_scale * pixel_color);
            }
        }
        std::clog << "\rDone.               \n";


    }

};

#endif