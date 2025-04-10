#ifndef bvh_h
#define bvh_h

#include "utils/common.h"
#include "utils/color.h"

#include "physics/hittable.h"
#include "physics/bvh_node.h"



// ----------------------------------------------------- //
// bvh_container
// ----------------------------------------------------- //

class bvh_container {
private:
    shared_ptr<bvh_node> _root;
    
    int _max_depth;
    aabb _world_bounding_box;

public:
    bvh_container(): _root(nullptr), _max_depth(0) {};
    bvh_container(const shared_ptr<std::vector<shared_ptr<hittable>>> objects, int max_depth, point3 camera_pos)
        : _root(nullptr), _max_depth(max_depth) {
        
        rebuild(objects, camera_pos);
    }
    ~bvh_container() {
    }

    // ----------------------------------------------------- //
    // logic
    // ----------------------------------------------------- //

    void rebuild(const shared_ptr<std::vector<shared_ptr<hittable>>> objects, point3 camera_pos) {
        // TODO : implement this function
        _world_bounding_box.set_min(vec3(1e9, 1e9, 1e9));
        _world_bounding_box.set_max(vec3(-1e9, -1e9, -1e9));

        for (const auto& obj : *objects) {
            aabb box = obj->bounding_box;
            _world_bounding_box.set_min(vec3::min(_world_bounding_box.min(), box.min()));
            _world_bounding_box.set_max(vec3::max(_world_bounding_box.max(), box.max()));
        }

        // create the root node
        bvh_node _temp_root(_world_bounding_box, objects, 0, _max_depth, camera_pos);
        _root = make_shared<bvh_node>(_temp_root);
    }

    std::vector<shared_ptr<bvh_node>> get_intersecting_nodes(const ray& r, interval ray_t, hit_record& rec) const {
        return _root->get_intersecting_nodes(r, ray_t, rec);;
    }

    // ----------------------------------------------------- //
    // getters
    // ----------------------------------------------------- //

    shared_ptr<bvh_node> get_root() const { return _root; }
    int max_depth() const { return _max_depth; }

    // ----------------------------------------------------- //
    // setters
    // ----------------------------------------------------- //
    void set_max_depth(int max_depth) { _max_depth = max_depth; }
};



#endif