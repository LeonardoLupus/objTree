#include <objtree/obj/circle.h>
#include <objtree/obj/points.h>
#include <objtree/obj/transform.h>

void Circle::set_normal(const Vector3d& n) {
    normal = n.norm();
}

Vector3d Circle::get_normal() const {
    return normal;
}

std::list<Vector3d> Circle::get_points() const {
    std::list<Vector3d> points = create::circle_of_point(radius, pointsPerRadius);
    transform::rotate(points, normal);
    transform::shift(points, center);
    return points;
}

std::size_t Circle::get_count_of_points() const {
    return static_cast<std::size_t>(radius * pointsPerRadius);
}
