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

    inline Matrix3d get_basis(Vector3d j) {
        Vector3d i, k;
        Matrix3d basis;

        j = j.norm();

        Vector3d i0 {1.f, 0.f, 0.f};
        if (std::abs(i0.dot(j)) > 0.9f) i0 = {0.f, 1.f, 0.f};
        if (std::abs(i0.dot(j)) > 0.9f) i0 = {0.f, 0.f, 1.f};

        Vector3d buffer = i0 - j * (i0.dot(j));
        i = buffer / buffer.len();
        k = i.vec(j);

        basis.m[0] = i.x; basis.m[3] = i.y; basis.m[6] = i.z;
        basis.m[1] = j.x; basis.m[4] = j.y; basis.m[7] = j.z;
        basis.m[2] = k.x; basis.m[5] = k.y; basis.m[8] = k.z;
        return basis;
    }
}

inline Vector3d parallel_transport(const Vector3d& x,
                                   const Vector3d& fromN,
                                   const Vector3d& toN)
{
    Vector3d axis = fromN.vec(toN);
    float s = axis.len();
    float c = std::clamp(fromN.dot(toN), -1.f, 1.f);

    if (s < 1e-6f) {
        // fromN и toN коллинеарны — поворот не нужен или 180°
        if (c > 0.f) {
            // одинаковые — просто проекция (страховка)
            Vector3d xp = x - toN * x.dot(toN);
            return xp.norm();
        } else {
            // противоположные — поворот на 180° вокруг любой перпендикулярной оси
            Vector3d ref {1.f, 0.f, 0.f};
            if (std::abs(ref.dot(fromN)) > 0.9f) ref = {0.f, 1.f, 0.f};
            Vector3d rotAxis = (ref - fromN * ref.dot(fromN)).norm();
            Vector3d xRot = rotAxis * (2.f * rotAxis.dot(x)) - x;
            return xRot.norm();
        }
    }

    axis = axis / s;
    float sn = std::sqrt(std::max(0.f, 1.f - c*c));

    // Rodrigues: x' = x·c + (axis × x)·sn + axis·(axis·x)·(1−c)
    Vector3d xRot = x * c
                  + axis.vec(x) * sn
                  + axis * (axis.dot(x) * (1.f - c));

    // На всякий случай — снова проекция на плоскость ⟂ toN
    Vector3d xp = xRot - toN * xRot.dot(toN);
    return xp.norm();
}