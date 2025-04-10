

#ifndef hittable_h
#define hittable_h


#include "utils/common.h"

class material;

int OBJECT_COUNTER = 0;

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
    int _object_id = 0;
public:
    aabb bounding_box;

    virtual ~hittable() = default;
    virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const = 0;
    virtual void calculate_bounding_box() = 0;

    void initialize_base_objects() {
        // TODO : implement this function
        // generate a unique id for the object
        _object_id = OBJECT_COUNTER++;
        // set the bounding box to a default value
        calculate_bounding_box();
    }

    int get_uuid() const {
        return _object_id;
    }
};


#endif