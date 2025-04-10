

#ifndef hittable_list_h
#define hittable_list_h


#include "hittable.h"
#include "bvh_container.h"

using std::make_shared;
using std::shared_ptr;

class hittable_list : public hittable {
public:
    shared_ptr<std::vector<shared_ptr<hittable>>> objects;
    shared_ptr<bvh_container> bvh;
    bool _finalized;

    hittable_list(): _finalized(false) {
        objects = make_shared<std::vector<shared_ptr<hittable>>>();
    }
    hittable_list(shared_ptr<hittable> object) {
        objects = make_shared<std::vector<shared_ptr<hittable>>>();
        add(object);
    }

    void clear() {
        objects->clear();
    }

    void add(shared_ptr<hittable> object) {
        objects->push_back(object);
    }

    void finalize(point3 cam_position, int bvh_depth) {
        calculate_bounding_box();
        // create bvh tree
        bvh = make_shared<bvh_container>(bvh_container(objects, bvh_depth, cam_position));
        _finalized = true;

        // output bounding box
        std::cout << "Bounding box: " << bounding_box.min() << ", " << bounding_box.max() << std::endl;
        std::cout << "Number of objects: " << objects->size() << std::endl;
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        if (!_finalized) {
            std::cerr << "Error: hittable_list not finalized. Call finalize() before using." << std::endl;
            return false;
        }

        // goal here: use the bvh to do 2 things
        // 1. determine leaf nodes that interesect with ray
        // 2. check if any of the objects in the leaf nodes intersect with the ray

        // keep track of all collisions with these variables
        hit_record temp_rec, temp_node_rec;
        temp_node_rec.t = 1e9;          // use t as a distance variable
        bool hit_anything = false;
        auto closest_so_far = ray_t.max;

        std::vector<shared_ptr<bvh_node>> leaf_nodes = bvh->get_intersecting_nodes(r, ray_t, temp_node_rec);

        // output # of nodes to check
        int total_size = 0;
        for (int i = 0; i < leaf_nodes.size(); i++) {
            total_size += leaf_nodes[i]->get_relevant_objects()->size();
        }
        // std::cout << "# of nodes: " << leaf_nodes.size() << std::endl;
        // std::cout << "# of objects: " << total_size << std::endl;

        // iterate through all nodes and children in nodes
        shared_ptr<hittable> object;
        for(shared_ptr<bvh_node> node : leaf_nodes) {

            // iterate through all relevant objects in the node 
            for(int obj_index : *(node->get_relevant_objects())){
                object = objects->at(obj_index);
                if (object -> hit(r, interval(ray_t.min, closest_so_far), temp_rec)) {
                    // update data for closest valid collision so far
                    hit_anything = true;
                    closest_so_far = temp_rec.t;
                    rec = temp_rec;
                }

            }

        }


        return hit_anything;
    }

    void calculate_bounding_box() override {
        // TODO : implement this function
        vec3 min(1e9, 1e9, 1e9);
        vec3 max(-1e9, -1e9, -1e9);

        for (const auto& object : *objects) {
            aabb box = object->bounding_box;
            min = vec3::min(min, box.min());
            max = vec3::max(max, box.max());
        }

        bounding_box.set_min(min);
        bounding_box.set_max(max);
    }

};


#endif