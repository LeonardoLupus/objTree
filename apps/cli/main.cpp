#include <iostream>

#include <objtree/objtree.hpp>
#include <objtree/tree/branch.h>

int main() {
    std::cout << objtree::greeting() << '\n';
    Branch cilynder;
    Circle a(10.f, 8.f);
    Circle b(10.f, 8.f);
    b.center = {0.f, 20.f, 0.f};
    cilynder.base = a;
    cilynder.end = b;
    Mesh origrin = cilynder.create_mesh();
    origrin.save();
    return 0;
}
