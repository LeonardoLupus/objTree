#pragma once

#include <list>

#include <objtree/math/vector3d.h>

namespace create {
    std::list<Vector3d> circle_of_point(float radius, float pointPerRadius);
}