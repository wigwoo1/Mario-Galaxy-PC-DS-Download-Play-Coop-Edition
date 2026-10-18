#pragma once
#include <cstddef>
namespace Native {
template<class T,class Predicate>
T* findLastArea(T* const* values,std::size_t count,Predicate contains){
    while(count){auto* candidate=values[--count];if(contains(candidate))return candidate;}
    return nullptr;
}
}
