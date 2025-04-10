
#ifndef aabb_h
#define aabb_h

#include "utils/common.h"

class aabb {
private:
    vec3 _min;
    vec3 _max;

public:
    aabb() : _min(vec3(0, 0, 0)), _max(vec3(0, 0, 0)) {}
    aabb(const vec3& min, const vec3& max) : _min(min), _max(max) {}

    // getters for min and max
    vec3 min() const { return _min; }
    vec3 max() const { return _max; }
    vec3 center() const { return (_min + _max) / 2; }
    vec3 size() const { return _max - _min; }
    vec3 diagonal() const { return _max - _min; }

    // setters
    void set_min(const vec3& min) { _min = min; }
    void set_max(const vec3& max) { _max = max; }

    // check if two aabbs intersect
    bool intersect(const aabb& other) const {
        return (_min.x() <= other.max().x() && _max.x() >= other.min().x()) &&
               (_min.y() <= other.max().y() && _max.y() >= other.min().y()) &&
               (_min.z() <= other.max().z() && _max.z() >= other.min().z());
    }

    // check if aabb intersects with a ray
    bool intersect(const ray& r, interval ray_t) const {
        float t_min = (min().x() - r.origin().x()) / r.direction().x();
        float t_max = (max().x() - r.origin().x()) / r.direction().x();

        if (t_min > t_max) std::swap(t_min, t_max);

        float ty_min = (min().y() - r.origin().y()) / r.direction().y();
        float ty_max = (max().y() - r.origin().y()) / r.direction().y();

        if (ty_min > ty_max) std::swap(ty_min, ty_max);

        if ((t_min > ty_max) || (ty_min > t_max)) return false;

        if (ty_min > t_min) t_min = ty_min;
        if (ty_max < t_max) t_max = ty_max;

        float tz_min = (min().z() - r.origin().z()) / r.direction().z();
        float tz_max = (max().z() - r.origin().z()) / r.direction().z();

        if (tz_min > tz_max) std::swap(tz_min, tz_max);

        return !((t_min > tz_max) || (tz_min > t_max));
    }
    // check if aabb contains a point
    bool contains(const vec3& point) const {
        return (point.x() >= _min.x() && point.x() <= _max.x()) &&
               (point.y() >= _min.y() && point.y() <= _max.y()) &&
               (point.z() >= _min.z() && point.z() <= _max.z());
    }
    // check if aabb contains another aabb
    bool contains(const aabb& other) const {
        return (other.min().x() >= _min.x() && other.max().x() <= _max.x()) &&
               (other.min().y() >= _min.y() && other.max().y() <= _max.y()) &&
               (other.min().z() >= _min.z() && other.max().z() <= _max.z());
    }
};


// overload << operator for aabb
inline std::ostream& operator<<(std::ostream& os, const aabb& box) {
    os << "aabb(" << box.min() << " | " << box.max() << ")";
    return os;
}


#endif