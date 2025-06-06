#ifndef common_h
#define common_h

#include <cmath>
#include <iostream>
#include <cstdlib>
#include <limits>
#include <memory>
#include <vector>
#include <unordered_set>


// C++ Std Usings

using std::make_shared;
using std::shared_ptr;

// Constants

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility Functions

inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

inline double random_double() {
    // Returns a random double in [0, 1]
    return std::rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max) { 
    // Returns a random double in [min, max]
    return min + (max - min) * random_double();
}


// Common Headers

#include "utils/color.h"
#include "utils/utils.h"


#include "math/ray.h"
#include "math/vec3.h"
#include "math/interval.h"
#include "math/area.h"
#include "math/aabb.h"


#endif