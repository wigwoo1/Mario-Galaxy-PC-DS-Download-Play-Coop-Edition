#pragma once
#include <cstring>
template<class T> inline T nativeJMapRead(const void* p){T value;std::memcpy(&value,p,sizeof(value));return value;}
