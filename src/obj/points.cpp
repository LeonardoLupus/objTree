#define _USE_MATH_DEFINES
#include <objtree/obj/points.h>

#include <cmath>

std::list<Vector3d> create::circle_of_point(float radius, float pointPerRadius) {
    std::list<Vector3d> points;
    int countPoints = static_cast<int>(radius*pointPerRadius);
    if (countPoints < 3) {
        points.push_back({0.f, 0.f, 0.f});
        return points;
    }

    for (int i = 0; i < countPoints; i++)
    {
        points.emplace_back(Vector3d{
            radius*static_cast<float>(cos(static_cast<float>(i)/static_cast<float>(countPoints) * 2*M_PI)),
            0.f,
            radius*static_cast<float>(sin(static_cast<float>(i)/static_cast<float>(countPoints) * 2*M_PI))
        });
    }
    return points;
}
