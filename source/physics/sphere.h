

#ifndef sphere_h
#define sphere_h

#include "utils/common.h"


class sphere : public hittable {
private:
    point3 center;
    double radius;
    shared_ptr<material> mat;

public:
    sphere(const point3& center, double radius, shared_ptr<material> mat) : center(center), radius(std::fmax(0, radius)), mat(mat) {
        // initialize base objects
        initialize_base_objects();

        // std::cout << bounding_box << std::endl;
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        // direclty edit the hit_record object

        vec3 oc = center - r.origin();
    
        auto a = r.direction().length_squared();
        auto h = dot(r.direction(), oc);
        auto c = oc.length_squared() - radius*radius;
        auto disc = h*h - a*c;

        // if no collision exit
        if (disc < 0) return false;

        auto sqrtd = std::sqrt(disc);
 
        // find nearest root that lies in acceptable range
        auto root = (h - sqrtd) / a;
        if (!ray_t.surrounds(root)) {
            root = (h + sqrtd) / a;

            // if not within acceptable range, quit
            if (!ray_t.surrounds(root))
                return false;
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        vec3 outward_normal = (rec.p - center) / radius;
        rec.set_face_normal(r, outward_normal);
        rec.mat = mat;

        return true;
    }

    void calculate_bounding_box() override {
        // TODO : implement this function
        vec3 _min = center - vec3(radius, radius, radius);
        vec3 _max = center + vec3(radius, radius, radius);

        bounding_box = aabb(_min, _max);
    }

};

#endif