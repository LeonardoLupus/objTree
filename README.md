# objTree

A modular C++20 project.

## Requirements

- A C++20 compiler (GCC 11+, Clang 14+, MSVC 19.30+)
- CMake 3.23+
- Ninja (used by `CMakePresets.json`)

## Build & Run

```bash
cmake --preset release
cmake --build --preset release
./build/release/bin/objTree_cli
