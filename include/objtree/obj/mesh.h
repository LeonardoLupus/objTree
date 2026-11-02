#pragma once

#include <list>
#include <vector>
#include <string>

#include <objtree/math/vector3d.h>
#include <objtree/obj/face.h>

class Mesh
{
private:
    std::string name = "mesh";
    std::size_t idxVerticeStart = 1;
    std::list<Vector3d> vertices;
    std::list<Face> faces;

public:
    Mesh(std::string lable="mesh", std::size_t startIndex=1): name(lable), idxVerticeStart(startIndex) {}
    ~Mesh() = default;

    std::size_t countVertices();
    void add_vertices(const std::list<Vector3d>& v);
    void add_faces(const std::list<Face>& f);
    bool save();
};
