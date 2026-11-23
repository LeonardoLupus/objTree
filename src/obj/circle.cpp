#include <objtree/obj/circle.h>
#include <objtree/obj/points.h>
#include <objtree/obj/transform.h>

Vector3d Circle::get_normal() const {
    return normal;
}

std::size_t Circle::get_count_of_points() const {
    return static_cast<std::size_t>(radius * pointsPerRadius);
}

void Circle::set_normal(const Vector3d& n) {
    normal = n.norm();
    // пересчитать xAxis согласованно, но это только fallback —
    // в grow_tree/separate_branch используется set_frame с транспортом
    Vector3d ref {1.f, 0.f, 0.f};
    if (std::abs(ref.dot(normal)) > 0.9f) ref = {0.f, 0.f, 1.f};
    xAxis = (ref - normal * ref.dot(normal)).norm();
}

void Circle::set_frame(const Vector3d& n, const Vector3d& x) {
    normal = n.norm();
    Vector3d xp = x - normal * x.dot(normal);
    if (xp.len() < 1e-6f) {
        set_normal(normal);           // вырожденный случай — fallback
    } else {
        xAxis = xp.norm();
    }
}

matrix::Matrix3d Circle::get_frame() const {
    matrix::Matrix3d b;
    Vector3d i = xAxis;
    Vector3d j = normal;
    Vector3d k = i.vec(j);
    b.m[0]=i.x; b.m[3]=i.y; b.m[6]=i.z;
    b.m[1]=j.x; b.m[4]=j.y; b.m[7]=j.z;
    b.m[2]=k.x; b.m[5]=k.y; b.m[8]=k.z;
    return b;
}

std::list<Vector3d> Circle::get_points() const {
    std::list<Vector3d> points = create::circle_of_point(radius, pointsPerRadius);
    matrix::Matrix3d spin = get_frame();                 // ← вместо transform::rotate
    for (auto& p : points) { p = spin * p; p = p + center; }
    return points;
}
