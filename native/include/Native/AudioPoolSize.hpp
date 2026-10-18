#pragma once
#include <revolution/types.h>
#include <cstddef>
#include <limits>
#include <new>
inline u32 nativeAudioPoolSize(std::size_t size){if(size>std::numeric_limits<u32>::max())throw std::bad_alloc();return static_cast<u32>(size);}
