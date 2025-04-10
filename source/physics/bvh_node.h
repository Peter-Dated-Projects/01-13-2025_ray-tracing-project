
#ifndef bvh_node_h
#define bvh_node_h

#include "utils/common.h"
#include "physics/hittable.h"

// ----------------------------------------------------- //
// bvh_node
// ----------------------------------------------------- //

class bvh_node : public hittable {
private:

    shared_ptr<std::vector<shared_ptr<hittable>>> _objects;
    shared_ptr<std::vector<int>> _relevant_objects;

    std::vector<shared_ptr<bvh_node>> _children;
    aabb _detection_box;
    int _depth;
    bool is_leaf;


public: 
    double _distance_to_camera;

    bvh_node(): is_leaf(false), _depth(0), _objects(nullptr), _distance_to_camera(0.0) {
        calculate_bounding_box();
    }
    bvh_node(const aabb detection_box, const shared_ptr<std::vector<shared_ptr<hittable>>> objects, int depth, int max_depth, point3 camera_pos): 
        is_leaf(depth == max_depth), _depth(depth), _detection_box(detection_box) {
        
        // set the distance to camera
        _distance_to_camera = vec3::distance_to(camera_pos, detection_box.center());

        // keep a reference to the objects
        _objects = objects;

        // calcualte relevant objects
        _relevant_objects = make_shared<std::vector<int>>();
        for (int i = 0; i < _objects->size(); i++) {
            if (_detection_box.contains(_objects->at(i)->bounding_box.center())) {
                _relevant_objects->push_back(i);
            }
        }

        // calculate bounding box
        calculate_bounding_box();

        // if no relevant objects, return
        if (_relevant_objects->size() == 0) {
            is_leaf = true;
        }
        if (is_leaf) {
            // TODO - don't create child nodes -> can skip the rest of the code
            // output # of relevant objects
            std::cout << "Leaf node at depth: " << depth << " / " << max_depth << " with " << _relevant_objects->size() << " relevant objects." << std::endl;
            std::cout << "Bounding box: " << bounding_box << std::endl;
            return;
        }
        
        // ------------------------------------------------------- //
        // only leaf type nodes run following code
        // create children -- 2 children -- split longest axis in half

        // find longest axis
        vec3 size = _detection_box.size();
        int axis = 0;
        if (size.y() > size.x()) {
            axis = 1;
        }
        if (size.z() > size.y()) {
            axis = 2;
        }

        // create 2 sub aabb's that split the longest axis in half
        aabb child1_box;
        aabb child2_box;
        if (axis == 0) {
            child1_box.set_min(bounding_box.min());
            child1_box.set_max(vec3(bounding_box.max().x(), bounding_box.max().y(), bounding_box.center().z()));

            child2_box.set_min(vec3(bounding_box.center().x(), bounding_box.min().y(), bounding_box.min().z()));
            child2_box.set_max(bounding_box.max());
        }
        else if (axis == 1) {
            child1_box.set_min(bounding_box.min());
            child1_box.set_max(vec3(bounding_box.max().x(), bounding_box.center().y(), bounding_box.max().z()));

            child2_box.set_min(vec3(bounding_box.min().x(), bounding_box.center().y(), bounding_box.min().z()));
            child2_box.set_max(bounding_box.max());
        }
        else {
            child1_box.set_min(bounding_box.min());
            child1_box.set_max(vec3(bounding_box.max().x(), bounding_box.max().y(), bounding_box.center().z()));

            child2_box.set_min(vec3(bounding_box.min().x(), bounding_box.min().y(), bounding_box.center().z()));
            child2_box.set_max(bounding_box.max());
        }
        
        // create shared pointer of relevant objects
        // std::cout << "Creating BVH Node at depth: " << depth << " / " << max_depth << std::endl;
        std::vector<shared_ptr<hittable>> _child_objects;
        for (int i = 0; i < _relevant_objects->size(); i++) {
            _child_objects.push_back(_objects->at(_relevant_objects->at(i)));

            // print out data for debugging
            // std::cout << "child object [" << i << "]: " << _child_objects[i]->bounding_box.min() << " " << _child_objects[i]->bounding_box.max() << std::endl;
        }
        shared_ptr<std::vector<shared_ptr<hittable>>> _child_objects_ptr = make_shared<std::vector<shared_ptr<hittable>>>(_child_objects);

        // create children
        bvh_node* child1 = new bvh_node(child1_box, _child_objects_ptr, depth + 1, max_depth, camera_pos);
        bvh_node* child2 = new bvh_node(child2_box, _child_objects_ptr, depth + 1, max_depth, camera_pos);

        // add children to vector
        _children.push_back(shared_ptr<bvh_node>(child1));
        _children.push_back(shared_ptr<bvh_node>(child2));

    }
    ~bvh_node() {
        _children.clear();
    }

    // ----------------------------------------------------- //
    // logic
    // ----------------------------------------------------- //

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        // TODO : implement this function
        return bounding_box.intersect(r, ray_t);
    }

    void calculate_bounding_box() override {
        // TODO : implement this function
        if (_relevant_objects == nullptr || _relevant_objects->size() < 1) {
            return;
        }

        // calculate min + max of area
        vec3 min(1e9, 1e9, 1e9);
        vec3 max(-1e9, -1e9, -1e9);

        for (int i = 0; i < _relevant_objects->size(); i++) {
            // get the object
            auto obj = _objects->at(_relevant_objects->at(i));
            // get the bounding box
            aabb box = obj->bounding_box;
            min = vec3::min(min, box.min());
            max = vec3::max(max, box.max());
        }

        // set bounding box
        bounding_box.set_min(min);
        bounding_box.set_max(max);
    }

    std::vector<shared_ptr<bvh_node>> get_intersecting_nodes(const ray& r, interval ray_t, hit_record& rec) const {
        std::vector<shared_ptr<bvh_node>> result;

        // look into children but never enter children
        for (shared_ptr<bvh_node> child : _children) {
            // check if child is leaf
            if (child->is_leaf_node()) {
                // check if intersection
                if (child->hit(r, ray_t, rec)) {                    
                    // add child to result
                    result.push_back(child);
                }
                // if not collision pass
            } 
            // if not leaf
            else {
                // check if child intersects with ray
                if (child->hit(r, ray_t, rec)) {
                    // recurse into child
                    std::vector<shared_ptr<bvh_node>> temp = child->get_intersecting_nodes(r, ray_t, rec);
                    // add all children to result
                    result.insert(result.end(), temp.begin(), temp.end());
                }
                // else continue
            }
        }

        // return results
        return result;
        
    }

    // ----------------------------------------------------- //
    // getters
    // ----------------------------------------------------- //
    aabb get_bounding_box() const { return _detection_box; }
    shared_ptr<std::vector<int>> get_relevant_objects() const { return _relevant_objects; }
    bool is_leaf_node() const { return is_leaf; }
};


#endif