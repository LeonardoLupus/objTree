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

    constexpr float POINT_PER_RADIUS = 20;
    constexpr int COUNT_OF_BRANCH_MIN = 1;
    constexpr int COUNT_OF_BRANCH_MAX = 1;
    constexpr float COEFF_LEN_BY_RADIUS = 30.f;
    constexpr float COEFF_SEPARATION_M = 0.5f;
    constexpr float COEFF_SEPARATION_D = 0.1f;
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

    // int count_of_branches() {
    //     std::uniform_int_distribution<> rangeRandom(COUNT_OF_BRANCH_MIN, COUNT_OF_BRANCH_MAX);
    //     return rangeRandom(gen);
    // }

    float len_by_radius(float r) {
        return r*COEFF_LEN_BY_RADIUS;
    }

    float radius_at_height(float height, float downRadius) {
        float heightMax = downRadius*COEFF_LEN_BY_RADIUS;
        return (heightMax - height) * downRadius / heightMax;
    }

    // float radius_by_len(float len) {
    //     return len/COEFF_LEN_BY_RADIUS;
    // }

    // float volume_of_cone(float r) {
    //     return len_by_radius(r) * 2*M_PI*r;
    // }

    // float separate_at(float r) {
    //     float m = (r-static_cast<float>(RADIUS_MAX))*0.45/static_cast<float>(RADIUS_MIN - RADIUS_MAX) + 0.5f;
    //     return limit_normal_distribution(m, 0.1f, 0.5f, 0.95f);
    // }

    // int count_of_flex(float r) {
    //     float branchLvl = 1.f - r/RADIUS_MAX;
    //     int m = static_cast<int>(branchLvl * COEFF_COUNT_FLEX);
    //     return limit_normal_distribution(m, 1, 0, COEFF_COUNT_FLEX);
    // }

}

class ObjTree
{
private:

    // Branch root;
    std::size_t seed;

public:
    ObjTree() = default;
    ~ObjTree() = default;

    Mesh grow_tree() {
        seed = rd();
        gen.seed(42);
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
            // len *= separate_at(r);
            len *= 0.2f;

            bool lastBranch = false;
            float endRadius = radius_at_height(len, r);
            if(endRadius < RADIUS_MIN) {
                endRadius = 1.f/static_cast<float>(POINT_PER_RADIUS);
                lastBranch = true;
            }
            endBranch.radius = endRadius;
            endBranch.pointsPerRadius = POINT_PER_RADIUS;
            endBranch.center = base.center + base.get_normal()*len;
            // endBranch.set_normal(...)

            

            if(not lastBranch) {
                std::unique_ptr<Branch> next = std::make_unique<Branch>();
                next->base = endBranch;

                Branch* nextPtr = next.get();
                node->child.push_back(std::move(next));
                stack.push_back(nextPtr);
            }



            // int branches = count_of_branches();
            // float r = std::sqrt(node.get(). / branches)
            // for (int i = 0; i < branches; i++)
            // {
                
            // }
            
        }
        Mesh obj;
        const Branch* b = root.get();
        obj.add_vertices(b->base.get_points());
        while (b)
        {
            obj.add_vertices(b->end->get_points());
            obj.add_faces(
                b->get_branch_faces(
                    obj.countVertices() - b->base.get_count_of_points() - b->end->get_count_of_points() + 1,
                    obj.countVertices() - b->end->get_count_of_points() + 1
                )
            );

            if (b->child.empty()) {
                break;
            }

            b = b->child.back().get();
        }
        return obj;
    }
};

namespace objtree {

/// Returns a short greeting identifying this project.
[[nodiscard]] const char* greeting() noexcept;

} // namespace objtree
