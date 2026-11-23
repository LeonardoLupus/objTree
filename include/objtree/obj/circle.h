#pragma once

#include <list>

#include <objtree/math/vector3d.h>
#include <objtree/math/matrix3d.h>

class Circle
{
private:
    Vector3d normal = {0.f, 1.f, 0.f};
    Vector3d xAxis  = {1.f, 0.f, 0.f};
public:
    Vector3d center = {0.f, 0.f, 0.f};
    float radius = 0.f;
    float pointsPerRadius = 0.f;

    Circle() = default;
    ~Circle() = default;
    Circle(float r, float pr): radius(r), pointsPerRadius(pr) {}
    void set_normal(const Vector3d& n);                       // пересчитать xAxis через get_basis
    void set_frame (const Vector3d& n, const Vector3d& x);    // задать нормаль и xAxis явно
    Vector3d get_normal() const;
    Vector3d get_x_axis() const { return xAxis; }             // ← новое
    matrix::Matrix3d get_frame() const;

    std::list<Vector3d> get_points() const;
    std::size_t get_count_of_points() const;
};

