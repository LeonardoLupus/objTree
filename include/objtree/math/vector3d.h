#pragma once

#include <cmath>

struct Vector3d
{
    float x=0.f, y=0.f, z=0.f;

    float dot(const Vector3d& v) const {
        return x*v.x + y*v.y + z*v.z;
    }
    Vector3d vec(const Vector3d& v) const {
        return {y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x};
    }
    float len() const {
        return std::sqrt(dot(*this));
    }
    Vector3d norm() const {
        float n = len();
        return {x/n, y/n, z/n};
    }
    Vector3d operator-(const Vector3d& v) const {
        return {x-v.x, y-v.y, z-v.z};
    }
    Vector3d operator+(const Vector3d& v) const {
        return {x+v.x, y+v.y, z+v.z};
    }
    Vector3d operator/(float a) const {
        return {x/a, y/a, z/a};
    }
    Vector3d operator*(float t) const {
        return {x*t, y*t, z*t};
    }
};
