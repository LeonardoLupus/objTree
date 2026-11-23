#pragma once
#define _USE_MATH_DEFINES

#include <objtree/tree/branch.h>
#include <objtree/math/matrix3d.h>
#include <objtree/obj/mesh.h>

#include <iostream>
#include <algorithm>
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

        // 1) Число детей 1..4 с весами: 2 чаще всего, 4 редко
        std::discrete_distribution<int> countDist({10, 55, 25, 10});
        const int count = countDist(gen) + 1;   // 1..4

        if (count <= 1) {
            Circle c;
            c.pointsPerRadius = POINT_PER_RADIUS;
            c.radius = radius;   // grow_tree перезапишет через radius_at_height
            circles.emplace_back(c);
            return circles;
        }

        // 2) Относительная толщина родителя, 0 = тонкий, 1 = толстый
        const float rel = std::clamp(
            (radius - RADIUS_MIN) / (RADIUS_MAX - RADIUS_MIN),
            0.f, 1.f);

        // 3) Разброс весов:
        //    толстый (rel→1): spread ≈ 0.20 → дети почти равные
        //    тонкий  (rel→0): spread ≈ 2.0  → один может быть в ~3 раза толще
        const float spread = 0.20f * rel + 2.0f * (1.f - rel);

        // 4) Сырые веса c_i ∈ [1, 1+spread]
        std::uniform_real_distribution<float> uni(0.f, 1.f);
        std::vector<float> c(static_cast<std::size_t>(count));
        float sumSq = 0.f;
        for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i) {
            c[i] = 1.f + uni(gen) * spread;
            sumSq += c[i] * c[i];
        }

        // 5) Нормировка: Σ a_i² = 1  ⇒  Σ r_i² = r²
        const float norm = std::sqrt(sumSq);
        for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i) {
            Circle circle;
            circle.pointsPerRadius = POINT_PER_RADIUS;
            circle.radius = radius * (c[i] / norm);
            circles.emplace_back(circle);
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
        root->end  = std::make_unique<Circle>();

        const float lenR = len_by_radius(START_RADIUS) * separate_at(START_RADIUS);
        bool lastBranchStart = false;
        float endRadius = radius_at_height(lenR, START_RADIUS);
        if (endRadius < RADIUS_MIN) { endRadius = RADIUS_ONE_POINT; lastBranchStart = true; }

        root->end->radius = endRadius;
        root->end->pointsPerRadius = POINT_PER_RADIUS;
        root->end->center = root->base.center + root->base.get_normal() * lenR;
        root->end->set_frame(root->base.get_normal(), root->base.get_x_axis());

        if (!lastBranchStart) stack.push_back(root.get());

        const float PI = static_cast<float>(M_PI);

        // --- параметры случайностей ---
        const float spread        = 0.08f;              // тангенциальный perturb нормали
        const float thetaJitter   = PI / 12.f;          // ±15° на угол от оси
        const float dominanceK    = 1.2f;               // сила "тонкая уходит вбок"
        const float thetaMin      = PI * 0.02f;         // нижняя граница угла
        const float thetaMax      = PI * 0.45f;         // верхняя (≈81°)

        std::uniform_real_distribution<float> phaseDist(0.f, 2.f * PI);

        while (!stack.empty())
        {
            Branch* node = stack.back();
            stack.pop_back();

            std::list<Circle> circleBranches = get_child_circle(node->end->radius);
            const float countF = static_cast<float>(circleBranches.size());

            // Одна случайная фаза на узел — поворачивает плоскость кольца вокруг baseN
            const float phi0 = phaseDist(gen);

            float index = 0.f;
            for (const Circle& circle : circleBranches)
            {
                auto next = std::make_unique<Branch>();
                next->base = *(node->end);
                next->end  = std::make_unique<Circle>(circle);

                const float len = len_by_radius(next->base.radius) * separate_at(next->base.radius);
                Vector3d branchDir;
                Vector3d newNormal;
                bool lastBranch = false;

                const Vector3d baseN = next->base.get_normal();
                const Vector3d baseX = next->base.get_x_axis();

                if (countF == 1.f) {
                    branchDir = baseN;
                    next->end->radius = radius_at_height(len, next->base.radius);
                    newNormal = baseN;
                } else {
                    matrix::Matrix3d B = matrix::get_basis(baseN);
                    Vector3d i { B.m[0], B.m[3], B.m[6] };
                    Vector3d j { B.m[1], B.m[4], B.m[7] };
                    Vector3d k { B.m[2], B.m[5], B.m[8] };

                    const float baseTheta = PI / (2.f * countF);

                    // Эталон доминантности = 1/√N (при равном сплите dominance = refDominance)
                    const float refDominance = 1.f / std::sqrt(countF);

                    const float dominance = std::clamp(
                        circle.radius / next->base.radius, 0.f, 1.f);

                    const float thetaScale = 1.f + dominanceK * (refDominance - dominance);

                    const float thetaNoise = limit_normal_distribution(
                        0.f, thetaJitter, -2.f*thetaJitter, 2.f*thetaJitter);

                    float localTheta = baseTheta * thetaScale + thetaNoise;
                    localTheta = std::clamp(localTheta, thetaMin, thetaMax);

                    const float s = std::sin(localTheta);
                    const float c = std::cos(localTheta);
                    const float phi = 2.f * PI * index / countF + phi0;

                    branchDir = i * (s * std::cos(phi)) + j * c + k * (s * std::sin(phi));

                    matrix::Matrix3d B2 = matrix::get_basis(branchDir);
                    Vector3d bi { B2.m[0], B2.m[3], B2.m[6] };
                    Vector3d bk { B2.m[2], B2.m[5], B2.m[8] };
                    Vector3d perturb =
                        bi * limit_normal_distribution(0.f, spread, -3.f*spread, 3.f*spread) +
                        bk * limit_normal_distribution(0.f, spread, -3.f*spread, 3.f*spread);

                    newNormal = (branchDir + perturb).norm();
                }

                next->end->center = next->base.center + branchDir * len;

                Vector3d newX = parallel_transport(baseX, baseN, newNormal);
                next->end->set_frame(newNormal, newX);

                if (next->end->radius < RADIUS_MIN) {
                    next->end->radius = RADIUS_ONE_POINT;
                    lastBranch = true;
                }

                ++index;
                node->child.push_back(std::move(next));
                if (!lastBranch) stack.push_back(node->child.back().get());
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
