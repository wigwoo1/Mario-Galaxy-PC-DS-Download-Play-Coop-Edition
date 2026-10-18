#include "native_nand_path.hpp"
#include <cassert>
#include <cstdio>
int main() {
    auto root=std::filesystem::current_path()/"build"/"nand-path-fixture";
    auto base=std::filesystem::weakly_canonical(root);
    assert(Native::nandFilePath(root,"/save/mario.dat")==base/"save"/"mario.dat");
    assert(Native::nandFilePath(root,"config.dat")==base/"config.dat");
    for (const char* name : {"", "/", "../escape", "/save/../escape", "C:/escape", "save\\escape", "save//file", "save/", "save/./file", "save./file", "save /file"}) {
        bool rejected=false; try { Native::nandFilePath(root,name); } catch(const std::invalid_argument&) {rejected=true;} assert(rejected);
    }
    bool rejected=false;try {Native::nandFilePath(root,nullptr);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
    std::puts("PASS NAND path mapping: absolute/relative Wii names and invalid/escaping host names");
}
