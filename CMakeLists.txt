cmake_minimum_required(VERSION 3.23)

project(objTree
    VERSION 0.1.0
    DESCRIPTION "A modular C++ project"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# Strict warnings for a target.
function(set_compiler_warnings target)
    target_compile_options(${target} PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
            -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion>
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
    )
endfunction()

# --- Core library ---
add_library(objTree_core STATIC 
    src/objtree.cpp
    src/obj/mesh.cpp
    src/obj/face.cpp
    src/obj/points.cpp
    src/obj/circle.cpp
    src/tree/branch.cpp
)

target_include_directories(objTree_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_definitions(objTree_core PRIVATE PROJECT_NAME="objTree")

set_compiler_warnings(objTree_core)

add_library(objTree::Core ALIAS objTree_core)

# --- Subprojects ---
add_subdirectory(apps/cli)
