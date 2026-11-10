#include <objtree/tree/branch.h>

std::list<Face> Branch::get_branch_faces(std::size_t downStart, std::size_t upStart) const {
    std::size_t downCount = base.get_count_of_points();
    std::size_t upCount = end->get_count_of_points();
    std::list<Face> faces;
    if ((upCount < 3 && upCount != 1) || downCount < upCount) {
        return faces;
    }
    float coeff = static_cast<float>(upCount) / static_cast<float>(downCount);
    std::size_t idxUpVertice = 0;
    for (std::size_t i = 0; i < downCount-1; i++)
    {
        std::size_t idxNewUpVertice = static_cast<std::size_t>(static_cast<float>(i+1)*coeff)%upCount;
        faces.push_back({downStart + i+1, downStart + i, upStart + idxNewUpVertice});
        if (idxUpVertice != idxNewUpVertice) {
            faces.push_back({upStart + idxUpVertice, upStart + idxNewUpVertice, downStart + i});
            idxUpVertice = idxNewUpVertice;
        }
    }
    std::size_t idxNewUpVertice = static_cast<std::size_t>(static_cast<float>(downCount)*coeff)%upCount;
    faces.push_back({downStart, downStart + downCount-1, upStart + idxNewUpVertice});
    if (idxUpVertice != idxNewUpVertice) {
        faces.push_back({upStart + idxUpVertice, upStart + idxNewUpVertice, downStart + downCount-1});
    }

    return faces;
}

Mesh Branch::create_mesh() {
    Mesh origin;

    origin.add_vertices(base.get_points());
    origin.add_vertices(end->get_points());
    origin.add_faces(get_branch_faces(1, base.get_count_of_points() +1));

    return origin;
}