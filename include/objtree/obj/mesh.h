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
    std::list<Vector3d> vertices;
    std::list<Face> faces;

public:
    Mesh(std::string lable="mesh"): name(lable) {}
    ~Mesh() = default;

    std::size_t countVertices();
    const std::list<Vector3d>& get_vertices() const;
    const std::list<Face>& get_faces() const;
    void add_vertices(const std::list<Vector3d>& v);
    void add_faces(const std::list<Face>& f, std::size_t startIdx = 0);
    void add_mesh(const Mesh& m);
    bool save();
};
