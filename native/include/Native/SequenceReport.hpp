#pragma once
#include <cstdint>
#include <JSystem/JAudio2/JASReport.hpp>
namespace Native {
// Retain printf's integer argument width while passing string pointers intact.
template<unsigned Index=0,class... Args>
void reportSequence(const char* format,const std::uintptr_t* values,const unsigned char* types,Args... args){
 if constexpr(Index==4)JASReport(format,args...);
 else if(types[Index]==2)reportSequence<Index+1>(format,values,types,args...,reinterpret_cast<const char*>(values[Index]));
 else reportSequence<Index+1>(format,values,types,args...,static_cast<unsigned int>(values[Index]));
}
}
