#pragma once
#include <vector>
#include <stdexcept>
class NameObj;
// Native holder storage adapter; does not emulate Wii memory layout.
class NameObjHolder {
public:
    std::vector<NameObj*> objects;
    void add(NameObj* object){if(!object)throw std::runtime_error("Null native NameObj registration");objects.push_back(object);}
};
