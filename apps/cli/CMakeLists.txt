add_executable(objTree_cli main.cpp)
target_link_libraries(objTree_cli PRIVATE objTree::Core)
set_compiler_warnings(objTree_cli)
