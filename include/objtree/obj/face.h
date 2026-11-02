#pragma once

#include <array>

struct Face {
    std::array<std::size_t, 3> v;

    std::size_t operator()(std::size_t i) const {
        return v[i];
    }

    std::size_t& operator()(std::size_t i) {
        return v[i];
    }

    Face operator+(std::size_t n) const {
        return {v[0]+n , v[1]+n, v[2]+n};
    }
};

