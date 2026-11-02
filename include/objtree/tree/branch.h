#pragma once

#include <objtree/obj/circle.h>

#include <vector>
#include <memory>

#include <objtree/obj/face.h>
#include <objtree/obj/mesh.h>

struct Branch
{
    Circle base;
    Circle end;
    float (*func_chage_radius)(float x) = nullptr;
    // std::unique_ptr<Branch> prev = std::make_unique<Branch>(nullptr);
    // std::vector<std::unique_ptr<Branch>> child;

    Mesh create_mesh();

    protected:
    std::list<Face> get_branch_faces(std::size_t downStart, std::size_t upStart);
    // std::list<Face> get_branch_faces(std::size_t downStart, std::size_t downCount, std::size_t upStart, std::size_t upCount);
};
