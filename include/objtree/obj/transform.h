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

    void rotate(std::list<Vector3d>& points, Vector3d n) {
        n = n.norm();
        Vector3d axis {0.f, 1.f, 0.f};
        if ((axis.vec(n)).len() < 1e-6f) {
            return;
        }
        matrix::Matrix3d spin = matrix::get_basis(n);
        for (auto& point : points)
        {
            point = spin*point;
        }
    }
}