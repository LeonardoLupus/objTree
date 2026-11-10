#pragma once

#include <objtree/obj/circle.h>

#include <vector>
#include <memory>

#include <objtree/obj/face.h>
#include <objtree/obj/mesh.h>

struct Branch
{
    Branch() = default;
    Branch(Branch&&) = default;              // можно перемещать
    Branch& operator=(Branch&&) = default;

    Branch(const Branch&) = delete;           // копирование запрещено
    Branch& operator=(const Branch&) = delete;
    
    Circle base;
    std::unique_ptr<Circle> end;
    std::vector<std::unique_ptr<Branch>> child;

    Mesh create_mesh();

    std::list<Face> get_branch_faces(std::size_t downStart, std::size_t upStart) const;
};
