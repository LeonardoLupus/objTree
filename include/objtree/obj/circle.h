#pragma once

#include <list>

#include <objtree/math/vector3d.h>

class Circle
{
private:
    Vector3d normal = {0.f, 1.f, 0.f};
public:
    Vector3d center = {0.f, 0.f, 0.f};
    float radius = 0.f;
    float pointsPerRadius = 0.f;

    Circle() = default;
    ~Circle() = default;
    Circle(float r, float pr): radius(r), pointsPerRadius(pr) {}
    void set_normal(const Vector3d& n);
    std::list<Vector3d> get_points();
    std::size_t get_count_of_points();
};

