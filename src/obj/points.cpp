#define _USE_MATH_DEFINES
#include <objtree/obj/points.h>

#include <cmath>

std::list<Vector3d> create::circle_of_point(float radius, float pointPerRadius) {
    std::list<Vector3d> points;
    if (radius*pointPerRadius < 3) {
        points.push_back({0.f, 0.f, 0.f});
        return points;
    }
    for (float i = 0.f; i < radius*pointPerRadius; i+=1.f)
    {
        points.emplace_back(Vector3d{
            radius*static_cast<float>(cos(i/(radius*pointPerRadius) * 2*M_PI)),
            0.f,
            radius*static_cast<float>(sin(i/(radius*pointPerRadius) * 2*M_PI))
        });
    }
    return points;
}
