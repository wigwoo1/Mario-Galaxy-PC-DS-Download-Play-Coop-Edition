#pragma once
#include <stddef.h>
bool nativeBindHeapArena(void* memory,size_t bytes);
bool nativeReleaseHeapArena();
// Caller-owned secondary storage must outlive every heap constructed within it.
bool nativeBindSecondaryHeapArena(void* memory,size_t bytes);
bool nativeReleaseSecondaryHeapArena();
