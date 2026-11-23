#pragma once
#define _USE_MATH_DEFINES

#include <objtree/tree/branch.h>
#include <objtree/math/matrix3d.h>
#include <objtree/obj/mesh.h>

#include <cstdint>
#include <string>

namespace objtree {

// Все параметры генерации. Передаются в grow_tree().
// Значения по умолчанию — текущее поведение.
struct TreeConfig {
    // --- RNG ---
    // 0 → использовать std::random_device (каждый запуск — новое дерево)
    // !=0 → детерминированно с этим сидом
    std::uint32_t seed = 0;

    // --- Разрешение / геометрия ---
    float pointsPerRadius       = 60.f;   // точек на единицу радиуса
    float startRadius           = 2.f;    // радиус корня
    float coeffLenByRadius      = 30.f;   // длина ветки = radius * coeff
    float radiusMax             = 2.5f;   // радиус "толстой" ветки для весов

    // --- Длина ветки (нормальное распределение) ---
    float coeffSeparationM      = 0.5f;   // среднее
    float coeffSeparationD      = 0.3f;   // СКО

    // Насколько сильно тонкие ветви клонятся вниз. 0 — без эффекта.
    float droop = 0.f;

    // --- Число детей (нормальное распределение) ---
    int   countBranchMin        = 1;
    int   countBranchMax        = 4;
    float countBranchMean       = 2.0f;
    float countBranchStd        = 0.8f;

    // --- Асимметрия радиусов детей ---
    // 0 = все равны; больше — разброс сильнее.
    // Толстый родитель → почти равные; тонкий → один доминирует.
    float spreadThick           = 0.20f;
    float spreadThin            = 2.00f;

    // --- "Раскидистость" 3D-ветвления ---
    // Три параметра вместе задают общую "ширину" дерева:
    float tangentialSpread      = 0.08f;   // тангенциальный perturb нормали (rad)
    float thetaJitter           = 0.2618f; // π/12 — шум угла конуса (rad)
    float dominanceK            = 1.2f;    // насколько тонкая ветка уходит вбок

    // Границы угла конуса
    float thetaMin              = 0.0628f; // 0.02π
    float thetaMax              = 1.4137f; // 0.45π

    // Высота «голого» ствола вдоль мировой оси Y (ствол растёт от нуля).
    // Ниже — без ветвления, выше — обычное. 0 — ветвление с самого низа.
    float trunkHeight = 0.f;

    // --- Разбиение ветки (Hermite) ---
    int   coeffCountSeparate    = 6;
    float tangentScale          = 1.2f;
};

TreeConfig load_or_create_config(const std::string& path);

void save_config(const TreeConfig& cfg, const std::string& path);

// Сгенерировать дерево. cfg.seed == 0 → случайный сид.
Mesh grow_tree(TreeConfig cfg = {});

[[nodiscard]] const char* greeting() noexcept;

} // namespace objtree