#include <objtree/objtree.hpp>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    const std::string path = (argc > 1) ? argv[1] : "tree.json";
    std::cout << "Config: " << path << "\n";

    auto cfg  = objtree::load_or_create_config(path);
    auto mesh = objtree::grow_tree(cfg);
    mesh.save();
    return 0;
}