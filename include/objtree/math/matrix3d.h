#pragma once

#include <cmath>
#include <array>

#include <objtree\math\vector3d.h>

namespace matrix {
    struct Matrix3d
    {
        std::array<float, 9> m;
        Vector3d operator*(const Vector3d& v) const {
            return Vector3d{
                m[0]*v.x + m[1]*v.y + m[2]*v.z,
                m[3]*v.x + m[4]*v.y + m[5]*v.z,
                m[6]*v.x + m[7]*v.y + m[8]*v.z
            };
        }
        float operator()(const std::size_t i) const {
            return m.at(i);
        }
        float& operator()(const std::size_t i)  {
            return m.at(i);
        }
    };

    Matrix3d get_basis(Vector3d j) {
        Vector3d i, k;
        Matrix3d basis;
        Vector3d i0 {1.f, 0.f, 0.f};

        j = j.norm();
        
        Vector3d buffer = i0 - (j*(i0.dot(j)));
        i = buffer/buffer.len();
        k = i.vec(j);
        
        basis.m[0] = i.x;
        basis.m[3] = i.y;
        basis.m[6] = i.z;

        basis.m[1] = j.x;
        basis.m[4] = j.y;
        basis.m[7] = j.z;

        basis.m[2] = k.x;
        basis.m[5] = k.y;
        basis.m[8] = k.z;

        return basis;
    }
}