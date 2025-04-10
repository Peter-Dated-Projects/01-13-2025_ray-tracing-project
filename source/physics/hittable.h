

#ifndef hittable_h
#define hittable_h

#include "utils/common.h"

class material;


class hit_record {
public:
    point3 p;
    vec3 normal;
    shared_ptr<material> mat;
    double t;
    bool front_face;

    void set_face_normal(const ray& r, const vec3& outward_normal) {
        // sets the hit record normal vector -- tracking for hitting the "outer" surface of an object
        // NOTE: the paramterer `outward_normal` is assumed to have unit length

        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class hittable {
private:
    // private members
    // TODO : add private members if needed
public:
    aabb bounding_box;

    virtual ~hittable() = default;
    virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const = 0;
    virtual void calculate_bounding_box() = 0;
};


#endif