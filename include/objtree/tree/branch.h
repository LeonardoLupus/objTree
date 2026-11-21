#pragma once

#include <objtree/obj/circle.h>

#include <list>
#include <memory>

#include <objtree/obj/face.h>
#include <objtree/obj/mesh.h>

struct Branch
{
    Branch() = default;
    Branch(Branch&&) noexcept;              // можно перемещать
    Branch& operator=(Branch&&) noexcept;

    Branch(const Branch&) = delete;           // копирование запрещено
    Branch& operator=(const Branch&) = delete;
    
    Circle base;
    std::unique_ptr<Circle> end;
    std::list<std::unique_ptr<Branch>> child;

    void separate_branch(int count, float tangentScale = 1.0f);

    Mesh create_mesh();

    std::list<Face> get_branch_faces(std::size_t downStart, std::size_t upStart) const;
};
