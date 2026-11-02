#pragma once

#include <objtree/obj/circle.h>

#include <vector>
#include <memory>

#include <objtree/obj/face.h>
#include <objtree/obj/mesh.h>

struct Branch
{
    Circle base;
    std::unique_ptr<Circle> end;
    float (*func_chage_radius)(float x) = nullptr;
    std::vector<std::unique_ptr<Branch>> child;

    Mesh create_mesh();

    protected:
    std::list<Face> get_branch_faces(std::size_t downStart, std::size_t upStart);
};
