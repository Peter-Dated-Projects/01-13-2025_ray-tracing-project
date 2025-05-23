
#include "utils/common.h"
#include "utils/color.h"

#include "physics/camera.h"
#include "physics/hittable.h"
#include "physics/material.h"
#include "physics/hittable_list.h"
#include "physics/sphere.h"


#include <time.h>

int main () {

    hittable_list world;

    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

    for (int a = -10; a < 10; a++) {
        for (int b = -10; b < 10; b++) {
            auto choose_mat = random_double();
            double radius = random_double() * 0.4 + 0.1;
            point3 center(a + 0.9*random_double(), radius, b + 0.9*random_double());

            if ((center - point3(4, radius, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    world.add(make_shared<sphere>(center, radius, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, radius, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(center, radius, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));


    // auto material_ground = make_shared<lambertian>(color(0.8, 0.8, 0.0));
    // auto material_center = make_shared<lambertian>(color(0.1, 0.2, 0.5));
    // auto material_left = make_shared<dielectric>(1.50);
    // auto material_bubble = make_shared<dielectric>(1.00 / 1.50);
    // auto material_right = make_shared<metal>(color(0.8, 0.6, 0.2), 1.0);

    // world.add(make_shared<sphere>(point3(0, -100.5, -1), 100, material_ground));
    // world.add(make_shared<sphere>(point3(0, 0, -1.1), 0.5, material_center));
    // world.add(make_shared<sphere>(point3(-1.0, 0, -1.0), 0.5, material_left));
    // world.add(make_shared<sphere>(point3(-1.0, 0, -1.0), 0.4, material_bubble));
    // world.add(make_shared<sphere>(point3(1.0, 0, -1.0), 0.5, material_right));

    
    // camera cam;
    camera cam;

    // high res
    // cam.aspect_ratio      = 16.0 / 9.0;
    // cam.width       = 1200;
    // cam.samples_per_pixel = 500;
    // cam.max_depth         = 50;

    // low res
    cam.aspect_ratio      = 16.0 / 9.0;
    cam.width       = 700;
    cam.samples_per_pixel = 5;
    cam.max_depth         = 30;

    // cam.aspect_ratio      = 16.0 / 9.0;
    // cam.width       = 300;
    // cam.samples_per_pixel = 5;
    // cam.max_depth         = 30;

    cam.vfov     = 30;
    cam.lookfrom = point3(15,10,10);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0.8;
    cam.focus_dist    = 13.0;

    int bvh_depth = 9;
    world.finalize(cam.get_center(), bvh_depth);


    cam.multi_process_render(&world, 0, cam.width);
    // cam.render(world, 0, cam.width);

}