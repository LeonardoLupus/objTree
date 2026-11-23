#include <objtree/tree/branch.h>
#include <iostream>

Branch::Branch(Branch&& b) noexcept {
    this->base = b.base;
    this->end = std::move(b.end);
    this->child = std::move(b.child);
}
Branch& Branch::operator=(Branch&& b) noexcept {
    this->base = b.base;
    this->end = std::move(b.end);
    this->child = std::move(b.child);
    return *this;
}

void Branch::separate_branch(int count, float tangentScale) {
    if (count <= 0 || !end) return;

    const Vector3d P0 = base.center;
    const Vector3d P1 = end->center;

    // Касательные Эрмита: chord * tangentScale
    const float chord  = (P1 - P0).len();
    const float tScale = chord * tangentScale;
    const Vector3d T0  = base.get_normal() * tScale;
    const Vector3d T1  = end->get_normal() * tScale;

    const float normalLen = base.get_normal().len();
    const float r0 = base.radius;
    const float r1 = end->radius;

    // Кэш фрейма предыдущей окружности — отсюда ведём параллельный перенос
    Vector3d prevN = base.get_normal();
    Vector3d prevX = base.get_x_axis();

    Branch* last = this;

    for (int i = 1; i <= count; ++i) {
        const float t  = static_cast<float>(i) / static_cast<float>(count + 1);
        const float t2 = t * t;
        const float t3 = t2 * t;

        // --- P(t) — кубический Hermite ---
        const float h00 =  2.0f*t3 - 3.0f*t2 + 1.0f;
        const float h10 =        t3 - 2.0f*t2 + t;
        const float h01 = -2.0f*t3 + 3.0f*t2;
        const float h11 =        t3 -      t2;
        const Vector3d P = P0*h00 + T0*h10 + P1*h01 + T1*h11;

        // --- P'(t) — направление новой нормали ---
        const float dh00 =  6.0f*t2 - 6.0f*t;
        const float dh10 =  3.0f*t2 - 4.0f*t + 1.0f;
        const float dh01 = -6.0f*t2 + 6.0f*t;
        const float dh11 =  3.0f*t2 - 2.0f*t;
        const Vector3d tangent = P0*dh00 + T0*dh10 + P1*dh01 + T1*dh11;

        const float tlen = tangent.len();
        Vector3d newN = (tlen > 1e-6f && normalLen > 1e-6f)
            ? (tangent * (normalLen / tlen)).norm()
            : prevN;

        // --- радиус с smoothstep ---
        const float s = t * t * (3.0f - 2.0f * t);
        const float r = r0 + (r1 - r0) * s;

        // --- фрейм: x-ось параллельно переносим от предыдущей окружности ---
        const Vector3d newX = parallel_transport(prevX, prevN, newN);

        Circle boundary = base;
        boundary.center = P;
        boundary.radius = r;
        boundary.set_frame(newN, newX);

        auto next = std::make_unique<Branch>();
        next->base  = std::move(boundary);
        next->end   = std::move(last->end);
        next->child = std::move(last->child);

        last->end = std::make_unique<Circle>(next->base);

        last->child.clear();
        last->child.emplace_back(std::move(next));
        last = last->child.back().get();

        prevN = newN;
        prevX = newX;
    }
}

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
    struct BranchAndIndex
    {
        Branch* ptrBranch = nullptr;
        std::size_t baseIndex = 0;
    };

    Mesh obj;
    std::list<BranchAndIndex> stack;

    obj.add_vertices(base.get_points());

    std::size_t indexVertices = 1;   // 1-based индекс начала base рута
    auto ptrBranch = this;
    while (ptrBranch)
    {
        // Конец текущей ветки ляжет ровно в позицию (countVertices()+1)
        // (1-based) — сразу после уже добавленных вершин.
        std::size_t endVertexStart = obj.countVertices() + 1;

        obj.add_faces(ptrBranch->get_branch_faces(indexVertices, endVertexStart));
        obj.add_vertices(ptrBranch->end->get_points());

        // Дети «сидят» base-ом на end-вершинах родителя,
        // значит baseIndex = endVertexStart.
        for (auto& branchUniq : ptrBranch->child)
        {
            stack.emplace_back(BranchAndIndex{branchUniq.get(), endVertexStart});
        }

        if (stack.empty()) {
            ptrBranch = nullptr;
            break;
        }
        ptrBranch = stack.front().ptrBranch;
        indexVertices = stack.front().baseIndex;
        stack.pop_front();
    }

    return obj;
}