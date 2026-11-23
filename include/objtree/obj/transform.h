#pragma once

#include <list>

#include <objtree/math/vector3d.h>
#include <objtree/math/matrix3d.h>

namespace transform {

    void shift(std::list<Vector3d>& points, Vector3d n) {
        for (auto& point : points)
        {
            point = point + n;
        }
    }
}