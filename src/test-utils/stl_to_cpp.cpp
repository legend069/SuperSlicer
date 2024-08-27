#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>

#include <libslic3r/Format/STL.hpp>
#include <libslic3r/Model.hpp>
#include <libslic3r/LocalesUtils.hpp>


#include "ClipboardXX/include/clipboardxx.hpp"

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        std::cout<<"usage: stl_to_cpp \"path/to/stl.stl\"\n";
        return 0;
    }
    std::string path_str = argv[1];
    if(path_str.front() == '\"' && path_str.back() == '\"')
        path_str = path_str.substr(1,path_str.size()-2);
    Slic3r::Model model;
    bool result = load_stl(path_str.c_str(), &model, "obj");
    if (!result) {
        std::cout << "error, can't read '" << path_str << "'\n";
        return 0;
    }
    clipboardxx::clipboard clipboard;
    //TriangleMesh tm2 = TriangleMesh(std::vector<Vec3f>{{-5, -5, -0.1}},std::vector<Vec3i32>{{1,4,3}});
    std::stringstream out_cpp;
    int idx_obj = 0;

    return 0;
}
