#pragma once
#define _USE_MATH_DEFINES

#include <objtree/tree/branch.h>

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
    constexpr float COEFF_SEPARATION_MIN = 0.1f;
    constexpr float COEFF_SEPARATION_MAX = 0.5f;
    constexpr float RADIUS_MAX = 2.5f;
    constexpr float RADIUS_MIN = 3 / POINT_PER_RADIUS;
    constexpr int COEFF_COUNT_FLEX = 5;

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
        return 0.2f;
        // float m = (r-static_cast<float>(RADIUS_MAX))*0.35f/static_cast<float>(RADIUS_MIN - RADIUS_MAX) + 0.2f;
        return limit_normal_distribution(0.2f, 0.1f, 0.1f, 0.3f);
    }

    // int count_of_flex(float r) {
    //     float branchLvl = 1.f - r/RADIUS_MAX;
    //     int m = static_cast<int>(branchLvl * COEFF_COUNT_FLEX);
    //     return limit_normal_distribution(m, 1, 0, COEFF_COUNT_FLEX);
    // }

    Vector3d direction_flex(float coeff = 1.f) {
        Vector3d v;
        float PI = 3.141592f;
        float az = limit_normal_distribution(0.f, PI/4.f, -PI, PI);
        float ap = limit_normal_distribution(0.f, PI/6.f, -PI/4.f, PI/4.f);
        v.x = std::cos(az);
        v.z = std::sin(az);
        v.y = std::cos(ap);
        return v*coeff;
    }

    // int count_of_branches() {
    //     std::uniform_int_distribution<> rangeRandom(COUNT_OF_BRANCH_MIN, COUNT_OF_BRANCH_MAX);
    //     return rangeRandom(gen);
    // }

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
        std::cout << "Seed: " << seed <<"\n";
        gen.seed(seed);
        std::vector<Branch*> stack;

        std::unique_ptr root = std::make_unique<Branch>();
        root->base = Circle(1.f, POINT_PER_RADIUS);

        stack.push_back(root.get());
        while(not stack.empty()) {
            auto node = stack.back();
            stack.pop_back();

            node->end = std::make_unique<Circle>();
            Circle& base = node->base;
            Circle& endBranch = *(node->end);
            

            float r = base.radius;
            float len = len_by_radius(r);
            float s = separate_at(r);
            len *= s;

            bool lastBranch = false;
            float endRadius = radius_at_height(len, r);
            if(endRadius < RADIUS_MIN) {
                endRadius = 1.f/static_cast<float>(POINT_PER_RADIUS);
                lastBranch = true;
            }
            endBranch.radius = endRadius;
            endBranch.pointsPerRadius = POINT_PER_RADIUS;
            endBranch.center = base.center + base.get_normal()*len;
            endBranch.set_normal(base.get_normal());
            endBranch.set_normal(base.get_normal() + direction_flex(1.f/(3*r)));

            if(lastBranch) {continue;}

            node->separate_branch(6, 1.2f);
            while(not node->child.empty()) {
                node = node->child.back().get();
            }

            

            // int countOfFlex = count_of_flex(r);
            // for (int i = 0; i < countOfFlex; i++)
            // {
            //     /* code */
            // }

            std::unique_ptr<Branch> next = std::make_unique<Branch>();
            next->base = endBranch;

            Branch* nextPtr = next.get();
            node->child.push_back(std::move(next));
            stack.push_back(nextPtr);
            std::cout << "Size build: " << stack.size() << "\n";

            // int branches = count_of_branches();
            // float r = std::sqrt(node.get(). / branches)
            // for (int i = 0; i < branches; i++)
            // {
                
            // }
            
        }
        return root->create_mesh();
    }
};

namespace objtree {

/// Returns a short greeting identifying this project.
[[nodiscard]] const char* greeting() noexcept;

} // namespace objtree
