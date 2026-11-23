#pragma once
#define _USE_MATH_DEFINES

#include <objtree/tree/branch.h>
#include <objtree/math/matrix3d.h>
#include <objtree/obj/mesh.h>

#include <iostream>
#include <random>
#include <memory>
#include <cmath>
#include <list>

namespace {

    std::mt19937 gen;
    std::random_device rd;

    constexpr float POINT_PER_RADIUS = 60;
    constexpr int COUNT_OF_BRANCH_MIN = 2;
    constexpr int COUNT_OF_BRANCH_MAX = 2;
    constexpr float COEFF_LEN_BY_RADIUS = 30.f;
    constexpr float COEFF_SEPARATION_M = 0.5f;
    constexpr float COEFF_SEPARATION_D = 0.3f;
    constexpr float RADIUS_MAX = 2.5f;
    constexpr float RADIUS_MIN = 3.f / POINT_PER_RADIUS;
    constexpr float RADIUS_ONE_POINT = 1.f / POINT_PER_RADIUS;
    constexpr int COEFF_COUNT_FLEX = 5;
    constexpr int COEFF_COUNT_SEPARATE = 6;

    template <class T>
    T limit_normal_distribution(T m, T d, T min , T max) {
        std::normal_distribution<T> normal_real(m, d);
        auto number = normal_real(gen);
        if (number < min) {
            number = min;
        }
        if (number > max) {
            number = max;
        }
        return number;
    }

    float len_by_radius(float r) {
        return r*COEFF_LEN_BY_RADIUS;
    }

    float radius_at_height(float height, float downRadius) {
        float heightMax = downRadius*COEFF_LEN_BY_RADIUS;
        return (heightMax - height) * downRadius / heightMax;
    }

    float separate_at(float r) {
        (void) r;
        // return 0.2f;
        // float m = (r-static_cast<float>(RADIUS_MAX))*0.35f/static_cast<float>(RADIUS_MIN - RADIUS_MAX) + 0.2f;
        return limit_normal_distribution(
            COEFF_SEPARATION_M, 
            COEFF_SEPARATION_D, 
            COEFF_SEPARATION_M+COEFF_SEPARATION_D,
            COEFF_SEPARATION_M-COEFF_SEPARATION_D
        );
    }

    // int count_of_flex(float r) {
    //     float branchLvl = 1.f - r/RADIUS_MAX;
    //     int m = static_cast<int>(branchLvl * COEFF_COUNT_FLEX);
    //     return limit_normal_distribution(m, 1, 0, COEFF_COUNT_FLEX);
    // }

    // Vector3d direction_flex(float coeff = 1.f) {
    //     Vector3d v;
    //     float PI = 3.141592f;
    //     float az = limit_normal_distribution(0.f, PI/4.f, -PI, PI);
    //     float ap = limit_normal_distribution(0.f, PI/6.f, -PI/4.f, PI/4.f);
    //     v.x = std::cos(az);
    //     v.z = std::sin(az);
    //     v.y = std::cos(ap);
    //     return v*coeff;
    // }

    std::list<Circle> get_child_circle(float radius) {
        std::list<Circle> circles;
        float count_branch_float = limit_normal_distribution(2.f, 2.f, 1.f, 4.f);
        int count_branch = static_cast<int>(count_branch_float);
        for (int i = 0; i < count_branch; i++)
        {
            Circle c;
            c.pointsPerRadius = POINT_PER_RADIUS;
            c.radius = radius / std::sqrt(static_cast<float>(count_branch));
            circles.emplace_back(c);
        }
        return circles;
    }

    // float volume_of_cone(float r) {
    //     return len_by_radius(r) * 2*M_PI*r;
    // }

    

}

class ObjTree
{
private:

    // Branch root;
    unsigned int seed;

public:
    ObjTree() = default;
    ~ObjTree() = default;

    Mesh grow_tree() {
        seed = rd();
        std::cout << "Seed: " << seed << "\n";
        gen.seed(seed);
        std::vector<Branch*> stack;

        std::unique_ptr<Branch> root = std::make_unique<Branch>();
        const float START_RADIUS = 2.f;
        root->base = Circle(START_RADIUS, POINT_PER_RADIUS);
        root->end = std::make_unique<Circle>();

        const float lenR = len_by_radius(START_RADIUS) * separate_at(START_RADIUS);
        bool lastBranchStart = false;
        float endRadius = radius_at_height(lenR, START_RADIUS);
        if (endRadius < RADIUS_MIN) {
            endRadius = RADIUS_ONE_POINT;
            lastBranchStart = true;
        }
        root->end->radius = endRadius;
        root->end->pointsPerRadius = POINT_PER_RADIUS;
        root->end->center = root->base.center + root->base.get_normal() * lenR;
        root->end->set_normal(root->base.get_normal());   // явно

        if (!lastBranchStart) {
            stack.push_back(root.get());
        }

        const float PI = static_cast<float>(M_PI);

        int sep = 1;

        while (!stack.empty())
        {
            Branch* node = stack.back();
            stack.pop_back();

            std::list<Circle> circleBranches = get_child_circle(node->end->radius);
            const float countF = static_cast<float>(circleBranches.size());

            std::cout << "sep at " << sep << " - " << circleBranches.size() << "\n";
            sep++;

            float index = 0.f;
            for (const Circle& circle : circleBranches)
            {
                auto next = std::make_unique<Branch>();
                next->base = *(node->end);
                next->end = std::make_unique<Circle>(circle);

                const float len = len_by_radius(next->base.radius) * separate_at(next->base.radius);
                Vector3d branchDir;
                bool lastBranch = false;

                if (circleBranches.size() == 1) {
                    branchDir = next->base.get_normal();
                    next->end->radius = radius_at_height(len, next->base.radius);
                } else {
                    matrix::Matrix3d B = matrix::get_basis(next->base.get_normal());
                    Vector3d i { B.m[0], B.m[3], B.m[6] };
                    Vector3d j { B.m[1], B.m[4], B.m[7] };
                    Vector3d k { B.m[2], B.m[5], B.m[8] };

                    const float theta = PI / (countF * 2.f);
                    const float s = std::sin(theta);
                    const float c = std::cos(theta);
                    const float phi = 2.f * PI * index / countF;

                    branchDir = i * (s * std::cos(phi)) + j * c + k * (s * std::sin(phi));
                }

                next->end->center = next->base.center + branchDir * len;

                // МАЛЕНЬКАЯ симметричная случайность вокруг направления ветки.
                // Было: base.normal + direction_flex(large) — перекос вверх + разрыв меша.
                const float spread = 0.2f;   // подобрать по вкусу: 0.2 — мягко, 0.6 — хаотично
                Vector3d perturb {
                    limit_normal_distribution(0.f, spread, -1.f, 1.f),
                    limit_normal_distribution(0.f, spread, -1.f, 1.f),
                    limit_normal_distribution(0.f, spread, -1.f, 1.f)
                };
                next->end->set_normal(branchDir + perturb);

                if (next->end->radius < RADIUS_MIN) {
                    next->end->radius = RADIUS_ONE_POINT;
                    std::cout << "It`s last!\n";
                    lastBranch = true;
                }

                ++index;
                node->child.push_back(std::move(next));

                if (!lastBranch) {
                    stack.push_back(node->child.back().get());
                }
            }
            node->separate_branch(COEFF_COUNT_SEPARATE, 1.2f);
        }
        return root->create_mesh();
    }
};

namespace objtree {

/// Returns a short greeting identifying this project.
[[nodiscard]] const char* greeting() noexcept;

} // namespace objtree
