#include <objtree/obj/mesh.h>

#include <fstream>
#include <cstdint>

std::size_t Mesh::countVertices() {
    return vertices.size();
}

void Mesh::add_vertices(const std::list<Vector3d>& v) {
    for (const auto& point : v)
    {
        vertices.push_back(point);
    }
}

void Mesh::add_faces(const std::list<Face>& f) {
    for (const auto& tr : f)
    {
        faces.push_back(tr);
    }
}

bool Mesh::save() {
    std::ofstream out(name + ".obj");
    if (!out) {return false;}
    out << "o "<< name <<"\n";
    for(const auto& point : vertices) {
        out << "v " << point.x << " " << point.y << " " << point.z << "\n";
    }
    out << "\n";
    for(const auto& tr : faces) {
        out << "f " << idxVerticeStart + tr(0) -1 << " " << idxVerticeStart + tr(1) -1 << " " << idxVerticeStart + tr(2) -1 << "\n";
    }
    out.close();
    return true;
}