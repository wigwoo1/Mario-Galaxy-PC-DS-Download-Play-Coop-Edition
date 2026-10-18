#pragma once
#include <cstddef>
bool nativeRegisterArc(const void* source,size_t size);
void nativeUnregisterArc(const void* source);


void nativeDetachArc(void* handle);
