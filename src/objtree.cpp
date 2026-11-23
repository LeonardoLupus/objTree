#include <objtree/objtree.hpp>

#include <objtree/util/json.hpp>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <list>
#include <memory>
#include <random>
#include <vector>

// ---------- helpers ----------

namespace {

std::mt19937       gen;
std::random_device rd;

template <class T>
T limit_normal_distribution(T m, T d, T lo, T hi) {
    std::normal_distribution<T> dist(m, d);
    return std::clamp(dist(gen), lo, hi);
}

float len_by_radius(float r, const objtree::TreeConfig& cfg) {
    return r * cfg.coeffLenByRadius;
}

float radius_at_height(float height, float downRadius, const objtree::TreeConfig& cfg) {
    const float heightMax = downRadius * cfg.coeffLenByRadius;
    return (heightMax - height) * downRadius / heightMax;
}

float separate_at(float /*r*/, const objtree::TreeConfig& cfg) {
    return limit_normal_distribution(
        cfg.coeffSeparationM,
        cfg.coeffSeparationD,
        cfg.coeffSeparationM + cfg.coeffSeparationD,
        cfg.coeffSeparationM - cfg.coeffSeparationD
    );
}

std::list<Circle> get_child_circle(float radius, const objtree::TreeConfig& cfg) {
    std::list<Circle> circles;

    const float radiusMin = 3.f / cfg.pointsPerRadius;

    // 1) Число детей
    const float countF = limit_normal_distribution(
        cfg.countBranchMean, cfg.countBranchStd,
        static_cast<float>(cfg.countBranchMin),
        static_cast<float>(cfg.countBranchMax));

    int count = static_cast<int>(std::round(countF));
    count = std::clamp(count, cfg.countBranchMin, cfg.countBranchMax);

    if (count <= 1) {
        Circle c;
        c.pointsPerRadius = cfg.pointsPerRadius;
        c.radius = radius;   // grow_tree перезапишет через radius_at_height
        circles.emplace_back(c);
        return circles;
    }

    // 2) Толщина родителя: 0 = тонкий, 1 = толстый
    const float rel = std::clamp(
        (radius - radiusMin) / (cfg.radiusMax - radiusMin),
        0.f, 1.f);

    // 3) Разброс весов: толстый → маленький, тонкий → большой
    const float spread = cfg.spreadThick * rel + cfg.spreadThin * (1.f - rel);

    // 4) Сырые веса c_i ∈ [1, 1+spread]
    std::uniform_real_distribution<float> uni(0.f, 1.f);
    std::vector<float> c(static_cast<std::size_t>(count));
    float sumSq = 0.f;
    for (std::size_t i = 0; i < c.size(); ++i) {
        c[i] = 1.f + uni(gen) * spread;
        sumSq += c[i] * c[i];
    }

    // 5) Нормировка: Σ r_i² = r²
    const float norm = std::sqrt(sumSq);
    for (std::size_t i = 0; i < c.size(); ++i) {
        Circle circle;
        circle.pointsPerRadius = cfg.pointsPerRadius;
        circle.radius = radius * (c[i] / norm);
        circles.emplace_back(circle);
    }
    return circles;
}

simplejson::JsonObject to_json(const objtree::TreeConfig& c) {
    using D = double;
    simplejson::JsonObject o;
    o["seed"]               = D(c.seed);
    o["pointsPerRadius"]    = D(c.pointsPerRadius);
    o["startRadius"]        = D(c.startRadius);
    o["coeffLenByRadius"]   = D(c.coeffLenByRadius);
    o["radiusMax"]          = D(c.radiusMax);
    o["coeffSeparationM"]   = D(c.coeffSeparationM);
    o["coeffSeparationD"]   = D(c.coeffSeparationD);
    o["droop"]              = D(c.droop);
    o["countBranchMin"]     = D(c.countBranchMin);
    o["countBranchMax"]     = D(c.countBranchMax);
    o["countBranchMean"]    = D(c.countBranchMean);
    o["countBranchStd"]     = D(c.countBranchStd);
    o["spreadThick"]        = D(c.spreadThick);
    o["spreadThin"]         = D(c.spreadThin);
    o["tangentialSpread"]   = D(c.tangentialSpread);
    o["thetaJitter"]        = D(c.thetaJitter);
    o["dominanceK"]         = D(c.dominanceK);
    o["thetaMin"]           = D(c.thetaMin);
    o["thetaMax"]           = D(c.thetaMax);
    o["trunkHeight"]        = D(c.trunkHeight);
    o["coeffCountSeparate"] = D(c.coeffCountSeparate);
    o["tangentScale"]       = D(c.tangentScale);
    return o;
}

template <class T>
void read_num(const simplejson::JsonObject& o, const char* key, T& out) {
    auto it = o.find(key);
    if (it == o.end()) return;
    if (auto d = std::get_if<double>(&it->second))   out = static_cast<T>(*d);
    else if (auto b = std::get_if<bool>(&it->second)) out = static_cast<T>(*b);
}

void from_json(const simplejson::JsonObject& o, objtree::TreeConfig& c) {
    if (auto it = o.find("seed"); it != o.end()) {
        if (auto d = std::get_if<double>(&it->second))
            c.seed = static_cast<std::uint32_t>(*d);
    }
    read_num(o, "pointsPerRadius",    c.pointsPerRadius);
    read_num(o, "startRadius",        c.startRadius);
    read_num(o, "coeffLenByRadius",   c.coeffLenByRadius);
    read_num(o, "radiusMax",          c.radiusMax);
    read_num(o, "coeffSeparationM",   c.coeffSeparationM);
    read_num(o, "coeffSeparationD",   c.coeffSeparationD);
    read_num(o, "droop",              c.droop);
    read_num(o, "countBranchMin",     c.countBranchMin);
    read_num(o, "countBranchMax",     c.countBranchMax);
    read_num(o, "countBranchMean",    c.countBranchMean);
    read_num(o, "countBranchStd",     c.countBranchStd);
    read_num(o, "spreadThick",        c.spreadThick);
    read_num(o, "spreadThin",         c.spreadThin);
    read_num(o, "tangentialSpread",   c.tangentialSpread);
    read_num(o, "thetaJitter",        c.thetaJitter);
    read_num(o, "dominanceK",         c.dominanceK);
    read_num(o, "thetaMin",           c.thetaMin);
    read_num(o, "thetaMax",           c.thetaMax);
    read_num(o, "trunkHeight",        c.trunkHeight);
    read_num(o, "coeffCountSeparate", c.coeffCountSeparate);
    read_num(o, "tangentScale",       c.tangentScale);
}

} // namespace

// ---------- public ----------

namespace objtree {

void save_config(const TreeConfig& cfg, const std::string& path) {
    std::ofstream out(path);
    if (!out) throw std::runtime_error("Cannot write config: " + path);
    out << simplejson::dump(to_json(cfg));
}

TreeConfig load_or_create_config(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        TreeConfig cfg;             // дефолтные значения
        save_config(cfg, path);     // создать файл
        return cfg;
    }
    std::stringstream ss;
    ss << in.rdbuf();
    const auto obj = simplejson::parse(ss.str());
    TreeConfig cfg;
    from_json(obj, cfg);
    return cfg;
}

const char* greeting() noexcept {
    return "Project " PROJECT_NAME " ready to work!";
}

Mesh grow_tree(TreeConfig cfg) {
    if (cfg.seed == 0) {
        cfg.seed = rd();
    }
    std::cout << "Seed: " << cfg.seed << "\n";
    gen.seed(cfg.seed);

    const float radiusMin      = 3.f / cfg.pointsPerRadius;
    const float radiusOnePoint = 1.f / cfg.pointsPerRadius;
    const float PI             = static_cast<float>(M_PI);

    std::vector<Branch*> stack;

    auto root = std::make_unique<Branch>();
    root->base = Circle(cfg.startRadius, cfg.pointsPerRadius);
    root->end  = std::make_unique<Circle>();

    const float lenR = len_by_radius(cfg.startRadius, cfg) * separate_at(cfg.startRadius, cfg);
    bool lastBranchStart = false;
    float endRadius = radius_at_height(lenR, cfg.startRadius, cfg);
    if (endRadius < radiusMin) {
        endRadius = radiusOnePoint;
        lastBranchStart = true;
    }
    root->end->radius          = endRadius;
    root->end->pointsPerRadius = cfg.pointsPerRadius;
    root->end->center          = root->base.center + root->base.get_normal() * lenR;
    root->end->set_frame(root->base.get_normal(), root->base.get_x_axis());

    if (!lastBranchStart) stack.push_back(root.get());

    std::uniform_real_distribution<float> phaseDist(0.f, 2.f * PI);

    while (!stack.empty()) {
        Branch* node = stack.back();
        stack.pop_back();

        std::list<Circle> circleBranches;
        if (cfg.trunkHeight > 0.f && node->end->center.y < cfg.trunkHeight) {
            Circle c;
            c.pointsPerRadius = cfg.pointsPerRadius;
            c.radius          = node->end->radius;
            circleBranches.push_back(c);
        } else {
            circleBranches = get_child_circle(node->end->radius, cfg);
        }
        const float countF = static_cast<float>(circleBranches.size());
        const float phi0   = phaseDist(gen);

        float index = 0.f;
        for (const Circle& circle : circleBranches) {
            auto next = std::make_unique<Branch>();
            next->base = *(node->end);
            next->end  = std::make_unique<Circle>(circle);

            const float len = len_by_radius(next->base.radius, cfg)
                            * separate_at(next->base.radius, cfg);

            Vector3d branchDir;
            Vector3d newNormal;
            bool lastBranch = false;

            const Vector3d baseN = next->base.get_normal();
            const Vector3d baseX = next->base.get_x_axis();

            if (countF == 1.f) {
                branchDir = baseN;
                next->end->radius = radius_at_height(len, next->base.radius, cfg);
                newNormal = baseN;
            } else {
                matrix::Matrix3d B = matrix::get_basis(baseN);
                Vector3d i { B.m[0], B.m[3], B.m[6] };
                Vector3d j { B.m[1], B.m[4], B.m[7] };
                Vector3d k { B.m[2], B.m[5], B.m[8] };

                const float baseTheta    = PI / (2.f * countF);
                const float refDominance = 1.f / std::sqrt(countF);
                const float dominance    = std::clamp(
                    circle.radius / next->base.radius, 0.f, 1.f);
                const float thetaScale   = 1.f + cfg.dominanceK * (refDominance - dominance);

                const float thetaNoise = limit_normal_distribution(
                    0.f, cfg.thetaJitter,
                    -2.f * cfg.thetaJitter, 2.f * cfg.thetaJitter);

                float localTheta = baseTheta * thetaScale + thetaNoise;
                localTheta = std::clamp(localTheta, cfg.thetaMin, cfg.thetaMax);

                const float s   = std::sin(localTheta);
                const float c   = std::cos(localTheta);
                const float phi = 2.f * PI * index / countF + phi0;

                branchDir = i * (s * std::cos(phi))
                          + j * c
                          + k * (s * std::sin(phi));

                matrix::Matrix3d B2 = matrix::get_basis(branchDir);
                Vector3d bi { B2.m[0], B2.m[3], B2.m[6] };
                Vector3d bk { B2.m[2], B2.m[5], B2.m[8] };
                Vector3d perturb =
                    bi * limit_normal_distribution(0.f, cfg.tangentialSpread,
                                                   -3.f * cfg.tangentialSpread,
                                                    3.f * cfg.tangentialSpread) +
                    bk * limit_normal_distribution(0.f, cfg.tangentialSpread,
                                                   -3.f * cfg.tangentialSpread,
                                                    3.f * cfg.tangentialSpread);

                newNormal = (branchDir + perturb).norm();
            }

            if (cfg.droop > 0.f) {
                const bool stillTrunk =
                    (cfg.trunkHeight > 0.f && node->end->center.y < cfg.trunkHeight);
                if (!stillTrunk) {
                    const float thinness = std::clamp(
                        1.f - next->end->radius / cfg.startRadius, 0.f, 1.f);
                    branchDir.y -= cfg.droop * thinness;
                    branchDir = branchDir.norm();
                }
            }
            next->end->center = next->base.center + branchDir * len;

            Vector3d newX = parallel_transport(baseX, baseN, newNormal);
            next->end->set_frame(newNormal, newX);

            if (next->end->radius < radiusMin) {
                next->end->radius = radiusOnePoint;
                lastBranch = true;
            }

            ++index;
            node->child.push_back(std::move(next));
            if (!lastBranch) stack.push_back(node->child.back().get());
        }
        node->separate_branch(cfg.coeffCountSeparate, cfg.tangentScale);
    }
    return root->create_mesh();
}

} // namespace objtree