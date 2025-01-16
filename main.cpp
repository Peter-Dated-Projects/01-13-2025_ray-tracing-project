
#include "utils/common.h"
#include "utils/color.h"

#include "physics/camera.h"
#include "physics/hittable.h"
#include "physics/hittable_list.h"
#include "physics/sphere.h"


int main () {
    hittable_list world;

    world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
    world.add(make_shared<sphere>(point3(0, -100.5, -1), 100));

    camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.width = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth = 50;

    cam.render(world);

    return 0;
}