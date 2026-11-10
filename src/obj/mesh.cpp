#include <objtree/obj/mesh.h>

#include <fstream>
#include <cstdint>

std::size_t Mesh::countVertices() {
    return vertices.size();
}

const std::list<Vector3d>& Mesh::get_vertices() const {
    return vertices;
}

const std::list<Face>& Mesh::get_faces() const {
    return faces;
}


void Mesh::add_vertices(const std::list<Vector3d>& v) {
    for (const auto& point : v)
    {
        vertices.push_back(point);
    }
}

void Mesh::add_faces(const std::list<Face>& f, std::size_t startIdx) {
    for (const auto& tr : f)
    {
        faces.push_back(tr + startIdx);
    }
}

void Mesh::add_mesh(const Mesh& m) {
    this->add_vertices(m.get_vertices());
    this->add_faces(m.get_faces(), countVertices());
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
        out << "f " << tr(0) << " " << tr(1) << " " << tr(2) << "\n";
    }
    out.close();
    return true;
}